/*
 * zpdc_can.h
 *
 * Created: 5/18/2017 1:32:40 PM
 *  Author: Andres Vasquez
 */ 


#ifndef ZPDC_CAN_H_
#define ZPDC_CAN_H_
#ifdef __cplusplus
/************************************************************************/
/*  HARDWARE MODULE CONSTANT STRUCTURES (ONLY 1 CAN MODULE IN C21)      */
/************************************************************************/
static const struct CanConfiguration_CAN0 {
	const static constexpr char *NAME = "CAN0\n";

	Can *hardware = CAN0;

	/* CAN Module Setup Parameters */
	enum can_nonmatching_frames_action nonmatching_action = CAN_NONMATCHING_FRAMES_FIFO_0;
	const enum system_interrupt_vector interrupt_vector = SYSTEM_INTERRUPT_MODULE_CAN0;
	const uint8_t tx_pin = PIN_PA24G_CAN0_TX;
	const uint8_t tx_mux = MUX_PA24G_CAN0_TX;
	const uint8_t rx_pin = PIN_PA25G_CAN0_RX;
	const uint8_t rx_mux = MUX_PA25G_CAN0_RX;

} zpdc_can0_configuration;

/************************************************************************/
/* CAN MODULE OBJECT ABSTRACTION                                        */
/************************************************************************/
class can_service;

class can_service : public Task {
public:
	const constexpr static char* NAME = "CAN_BUS\0";

	can_service(CanConfiguration_CAN0 config, can_service *obj);

	void task(void);

	void callback(void);
private:
	struct can_module can_instance;
	struct can_rx_element_fifo_0 rx_element_fifo_0;
	struct can_rx_element_fifo_0 rx_element_fifo_1;
	struct can_rx_element_fifo_0 rx_element_buffer;

	// Data Variables
		// TX
	uint8_t tx_message_0[CONF_CAN_ELEMENT_DATA_SIZE];
	uint8_t tx_message_1[CONF_CAN_ELEMENT_DATA_SIZE];
		// RX
	volatile uint8_t standard_receive_index;
	volatile uint8_t extended_receive_index;
};

#endif
#endif /* ZPDC_CAN_H_ */
