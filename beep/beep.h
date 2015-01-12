#ifndef __BEEP_HH_
#define __BEEP_HH_

#include "stm32f10x.h"

typedef struct {
	uint8_t numOfBeeps;
	uint32_t periodInMs;
	uint32_t startTime;
}Beep;

void initBeepIO ();
void setBeep (uint8_t num, uint32_t periodInMs);
void cancelBeep ();
inline void processBeeps();

#endif
