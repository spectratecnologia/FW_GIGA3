#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"

int main(void)
{


	initUSART();

	initMultiTask();

    while(1)
    {
    }
}
