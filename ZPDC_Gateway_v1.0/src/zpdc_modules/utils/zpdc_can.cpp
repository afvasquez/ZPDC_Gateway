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

	queue_net_devices = xQueueCreate(10, sizeof(uint32_t));

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
	can_enable_interrupt(&can0_instance, CAN_TX_EVENT_FIFO_NEW_ENTRY);

	/*can_enable_interrupt(&can0_instance, CAN_TX_EVENT_FIFO_ELEMENT_LOST);
	can_enable_interrupt(&can0_instance, CAN_MESSAGE_RAM_ACCESS_FAILURE);
	can_enable_interrupt(&can0_instance, CAN_TIMEOUT_OCCURRED);
	can_enable_interrupt(&can0_instance, CAN_BIT_ERROR_CORRECTED);
	can_enable_interrupt(&can0_instance, CAN_BIT_ERROR_UNCORRECTED);
	can_enable_interrupt(&can0_instance, CAN_ERROR_LOGGING_OVERFLOW);
	can_enable_interrupt(&can0_instance, CAN_ERROR_PASSIVE);
	can_enable_interrupt(&can0_instance, CAN_PROTOCOL_ERROR_ARBITRATION);
	can_enable_interrupt(&can0_instance, CAN_PROTOCOL_ERROR_DATA);*/

	t_init("CAN", 1, 2);	// Task-Thread Initialization
 }

 void can_service::task(void) {
	uint32_t queue_item = 0;

	for(;;) {
		if (xQueueReceive(system_data->queue_to_can, &queue_item, portMAX_DELAY)) {
			switch (system_data->get_queue_entry_parameter(queue_item, 4)) {
				case CAN_DISCOVERY_REQUEST: {
					eth0->printnl("CAN DISCOVERY v0.1");
						// Send Discovery Request across network
					tx_message_0[0] = CAN_DISCOVERY_REQUEST;
					port_pin_set_output_level(PIN_PA07, true);
					send(1, CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);

					uint8_t net_size = 0;
					while(xQueueReceive(queue_net_devices, &queue_item, 150) == pdTRUE) {
						net_size++;
						eth0->print(" -> [");
						eth0->print((uint16_t)((queue_item >> 16) & 0xFF));
						eth0->print(",");
						eth0->print((uint16_t)((queue_item >> 8) & 0xFF));
						eth0->print("]\t");
						if ((queue_item & 0xFF) == 0xFF) eth0->print("NO_ADD");
						else eth0->print((uint16_t)(queue_item & 0xFF));
						switch ((queue_item >> 24) & 0xFF) {
							case CAN_DEVICE_GATEWAY:
								eth0->printnl("\tGATEWAY");
							break;
							case CAN_DEVICE_HYBRID:
								eth0->printnl("\tHYBRID_LEADER");
							break;
							case CAN_DEVICE_DRIVE_CARD:
								eth0->printnl("\tDRIVE_CARD");
							break;
							default:
								eth0->printnl("\tUNKNOWN");
							break;
						}
					}
					if (net_size == 0) CancelTransmission(CAN_BUFFER_0);

					vTaskResume(eth0->handle);
				}
				break;
				default:
					eth0->printnl("Unknown Request...");
				break;
			}
		}
	}
 }

 void can_service::callback(void) {
	BaseType_t xHigherPriorityWoken = pdFALSE;
	uint32_t status = can_read_interrupt_status(&can0_instance);

	if (!(status & CAN_RX_FIFO_0_NEW_MESSAGE) && !(status & CAN_TX_EVENT_FIFO_NEW_ENTRY)) {
		port_pin_set_output_level(PIN_PA04, true);
		can_clear_interrupt_status(&can0_instance, CAN_RX_FIFO_0_NEW_MESSAGE);
	}

	if (status & CAN_RX_FIFO_0_NEW_MESSAGE) {
		can_clear_interrupt_status(&can0_instance, CAN_RX_FIFO_0_NEW_MESSAGE);

		can_get_rx_fifo_0_element(&can0_instance, &rx_element_fifo_0, standard_receive_index++);
		if (standard_receive_index == CONF_CAN0_RX_BUFFER_NUM) standard_receive_index = 0;

		uint8_t sub_net = CAN_RX_FIFO_ID_SUBNET(rx_element_fifo_0.R0.reg);
		if(sub_net == CAN_SUBNET_NETWORK_REQUEST) {
			switch(rx_element_fifo_0.data[0]) {
				case CAN_DISCOVERY_REQUEST:
					port_pin_set_output_level(PIN_PA03, true);
					tx_message_0[0] = CAN_DISCOVERY_RETURN;
					tx_message_0[1] = system_data->get_uid_high();
					tx_message_0[2] = system_data->get_uid_low();
					tx_message_0[3] = system_data->get_address();
					send(4, CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);
					port_pin_set_output_level(PIN_PA03, false);
				break;
				case CAN_DISCOVERY_RETURN: {
					uint32_t i_data = (uint32_t)((uint32_t)((CAN_RX_FIFO_ID_DEVICE(rx_element_fifo_0.R0.reg) << 24 ) | 
															rx_element_fifo_0.data[1] << 16) | 
															(uint32_t)(rx_element_fifo_0.data[2] << 8) | 
															(uint32_t)(rx_element_fifo_0.data[3]));
					xQueueSendFromISR(queue_net_devices,
										&i_data, 
										&xHigherPriorityWoken);
				}
				break;
			}	
		}
	}

	if (status & CAN_TX_EVENT_FIFO_NEW_ENTRY) {
		port_pin_set_output_level(PIN_PA07, false);
		can_clear_interrupt_status(&can0_instance, CAN_TX_EVENT_FIFO_NEW_ENTRY);
	}

	if (status & CAN_TX_EVENT_FIFO_FULL) {
		can_clear_interrupt_status(&can0_instance, CAN_TX_EVENT_FIFO_FULL);
		can_tx_event_fifo_acknowledge(&can0_instance, CAN_TX_EVENT_FIFO_FULL);
	}

	portYIELD_FROM_ISR(xHigherPriorityWoken);
 }

 void can_service::send(uint8_t length, uint8_t device, uint8_t sub_net, uint8_t buffer) {
	struct can_tx_element tx_element;
	can_get_tx_buffer_element_defaults(&tx_element);

	tx_element.T0.reg |= CAN_TX_ELEMENT_T0_STANDARD_ID((device << 9) | (sub_net << 7) | ((uint8_t) ((system_data->get_uid() >> 1) & 0x7F)));
	tx_element.T1.bit.DLC = (uint32_t)length;
	for (uint8_t i=0; i<length; i++) tx_element.data[i] = tx_message_0[i];
	can_set_tx_buffer_element(&can0_instance, &tx_element, (uint32_t)buffer);
	can_tx_transfer_request(&can0_instance, (uint32_t)(1 << buffer));
	//if (can_tx_transfer_request(&can0_instance, (uint32_t)(1 << buffer)) == STATUS_OK) port_pin_set_output_level(PIN_PA07, true);
 }
