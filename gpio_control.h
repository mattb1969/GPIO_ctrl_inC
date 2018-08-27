/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gpio_control.h
 * Author: Matthew
 *
 * Created on 22 August 2018, 15:54
 */

#ifndef GPIO_CONTROL_HEADER
#define GPIO_CONTROL_HEADER

#define BLOCK_SIZE		(4*1024)

/* The range of GPIO pins available to use*/
#define LOWEST_GPIO_PIN					0			// Lowest acceptable GPIO pin
#define HIGHEST_GPIO_PIN				53			// Highest acceptable GPIO pin

/* Offset to the registers. The first register is zero, the rest are listed below
 * The offset is to the first register in a type, e.g. The PIn register for GPIO's 0 - 31
 * There is no offset for the first register as this starts at zero */
#define GPIO_SET_OFFSET					7			// GPSET0 Pin Output Set registers
#define GPIO_CLEAR_OFFSET				10			// GPCLR0 Pin Output Clear registers
#define GPIO_LEVEL_OFFSET				13			// GPLEV0 Pin Level register
#define GPIO_EVENT_DETECT_OFFSET		16			// GPEDS0 Pin Event Detect Status register
#define GPIO_RISING_EDGE_OFFSET			19			// GPEDS0 Rising Edge Detection Enable register
#define GPIO_FALLING_EDGE_OFFSET		22			// GPFEN0 Falling Edge Detection Enable register
#define GPIO_HIGH_DETECT_OFFSET			25			// GPHEN0 High Detection Enable register
#define GPIO_LOW_DETECT_OFFSET			28			// GPLEN0 Low Detection Enable register
#define GPIO_ASYNC_RISING_EDGE_OFFSET	31			// GPAREN0 Async Rising Edge Detection Enable register
#define GPIO_ASYNC_FALLING_EDGE_OFFSET	34			// GPAFEN0 Async Falling Edge Detection Enable register

/* The various event detection selection options are listed below*/
#define GPIO_RISING						100			// Rising Edge Detection mode
#define GPIO_FALLING					101			// Falling Edge Detection mode
#define GPIO_HIGH_DETECT				102			// High Level Detection mode
#define GPIO_LOW_DETECT					103			// Low Level Detection mode
#define ASYNC_RISING					104			// Asynchronus Rising Edge Detection
#define ASYNC_FALLING					105			// Asynchronus Falling Edge Detection

#define DETECT_ON						1			// Set the detect mode on
#define DETECT_OFF						0			// Set the detect mode off

#include <stdint.h>

// functions
int gpio_init();							// Initialise the gpio system
int set_gpio_for_input(int pin_no);			// Set a GPIO pin for input
int set_gpio_for_output(int pin_no);		// Set a GPIO pin for output
int set_gpio_value (int pin_no, int value);	// Set the value of a GPIO pin
int read_gpio_value(int pin_no);			// Read a GPIO pin value

int read_gpio_event_detection(int pin_no);	// Read the event detection flag (and clear it if set))
int configure_gpio_event_detection_type (int pin_no, int mode, int detect_onoff );
											// Configures the event detection for 1 of 6 modes to be 
											// either on on or off.
int clear_gpio_event_detection();			// Clear all the event detection registers

// Global Variables
static volatile uint32_t *gpio_mmap;                  // the mmap pointer for the gpio map


#endif


