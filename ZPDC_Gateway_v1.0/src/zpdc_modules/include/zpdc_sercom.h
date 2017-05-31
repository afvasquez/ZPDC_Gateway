/*
 * zpdc_sercom.h
 *
 * Created: 5/11/2017 1:47:50 PM
 *  Author: Andres Vasquez
 */ 


#ifndef ZPDC_SERCOM_H_
#define ZPDC_SERCOM_H_
#ifdef __cplusplus

/************************************************************************/
/* CONSTANTS FOR SERCOM CONFIGURATION (CHANGE AS NEEDED)                */
/************************************************************************/
static const struct SerialEthernetConfiguration_SERCOM0 {
	const static constexpr char *NAME = "SERCOM1\0";
	uint8_t resource_id = 1;

	Sercom *hardware = SERCOM1;

	uint32_t baudrate = 115200;

	uint32_t pmux_p0 = PINMUX_PA16C_SERCOM1_PAD0;
	uint32_t pmux_p1 = PINMUX_UNUSED;
	uint32_t pmux_p2 = PINMUX_PA18C_SERCOM1_PAD2;
	uint32_t pmux_p3 = PINMUX_UNUSED;

	enum usart_signal_mux_settings pm_settings = USART_RX_0_TX_2_XCK_3;

	enum usart_parity parity = USART_PARITY_EVEN;
} zpdc_sercom0_configuration;

/************************************************************************/
/* SERCOM0 Class														*/
/************************************************************************/
class zpdc_sercom {
public:
	status_code sercom_init(SerialEthernetConfiguration_SERCOM0 hw_instance);

	//virtual usart_module *getModule() =0;
	virtual void buffer_received_callback( TaskHandle_t task_handler ) =0;		// struct usart_module *const module
	virtual void buffer_transmitted_callback( void ) =0;	// struct usart_module *const module

	status_code_genare_t getStatus() { return status; }
	
	usart_module *getModule() { return &module; }
	void setModule(void *mod) { module.object_instance_pointer = mod; }
private:
	usart_module module;
	status_code status;
};

enum resource_register_code {
	REGISTER_OK = 0,
	REGISTER_ERROR_DUPLICATE,
	REGISTER_FULL_RESOURCES,
	REGISTER_FULL_SERVICES,
	REGISTER_ERROR_OTHER,
};

class sercom_resource_manager {	// TODO: Here, we store the resource ID counter. This gets moved later
public:
	static const constexpr uint8_t max_resources = 5;	// 5 SERCOMs in C21 HW
	static const constexpr uint8_t max_services = 16;	// 16 Services for the time being
	static uint8_t service_counter;
	static uint8_t hw_resource[max_resources];

	resource_register_code register_service() { 
		if (service_counter >= max_services)
			return REGISTER_FULL_SERVICES;
		
		service_counter++;

		for (int i=0; i<max_resources; i++ ) {
			if( hw_resource[i] == 0 ) {
				hw_resource[i] = service_counter;
				setServiceId(service_counter);
				return REGISTER_OK;
			}
			else if ( hw_resource[i] == service_counter ) 
				return REGISTER_ERROR_DUPLICATE;
		}

		return REGISTER_ERROR_OTHER;
	}

	virtual void setServiceId(uint8_t id) =0;
	virtual uint8_t getServiceId(void) =0;
};

#endif // __cplusplus
#endif /* ZPDC_SERCOM_H_ */