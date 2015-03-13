/* Includes ------------------------------------------------------------------*/
#include "giga3_sr.h"

/* Functions -----------------------------------------------------------------*/
void initShiftRegisters()
{
	/* Initializes SPI2 peripheric which will control the shift registers */
	GPIO_InitTypeDef  		GPIO_InitStructure;
	SPI_InitTypeDef        	SPI_InitStructure;
	NVIC_InitTypeDef  		NVIC_InitStructure;

	/* Enable SPI2 clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* 	Init SPI2 IOs */
	GPIO_InitStructure.GPIO_Pin		= SPI2_SCK | SPI2_MISO | SPI2_MOSI;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(SPI2_PORT, &GPIO_InitStructure);

	/* Configure SPI2 */
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

	/* 	Init RW pins to read and write data from shift registers. */
	GPIO_InitStructure.GPIO_Pin		= SR_READ | SR_WRITE;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(SR_WR_PORT, &GPIO_InitStructure);
}

void sendDataToSR(uint32_t data)
{
	/* Read buffer's data to RX does not subscribe TX. */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
		SPI_I2S_ReceiveData(SPI2);

	/* Send RX in two parts and wait until data be sent. */
    SPI_I2S_SendData(SPI2, (uint16_t)(data));
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

    /* Read buffer's data to RX does not subscribe TX. */
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
		SPI_I2S_ReceiveData(SPI2);

    SPI_I2S_SendData(SPI2, (uint16_t)(data>>16));
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
		SPI_I2S_ReceiveData(SPI2);

    /* Give a low pulse to save data in the shift registers ports. */
    GPIO_SetBits(SR_WR_PORT, SR_WRITE);
    GPIO_ResetBits(SR_WR_PORT, SR_WRITE);
    GPIO_SetBits(SR_WR_PORT, SR_WRITE);
}

uint32_t readDataFromSR(void)
{
	uint32_t data=0x0;

	GPIO_SetBits(SR_WR_PORT, SR_READ);
	GPIO_ResetBits(SR_WR_PORT, SR_READ);
	GPIO_SetBits(SR_WR_PORT, SR_READ);

	SPI_I2S_SendData(SPI2, (uint16_t)(data));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	data =  (uint32_t)(SPI_I2S_ReceiveData(SPI2));

	SPI_I2S_SendData(SPI2, (uint16_t)(data));
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	data +=  (uint32_t)(SPI_I2S_ReceiveData(SPI2)<<16);

	return data;
}

uint8_t getSRBitStatus(uint8_t bit_x)
{
	uint32_t maskbits = 0x1;
	uint32_t SRdata = readDataFromSR();

	if (SRdata & (maskbits << bit_x))
		return 1;
	else
		return 0;
}

