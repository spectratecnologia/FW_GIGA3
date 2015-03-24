/* Includes ------------------------------------------------------------------*/
#include "ptc24.h"

/* Intern Variables Declarations ---------------------------------------------*/
static bool boolEnableToogleOdo;
static bool boolEnableToogleTaco;

/* Intern Functions Declarations ---------------------------------------------*/
void initPTC24Ign();
void initPTC24Taco();
void initPTC24Odo();

/* Functions -----------------------------------------------------------------*/
void initPTC24config()
{
	/* Initialize ignition command pin */
	initPTC24Ign();
	initPTC24Taco();
	//initPTC24Odo();

	ptc24.PtcAlreadyInit = true;
}

void deInitPTC24config()
{
	GPIO_ResetBits(CMD_IGN2_PORT, CMD_IGN2);
	ptc24.PtcAlreadyInit = false;
}

void initPTC24Ign()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= CMD_IGN2;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(CMD_IGN2_PORT, &gpio_InitTypeDef);

	GPIO_SetBits(CMD_IGN2_PORT, CMD_IGN2);
}

void tooglePTC24Ign()
{
	GPIO_ResetBits(CMD_IGN2_PORT, CMD_IGN2);
	GPIO_SetBits(CMD_IGN2_PORT, CMD_IGN2);
}

void initPTC24Taco()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= CMDFL_TACO | CMDFL_ODO;;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(CMDFL_TACO_PORT, &gpio_InitTypeDef);

	GPIO_ResetBits(CMDFL_TACO_PORT, CMDFL_TACO);
}

void initPTC24Odo()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= CMDFL_ODO;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(CMDFL_ODO_PORT, &gpio_InitTypeDef);

	GPIO_ResetBits(CMDFL_ODO_PORT, CMDFL_TACO);
}

void ptc24_enableToogleTaco(bool ena)
{
	boolEnableToogleTaco = ena;
}

void ptc24_toggleTaco()
{
	if(boolEnableToogleTaco)
	{
		if(GPIO_ReadOutputDataBit(CMDFL_TACO_PORT, CMDFL_TACO))
			GPIO_ResetBits(CMDFL_TACO_PORT, CMDFL_TACO);
		else
			GPIO_SetBits(CMDFL_TACO_PORT, CMDFL_TACO);
	}
}

void ptc24_enableToogleOdo(bool ena)
{
	boolEnableToogleOdo = ena;
}

void ptc24_toggleOdo()
{
	if (boolEnableToogleOdo)
	{
		if(GPIO_ReadOutputDataBit(CMDFL_ODO_PORT, CMDFL_ODO))
			GPIO_ResetBits(CMDFL_ODO_PORT, CMDFL_ODO);
		else
			GPIO_SetBits(CMDFL_ODO_PORT, CMDFL_ODO);
	}
}

void emulateMpx()
{
	uint8_t data[8]={0,0,0,0,0,0,0,0};
	sendCanPacket(CAN1, CAN_COMMAND_BROADCAST, CAN_BROADCAST_MPX_INFO, MXP_DEVICE_ID0, BROADCAST_DEST_ADDR, &data[0], 8);
}
