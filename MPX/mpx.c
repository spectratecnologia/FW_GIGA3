/* Includes ------------------------------------------------------------------*/
#include "MPX/mpx.h"

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initializes the pins referent to the interface between GIGA3 and
  * 	 	the MPX
  * @param  None
  * @retval None
  */
void initMPXconfig()
{
	/* Initializes the Shift Registers Out's RCLK pin */
	initSHWRHSPI2();
	/* Initializes the High and Low side Push Pull 10A transistors' pins  */
	initPP10Apins();
}

/**
  * @brief  Initializes the Shift Registers Out's RCLK pin
  * @param  None
  * @retval None
  */
void initSHWRHSPI2()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOB's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	/* Initializes the Shift Registers Out's RCLK pin */
	gpio_InitTypeDef.GPIO_Pin	= SROUTRCLKPIN;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(SROUTRCLKPORT, &gpio_InitTypeDef);
}

/**
  * @brief  Initializes the High and Low side Push Pull 10A transistors' pins
  * @param  None
  * @retval None
  */
void initPP10Apins()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= CMDFH1 | CMDFH2 | CMDFH3 | CMDFH4
								| CMDFL1 | CMDFL2 | CMDFL3 | CMDFL4;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(CMDFHLPORT, &gpio_InitTypeDef);
}

/**
  * @brief  Active transistors referent to the MPX port X at GIGA3.
  * @param  portx: where x can be 0 to 35. See "Portas MPX2808" excel file.
  * @param  state: represents Push Pull 10A resistors status. Can be
  * 		PORT_HIGH: resistor receives VCC or
  *		 	PORT_LOW: resistor receives 0V your terminal
  * @retval None
  */
void activeMPXports(uint8_t portx, FunctionalState_PP10A state)
{
	/* Deactivate all ports because only one port will be set at once */
	deactiveallMPXports();

	/* Handle with the PUSH PULL 10A ports */
	if (portx >= 0 && portx <= 3)
	{
		activePP10Aports(portx, state);
	}

	/* Handle with the ports controlled by the output Shift Registers */
	else if (portx >= 4 && portx <= 35)
	{
		activeSPI2MPXports(portx);
	}

	/* Handle with the ports which does not exist */
	else
	{
		printf("[MPXports]Current port does not exist");
	}

}

/**
  * @brief  Active transistors referent to the PUSH PULL 10A port X at GIGA3.
  * @param  portx: where x can be 0 to 3. See "Portas MPX2808" excel file.
  * @param  state: represents Push Pull 10A resistors status. Can be
  * 		PORT_HIGH: resistor receives VCC or
  *		 	PORT_LOW: resistor receives 0V your terminal.
  * @retval None
  */
inline void activePP10Aports(uint8_t portx, FunctionalState_PP10A state)
{
	switch (portx)
	{
	case 0:
		if (state==PORT_HIGH)
		{
			GPIO_SetBits(CMDFHLPORT, CMDFL1);
			GPIO_ResetBits(CMDFHLPORT, CMDFH1);
			activeMPXdeviceports(portx, PORT_HIGH);
		}

		else if (state==PORT_LOW)
		{
			GPIO_ResetBits(CMDFHLPORT, CMDFL1);
			GPIO_SetBits(CMDFHLPORT, CMDFH1);
			activeMPXdeviceports(portx, PORT_LOW);
		}

		break;

	case 1:
		if (state==PORT_HIGH)
		{
			GPIO_SetBits(CMDFHLPORT, CMDFL2);
			GPIO_ResetBits(CMDFHLPORT, CMDFH2);
			activeMPXdeviceports(portx, PORT_HIGH);
		}

		else if (state==PORT_LOW)
		{
			GPIO_ResetBits(CMDFHLPORT, CMDFL2);
			GPIO_SetBits(CMDFHLPORT, CMDFH2);
			activeMPXdeviceports(portx, PORT_LOW);
		}
		break;

	case 2:
		if (state==PORT_HIGH)
		{
			GPIO_SetBits(CMDFHLPORT, CMDFL3);
			GPIO_ResetBits(CMDFHLPORT, CMDFH3);
			activeMPXdeviceports(portx, PORT_HIGH);
		}

		else if (state==PORT_LOW)
		{
			GPIO_ResetBits(CMDFHLPORT, CMDFL3);
			GPIO_SetBits(CMDFHLPORT, CMDFH3);
			activeMPXdeviceports(portx, PORT_LOW);
		}
		break;

	case 3:
		if (state==PORT_HIGH)
		{
			GPIO_SetBits(CMDFHLPORT, CMDFL4);
			GPIO_ResetBits(CMDFHLPORT, CMDFH4);
			activeMPXdeviceports(portx, PORT_HIGH);
		}

		else if (state==PORT_LOW)
		{
			GPIO_ResetBits(CMDFHLPORT, CMDFL4);
			GPIO_SetBits(CMDFHLPORT, CMDFH4);
			activeMPXdeviceports(portx, PORT_LOW);
		}
		break;
	}

}

/**
  * @brief  Active transistors controlled by the output shift registers
  *    		referent to the port X.
  * @param  portx: where x can be 4 to 35. See "Portas MPX2808" excel file.
  * @retval None
  */
inline void activeSPI2MPXports(uint8_t portx)
{
	uint32_t position=0x00000001;

	/* Return int32 position which specify data needed to turn on port X */
	position<<=8*((portx-4)/8)+(7-(portx-4)%8);

	/* Send int32 position to update output shift registers pins */
	senddataSRout(position);

	/* Actives port X on MPX device */
	activeMPXdeviceports(portx, PORT_HIGH);
}

/**
  * @brief  Deactive all MPX port transistors
  * @param  None
  * @retval None
  */
void deactiveallMPXports()
{
	uint8_t i;

	/* Disable all ports on MPX device */
	for (i=0; i<NUM_MPXPORTS; i++)
	{
		activeMPXdeviceports(i, PORT_OFF);
	}

	/* Deactive low side PUSH PULL 10A transistors */
	GPIO_ResetBits(CMDFHLPORT, CMDFL1);
	GPIO_ResetBits(CMDFHLPORT, CMDFL2);
	GPIO_ResetBits(CMDFHLPORT, CMDFL3);
	GPIO_ResetBits(CMDFHLPORT, CMDFL4);

	/* Deactive high side PUSH PULL 10A transistors */
	GPIO_ResetBits(CMDFHLPORT, CMDFH1);
	GPIO_ResetBits(CMDFHLPORT, CMDFH2);
	GPIO_ResetBits(CMDFHLPORT, CMDFH3);
	GPIO_ResetBits(CMDFHLPORT, CMDFH4);

	/* Deactive transistors controlled by the output shift registers */
	senddataSRout(0);

}

/**
  * @brief  Send int32 position to update output shift registers pins
  * @param  data MUST have only one bit high because only one port will be
  * 		active at once
  * @retval None
  */
void senddataSRout(uint32_t data)
{
	SPI_I2S_SendData(SPI2, (uint16_t)(data));
	delayMsUsingSysTick(1);

	SPI_I2S_SendData(SPI2, (uint16_t)(data>>16));
	delayMsUsingSysTick(1);

	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	delayMsUsingSysTick(1);
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

/**
  * @brief  Active transistors referent to the PUSH PULL 10A port X at MPX device.
  * @param  portx: where x can be 0 to 35. See "Portas MPX2808" excel file.
  * @param  state: represents Push Pull 10A resistors status. Can be
  * 		PORT_HIGH: resistor receives VCC or
  *		 	PORT_LOW: resistor receives 0V your terminal.
  * @retval None
  */
inline void activeMPXdeviceports(uint8_t portx, FunctionalState_PP10A state)
{
	mpx.mpxId=0x82;

	/* This variable will allow a certain number of write ack received by CAN */
	uint8_t tries=10;
	/* This command turn high MPX device port */
	uint8_t highMPXdeviceports[8]={0x03, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	/* This command turn low MPX device port */
	uint8_t lowMPXdeviceports[8]={0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	/* This command turn off MPX device port */
	uint8_t offMPXdeviceports[8]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//toggleCPULED();

	/* This loop will be active until GIGA3 receive write ack command or the number
	 * of tries reaches a defined value.
	 */
	while(1)
	{
		if (state == PORT_LOW)
		{
			sendCanPacket(CAN1, CAN_COMMAND_WRITE, portx, MY_ID, mpx.mpxId, &lowMPXdeviceports, 8);
		}

		else if (state == PORT_HIGH)
		{
			sendCanPacket(CAN1, CAN_COMMAND_WRITE, portx, MY_ID, mpx.mpxId, &highMPXdeviceports, 8);
		}

		else if (state == PORT_OFF)
		{
			sendCanPacket(CAN1, CAN_COMMAND_WRITE, portx, MY_ID, mpx.mpxId, &offMPXdeviceports, 8);
		}

		delayMsUsingSysTick(4);

		/* Break the loop if write ack corresponds to the expected value write ack */
		if(ackWriteReceived.index == portx && ackWriteReceived.SA == mpx.mpxId && ackWriteReceived.DA == MY_ID)
		{
			sendCanPacket(CAN1, 0xAB, 0xCD, 0xEF, 0xAA, &offMPXdeviceports, 8);
			break;
		}

		/* Break the loop if the number of tries was reached and call a properly function */
		if (tries == 0)
		{
			communicationWriteACKMPXerror();
			break;
		}

		tries--;
	}
}


