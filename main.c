#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"
#include "rtc/rtc.h"
#include "beep/beep.h"
#include "virtual_keyboard/virtual_keyboard.h"
#include "usb/user/usbd_cdc_vcp.h"

int main(void)
{
	initUSART();

	//initUSBVCP();

	//initRTC();

	initMultiTask();

	initIOs();

	initLCD();

	//initVitualKeyboard();

	//initBeepIO();

	//LCD_printLine(0, "Xupa A");


    while(1)
    {
    	//printf("Running\n");

    	executeEveryInterval(0, 1000, &toggleCPULED);

    	//executeEveryInterval(1, 1, &processKeysAndDeadTime);

    	//executeEveryInterval(2, 1, &processBeeps);
    }
}
