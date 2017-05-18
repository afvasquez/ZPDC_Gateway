/*
 * zpdc_can.cpp
 *
 * Created: 5/18/2017 2:17:47 PM
 *  Author: Andres Vasquez
 */ 
#include <asf.h>

 can_service::can_service(CanConfiguration_CAN0 config, can_service *obj) {
		// ISO C++ Compliance
	standard_receive_index = 0; 
	extended_receive_index = 0;

	obj = this;

		// CAN Module Setup
	struct system_pinmux_config can_pin_config;
	system_pinmux_get_config_defaults(&can_pin_config);
	can_pin_config.mux_position = config.tx_mux;
	system_pinmux_pin_set_config(config.tx_pin, &can_pin_config);
	can_pin_config.mux_position = config.rx_mux;
	system_pinmux_pin_set_config(config.rx_pin, &can_pin_config);

	struct can_config config_can;
	can_get_config_defaults(&config_can);
	config_can.nonmatching_frames_action_standard = config.nonmatching_action;

	can_init(&can_instance, config.hardware, &config_can);
	can_start(&can_instance);
	
	system_interrupt_enable(config.interrupt_vector);
	can_enable_interrupt(&can_instance, CAN_RX_FIFO_0_NEW_MESSAGE);

	t_init("CAN", 1);	// Task-Thread Initialization
 }

 void can_service::task(void) {
	for(;;) {
		vTaskDelay(250);
	}
 }

 void can_service::callback(void) {
	
 }