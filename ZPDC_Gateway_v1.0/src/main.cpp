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

	ser_ethernet eth_obj(zpdc_sercom0_configuration);

	/* Replace with you application code */
	vTaskStartScheduler();
 }