/* Includes ------------------------------------------------------------------*/
#include "ptc16.h"

/* Intern Variables Declarations ---------------------------------------------*/
static bool ptc16_boolEnableToogleOdo;
static bool ptc16_boolEnableToogleTaco;

Ptc ptc16;

/* Intern Functions Declarations ---------------------------------------------*/
void initPTC16Ign();
void initPTC16Taco();
void initPTC16Odo();

/* Functions -----------------------------------------------------------------*/
void initPTC16config()
{
	/* Initialize ignition command pin */
	initPTC16Ign();
	initPTC16Taco();
	initPTC16Odo();

	ptc16.PtcAlreadyInit = true;
}

void deInitPTC16config()
{
	GPIO_ResetBits(PTC16_CMD_IGN2_PORT, PTC16_CMD_IGN2);
	ptc16.PtcAlreadyInit = false;
}

void initPTC16Ign()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= PTC16_CMD_IGN2;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(PTC16_CMD_IGN2_PORT, &gpio_InitTypeDef);

	GPIO_SetBits(PTC16_CMD_IGN2_PORT, PTC16_CMD_IGN2);
}

void activePTC16Ign(FunctionalState fs)
{
	if(fs == DISABLE)
		GPIO_ResetBits(PTC16_CMD_IGN2_PORT, PTC16_CMD_IGN2);
	else
		GPIO_SetBits(PTC16_CMD_IGN2_PORT, PTC16_CMD_IGN2);
}

void initPTC16Taco()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= PTC16_CMDFL_TACO;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(PTC16_CMDFL_TACO_PORT, &gpio_InitTypeDef);

	GPIO_ResetBits(PTC16_CMDFL_TACO_PORT, PTC16_CMDFL_TACO);
}

void initPTC16Odo()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= PTC16_CMDFL_ODO;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(PTC16_CMDFL_ODO_PORT, &gpio_InitTypeDef);

	GPIO_ResetBits(PTC16_CMDFL_ODO_PORT, PTC16_CMDFL_TACO);
}

void ptc16_enableToogleTaco(bool ena)
{
	ptc16_boolEnableToogleTaco = ena;
}

void ptc16_toggleTaco()
{
	if(ptc16_boolEnableToogleTaco)
	{
		if(GPIO_ReadOutputDataBit(PTC16_CMDFL_TACO_PORT, PTC16_CMDFL_TACO))
			GPIO_ResetBits(PTC16_CMDFL_TACO_PORT, PTC16_CMDFL_TACO);
		else
			GPIO_SetBits(PTC16_CMDFL_TACO_PORT, PTC16_CMDFL_TACO);
	}
}

void ptc16_enableToogleOdo(bool ena)
{
	ptc16_boolEnableToogleOdo = ena;
}

void ptc16_toggleOdo()
{
	if (ptc16_boolEnableToogleOdo)
	{
		if(GPIO_ReadOutputDataBit(PTC16_CMDFL_ODO_PORT, PTC16_CMDFL_ODO))
			GPIO_ResetBits(PTC16_CMDFL_ODO_PORT, PTC16_CMDFL_ODO);
		else
			GPIO_SetBits(PTC16_CMDFL_ODO_PORT, PTC16_CMDFL_ODO);
	}
}

