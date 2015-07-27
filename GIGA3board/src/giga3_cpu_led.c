/* Includes ------------------------------------------------------------------*/
#include "giga3_cpu_led.h"

/* Functions -----------------------------------------------------------------*/
void initCPULed()
{
	GPIO_InitTypeDef 		gpio_InitTypeDef;

	/* Enable GPIOs clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	/* Configure LEDs PD0 (CPU), PD1(LD1) and PD2(LD2) in output pushpull mode */
	gpio_InitTypeDef.GPIO_Pin = IO_STATUS_LED_CPU_PIN;
	gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(IO_STATUS_LED_CPU_PORT, &gpio_InitTypeDef);
}

void setStatusLed()
{
	GPIO_WriteBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN,Bit_SET);
}

void resetStatusLed()
{
	GPIO_WriteBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN,Bit_RESET);
}

bool getCPULEDstatus()
{
	return GPIO_ReadInputDataBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN);
}

void toggleStatusLed()
{
	GPIO_WriteBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN,!GPIO_ReadInputDataBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN));
}

void toggleCPULED()
{
	toggleStatusLed();

}

