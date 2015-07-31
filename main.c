#include "stm32f10x.h"
#include "usart/usart.h"
#include "multitask/multitask.h"
#include "giga3_cpu_led.h"
#include "can/can.h"
#include "rtc/rtc.h"
#include "LCD/lcd_screen.h"
#include "beep/beep.h"
#include "virtual_keyboard/virtual_keyboard.h"
#include "usb/user/usbd_cdc_vcp.h"
#include "mpx.h"
#include "ptc24.h"
#include "ptc16.h"
#include "mpx_statemachine.h"
#include "ptc24_statemachine.h"
#include "ptc16_statemachine.h"

void verifyMpxTimout() {
	if(getMpxTimeSinceLastMessage() > MPX_TIMOUT) {
		printf("Emergency Mode call \n");
		//callEmergencyMode();
	}
}

int main(void) {
	/* Functions already working - Pedro */
	initUSART();
	initUSBVCP();
	initRTC();
	initCANs();
	initMultiTask();
	initCPULed();
	initBeepIO();
	initLCD();
	initVitualKeyboard();
	LCD_vStateMachineInit();
	mpxTest_vStateMachineInit();
	ptc24Test_vStateMachineInit();
	ptc16Test_vStateMachineInit();

	while (1) {
		executeEveryInterval(0, 1000, &toggleCPULED);

		executeEveryInterval(1, 10, &processBeeps);

		executeEveryInterval(2, 50, &processKeysAndDeadTime);

		executeEveryInterval(3, 100, &LCD_vStateMachineLoop);

		if (mpx.MpxAlreadyInit) {
			executeEveryInterval(4, 10, &mpxTest_vStateMachineLoop);

			executeEveryInterval(5, 80, &turningOffMpxEmergencyMode);

			executeEveryInterval(6, 5, &sendChangedOutputsToMPXs);

			executeEveryInterval(7, 1, &toogleMPXNTC);

			//executeEveryInterval(15, 5, &verifyMpxTimout);
		}

		else if (ptc24.PtcAlreadyInit) {
			executeEveryInterval(8, 40, &emulateMpx);

			executeEveryInterval(9, 10, &ptc24Test_vStateMachineLoop);

			executeEveryInterval(10, 10, &ptc24_toggleOdo);

			executeEveryInterval(11, 8, &ptc24_toggleTaco);
		}

		else if (ptc16.PtcAlreadyInit) {
			executeEveryInterval(8, 40, &emulateMpx);

			executeEveryInterval(12, 10, &ptc16Test_vStateMachineLoop);

			executeEveryInterval(13, 10, &ptc16_toggleOdo);

			executeEveryInterval(14, 8, &ptc16_toggleTaco);

			//executeEveryInterval(15, 10000, &teste);
		}

	}
}
