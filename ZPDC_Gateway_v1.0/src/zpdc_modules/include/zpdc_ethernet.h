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

/********************** Dictionary Definitions **************************/
typedef struct {
	const uint8_t cmd_id;
	const uint8_t num_args;
	const uint8_t cmd_length;
	const char *command;
} CommandDictionary;

const static CommandDictionary Commands[] = {
	{1, 0, 7, "restart"},
	{2, 0, 9, "resources"},
	{3, 0, 7, "version"}
};
/************************************************************************/

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
	const constexpr static char* CLRS = "\e[2J\e[3J\e[H\0";
	const constexpr static char* EDGE = "#######################################################\0";
	const constexpr static char* BANN = "#            ZPDC GATEWAY INTERFACE V0.1.1            #\0";
	const constexpr static char* KEYS = ">> \0";

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

		// I/O Functionality
	SemaphoreHandle_t xTxMutex;
	uint8_t rx_buffer_index;
	void print(const char* string_to_print);
	void printnl(const char* string_to_print);


		// Utility Methods
	void setServiceId(uint8_t id) { service_id = id; }
	uint8_t getServiceId() { return service_id; }
private:
	uint8_t rx_buffer[64];
	uint8_t tx_buffer[64];
	uint8_t service_id;

	inline void send(uint8_t length) { usart_write_buffer_job((struct usart_module *const)getModule(), tx_buffer, length); }
	uint8_t getCommandID(void);
};

#endif // __cplusplus
#endif /* ZPDC_ETHERNET_H_ */