#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"
#include "rtc/rtc.h"
#include "beep/beep.h"
#include "virtual_keyboard/virtual_keyboard.h"
#include "usb/user/usbd_cdc_vcp.h"

void teste () {

	setBeep(3, 1000);

}

int main(void)
{
	initUSART();

	initUSBVCP();

	initRTC();

	initMultiTask();

	initIOs();

	initLCD();

	initVitualKeyboard();

	initBeepIO();

	writeTime(2015, 2, 5, 11, 20, 0);

	teste();

    while(1)
    {

    	executeEveryInterval(0, 1000, &toggleCPULED);

    	executeEveryInterval(1, 1, &processKeysAndDeadTime);

    	executeEveryInterval(2, 1, &processBeeps);


    	//executeEveryInterval(3, 1000, &teste);
    }
}
