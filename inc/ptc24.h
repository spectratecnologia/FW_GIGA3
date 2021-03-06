/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PTC24_H
#define __PTC24_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "functions.h"
#include "can.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define CMD_IGN2 GPIO_Pin_13
#define CMD_IGN2_PORT GPIOC

#define CMDFL_TACO GPIO_Pin_0
#define CMDFL_TACO_PORT GPIOA

#define CMDFL_ODO GPIO_Pin_1
#define CMDFL_ODO_PORT GPIOA

/* Exported declared Types -------------------------------------------------- */

/* Exported functions ------------------------------------------------------- */
void deInitPTC24config(void);
void initPTC24config(void);
void activePTC24Ign(FunctionalState);
void ptc24_enableToogleTaco(bool);
void ptc24_toggleTaco(void);
void ptc24_enableToogleOdo(bool);
void ptc24_toggleOdo(void);
void emulateMpx(void);

#endif /* __MPX_H */
