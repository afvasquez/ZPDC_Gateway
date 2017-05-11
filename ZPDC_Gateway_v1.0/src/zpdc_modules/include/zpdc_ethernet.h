/*
 * zpdc_ethernet.h
 *
 * Created: 5/11/2017 1:52:39 PM
 *  Author: avasquez
 */ 


#ifndef ZPDC_ETHERNET_H_
#define ZPDC_ETHERNET_H_
#ifdef __cplusplus

#include "zpdc_sercom.h"
class zpdc_sercom;

class ser_ethernet : zpdc_sercom, sercom_resource_manager
{
public:
	ser_ethernet();

	const constexpr static char* NAME = "ETHERNET\0";

	usart_callback_t buffer_transmitted_callback(struct usart_module *const module);
	usart_callback_t buffer_received_callback(struct usart_module *const module);

	usart_module *getModule() { return &module; }
	void setServiceId(uint8_t id) { service_id = id; }
	uint8_t getServiceId(uint8_t) { return service_id; }
private:
	usart_module module;
	uint8_t service_id;
};

#endif // __cplusplus
#endif /* ZPDC_ETHERNET_H_ */