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
	if ((pin_no < 0) | (pin_no > 53)) {
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
	long mode = 0x0006;					    // this is the inverse of what required
	
	// Check if the requested pin is within the available range
	if ((pin_no < 0) | (pin_no > 53)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};
	
	// set the block number to a tenth of the pin number, ignoring remainder as block_addr is an int
	block_addr = (pin_no / 10);
	
	// Create the mask to write
	// 32 - - - - - - - - - - - - - -0
	// xxxxxxxxxx001xxxxxxxxxxxxxxxxxx  value to set in map
	// 0000000000000000000000000000110	starting mode value
	// 0000000000110000000000000000000	shifted into position
	// 1111111111001111111111111111111  Not'd (1's Compliment))
	// xxxxxxxxxx001xxxxxxxxxxxxxxxxxx	Bitwise AND
	
	// move 'mode' bits the the correct position based on pin number
	// calculate the remainder as this is the part within the block
	// the shift it 3 times that distance as 3 bits per pin
	// Then 1's compliment '~' to convert 1 -> 0 and 0 -> 1
	setting = ~(mode << ((pin_no%10)*3));					// What if bigger than 4 bytes, will the extra be ignored?
	
	// Set the relevant map address to itself bitwise AND'd (&) with setting
	*(gpio_mmap + block_addr) &= setting;
			
	return 0;
}

/* For setting the output value of a GPIO pin.
 * - Requires an int for pin number and int for the value to be written (1 or 0)
 * - returns o or -1 if there is an error
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
	if ((pin_no < 0) | (pin_no > 53)) {
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
		block_addr = 7 + (pin_no/32);		// (pin/32) will only return 1 for register 32 and above
	}
	else {
		// default to the Clear register
		block_addr = 10 + (pin_no/32);			// (pin/32) will only return 1 for register 32 and above
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
	if ((pin_no < 0) | (pin_no > 53)) {
		printf("Requested GPIO Pin (%d) is outside allowed range of 0 - 53\n", pin_no);
		return -1;
	};
	
	// Work out the block number;
	block_addr = 13 + (pin_no/32);			// Sets the block address to either 13 for the lower 32 pins 
											// Or 14 for the remainder
	
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