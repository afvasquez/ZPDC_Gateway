/*
 * main.cpp
 *
 * Created: 5/11/2017 11:07:24 AM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h>

 can_service* Object;

 //struct can_module can_instance;

 int main(void) {
	/* Initialize the SAM system */
	system_init();

	ZpdcSystem system_data;

	ser_ethernet eth_obj(zpdc_sercom0_configuration, &system_data);

	can_service can_obj(zpdc_can0_configuration, &eth_obj, &system_data);
	Object = &can_obj;

	/* Replace with you application code */
	vTaskStartScheduler();
 }

 void CAN0_Handler(void) {
	Object->callback();
 }