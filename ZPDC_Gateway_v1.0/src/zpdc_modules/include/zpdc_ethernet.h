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

class Task {
public:
	BaseType_t t_init(void) {
		return xTaskCreate(
					&taskfun,
					"ETH",
					configMINIMAL_STACK_SIZE,
					this,
					tskIDLE_PRIORITY + 1,
					&handle);
	}

	virtual void task(void) =0;
	static void taskfun(void *parm) {
		((Task*)parm)->task();
		vTaskDelete(NULL);
	}

	TaskHandle_t handle;
};

class ser_ethernet : public zpdc_sercom , sercom_resource_manager, Task
{
public:
	const constexpr static char* NAME = "ETHERNET\0";

	ser_ethernet(SerialEthernetConfiguration_SERCOM0 ser_config);

	status_code ethernet_init(void);

	void buffer_transmitted_callback(void);		// struct usart_module *const module
	void buffer_received_callback(TaskHandle_t task_handler);		// struct usart_module *const module
	static void wrapper_received_callback( struct usart_module *const module ) {
		ser_ethernet* self = (ser_ethernet*) module->object_instance_pointer;
		self->buffer_received_callback(self->handle);
	}
	static void wrapper_transmitted_callback( struct usart_module *const module ) {
		ser_ethernet* self = (ser_ethernet*) module->object_instance_pointer;
		self->buffer_transmitted_callback();
	}

	void task(void);

		// Utility Methods
	void setServiceId(uint8_t id) { service_id = id; }
	uint8_t getServiceId() { return service_id; }
private:
	uint8_t rx_buffer[16];
	uint8_t service_id;
};

#endif // __cplusplus
#endif /* ZPDC_ETHERNET_H_ */