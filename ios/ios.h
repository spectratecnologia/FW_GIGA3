/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IOS_H
#define __IOS_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define IO_STATUS_LED_CPU_PORT GPIOD
#define IO_STATUS_LED_CPU_PIN GPIO_Pin_15

#define IO_BEEP_PORT GPIOE
#define IO_BEEP_PIN GPIO_Pin_14

#define IO_IGNITION_CMD_PORT GPIOC
#define IO_IGNITION_CMD1_PIN GPIO_Pin_12
#define IO_IGNITION_CMD2_PIN GPIO_Pin_13

#define IO_TACT_PORT GPIOD
#define IO_TACT_UP_PIN GPIO_Pin_11
#define IO_TACT_DOWN_PIN GPIO_Pin_12
#define IO_TACT_CANCEL_PIN GPIO_Pin_13
#define IO_TACT_ENTER_PIN GPIO_Pin_14

#define IO_ADC_PORT GPIOA
#define IO_ADC_VBB_PIN GPIO_Pin_0
#define IO_ADC_TEMP_PIN GPIO_Pin_1

enum {
	IO_TACT_UP = 0,
	IO_TACT_DOWN = 1,
	IO_TACT_ENTER = 2,
	IO_TACT_CANCEL = 3
};

/* Exported functions ------------------------------------------------------- */
void initIOs();

void setStatusLed();
void resetStatusLed();
void readCurrentTactsStatus ();

bool getCPULEDstatus();
void toggleCPULED();

float getTemperatureVoltage();
float getVbbVoltage();
float getVvext1Voltage();
float getVvext2Voltage();

bool isTactInputPressed(uint8_t tactNumber);

void setBeepOutput(bool beepOn);

/*
 * Function: setIgnitionOutput
 * Ignition Number must be 1 or 2.
 *
 */
void setIgnitionOutput (uint8_t ignNumber, bool ignOn);

/* Test Function */
void toggleBeep ();
void printIgnitionStatus();


#endif /* __IOS_H */
