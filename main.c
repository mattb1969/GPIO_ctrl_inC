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
#include <time.h>

#include "gpio_control.h"				//Is this correct???

#define TEST_DURATION		5

/*
 * 
 */
int main(int argc, char** argv) {

	int reading_pin = 17;
	int flashing_pin = 22;
	int detection = 0;
	int endloop;
	time_t starttime;
	
	printf("Starting the GPIO Investigator\n");
	
    // setup the GPIO requirements
    gpio_init();
	
	// Setup an input and output pin
	set_gpio_for_input(reading_pin);
	set_gpio_for_output(flashing_pin);
	//clear_gpio_event_detection();

	printf("Test the GPIO Input on pin:%d", reading_pin);
	starttime = time(NULL);
	while (difftime(time(NULL), starttime) < TEST_DURATION) {
		printf("Reading from GPIO%d:%d\n", reading_pin, read_gpio_value(reading_pin));
		usleep(50);
	}
	
	// Test the LED Output
	//clear_gpio_event_detection();
	starttime = time(NULL);
	while (difftime(time(NULL), starttime) < TEST_DURATION) {
		printf("Turning on the led\n");
		set_gpio_value(flashing_pin, 1);
		usleep(500000);
		printf("Turning off the led\n");
		set_gpio_value(flashing_pin, 0);
		usleep(500000);
	}

#if(0)
	// Test the rising edge detection
	clear_gpio_event_detection();
	detection = 0;
	configure_gpio_event_detection_type(reading_pin, GPIO_RISING, DETECT_ON);
	printf("Trigger rising edge detection\n");
	starttime = time(NULL);
	while ((detection == 0) | (difftime(time(NULL), starttime) < TEST_DURATION)) {
		detection = read_gpio_event_detection(reading_pin);
		usleep(50);
	}
	printf("Rising Edge Detection registered:%d\n", detection);
	configure_gpio_event_detection_type(reading_pin, GPIO_RISING, DETECT_OFF);

	// Test the falling edge detection
	detection = 0;
	configure_gpio_event_detection_type(reading_pin, GPIO_FALLING, DETECT_ON);
	printf("Trigger falling edge detection\n");
	starttime = time(NULL);
	while ((detection == 0) | (difftime(time(NULL), starttime) < TEST_DURATION)) {
		detection = read_gpio_event_detection(reading_pin);
	}
	printf("Falling Edge Detection registered:%d\n", detection);
	configure_gpio_event_detection_type(reading_pin, GPIO_FALLING, DETECT_OFF);

	// Test the high level detection
	detection = 0;
	configure_gpio_event_detection_type(reading_pin, GPIO_HIGH_DETECT, DETECT_ON);
	printf("Trigger high level detection\n");
	starttime = time(NULL);
	while ((detection == 0) | (difftime(time(NULL), starttime) < TEST_DURATION)) {
		detection = read_gpio_event_detection(reading_pin);
	}
	printf("High Level Detection registered:%d\n", detection);
	configure_gpio_event_detection_type(reading_pin, GPIO_HIGH_DETECT, DETECT_OFF);

	// Test the low level detection
	detection = 0;
	configure_gpio_event_detection_type(reading_pin, GPIO_LOW_DETECT, DETECT_ON);
	printf("Trigger low level detection\n");
	starttime = time(NULL);
	while ((detection == 0) | (difftime(time(NULL), starttime) < TEST_DURATION)) {
		detection = read_gpio_event_detection(reading_pin);
	}
	printf("Low Level Detection registered:%d\n", detection);		
	configure_gpio_event_detection_type(reading_pin, GPIO_LOW_DETECT, DETECT_OFF);

	// Test the async rising edge detection
	detection = 0;
	configure_gpio_event_detection_type(reading_pin, ASYNC_RISING, DETECT_ON);
	printf("Trigger async rising edge detection\n");
	starttime = time(NULL);
	while ((detection == 0) | (difftime(time(NULL), starttime) < TEST_DURATION)) {
		detection = read_gpio_event_detection(reading_pin);
	}
	printf("Async Rising Edge Detection registered:%d\n", detection);		
	configure_gpio_event_detection_type(reading_pin, ASYNC_RISING, DETECT_OFF);	

	// Test the async falling edge detection
	detection = 0;
	configure_gpio_event_detection_type(reading_pin, ASYNC_FALLING, DETECT_ON);
	printf("Trigger async falling edge detection\n");
	starttime = time(NULL);
	while ((detection == 0) | (difftime(time(NULL), starttime) < TEST_DURATION)) {
		detection = read_gpio_event_detection(reading_pin);
	}
	printf("Async Falling Edge Detection registered:%d\n", detection);		
	configure_gpio_event_detection_type(reading_pin, ASYNC_FALLING, DETECT_OFF);

	// Test the no detection working
	detection = 0;
	printf("Attempt to trigger detection\n");
	starttime = time(NULL);
	while ((detection == 0) | (difftime(time(NULL), starttime) < TEST_DURATION)) {
		detection = read_gpio_event_detection(reading_pin);
	}
	printf("No Detection registered:%d\n", detection);		
#endif
    return (0);
}

