/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MULTITASK_H
#define __MULTITASK_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
volatile uint64_t sysTickTimer;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define MAX_VIRTUAL_TIMERS 20

/* Exported functions ------------------------------------------------------- */
void initMultiTask();

void executeEveryInterval(uint8_t timerIndex, uint32_t intervalInMs, void (*function)(void));
void enableFunction(uint8_t timerIndex, uint8_t enable);

uint32_t getMaxExecutionTime(uint8_t timerIndex);
void delayMsUsingSysTick(uint32_t delayMs);
bool achievedTimeout(uint32_t startTimeInMs, uint32_t timeoutInMs);


/* SysTick interrupt handles */
void SysTick_Handler(void);


#endif /* __MULTITASK_H */
