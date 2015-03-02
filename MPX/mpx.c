/* Includes ------------------------------------------------------------------*/
#include "MPX/mpx.h"

/* Functions Intern Declarations ---------------------------------------------*/

/* Relative to the active MPX ID ports code */
void initMPXIDports();

/* Functions -----------------------------------------------------------------*/

/* Initializes the pins referent to the interface between GIGA3 and the MPX */
void initMPXconfig()
{
	/* Initializes the MPX ID pins */
	initMPXIDpins();
	/* Initializes the Shift Registers Out's RCLK pins */
	//initSHWRHSPI2();
	/* Initializes the High and Low side Push Pull 10A transistors' pins  */
	//initPP10Apins();

	mpx.MpxAlreadyInit = true;
}

void setMPXIDports(MPX_ID ID)
{
	GPIO_ResetBits(CMDFLID_PORT, CMDFLID0 | CMDFLID1 | CMDFLID2);

	if (ID == ID1)
		GPIO_SetBits(CMDFLID_PORT, CMDFLID0);

	else if (ID == ID2)
		GPIO_SetBits(CMDFLID_PORT, CMDFLID1);

	else if (ID == ID4)
		GPIO_SetBits(CMDFLID_PORT, CMDFLID2);
}


void initMPXIDpins()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOB's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* Initializes the Shift Registers Out's RCLK pin */
	gpio_InitTypeDef.GPIO_Pin	= CMDFLID0 | CMDFLID1 | CMDFLID2;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(CMDFLID_PORT, &gpio_InitTypeDef);

	/* Initializes MPX ID pin at low level */
	GPIO_ResetBits(CMDFLID_PORT, CMDFLID0 | CMDFLID1 | CMDFLID2);
}

