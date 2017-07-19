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
	CanQueueRequest *can_queue_item;

	for(;;) {
		if (xQueueReceive(system_data->queue_to_can, &queue_item, portMAX_DELAY)) {
			//switch (system_data->get_queue_entry_parameter(queue_item, 4) & 0xFC) {
			can_queue_item = (CanQueueRequest *)queue_item;
			tx_message_0[0] = 0;
			switch (can_queue_item->can_command) {
				case CAN_QUEUE_COMMAND_DISCOVERY: {
					eth0->printnl("CAN DISCOVERY v0.1");
						// Send Discovery Request across network
					tx_message_0[0] = CAN_DISCOVERY_REQUEST;
					//port_pin_set_output_level(PIN_PA07, true);
					send(1, CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);

					uint8_t net_size = 0;
					while(xQueueReceive(queue_net_devices, &queue_item, 150) == pdTRUE) {
						net_size++;
						PrintCanDeviceData(queue_item);
					}
					if (net_size == 0) CancelTransmission(CAN_BUFFER_0);

					vTaskResume(eth0->handle);
				}
				break;
				case CAN_QUEUE_COMMAND_ORDER:
					tx_message_0[0] = CAN_ORDER_UPDATE_REQUEST;
					tx_message_0[1] = can_queue_item->arg_1 >> 8;
					tx_message_0[2] = can_queue_item->arg_1 & 0x00FF;
					tx_message_0[3] = can_queue_item->arg_2 >> 8;
					tx_message_0[4] = can_queue_item->arg_2 & 0x0003;
					send(5,CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);
					if (xQueueReceive(queue_net_devices, &queue_item, 150)) PrintCanDeviceData(queue_item);
					else { CancelTransmission(CAN_BUFFER_0); eth0->printnl(" -> Device did not respond"); }
					vTaskResume(eth0->handle);
				break;
				case CAN_QUEUE_COMMAND_LED_TRIG:
					tx_message_0[0] = CAN_REQUEST_LED_TOG;
				case CAN_QUEUE_COMMAND_GET_PARSA:
					tx_message_0[0] = CAN_MOTOR_GET_PARAMA;
				case CAN_QUEUE_COMMAND_MOT_START:
					if(tx_message_0[0] == 0) tx_message_0[0] = CAN_MOTOR_START;
				case CAN_QUEUE_COMMAND_MOT_STOP:
					if(tx_message_0[0] == 0) tx_message_0[0] = CAN_MOTOR_STOP;
					tx_message_0[1] = can_queue_item->arg_1 >> 8;
					tx_message_0[2] = can_queue_item->arg_1 & 0x00FF;
					send(3,CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);
					if (tx_message_0[1] + tx_message_0[2] == 0) {
						uint8_t dev_response = 0;
						while(xQueueReceive(queue_net_devices, &queue_item, 150) == pdTRUE) {
							dev_response++;
							PrintCanDeviceCommandSetAlphaResponse(queue_item);
						}
						if(dev_response == 0) { CancelTransmission(CAN_BUFFER_0); eth0->printnl(" -> Devices did NOT respond"); }
					} else {
						if (xQueueReceive(queue_net_devices, &queue_item, 150)) {
							PrintCanDeviceCommandSetAlphaResponse(queue_item);
						} else { CancelTransmission(CAN_BUFFER_0); eth0->printnl(" -> Device did not respond"); }
					}
					vTaskResume(eth0->handle);
				break;
				case CAN_QUEUE_COMMAND_PID_PARS:
					tx_message_0[0] = CAN_MOTOR_PID_TUNE;
				case CAN_QUEUE_COMMAND_MOT_PARSA:
					if(tx_message_0[0] == 0) tx_message_0[0] = CAN_MOTOR_PARAM_A;
				case CAN_QUEUE_COMMAND_MOT_PARSB:
					if(tx_message_0[0] == 0) tx_message_0[0] = CAN_MOTOR_PARAM_B;
					tx_message_0[1] = (uint8_t)(can_queue_item->arg_1 & 0x00FF);
					tx_message_0[2] = (uint8_t)(can_queue_item->arg_2 >> 8);
					tx_message_0[3] = (uint8_t)(can_queue_item->arg_2 & 0x00FF);
					tx_message_0[4] = (uint8_t)(can_queue_item->arg_3 >> 8);
					tx_message_0[5] = (uint8_t)(can_queue_item->arg_3 & 0x00FF);
					tx_message_0[6] = (uint8_t)(can_queue_item->arg_4 >> 8);
					tx_message_0[7] = (uint8_t)(can_queue_item->arg_4 & 0x00FF);
					send(8,CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);
					while(xQueueReceive(queue_net_devices, &queue_item, 150) == pdTRUE) {	// TODO: Wait can be variable
						PrintCanDeviceOrder(system_data->get_queue_entry_parameter(queue_item, 3));
						uint8_t operation_result = system_data->get_queue_entry_parameter(queue_item, 2);
						if (operation_result == 0)		eth0->printnl("FAIL");
						else if (operation_result == 1)	eth0->printnl("DONE");
						else if (operation_result == 2)	eth0->printnl("NO_CHANGE");
						else eth0->printnl("ERROR");
					}
					vTaskResume(eth0->handle);
				break;
				default:
					eth0->print(can_queue_item->can_command); eth0->printnl(" :: Unknown Request..."); 
					vTaskResume(eth0->handle);
				break;
			}
		}
	}
 }

 void can_service::callback(void) {
	BaseType_t xHigherPriorityWoken = pdFALSE;
	uint32_t status = can_read_interrupt_status(&can0_instance);
	bool isCanToSend = false;

	if (!(status & CAN_RX_FIFO_0_NEW_MESSAGE) && !(status & CAN_TX_EVENT_FIFO_NEW_ENTRY)) {
		port_pin_set_output_level(PIN_PA04, true);
		can_clear_interrupt_status(&can0_instance, CAN_RX_FIFO_0_NEW_MESSAGE);
	}

	isCanToSend = false;
	if (status & CAN_RX_FIFO_0_NEW_MESSAGE) {
		can_clear_interrupt_status(&can0_instance, CAN_RX_FIFO_0_NEW_MESSAGE);

		can_get_rx_fifo_0_element(&can0_instance, &rx_element_fifo_0, standard_receive_index++);
		if (standard_receive_index == CONF_CAN0_RX_BUFFER_NUM) standard_receive_index = 0;

		uint8_t sub_net = CAN_RX_FIFO_ID_SUBNET(rx_element_fifo_0.R0.reg);
		if(sub_net == CAN_SUBNET_NETWORK_REQUEST) {
			switch(rx_element_fifo_0.data[0]) {
				case CAN_ORDER_UPDATE_REQUEST:
					if (rx_element_fifo_0.data[1] == system_data->get_uid_high() && rx_element_fifo_0.data[2] == system_data->get_uid_low())
						{ system_data->set_address(rx_element_fifo_0.data[3]); isCanToSend = true; }
					tx_message_0[0] = CAN_ORDER_UPDATE_RETURN;
				case CAN_DISCOVERY_REQUEST:
					port_pin_set_output_level(PIN_PA03, true);

					if (rx_element_fifo_0.data[0] == CAN_DISCOVERY_REQUEST) 
						{ tx_message_0[0] = CAN_DISCOVERY_RETURN; isCanToSend = true; }

					tx_message_0[1] = system_data->get_uid_high();
					tx_message_0[2] = system_data->get_uid_low();
					tx_message_0[3] = system_data->get_address();
					if (isCanToSend) send(4, CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);
					port_pin_set_output_level(PIN_PA03, false);
				break;
				case CAN_REQUEST_LED_TOG:
					if (rx_element_fifo_0.data[1] == system_data->get_uid_high() && rx_element_fifo_0.data[2] == system_data->get_uid_low()) {
						port_pin_toggle_output_level(PIN_PA07);
						tx_message_0[0] = CAN_REQUEST_LED_TOG_RETURN;
						tx_message_0[1] = system_data->get_uid_high();
						tx_message_0[2] = system_data->get_uid_low();
						tx_message_0[3] = (port_pin_get_output_level(PIN_PA07) ? 0x01 : 0x00);
						send(4, CAN_DEVICE_GATEWAY, CAN_SUBNET_NETWORK_REQUEST, CAN_BUFFER_0);
						port_pin_set_output_level(PIN_PA03, false);
					}
				break;
				case CAN_MOTOR_GET_PARAMA_RETURN:
				case CAN_MOTOR_START_RETURN:
				case CAN_MOTOR_STOP_RETURN:
				case CAN_REQUEST_LED_TOG_RETURN:
				case CAN_ORDER_UPDATE_RETURN:
				case CAN_MOTOR_PID_TUNE_RETURN:
				case CAN_MOTOR_PARAM_A_RETURN:
				case CAN_MOTOR_PARAM_B_RETURN:
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
				case CAN_MOTOR_STATREPA_RETURN:
					uint32_t e_data = (uint32_t)((rx_element_fifo_0.data[1] << 24)|
												 (rx_element_fifo_0.data[2] << 16)|
												 (rx_element_fifo_0.data[3] << 8));
					xQueueSendFromISR(eth0->eth_error_report.error_queue,
										&e_data,
										&xHigherPriorityWoken);
				break;
			}	
		}
	}

	if (status & CAN_TX_EVENT_FIFO_NEW_ENTRY) {
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
