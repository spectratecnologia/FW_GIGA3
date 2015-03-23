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
#include "devices/MPX/mpx.h"
#include "devices/PTC24/ptc24.h"
#include "tests/mpx_statemachine.h"
#include "tests/ptc24_statemachine.h"

void teste()
{
	sendCanPacket(CAN2, 0x00, 0x00, 0xAB, 0x20, 0, 0);
}

int main(void)
{
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

	//teste();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    	executeEveryInterval(1, 10, &processBeeps);

    	executeEveryInterval(2, 50, &processKeysAndDeadTime);

    	executeEveryInterval(3, 100, &LCD_vStateMachineLoop);

    	if (mpx.MpxAlreadyInit)
    	{
    		executeEveryInterval(4, 10, &mpxTest_vStateMachineLoop);

    		executeEveryInterval(5, 80, &turningOffMpxEmergencyMode);

    		executeEveryInterval(6, 5, &sendChangedOutputsToMPXs);
    	}

    	if (ptc24.PtcAlreadyInit)
    	{
    		executeEveryInterval(7, 40, &emulateMpx);

    		executeEveryInterval(8, 10, &ptc24Test_vStateMachineLoop);

    		executeEveryInterval(9, 10, &toggleOdo);

    		executeEveryInterval(10, 8, &toggleTaco);

    		//executeEveryInterval(11, 10, &teste);
    	}
    }
}
