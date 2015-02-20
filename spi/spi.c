/* Includes ------------------------------------------------------------------*/
#include "spi.h"

/* Local functions declaration -----------------------------------------------*/
void initSPIs();
void initSPI2();

/* Local functions -----------------------------------------------------------*/
void initSPIs(){
	printf("[SPIs]Starting Setup\n");

	initSPI2();

	printf("[SPIs]Setup complete\n");
}

/*
 *Enable SPI2 to begin communication with the I/O shift registers
 */
void initSPI2(){
	GPIO_InitTypeDef  		GPIO_InitStructure;
	SPI_InitTypeDef        	SPI_InitStructure;
	NVIC_InitTypeDef  		NVIC_InitStructure;

	/* Enable SPI2 clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* 	Init SPI2 IOs */
	GPIO_InitStructure.GPIO_Pin	= SPI2_NSS | SPI2_SCK | SPI2_MISO | SPI2_MOSI;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(SPI2_PORT, &GPIO_InitStructure);

	/* Configure SPI */
	SPI_InitStructure.SPI_Direction			= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode				= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize			= SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL				= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA				= SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS				= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit			= SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_CRCPolynomial		= 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	SPI_Cmd(SPI2, ENABLE);
}
