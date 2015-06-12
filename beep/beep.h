/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BEEP_HH_
#define __BEEP_HH_

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "multitask/multitask.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
	uint8_t numOfBeeps;
	uint32_t periodInMs;
	uint32_t startTime;
}Beep;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define IO_BEEP_PORT GPIOE
#define IO_BEEP_PIN GPIO_Pin_14

/* Exported functions ------------------------------------------------------- */
void initBeepIO ();
void initBeep();
void setBeep (uint8_t num, uint32_t periodInMs);
void cancelBeep ();
void processBeeps();
void setBeepOutput(bool);
void errorBeep(void);
void notErrorBeep(void);


#endif
