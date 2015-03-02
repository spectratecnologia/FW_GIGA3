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

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* Relative to the active MPX ID ports code */
typedef enum
{
	ID0,
	ID1,
	ID2,
	ID4
} MPX_ID;

/* Relative to the active ports code */
typedef enum {PORT_LOW = 0, PORT_HIGH = 1, PORT_OFF = 2} FunctionalState_MPXports;

/* Exported macro ------------------------------------------------------------*/

/* Relative to the active MPX ID ports code */

#define CMDFLID0 GPIO_Pin_9
#define CMDFLID1 GPIO_Pin_10
#define CMDFLID2 GPIO_Pin_11
#define CMDFLID_PORT GPIOC

/* Relative to the active ports code */

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

/* Exported declared Types -------------------------------------------------- */
Mpx mpx;

/* Exported functions ------------------------------------------------------- */
void initMPXconfig();

void setMPXIDports(MPX_ID);

#endif /* __MPX_H */
