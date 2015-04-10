/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PTC16_H
#define __PTC16_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "functions/functions.h"
#include "can/can.h"
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define PTC16_CMD_IGN2 GPIO_Pin_13
#define PTC16_CMD_IGN2_PORT GPIOC

#define PTC16_CMDFL_TACO GPIO_Pin_0
#define PTC16_CMDFL_TACO_PORT GPIOA

#define PTC16_CMDFL_ODO GPIO_Pin_1
#define PTC16_CMDFL_ODO_PORT GPIOA
/* Exported declared Types -------------------------------------------------- */
Ptc ptc16;

/* Exported functions ------------------------------------------------------- */
void deInitPTC16config(void);
void initPTC16config(void);
void activePTC16Ign(FunctionalState);
void ptc16_enableToogleTaco(bool);
void ptc16_toggleTaco(void);
void ptc16_enableToogleOdo(bool);
void ptc16_toggleOdo(void);

#endif /* __MPX_H */
