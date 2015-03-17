/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MPX_H
#define __MPX_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "functions/functions.h"
#include "can/can.h"
#include "giga3_sr.h"

/* Exported constants --------------------------------------------------------*/
#define MAX_TIME_TO_PORT_BE_ACTIVE 1000

/* Exported types ------------------------------------------------------------*/

/* Relative to the active MPX ID ports code */
typedef enum
{
	ID1,
	ID2,
	ID4,
	ID0
} MPX_ID;

/* Relative to the active ports code */
typedef enum {PORT_LOW = 0, PORT_HIGH = 1, PORT_OFF = 2} FunctionalState_MPXports;

/* Exported macro ------------------------------------------------------------*/
/* Ignition Pin */
#define CMD_IGN1 GPIO_Pin_12
#define CMD_IGN1_PORT GPIOC

/* ID ports code */
#define CMDFLID0 GPIO_Pin_9
#define CMDFLID1 GPIO_Pin_10
#define CMDFLID2 GPIO_Pin_11
#define CMDFLID_PORT GPIOC

/* Push Pull 10A ports code */
#define CMDFH1 GPIO_Pin_0
#define CMDFH2 GPIO_Pin_1
#define CMDFH3 GPIO_Pin_2
#define CMDFH4 GPIO_Pin_3

#define CMDFL1 GPIO_Pin_4
#define CMDFL2 GPIO_Pin_5
#define CMDFL3 GPIO_Pin_6
#define CMDFL4 GPIO_Pin_7
#define CMDFHL_PORT GPIOC

/* SPI2 input/output (shift register control) */
#define SR_READ GPIO_Pin_11
#define SR_WRITE GPIO_Pin_12
#define SR_WR_PORT GPIOB

#define SPI2_SCK GPIO_Pin_13
#define SPI2_MISO GPIO_Pin_14
#define SPI2_MOSI GPIO_Pin_15
#define SPI2_PORT GPIOB

/* Exported declared Types -------------------------------------------------- */
Mpx mpx;

/* Exported functions ------------------------------------------------------- */
void initMPXconfig();
void setMPXIDports(MPX_ID);
void activeMPXports(uint8_t, FunctionalState_MPXports);
uint8_t getPortStatus(uint8_t);
void activeMPXIgnition(FunctionalState_MPXports);
void turningOffMpxEmergencyMode();
void turnOffMpxPorts(void);
void sendChangedOutputsToMPXs();

#endif /* __MPX_H */
