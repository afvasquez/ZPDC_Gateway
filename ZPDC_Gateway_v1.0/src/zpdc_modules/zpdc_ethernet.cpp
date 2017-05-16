/*
 * zpdc_ethernet.cpp
 *
 * Created: 5/11/2017 1:48:36 PM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h>

 ser_ethernet::ser_ethernet (SerialEthernetConfiguration_SERCOM0 ser_config) {
	for ( uint8_t i=0; i<16; i++ )
		rx_buffer[i] = '\0';

	if ( register_service() == REGISTER_OK ) {
		sercom_init(ser_config);
		ethernet_init();

		setModule(this);

		t_init();
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
	xYieldRequired = xTaskResumeFromISR(task_handler);

	portYIELD_FROM_ISR(xYieldRequired);
 }

 void ser_ethernet::buffer_transmitted_callback(void) {

 }

void ser_ethernet::task(void) {
	rx_buffer[0] = 'A';
	//usart_write_buffer_job(u_module, rx_buffer, 1);
	usart_read_buffer_job((struct usart_module *const)getModule(), rx_buffer, 1);

	for (;;) {
		vTaskSuspend(handle);
		usart_write_buffer_job((struct usart_module *const)getModule(), rx_buffer, 1);
		usart_read_buffer_job((struct usart_module *const)getModule(), rx_buffer, 1);
	}
}