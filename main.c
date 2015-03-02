#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"
#include "can/can.h"
#include "rtc/rtc.h"
#include "LCD/lcd_screen.h"
#include "beep/beep.h"
#include "virtual_keyboard/virtual_keyboard.h"
#include "usb/user/usbd_cdc_vcp.h"
#include "MPX/mpx.h"

void processLCD();
void processTest();
void processAnalysisTest();

void teste ()
{

	setMPXIDports(ID1);

}

int main(void)
{
	/* Functions already working - Pedro */
	initUSART();
	initUSBVCP();
	initRTC();
	initMultiTask();
	initBeepIO();
	initLCD();
	initIOs();
	LCD_vStateMachineInit();
	initVitualKeyboard();

	/* --------------------------------- */

	test_vStateMachineInit();

	initCANs();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    	executeEveryInterval(1, 10, &processBeeps);

    	executeEveryInterval(2, 50, &processKeysAndDeadTime);

    	executeEveryInterval(3, 100, &processLCD);

    	executeEveryInterval(4, 10, &processTest);

    	executeEveryInterval(5, 80, &processAnalysisTest);

    }
}

void processLCD()
{
	LCD_vStateMachineLoop();
}

void processTest()
{
	test_vStateMachineLoop();

	if (mpx.MpxAlreadyInit == true)
		executeTest_MPX();
}

void processAnalysisTest()
{
	if (mpx.MpxAlreadyInit == true)
		analysisTest_MPX();
}
