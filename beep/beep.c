#include "beep.h"
#include "ios/ios.h"
#include "multitask/multitask.h"

static Beep beepConfig;

void initBeepIO () {
#if DEBUG != 0
	printf("[BEEP]Beep Setup\n");
#endif

	initBeep();

	beepConfig.numOfBeeps = 0;
	beepConfig.periodInMs = 0;
	beepConfig.startTime = 0;

#if DEBUG != 0
	printf("[BEEP]Beep Setup done\n");
#endif
}

void setBeep (uint8_t numOfBeeps, uint32_t periodInMs) {
	beepConfig.numOfBeeps = numOfBeeps;
	beepConfig.periodInMs = periodInMs;
	beepConfig.startTime = sysTickTimer;
}

void cancelBeep () {
	beepConfig.numOfBeeps = 0;
}

inline void processBeeps() {
	if(beepConfig.numOfBeeps > 0) {

		uint16_t currentPeriodNumber = (sysTickTimer - beepConfig.startTime)/beepConfig.periodInMs;

		if (currentPeriodNumber<beepConfig.numOfBeeps*2){
			/* +1 to start with beep */
			setBeepOutput((currentPeriodNumber+1)%2);
		}
		/* finished blinking */
		else{
			beepConfig.numOfBeeps = 0;
			setBeepOutput(0);
		}
	}
}

