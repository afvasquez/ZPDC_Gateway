/*
 * zpdc_ethernet.h
 *
 * Created: 5/11/2017 1:52:39 PM
 *  Author: Andres Vasquez
 */ 


#ifndef ZPDC_ETHERNET_H_
#define ZPDC_ETHERNET_H_
#ifdef __cplusplus

#include "zpdc_sercom.h"

/********************** Dictionary Definitions **************************/
#define CAN_DICTIONARY_LENGTH			6
#define CAN_DICTIONARY_ARG_NOT_FOUND	(uint16_t)0xFFFF
typedef struct {
	const uint8_t cmd_id;
	const uint8_t num_args;
	const uint8_t cmd_length;
	const char *command;
} CommandDictionary;

const static CommandDictionary Commands[] = {
	{1, 0, 7, "restart"},
	{2, 0, 9, "resources"},
	{3, 0, 7, "version"},
	{4, 0, 7, "candisc"},
	{5, 0, 8, "canorder"},
	{CAN_DICTIONARY_LENGTH, 0, 6, "canled"}
};
/************************************************************************/

class zpdc_sercom;

class ser_ethernet : public Task, public zpdc_sercom , sercom_resource_manager
{
public:
	const constexpr static char* NAME = "ETHERNET\0";
	const constexpr static char* CLRS = "\e[2J\e[3J\e[H\0";
	const constexpr static char* EDGE = "#######################################################\0";
	const constexpr static char* BANN = "#            ZPDC GATEWAY INTERFACE V0.3.4            #\0";
	const constexpr static char* KEYS = ">> \0";

	ser_ethernet(SerialEthernetConfiguration_SERCOM0 ser_config, ZpdcSystem *system_module);

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
	void print(int16_t value);
	void print(const char* string_to_print);
	void printnl(const char* string_to_print);

		// Utility Methods
	void setServiceId(uint8_t id) { service_id = id; }
	uint8_t getServiceId() { return service_id; }
private:
	uint8_t rx_buffer[64];
	uint8_t tx_buffer[64];
	uint8_t service_id;
	ZpdcSystem *system_data;

	inline void send(uint8_t length) { usart_write_buffer_job((struct usart_module *const)getModule(), tx_buffer, length); }
	uint8_t getCommandID(void);
	void getArgumentParameters(uint8_t buffer_offset, uint8_t *p_start, uint8_t *p_end);
	uint16_t getArgumentValue(uint8_t buffer_offset);
};

#endif // __cplusplus
#endif /* ZPDC_ETHERNET_H_ */