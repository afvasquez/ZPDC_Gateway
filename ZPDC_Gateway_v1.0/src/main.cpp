/*
 * main.cpp
 *
 * Created: 5/11/2017 11:07:24 AM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h> 

 int main(void) {
	/* Initialize the SAM system */
	system_init();

	/* Replace with you application code */
	vTaskStartScheduler();
 }