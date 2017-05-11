/*
 * zpdc_ethernet.cpp
 *
 * Created: 5/11/2017 1:48:36 PM
 *  Author: Andres Vasquez
 */ 
 #include <asf.h>

 ser_ethernet::ser_ethernet () {
	if ( register_service() == REGISTER_OK ) {
	}
 }

 usart_callback_t zpdc_sercom::buffer_received_callback(struct usart_module *const module) {

 }

 usart_callback_t zpdc_sercom::buffer_transmitted_callback(struct usart_module *const module) {

 }