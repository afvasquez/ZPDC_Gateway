/*
 * zpdc_ethernet.cpp
 *
 * Created: 5/11/2017 1:48:36 PM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h>

 ser_ethernet::ser_ethernet (SerialEthernetConfiguration_SERCOM0 ser_config) {
	for ( uint8_t i=0; i<64; i++ )
		rx_buffer[i] = '\0';
	rx_buffer_index = 0;

	if ( register_service() == REGISTER_OK ) {
		xTxMutex = xSemaphoreCreateMutex();

		sercom_init(ser_config);
		ethernet_init();

		setModule(this);

		t_init("ETH", 1);	// Task-Thread Initialization
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

void ser_ethernet::task(void) {
	uint8_t rx_command;
	rx_buffer_index = 0;

	print(CLRS);
	printnl(EDGE);
	printnl(BANN);
	printnl(EDGE);
	print(KEYS);

	usart_read_buffer_job((struct usart_module *const)getModule(), &rx_buffer[rx_buffer_index], 1);
	for (;;) {
		vTaskSuspend(handle);
		
		rx_command = getCommandID();


		//xSemaphoreTake(xTxMutex, portMAX_DELAY);

		switch (rx_command) {
			case 1:
				print(CLRS);
				printnl(EDGE);
				printnl(BANN);
				printnl(EDGE);
				print(KEYS);
			break;
			case 2:
				printnl("Thread started on background...");
			break;
			case 3:
				printnl("ZPDC Gateway v0.1.1. May 2017");
			break;
			default:
				printnl("Command not found!");
			break;
		}

		//printnl(Commands[rx_command].command);
		if (rx_command != 1) {
			printnl(" ");
			print(KEYS);
		}

		rx_buffer_index = 0;
		usart_read_buffer_job((struct usart_module *const)getModule(), &rx_buffer[rx_buffer_index], 1);
	}
}

uint8_t ser_ethernet::getCommandID(void) {
	rx_buffer_index--;	// Omit Line endings
	for (uint8_t i=0; i<3; i++) {	// Iterate through all the available commands
		for (uint8_t j=0; j<rx_buffer_index && rx_buffer_index == Commands[i].cmd_length; j++)
			if (rx_buffer[j] == Commands[i].command[j]) {
				if ( j == rx_buffer_index - 1) return Commands[i].cmd_id;
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

void ser_ethernet::printnl(const char* string_to_print) {
	print(string_to_print);

	xSemaphoreTake(xTxMutex, portMAX_DELAY);
	tx_buffer[0] = '\r';
	tx_buffer[1] = '\n';
	send(2);
}