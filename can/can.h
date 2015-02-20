/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H
#define __CAN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "functions/functions.h"
#include "stm32f10x_can.h"
#include "MPX/mpx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define MY_ID 0x03

typedef enum {CAN_COMMAND_BROADCAST=0x19, CAN_COMMAND_WRITE=0x1B,
					   CAN_COMMAND_WRITE_ACK=0x1C, CAN_COMMAND_READ=0x1E,
					   CAN_COMMAND_READ_ACK=0x1F}  CanCommandType;



#define CAN_BROADCAST_COMMAND_MASK 0x10

typedef enum {CAN_BROADCAST_MPX_INFO=0x01, CAN_BROADCAST_MPX_FLASH_INFO=0x02, CAN_BROADCAST_DIGITAL_INPUT_MASK=0x10, CAN_BROADCAST_ANALOG_INPUT_MASK=0x30,
		  	  CAN_BROADCAST_FLAGS_MASK=0x50, CAN_BROADCAST_KEY_STATE_MASK=0x70, CAN_BROADCAST_RTC=0x90}  BroadcastType;

typedef union {
	uint32_t extId;
	struct{
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
inline void onCAN1ReceiveInterrupt();
inline void onCAN2ReceiveInterrupt();
inline void onCAN1StatusChangeError();
inline void onCAN2StatusChangeError();
uint32_t receivedCanPacketsCount();
uint32_t transmitErrorsCount();

void sendCanPacket(CAN_TypeDef*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t*, uint8_t);
void CAN_writePort(CAN_TypeDef*, uint8_t, uint8_t, PortParameter*);

#endif /* __CAN_H */
