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

#include <stdint.h>

// functions
int gpio_init();							// Initialise the gpio system
int set_gpio_for_input(int pin_no);			// Set a GPIO pin for input
int set_gpio_for_output(int pin_no);		// Set a GPIO pin for output
int set_gpio_value (int pin_no, int value);	// Set the value of a GPIO pin
int read_gpio_value(int pin_no);			// Read a GPIO pin value


// Global Variables
//unsigned *gpio_mmap;                  // the mmap pointer for the gpio map
static volatile uint32_t *gpio_mmap;                  // the mmap pointer for the gpio map


#endif


