#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"

int main(void)
{

	initUSART();

	initMultiTask();

	initIOs();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    }
}
