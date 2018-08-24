/* The code here is experimental, and is not intended to be used
 * in a production environment. It is an investigation of the GPIO
 * functionality in C using gpiomem
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation as version 2 of the License.
 *
 */

/* 
 * File:   main.c
 * Author: Matthew Bennett
 *
 * Created on 22 August 2018, 15:45
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gpio_control.h"				//Is this correct???

/*
 * 
 */
int main(int argc, char** argv) {

	int reading_pin = 17;
	int flashing_pin = 27;
	
	printf("Starting the GPIO Investigator\n");
	
    // setup the GPIO requirements
    gpio_init();
	
	// Setup an input and output pin
	set_gpio_for_input(reading_pin);
	set_gpio_for_output(flashing_pin);
	
	while (1) {
		printf("Reading from GPIO%d:%d\n", reading_pin, read_gpio_value(reading_pin));
		printf("Turning on the led\n");
		set_gpio_value(flashing_pin, 1);
		usleep(500000);
		printf("turning off the led\n");
		set_gpio_value(flashing_pin, 0);
		usleep(500000);		
	}
	
    return (EXIT_SUCCESS);
}

