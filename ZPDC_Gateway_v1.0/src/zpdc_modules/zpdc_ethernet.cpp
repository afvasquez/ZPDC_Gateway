/*
 * zpdc_ethernet.cpp
 *
 * Created: 5/11/2017 1:48:36 PM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h>

 ser_ethernet::ser_ethernet (SerialEthernetConfiguration_SERCOM0 ser_config, ZpdcSystem *system_module) {
	for ( uint8_t i=0; i<64; i++ )
		rx_buffer[i] = '\0';
	rx_buffer_index = 0;
	system_data = system_module;

	if ( register_service() == REGISTER_OK ) {
		xTxMutex = xSemaphoreCreateMutex();

		sercom_init(ser_config);
		ethernet_init();

		setModule(this);

		t_init("ETH", 1, 2);	// Task-Thread Initialization
	}
 }

 status_code ser_ethernet::ethernet_init(void) {
	 usart_register_callback((struct usart_module *const)getModule(), wrapper_transmitted_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
	 usart_enable_callback((struct usart_module *const)getModule(), USART_CALLBACK_BUFFER_TRANSMITTED);

	 usart_register_callback((struct usart_module *const)getModule(), wrapper_received_callback, USART_CALLBACK_BUFFER_RECEIVED);
	 usart_enable_callback((struct usart_module *const)getModule(), USART_CALLBACK_BUFFER_RECEIVED);

	 return STATUS_OK;
 }

 void ser_ethernet::buffer_received_callback(TaskHandle_t task_handler) {
	BaseType_t xYieldRequired;

	// DEBUG, find the end of the transmission
	if ( rx_buffer[rx_buffer_index] == '\n' ) {
		xYieldRequired = xTaskResumeFromISR(task_handler);
		portYIELD_FROM_ISR(xYieldRequired);
	} else {
		rx_buffer_index++;
		usart_read_buffer_job((struct usart_module *const)getModule(), &rx_buffer[rx_buffer_index], 1);
	}
 }
 void ser_ethernet::buffer_transmitted_callback(void) {
	BaseType_t xHigherPriorityWoken;

	xSemaphoreGiveFromISR(xTxMutex, &xHigherPriorityWoken);

	portYIELD_FROM_ISR( xHigherPriorityWoken );
 }

static CanQueueRequest can_requests[2];
void ser_ethernet::task(void) {
	uint8_t rx_command;
	uint16_t arg_holder;
	rx_buffer_index = 0;

	can_requests[0].point_self = (uint32_t)&(can_requests[0]);
	can_requests[1].point_self = (uint32_t)&(can_requests[1]);
	
	print(CLRS);
	printnl(EDGE);
	printnl(BANN);
	printnl(EDGE);
	print(KEYS);

	usart_read_buffer_job((struct usart_module *const)getModule(), &rx_buffer[rx_buffer_index], 1);
	for (;;) {
		vTaskSuspend(handle);
		
		rx_command = getCommandID();

		switch (rx_command) {
			case 1:
				print(CLRS);
				printnl(EDGE);
				printnl(BANN);
				printnl(EDGE);
				print(KEYS);
			break;
			case 2:
				print("Device Unique ID\r\n[");
				print((int16_t)((system_data)->get_uid() >> 8));
				print(".");
				print((int16_t)((system_data)->get_uid() & 0x00FF));
				printnl("]");
			break;
			case 3:
				printnl("ZPDC Gateway v0.3.5. July 2017");
			break;
			case 4:
				can_requests[0].can_command = CAN_QUEUE_COMMAND_DISCOVERY;
 				xQueueSend(system_data->queue_to_can, &(can_requests[0].point_self), portMAX_DELAY);
 				vTaskSuspend(handle);
			break;
			case 5:
				can_requests[0].can_command = CAN_QUEUE_COMMAND_ORDER;
 				if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
 					can_requests[0].arg_1 = (arg_holder << 8);
 					if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
 						can_requests[0].arg_1 |= (arg_holder & 0x00FF);
 						if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
 							can_requests[0].arg_2 = (arg_holder << 8);
 							if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
 								if ((uint8_t)arg_holder == 'D') can_requests[0].arg_2 |= CAN_DEVICE_DRIVE_CARD;
 								else if ((uint8_t)arg_holder == 'H') can_requests[0].arg_2 |= CAN_DEVICE_HYBRID;
 								else if ((uint8_t)arg_holder == 'G') can_requests[0].arg_2 |= CAN_DEVICE_GATEWAY;
 								else arg_holder = 0;
 							} else arg_holder = 0;
 						} else arg_holder = 0;
 					} else arg_holder = 0;
 				} else arg_holder = 0;
				
				if (arg_holder) {
					xQueueSend(system_data->queue_to_can, &(can_requests[0].point_self), portMAX_DELAY);
					vTaskSuspend(handle);
				} else printnl("ERROR: Error parsing arguments.");
			break;
			case 6:
				can_requests[0].can_command = CAN_QUEUE_COMMAND_MOT_START;
			case 7:
				if (rx_command == 7) can_requests[0].can_command = CAN_QUEUE_COMMAND_MOT_STOP;
				if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
					can_requests[0].arg_1 = (arg_holder << 8);
					if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
						can_requests[0].arg_1 |= (arg_holder & 0x00FF);
					} else arg_holder = 0;
				} else arg_holder = 0;

				if (arg_holder) {
					xQueueSend(system_data->queue_to_can, &(can_requests[0].point_self), portMAX_DELAY);
					vTaskSuspend(handle);
				} else printnl("ERROR: Error parsing arguments.");				
			break;
			case 8:
				can_requests[0].can_command = CAN_QUEUE_COMMAND_PID_PARS;
			case 9:
				if (rx_command == 9) can_requests[0].can_command = CAN_QUEUE_COMMAND_MOT_PARSA;
			case 10:
				if (rx_command == 10) can_requests[0].can_command = CAN_QUEUE_COMMAND_MOT_PARSB;
				if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
					can_requests[0].arg_1 = (arg_holder & 0x00FF);		// Order #
					if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
						can_requests[0].arg_2 = arg_holder;				// Ramp Duration in ms
						if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
							can_requests[0].arg_3 = arg_holder;			// Speed Setting in RPM
							if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
								can_requests[0].arg_4 = arg_holder;		// Gear Ratio Setting
							} else arg_holder = CAN_DICTIONARY_ARG_NOT_FOUND;
						} else arg_holder = CAN_DICTIONARY_ARG_NOT_FOUND;
					} else arg_holder = CAN_DICTIONARY_ARG_NOT_FOUND;
				} else arg_holder = CAN_DICTIONARY_ARG_NOT_FOUND;
				
				if (arg_holder != CAN_DICTIONARY_ARG_NOT_FOUND) {
					xQueueSend(system_data->queue_to_can, &(can_requests[0].point_self), portMAX_DELAY);
					vTaskSuspend(handle);
				} else printnl("ERROR: Error parsing arguments.");
			break;
			case 11:
				can_requests[0].can_command = CAN_QUEUE_COMMAND_LED_TRIG;
				if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
					can_requests[0].arg_1 = (arg_holder << 8);
					if ((arg_holder = (getArgumentValue(rx_buffer[0]))) != CAN_DICTIONARY_ARG_NOT_FOUND) {
						can_requests[0].arg_1 |= (arg_holder & 0x00FF);
					} else arg_holder = 0;
				} else arg_holder = 0;

				if (arg_holder) {
					xQueueSend(system_data->queue_to_can, &(can_requests[0].point_self), portMAX_DELAY);
					vTaskSuspend(handle);
				} else printnl("ERROR: Error parsing arguments.");
			break;
			default:
				printnl("Command not found!");
			break;
		}

		if (rx_command != 1) {
			printnl(" ");
			print(KEYS);
		}

		rx_buffer_index = 0;
		usart_read_buffer_job((struct usart_module *const)getModule(), &rx_buffer[rx_buffer_index], 1);
	}
}

void ser_ethernet::getArgumentParameters(uint8_t buffer_offset, uint8_t *p_start, uint8_t *p_end) {
	bool start_found = false;
	for(int i=buffer_offset; i<64; i++)
		if(!start_found) {
			if(rx_buffer[i] != ' ') { 
				*p_start = i; 
				start_found = true; 
			}
		} else if(rx_buffer[i] == ' ' || rx_buffer[i] == '\r' || rx_buffer[i] == '\n') { 
			*p_end = i-1; 
			return; 
		} 
}

uint16_t ser_ethernet::getArgumentValue(uint8_t buffer_offset) {
	volatile uint8_t p_start = 0;
	volatile uint8_t p_end = 0;
	volatile uint16_t result = 0;
	volatile uint16_t multiplier = 0;
	bool start_found = false;

	for(int i=buffer_offset; i<64; i++)
		if(!start_found) {
			if(rx_buffer[i] != ' ') { 
				p_start = i; 
				start_found = true; 
			}
		} else if(rx_buffer[i] == ' ' || rx_buffer[i] == '\r' || rx_buffer[i] == '\n') {
			p_end = i-1;
			rx_buffer[0] = p_end + 1;
			i=64; 
		}

	if (buffer_offset >= p_end) {
		return 0xFFFF;
	} else {
		for(int i=0; i<(p_end - p_start + 1); i++) {
			multiplier = (rx_buffer[p_start + i] > '9'? rx_buffer[p_start + i] : rx_buffer[p_start + i] - 48);
			for(int j=(p_end - p_start + 1); j>i+1;j--) {
				multiplier = multiplier * 10;
			}
			result += multiplier;
		}
	}
	return result;
}

uint8_t ser_ethernet::getCommandID(void) {
	uint8_t arg_start, arg_end;
	getArgumentParameters(0, &arg_start, &arg_end);

	rx_buffer_index--;	// Omit Line endings
	for (uint8_t i=0; i<CAN_DICTIONARY_LENGTH; i++) {	// Iterate through all the available commands
		for (uint8_t j=arg_start; j<(arg_end - arg_start + 1) && (arg_end - arg_start + 1) == Commands[i].cmd_length; j++)
			if (rx_buffer[j] == Commands[i].command[j]) {
				if ( j == (arg_end - arg_start)) { rx_buffer[0]=arg_end + 1; return Commands[i].cmd_id; }
			} else j = rx_buffer_index;
	}
	return 0;
}

// SERIAL ETHERNET UTILITIES
void ser_ethernet::print(const char* string_to_print) {
	uint8_t length_counter = 0;

	xSemaphoreTake(xTxMutex, portMAX_DELAY);

	while( (*(string_to_print + length_counter) != '\0') && (length_counter < 64) ) {
		tx_buffer[length_counter] = *(string_to_print + length_counter);
		length_counter++;
	}

	// TODO: A string length check to be implemented
	send(length_counter);
}
void ser_ethernet::print(int16_t value) {
		// Negative sign
	if (value < 0) {
		value *= -1;
		print("-");
	}

	xSemaphoreTake(xTxMutex, portMAX_DELAY);
	bool isLeftZero = true;
	uint16_t divider = 10000;
	uint16_t index_iterator = 0;
	while (divider > 0) {
		if ((tx_buffer[index_iterator] = value / divider) > 0) { 
			isLeftZero = false;
			tx_buffer[index_iterator] += 48;	// To ASCII
			index_iterator++;
		} else if (!isLeftZero) {
			tx_buffer[index_iterator] += 48;	// To ASCII
			index_iterator++;
		}

		value %= divider;
		divider /= 10;

		if (divider == 0 && isLeftZero) tx_buffer[index_iterator++] = '0';
	}
	send(index_iterator);
}

void ser_ethernet::printnl(const char* string_to_print) {
	print(string_to_print);

	xSemaphoreTake(xTxMutex, portMAX_DELAY);
	tx_buffer[0] = '\r';
	tx_buffer[1] = '\n';
	send(2);
}