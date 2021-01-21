/*
******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 9.3.0   2021-01-18

The MIT License (MIT)
Copyright (c) 2019 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/

/* Includes */
#include <stddef.h>
#include <stdbool.h>
#include "stm32f10x.h"
#include "usart.h"
#include "multitask.h"
#include "giga3_cpu_led.h"
#include "can.h"
#include "rtc.h"
#include "lcd_screen.h"
#include "beep.h"
#include "virtual_keyboard.h"
//#include "usb/user/usbd_cdc_vcp.h"
#include "mpx.h"
#include "ptc24.h"
#include "ptc16.h"
#include "mpx_statemachine.h"
#include "ptc24_statemachine.h"
#include "ptc16_statemachine.h"
#include "HD44780.h"

extern Mpx mpx;
extern Ptc ptc16;
extern Ptc ptc24;


/* Private typedef */

/* Private define  */

/* Private macro */

/* Private variables */

/* Private function prototypes */

/* Private functions */
void forceShutDownMpxPorts() {
	turnOffMpxPorts();
}
/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
	initUSART();
//	initUSBVCP();
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




	while (1)
	  {
		executeEveryInterval(0, 1000, &toggleCPULED);

		executeEveryInterval(1, 10, &processBeeps);

		executeEveryInterval(2, 50, &processKeysAndDeadTime);

		executeEveryInterval(3, 100, &LCD_vStateMachineLoop);

		if (!mpx.MpxAlreadyInit) {
			executeEveryInterval(15, 50, &forceShutDownMpxPorts);
		}

		if (mpx.MpxAlreadyInit) {
			executeEveryInterval(4, 10, &mpxTest_vStateMachineLoop);

			executeEveryInterval(5, 80, &turningOffMpxEmergencyMode);

			executeEveryInterval(6, 5, &sendChangedOutputsToMPXs);

//			executeEveryInterval(7, 1, &toogleMPXNTC);	// pin will be held LO for MPX V2.x compatibility

			//executeEveryInterval(15, 50, &verifyMpxTimout);
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

		executeEveryInterval(16, 10, &bingUpCANs);
	  }
}

/*
 * Minimal __assert_func used by the assert() macro
 * */
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  while(1)
  {}
}

/*
 * Minimal __assert() uses __assert__func()
 * */
void __assert(const char *file, int line, const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
}


