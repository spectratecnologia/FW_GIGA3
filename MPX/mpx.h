/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MPX_H
#define __MPX_H

/* Includes ------------------------------------------------------------------*/
	#include "stm32f10x.h"
	#include "stm32f10x_gpio.h"
	#include "stm32f10x_rcc.h"
	#include "functions/functions.h"
	#include "spi.h"
	#include "can/can.h"

/* Exported types ------------------------------------------------------------*/
	typedef enum {PORT_LOW = 0, PORT_HIGH = 1, PORT_OFF = 2} FunctionalState_PP10A;

	/* This parameter is treated at onCAN1ReceiveInterrupt() function in can.c */
	typedef union {
		uint32_t extId;
		struct{
			uint8_t DA;
			uint8_t SA;
			uint8_t index;
			uint8_t command;
		};
	} ackWriteReceivedtype;

	ackWriteReceivedtype ackWriteReceived;
	Mpx mpx;

/* Exported constants --------------------------------------------------------*/
	#define NUM_MPXPORTS 36

/* Exported macro ------------------------------------------------------------*/

	/* Relative to the mpx_activeports.c file---------------------------------*/
	#define SROUTRCLKPIN GPIO_Pin_12
	#define SROUTRCLKPORT GPIOB

	#define CMDFH1 GPIO_Pin_0
	#define CMDFH2 GPIO_Pin_1
	#define CMDFH3 GPIO_Pin_2
	#define CMDFH4 GPIO_Pin_3

	#define CMDFL1 GPIO_Pin_4
	#define CMDFL2 GPIO_Pin_5
	#define CMDFL3 GPIO_Pin_6
	#define CMDFL4 GPIO_Pin_7
	#define CMDFHLPORT GPIOC


/* Exported functions ------------------------------------------------------- */

	/* Relative to the mpx_activeports.c file---------------------------------*/
	void initMPXconfig();
	void initSHWRHSPI2();
	void initPP10Apins();

	void activeMPXports(uint8_t, FunctionalState_PP10A);
	inline void activePP10Aports(uint8_t, FunctionalState_PP10A);
	inline void activeSPI2MPXports(uint8_t);
	inline void sendChangedOutputsToMPXs();
	inline void activeMPXdeviceports(uint8_t, FunctionalState_PP10A);
	void deactiveallMPXports();

	void senddataSRout(uint32_t);

#endif /* __MPX_H */
