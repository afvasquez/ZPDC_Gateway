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
/*  TYPE DEFINITIONS FOR NETWORK DEVICE TABLE                           */
/************************************************************************/
#define CAN_DISCOVERY_REQUEST		(uint8_t)('A')
#define CAN_ORDER_UPDATE_REQUEST	(uint8_t)('B')
#define CAN_REQUEST_LED_TOG			(uint8_t)('C')
#define CAN_MOTOR_GET_PARAMA		(uint8_t)('D')
#define CAN_MOTOR_START				(uint8_t)('E')
#define CAN_MOTOR_STOP				(uint8_t)('F')
#define CAN_MOTOR_PID_TUNE			(uint8_t)('G')
#define CAN_MOTOR_PARAM_A			(uint8_t)('H')
#define CAN_MOTOR_PARAM_B			(uint8_t)('I')
#define CAN_MOTOR_STATREPA			(uint8_t)('J')
// Return Values
#define CAN_DISCOVERY_RETURN		(uint8_t)('a')
#define CAN_ORDER_UPDATE_RETURN		(uint8_t)('b')
#define CAN_REQUEST_LED_TOG_RETURN	(uint8_t)('c')
#define CAN_MOTOR_GET_PARAMA_RETURN (uint8_t)('d')
#define CAN_MOTOR_START_RETURN		(uint8_t)('e')
#define CAN_MOTOR_STOP_RETURN		(uint8_t)('f')
#define CAN_MOTOR_PID_TUNE_RETURN	(uint8_t)('g')
#define CAN_MOTOR_PARAM_A_RETURN	(uint8_t)('h')
#define CAN_MOTOR_PARAM_B_RETURN	(uint8_t)('i')
#define CAN_MOTOR_STATREPA_RETURN	(uint8_t)('j')
	/******** QUEUE COMPRESSION CONSTANTS *******************************/
#define CAN_QUEUE_COMMAND_DISCOVERY	((uint8_t) 1)
#define CAN_QUEUE_COMMAND_ORDER		((uint8_t) 2)
#define CAN_QUEUE_COMMAND_LED_TRIG	((uint8_t) 3)
#define CAN_QUEUE_COMMAND_GET_PARSA	((uint8_t) 4)
#define CAN_QUEUE_COMMAND_MOT_START	((uint8_t) 5)
#define CAN_QUEUE_COMMAND_MOT_STOP	((uint8_t) 6)
#define CAN_QUEUE_COMMAND_PID_PARS	((uint8_t) 7)
#define CAN_QUEUE_COMMAND_MOT_PARSA	((uint8_t) 8)
#define CAN_QUEUE_COMMAND_MOT_PARSB	((uint8_t) 9)

	/********************************************************************/
#define CAN_SUBNET_NETWORK_REQUEST	((uint8_t) 0)
#define CAN_SUBNET_PARAMETER_SETUP	((uint8_t) 2)
#define CAN_SUBNET_ZPDC_OPERATION	((uint8_t) 3)

#define CAN_DEVICE_GATEWAY			((uint8_t) 0)
#define CAN_DEVICE_HYBRID			((uint8_t) 2)
#define CAN_DEVICE_DRIVE_CARD		((uint8_t) 3)

#define CAN_BUFFER_0				((uint8_t) 0)
#define CAN_BUFFER_1				((uint8_t) 1)

#define CAN_RX_FIFO_ID_SUBNET(value)	(uint8_t)((0x03ul & ((value) >> (CAN_TX_ELEMENT_T0_STANDARD_ID_Pos + 7))))
#define CAN_RX_FIFO_ID_DEVICE(value)	(uint8_t)((0x03ul & ((value) >> (CAN_TX_ELEMENT_T0_STANDARD_ID_Pos + 9))))

/************************************************************************/
/*  TYPE DEFINITION FOR PID TUNNING STRUCTURE                           */
/************************************************************************/
typedef struct {
	uint8_t can_command;
	uint16_t arg_1, arg_2, arg_3, arg_4;
	uint32_t point_self;
} CanQueueRequest;

/************************************************************************/
/*  TYPE DEFINITIONS FOR NETWORK DEVICE TABLE                           */
/************************************************************************/
typedef struct {
	uint16_t uid;
	uint8_t net_address;
} NetworkDevices;

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
static can_module can0_instance;

/************************************************************************/
/* CAN MODULE OBJECT ABSTRACTION                                        */
/************************************************************************/

class can_service : public Task {
public:
	const constexpr static char* NAME = "CAN_BUS\0";

	can_service(CanConfiguration_CAN0 config, ser_ethernet *eth_interface, ZpdcSystem *system_module);

	void send(uint8_t length, uint8_t device, uint8_t sub_net, uint8_t buffer);

	void task(void);

	void callback(void);
private:
	NetworkDevices network_devices[100];
	uint8_t network_size;
	QueueHandle_t queue_net_devices;
	
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

	// Data Interfaces
	ser_ethernet *eth0;
	// System Data
	ZpdcSystem *system_data;

	
	inline void CancelTransmission(uint32_t buffer) { can_tx_cancel_request(&can0_instance, (uint32_t)(1 << buffer)); }
	void PrintCanDeviceOrder(uint8_t data) {
		eth0->print(" -> [");
		eth0->print((uint16_t)(data));
		eth0->print("]\t");
	}
	void PrintCanDeviceAddress(uint32_t data) {
		eth0->print(" -> [");
		eth0->print((uint16_t)((data >> 16) & 0xFF));
		eth0->print(",");
		eth0->print((uint16_t)((data >> 8) & 0xFF));
		eth0->print("]\t");
	}
	void PrintCanDeviceData(uint32_t data) {
		PrintCanDeviceAddress(data);
		if ((data & 0xFF) == 0xFF) eth0->print("NO_ADD");
		else eth0->print((uint16_t)(data & 0xFF));
		switch ((data >> 24) & 0xFF) {
			case CAN_DEVICE_GATEWAY:
			eth0->printnl("\tGATEWAY");
			break;
			case CAN_DEVICE_HYBRID:
			eth0->printnl("\tHYBRID_LEADER");
			break;
			case CAN_DEVICE_DRIVE_CARD:
			eth0->printnl("\tDRIVE_CARD");
			break;
			default:
			eth0->printnl("\tUNKNOWN");
			break;
		}
	}
	void PrintCanDeviceCommandSetAlphaResponse(uint32_t data) {
		PrintCanDeviceAddress(data);
		if (tx_message_0[0] == CAN_REQUEST_LED_TOG) eth0->print("LED ");
		else eth0->print("MOTOR\t");
		if (rx_element_fifo_0.data[0] == CAN_MOTOR_GET_PARAMA_RETURN) {
			uint16_t parm_value = (uint16_t)(rx_element_fifo_0.data[3] << 8);
			parm_value |= (uint16_t)(rx_element_fifo_0.data[4]);
			eth0->print(parm_value);
			eth0->print("\t");
			parm_value = (uint16_t)(rx_element_fifo_0.data[5] << 8);
			parm_value |= (uint16_t)(rx_element_fifo_0.data[6]);
			eth0->print(parm_value);
			eth0->print("\t");
			parm_value = (uint16_t)(rx_element_fifo_0.data[7]);
			if (parm_value)
				eth0->print("CCW");
			else
				eth0->print("CW");
			eth0->printnl(" ");
		} else {
			if (system_data->get_queue_entry_parameter(data, 1)) eth0->printnl("ON");
			else eth0->printnl("OFF");
		}
	}
};



#endif
#endif /* ZPDC_CAN_H_ */
