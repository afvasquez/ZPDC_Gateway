/*
 * zpdc_can.cpp
 *
 * Created: 5/18/2017 2:17:47 PM
 *  Author: Andres Vasquez
 */ 
#include <asf.h>

 can_service::can_service(CanConfiguration_CAN0 config, ser_ethernet *eth_interface, ZpdcSystem *system_module) {
		// ISO C++ Compliance
	standard_receive_index = 0; 
	extended_receive_index = 0;
	network_size = 0;

	eth0 = eth_interface;
	system_data = system_module;

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

	can_init(&can0_instance, config.hardware, &config_can);
	can_start(&can0_instance);
	
	system_interrupt_enable(config.interrupt_vector);
	can_enable_interrupt(&can0_instance, CAN_RX_FIFO_0_NEW_MESSAGE);
	can_enable_interrupt(&can0_instance, CAN_TX_FIFO_EMPTY); 

	t_init("CAN", 1, 2);	// Task-Thread Initialization
 }

 void can_service::task(void) {
	uint32_t queue_item = 0;
	for(;;) {
		if (xQueueReceive(system_data->queue_to_can, &queue_item, portMAX_DELAY)) {
			switch (system_data->get_queue_entry_parameter(queue_item, 4)) {
				case CAN_DISCOVERY_REQUEST:
					eth0->printnl("CAN DISCOVERY v0.1");
						// Send Discovery Request across network
					tx_message_0[0] = '#';
					tx_message_0[1] = '#';
					tx_message_0[2] = 'T';
					tx_message_0[3] = 'E';
					tx_message_0[4] = 'S';
					tx_message_0[5] = 'T';
					tx_message_0[6] = '*';
					tx_message_0[7] = '\0';
					//port_pin_set_output_level(PIN_PA07, true);
					send(8, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);
					vTaskResume(eth0->handle);
				break;
				default:
					eth0->printnl("Unknown Request...");
				break;
			}
		}
	}
 }

 void can_service::callback(void) {
	uint32_t status = can_read_interrupt_status(&can0_instance);

	if (status & CAN_RX_FIFO_0_NEW_MESSAGE) {
		port_pin_toggle_output_level(PIN_PA07);
		//port_pin_set_output_level(PIN_PA07, true);
		can_clear_interrupt_status(&can0_instance, CAN_RX_FIFO_0_NEW_MESSAGE);
	}

	if (status & CAN_TX_FIFO_EMPTY) {
		port_pin_set_output_level(PIN_PA07, false);
		can_clear_interrupt_status(&can0_instance, CAN_TX_FIFO_EMPTY);
	}
 }

 void can_service::send(uint8_t length, uint8_t sub_net, uint8_t buffer) {
	struct can_tx_element tx_element;
	can_get_tx_buffer_element_defaults(&tx_element);

	tx_element.T0.reg |= CAN_TX_ELEMENT_T0_STANDARD_ID((sub_net << 7) | ((uint8_t) (system_data->get_uid()) & 0xFF));
	tx_element.T1.bit.DLC = (uint32_t)length;
	for (uint8_t i=0; i<length; i++) tx_element.data[i] = tx_message_0[i];
	can_set_tx_buffer_element(&can0_instance, &tx_element, (uint32_t)buffer);
	if (can_tx_transfer_request(&can0_instance, (uint32_t)(1 << buffer)) == STATUS_OK) port_pin_set_output_level(PIN_PA07, true);
 }