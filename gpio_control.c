/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/* 
 * File:   gpio_control.c
 * Author: Matthew
 *
 * Created on 22 August 2018, 15:54
 */
/* Setup the device ready for GPIO control
 * This doesn't specifically set any pins for input or output
 */

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>					// required for mmap
#include <fcntl.h>						// required by open
#include <errno.h>						// So I can get error numbers out
//#include <unistd.h>
#include "gpio_control.h"				// Is this correct???


/* Setup the GPIO system ready for use
 * extracts the map and check it is ready to use
 * Doesn't include specific pin assignments*/
int gpio_init() {
  
  int gpio_mem;
  //void *gpio_map;
  
  // Open the map
  gpio_mem = open("/dev/gpiomem", O_RDWR | O_SYNC);
  if (gpio_mem < 0) {
      printf("Failed to open GPIO Memory, try checking permissions.\n");
      exit(-1);
    };
  
  // mmap the gpio, will return a pointer to the mapped error, returns MAP_FAILED on error
  gpio_mmap = mmap(
		  NULL,									// Address offset (not required))
		  BLOCK_SIZE,							// Map Length
		  PROT_READ|PROT_WRITE,					// Enable Read and Write
		  MAP_SHARED,							// The map can be used by other processes
		  gpio_mem,								// The file opened for the map
		  0										// Offset - none required
		  );

  // close the map now the data is extracted
  close(gpio_mem);
  
  if (gpio_mmap == MAP_FAILED) {
	  // failed to load the map, display the error information
	  printf("Failed to load the mmap with error:%p - reason:%d\n", gpio_mmap, errno);
	  exit(-1);
  };
  
  return 0;
}

/* Function to set the GPIO pin to be an input.
 * - Requires an int for the GPIO pin to be set
 * - Returns 0 or -1 if error
 * The first 5 blocks of the map are the function select pins, each block being 32bits (4 bytes)
 * long and consisting of 3 bits per gpio pin, addressed as a single 32bit.
 * Address for gpio0 is 0 in the gpio_mmap, address for gpio9 is also 0, but different bits
 * address for gpio15 is 1 as it is the next block
 */
int set_gpio_for_input(int pin_no){
	
	int block_addr =0;						// The address for the block
	long setting;							// The value that will be AND'd into the block
	long mode = 0x0007;					    // this is the inverse of what required
	
	// Check if the requested pin is within the available range
	if ((pin_no < LOWEST_GPIO_PIN) | (pin_no > HIGHEST_GPIO_PIN)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};
	
	// set the block number to a tenth of the pin number, ignoring remainder as block_addr is an int
	block_addr = (pin_no / 10);
	
	// Create the mask to write
	// 32 - - - - - - - - - - - - - -0
	// xxxxxxxxxx000xxxxxxxxxxxxxxxxxx  value to set in map
	// 0000000000000000000000000000111	starting mode value
	// 0000000000111000000000000000000	shifted into position
	// 1111111111000111111111111111111  Not'd (1's Compliment))
	// xxxxxxxxxx000xxxxxxxxxxxxxxxxxx	Bitwise AND
	
	// start with 00000111, shift it to the right place and then NOT it.
	// This will give all 1's except the bit we want setting to zero
	// bitwise AND this with the register to set the value
	// modulo % leaves remainder
	
	// move 'mode' bits the the correct position based on pin number
	// calculate the remainder as this is the part within the block
	// the shift it 3 times that distance as 3 bits per pin
	// Then 1's compliment '~' to convert 1 -> 0 and 0 -> 1
	setting = ~(mode << ((pin_no%10)*3));					// What if bigger than 4 bytes, will the extra be ignored?
	
	// Set the relevant map address to itself bitwise AND'd (&) with setting
	*(gpio_mmap + block_addr) &= setting;
			
	return 0;

}

/* Function to set the GPIO pin to be an output
 * - Requires and int for the GPIO pin number
 * - returns 0
 * Very similar to the set_gpio_for_input above except it is writing a different value
 * For an explanation of how it works, please refer to above.
 * value to be written for output is 001, not 000 for input
 */
int set_gpio_for_output(int pin_no) {
	
	int block_addr =0;						// The address for the block
	long setting;							// The value that will be AND'd into the block
	long mode = 0x0001;					    // this is the values that are what required
	
	// Check if the requested pin is within the available range
	if ((pin_no < LOWEST_GPIO_PIN) | (pin_no > HIGHEST_GPIO_PIN)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};
	
	// set the block number to a tenth of the pin number, ignoring remainder as block_addr is an int
	block_addr = (pin_no / 10);
	
	//Need to clear the register for the pin first
	set_gpio_for_input(pin_no);
	
	// Create the mask to write
	// 32 - - - - - - - - - - - - - -0
	// xxxxxxxxxx001xxxxxxxxxxxxxxxxxx  value to set in map
	// 0000000000000000000000000000001	starting mode value
	// 0000000000001000000000000000000	shifted into position
	// xxxxxxxxxx001xxxxxxxxxxxxxxxxxx	Bitwise OR
	
	// move 'mode' bits the the correct position based on pin number
	// calculate the remainder as this is the part within the block
	// the shift it 3 times that distance as 3 bits per pin
	// Then 1's compliment '~' to convert 1 -> 0 and 0 -> 1
	setting = (mode << ((pin_no%10)*3));
	
	// Set the relevant map address to itself bitwise OR'd (|) with setting
	*(gpio_mmap + block_addr) |= setting;
			
	return 0;
}

/* For setting the output value of a GPIO pin.
 * - Requires an int for pin number and int for the value to be written (1 or 0)
 * - returns 0 or -1 if there is an error
 * To set the GPIO pin one of 2 different operations are required as the 
 * registers are organised as Set and Clear registers
 * For example, to turn the GPIO pin on, write 1 to the set register, but to turn
 * it off write 1 to the clear register
 * There are 2 registers per Set & Clear and are organised as GPIO pins 0 - 31 and 32 - 53
 */
int set_gpio_value (int pin_no, int value) {
	
	int block_addr;							// The address for the block
	long setting;							// The value to be AND'd into the block
	
	
	// Check the input parameters are within the available range
	if ((pin_no < LOWEST_GPIO_PIN) | (pin_no > HIGHEST_GPIO_PIN)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};
	if ((value < 0) | (value > 1)) {
		printf("Requested value (%i) is outside allowed range of 0 - 1\n", value);
		return -1;
	};
	
	// Set the Register address based on the value requested
	// Set registers are 7 & 8, Clear registers are 10 & 11
	if (value == 1) {
		// Require the Set register
		block_addr = GPIO_SET_OFFSET + (pin_no/32);		// (pin/32) will only return 1 for register 32 and above
	}
	else {
		// default to the Clear register
		block_addr = GPIO_CLEAR_OFFSET + (pin_no/32);			// (pin/32) will only return 1 for reg 32 and above
	}
	
	// Create the mask to write, the value written is irrespective of the register
	// Value to be written is a 1, shifted to the correct position based on the remainder of
	// pin_no modulo by 32, e.g. 17 % 32 = 17, but 41 % 32 = 9
	// Writing a zero has no effect
	setting = (0x0001 << (pin_no%32));
	
	*(gpio_mmap + block_addr) = setting;
	
	return 0;
	
}

/* For reading of the GPIO value
 * - Requires an int pin number in range 0 - 53
 * - Returns an int with the value either 1 or 0 or a negative number if an error
 * To read the GPIO pin, select the register (13 or 14) and return the required bit
 */
int read_gpio_value(int pin_no) {
	
	int block_addr;								// The block address to use
	long mask = 0x0001;							// The starting value of the mask
	long reading;								// The returned value form the register
	int value;									// The converted int to return
	
	// Check the input parameters are within the available range
	if ((pin_no < LOWEST_GPIO_PIN) | (pin_no > HIGHEST_GPIO_PIN)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};
	
	// Work out the block number;
	block_addr = GPIO_LEVEL_OFFSET + (pin_no/32);			// Sets the block address to either 13 for 
															// the lower 32 pins or 14 for the remainder
	
	// Create the mask to write
	// 32 - - - - - - - - - - - - - -0
	// xxxxxxxxxxxx?xxxxxxxxxxxxxxxxxx  required value in the map
	// 0000000000000000000000000000001	starting mode value
	// 0000000000001000000000000000000	shifted into position
	// xxxxxxxxxxxx?xxxxxxxxxxxxxxxxxx	output value
	
	mask = (mask << (pin_no%32));			// Using module, move the set bit to the remainder from 32
	
	// Read the value by AND'ing with the mask to get the required bit
	reading = *(gpio_mmap + block_addr) & mask;
	
	// Shift the bit back, converting to an int at the same time by throwing away the upper part.
	value = (int)(reading >> (pin_no%32));
	
	return value;
}

/* For reading of the GPIO Event register
 * - Requires an int pin number in range 0 - 53
 * - Returns an int with the event value either 1 (event triggered), 0 (no event triggered)
 *    or a negative number if an error.
 * To read the GPIO pin event register, select the register (16 or 17) and return the required bit
 * If the event is detected, clear it by setting the bit to 1.
 */
int read_gpio_event_detection(int pin_no) {
	
	int block_addr;								// The block address to use
	long mask = 0x0001;							// The starting value of the mask
	long reading;								// The returned value form the register
	int event;									// The converted int to return
	
	// Check the input parameters are within the available range
	if ((pin_no < LOWEST_GPIO_PIN) | (pin_no > HIGHEST_GPIO_PIN)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};
	
	// Work out the block number;
	block_addr = GPIO_EVENT_DETECT_OFFSET + (pin_no/32);			// Sets the block address to either 16 for 
															// the lower 32 pins or 17 for the remainder
	
	// Create the mask to write
	// 32 - - - - - - - - - - - - - -0
	// xxxxxxxxxxxx?xxxxxxxxxxxxxxxxxx  required value in the map
	// 0000000000000000000000000000001	starting mode value
	// 0000000000001000000000000000000	shifted into position
	// xxxxxxxxxxxx?xxxxxxxxxxxxxxxxxx	output value
	
	mask = (mask << (pin_no%32));			// Using module, move the set bit to the remainder from 32
	
	// Read the value by AND'ing with the mask to get the required bit
	reading = *(gpio_mmap + block_addr) & mask;
	
	// Shift the bit back, converting to an int at the same time by throwing away the upper part.
	event = (int)(reading >> (pin_no%32));
	
	// If the event is set, clear it by writing the same bit with 1
	if (event == 1) {
		*(gpio_mmap + block_addr) = mask;
	}
	
	return event;
}

/* Set the detection type for event triggering
 *  - Requires the pin number as an int, the type of triggering (see .h file)
 *    and whether detection is to be turned on or off (default)
 *		- GPIO_RISING - Rising Edge Detection mode
 *		- GPIO_FALLING - Falling Edge Detection mode
 *		- GPIO_HIGH_DETECT - High Level Detection mode (default)
 *		- GPIO_LOW_DETECT - Low Level Detection mode
 *		- ASYNC_RISING - Asynchronous Rising Edge Detection
 *		- ASYNC_FALLING - Asynchronous Falling Edge Detection
 *  - Returns 0 or -1 if there is an error
 * Using case statements to protect against bad values being passed in
 */
int configure_gpio_event_detection_type (int pin_no, int mode, int detect_onoff ) {
	
	int block_addr;							// The address for the block
	long setting;							// The value to be AND'd into the block
	
	// Check the input parameters are within the available range
	if ((pin_no < LOWEST_GPIO_PIN) | (pin_no > HIGHEST_GPIO_PIN)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};

	// Set the Register address based on the value requested
	// default state us high detect if mode given is not valid value
	switch (mode) {
		case GPIO_RISING:
			block_addr = GPIO_RISING_EDGE_OFFSET + (pin_no/32);
			break;
		case GPIO_FALLING:
			block_addr = GPIO_FALLING_EDGE_OFFSET + (pin_no/32);
			break;
		case GPIO_HIGH_DETECT:
		default:
			block_addr = GPIO_HIGH_DETECT_OFFSET + (pin_no/32);
			break;
		case GPIO_LOW_DETECT:
			block_addr = GPIO_LOW_DETECT_OFFSET + (pin_no/32);
			break;
		case ASYNC_RISING:
			block_addr = GPIO_ASYNC_RISING_EDGE_OFFSET + (pin_no/32);
			break;
		case ASYNC_FALLING:
			block_addr = GPIO_ASYNC_FALLING_EDGE_OFFSET + (pin_no/32);
			break;
		
	}
	
	// Create the value to write, the value written is irrespective of the register
	// Value to be written is a 1, shifted to the correct position based on the remainder of
	// pin_no modulo by 32, e.g. 17 % 32 = 17, but 41 % 32 = 9
	// 32 - - - - - - - - - - - - - -0
	// xxxxxxxxxxxx?xxxxxxxxxxxxxxxxxx  required value in the map
	// 0000000000000000000000000000001	starting mode value
	// 0000000000001000000000000000000	shifted into position
	setting = (0x0001 << (pin_no%32));


	// Based on whether it is to be turned on or off, 
	switch (detect_onoff) {
		case DETECT_ON:
			// Bitwise OR the value into the mmap so that any settings already present are retained
			// The required bit will be set to a 1, regardless of the previous state
			printf("Detect ON setting:%d\n", setting);
			*(gpio_mmap + block_addr) |= setting;
			break;
		case DETECT_OFF:
		default:	
			// Need to bitwise AND the inverse of the setting into the register
			// Note the ~ to get a bitwise 1's Compliment (inverse) of the setting
			printf("Detect OFF setting:%d\n", (~setting));
			*(gpio_mmap + block_addr) &= (~setting);
			break;
	}
	
	return 0;
}

/* Routine sets all the GPIO event registers to zero's clearing them all
 * Doesn't require anything and returns 0 on completion
 */
int clear_gpio_event_detection() {
	int block_addr;							// The address for the block
	long setting = 0x0000;					// The value to be AND'd into the block for clearing
	int f;									// for loop variable
	int offsets[6] = {						// Holds all the offsets required
		GPIO_RISING_EDGE_OFFSET,
		GPIO_FALLING_EDGE_OFFSET,
		GPIO_HIGH_DETECT_OFFSET,
		GPIO_LOW_DETECT_OFFSET,
		GPIO_ASYNC_RISING_EDGE_OFFSET,
		GPIO_ASYNC_FALLING_EDGE_OFFSET
	};

	// Set the Register address to all zeros to clear any previous settings
	for (f = 0; f < 6; f++) {
		block_addr = offsets[f];
		*(gpio_mmap + block_addr) = setting;
		// repeat for the upper register
		block_addr = offsets[f] + 1;
		*(gpio_mmap + block_addr) = setting;
	}
	
	return 0;
}