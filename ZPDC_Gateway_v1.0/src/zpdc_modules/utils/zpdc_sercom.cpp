/*
 * zpdc_sercom.cpp
 *
 * Created: 5/11/2017 1:47:31 PM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h>

 uint8_t sercom_resource_manager::service_counter = 0;
 uint8_t sercom_resource_manager::hw_resource[sercom_resource_manager::max_resources] = { 0 };

 status_code zpdc_sercom::sercom_init(SerialEthernetConfiguration_SERCOM0 hw_instance) {
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);

	config_usart.baudrate = hw_instance.baudrate;
	config_usart.mux_setting = hw_instance.pm_settings;
	config_usart.pinmux_pad0 = hw_instance.pmux_p0;
	config_usart.pinmux_pad1 = hw_instance.pmux_p1;
	config_usart.pinmux_pad2 = hw_instance.pmux_p2;
	config_usart.pinmux_pad3 = hw_instance.pmux_p3;
	config_usart.parity = hw_instance.parity;

	// Block until module is initialized
	while ( usart_init((struct usart_module *const)getModule(), hw_instance.hardware, &config_usart) != STATUS_OK ) { }

	usart_enable((struct usart_module *const)getModule());

	return STATUS_OK;
 }

 void zpdc_sercom::eth_task_wrapper(void *pvParameters) {
	for (;;) {
		vTaskDelay(500);
	}
 }