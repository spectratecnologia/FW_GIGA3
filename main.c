#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"
#include "rtc/rtc.h"
#include "beep/beep.h"
#include "virtual_keyboard/virtual_keyboard.h"

int main(void)
{

	initUSART();

	initRTC();

	initMultiTask();

	initIOs();

	initVitualKeyboard();

	initBeepIO()();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    	executeEveryInterval(1, 1, &processKeysAndDeadTime);

    	executeEveryInterval(2, 1, &processBeeps);

    }
}
