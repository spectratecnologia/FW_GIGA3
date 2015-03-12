/* Includes ------------------------------------------------------------------*/
#include "beep.h"

/* Global Variables ----------------------------------------------------------*/
static Beep beepConfig;

/* Exported functions --------------------------------------------------------*/
void initBeepIO () {
#if DEBUG != 0
	printf("[BEEP]Beep Setup\n");
#endif

	initBeep();

	beepConfig.numOfBeeps = 0;
	beepConfig.periodInMs = 0;
	beepConfig.startTime = 0;

#if DEBUG != 0
	printf("[BEEP]Beep Setup done\n");
#endif
}

void initBeep(){
	GPIO_InitTypeDef 		gpio_InitTypeDef;

	/* Enable GPIOs clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	/* PD3(BEEPH) in output pushpull mode */
	gpio_InitTypeDef.GPIO_Pin =  IO_BEEP_PIN;
	gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(IO_BEEP_PORT, &gpio_InitTypeDef);
}

void setBeep (uint8_t numOfBeeps, uint32_t periodInMs) {
	beepConfig.numOfBeeps = numOfBeeps;
	beepConfig.periodInMs = periodInMs;
	beepConfig.startTime = sysTickTimer;
}

void cancelBeep () {
	beepConfig.numOfBeeps = 0;
}

void processBeeps()
{
	if(beepConfig.numOfBeeps > 0)
	{

		uint16_t currentPeriodNumber = (sysTickTimer - beepConfig.startTime)/beepConfig.periodInMs;

		if (currentPeriodNumber<beepConfig.numOfBeeps*2)
		{
			/* +1 to start with beep */
			setBeepOutput((currentPeriodNumber+1)%2);
		}
		/* finished blinking */
		else
		{
			beepConfig.numOfBeeps = 0;
			setBeepOutput(0);
		}
	}
}

void toggleBeep ()
{
	setBeepOutput(!GPIO_ReadInputDataBit(IO_BEEP_PORT, IO_BEEP_PIN));
}

void setBeepOutput(bool beepOn)
{
	GPIO_WriteBit(IO_BEEP_PORT,IO_BEEP_PIN,beepOn);
}

