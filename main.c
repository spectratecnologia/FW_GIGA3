#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"
#include "rtc/rtc.h"

int main(void)
{

	initUSART();

	initRTC();

	initMultiTask();

	initIOs();

	initTactButtons();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    }
}
