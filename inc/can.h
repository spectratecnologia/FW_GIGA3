/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H
#define __CAN_H

/* Includes ------------------------------------------------------------------*/
#include "functions.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define MY_ID 0x03
#define PTC24_DEVICE_ID 0x20
#define PTC16_DEVICE_ID 0x21
#define MPX_DEVICE_MASK 0x80
#define MXP_DEVICE_ID0 0x80
#define MXP_DEVICE_ID1 0x81
#define MXP_DEVICE_ID2 0x82
#define MPX_TIMOUT 200

typedef enum
{
	CAN_COMMAND_BROADCAST=0x19,
	CAN_COMMAND_WRITE=0x1B,
	CAN_COMMAND_WRITE_ACK=0x1C,
	CAN_COMMAND_READ=0x1E,
	CAN_COMMAND_READ_ACK=0x1F
} CanCommandType;

#define CAN_BROADCAST_COMMAND_MASK 0x10

typedef enum
{
	CAN_BROADCAST_MPX_INFO=0x01,
	CAN_BROADCAST_MPX_FLASH_INFO=0x02,
	CAN_BROADCAST_DIGITAL_INPUT_MASK=0x10,
	CAN_BROADCAST_ANALOG_INPUT_MASK=0x30,
	CAN_BROADCAST_FLAGS_MASK=0x50,
	CAN_BROADCAST_KEY_STATE_MASK=0x70,
	CAN_BROADCAST_RTC=0x90
} BroadcastType;

/* Read/Write definitions */
#define CAN_INDEX_EME_IGN_START 64
#define CAN_INDEX_EME_N_IGN_START 128

typedef union
{
	uint32_t extId;
	struct
	{
		uint8_t DA;
		uint8_t SA;
		uint8_t index;
		uint8_t command;
	};
} MsgId;

/* Broadcast definitions  */
#define BROADCAST_DEST_ADDR 0x00


/* Exported functions ------------------------------------------------------- */
void initCANs();

void CAN1_RX0_IRQHandler();
void CAN2_RX0_IRQHandler();
void CAN1_SCE_IRQHandler();
void CAN2_SCE_IRQHandler();

void onCAN1ReceiveInterrupt();
void onCAN1ReceiveInterrupt_MPX(CanRxMsg, MsgId);
void onCAN1ReceiveInterrupt_PTC24(CanRxMsg, MsgId);
void onCAN1ReceiveInterrupt_PTC16(CanRxMsg, MsgId);
uint16_t getMpxAnalogMemoryAddress(uint8_t, uint8_t);

void onCAN2ReceiveInterrupt();

void onCAN1StatusChangeError();

void onCAN2StatusChangeError();

uint32_t receivedCanPacketsCount();
uint32_t transmitErrorsCount();

void sendCanPacket(CAN_TypeDef*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t*, uint8_t);
void CAN_writePort(CAN_TypeDef*, uint8_t, uint8_t, PortParameter*);
void sendCanRTC(void);
uint32_t getMpxTimeSinceLastMessage();
void bingUpCANs();

#endif /* __CAN_H */
