/*
 * main.cpp
 *
 * Created: 5/11/2017 11:07:24 AM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h>

 can_service* Object;

 int main(void) {
	/* Initialize the SAM system */
	system_init();

	ZpdcSystem system_data;
	ser_ethernet eth_obj(zpdc_sercom0_configuration, system_data.get_uid());
	can_service can_obj(zpdc_can0_configuration, Object);

	/* Replace with you application code */
	vTaskStartScheduler();
 }

 void CAN0_Handler(void) {
	(*Object).callback();
 }