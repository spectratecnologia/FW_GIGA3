/* Includes ------------------------------------------------------------------*/
#include "mpx.h"

/* Functions Intern Declarations ---------------------------------------------*/

/* Relative to the active MPX ID ports code */
void initMPXstruct();
void initMPXIDports();
void initPP10Apins();
void initMPXIgn();
void initMPXTempPin();

inline void activePP10Aports(uint8_t, FunctionalState_MPXports);
inline void activePP10AportX(uint8_t, uint8_t, uint8_t, FunctionalState_MPXports);
inline void activeBIDIports(uint8_t, FunctionalState_MPXports);
inline void activeLODINports(uint8_t, FunctionalState_MPXports);

/* Functions -----------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* Initializes functions to active pins relative to the MPX device -----------*/
/* ---------------------------------------------------------------------------*/

/* Initializes the pins referent to the interface between GIGA3 and the MPX */
void initMPXconfig()
{
	initMPXstruct();
	/* Initialize the MPX ID pins */
	initMPXIDpins();
	/* Initialize GIGA3 shift registers */
	initShiftRegisters();
	/* Initialize the High and Low side Push Pull 10A transistors' pins  */
	initPP10Apins();
	/* Initialize ignition command pin */
	initMPXIgn();
	/* Initialize temperature command pin */
	initMPXTempPin();

	mpx.numPorts = NUM_PORTS;
	mpx.MpxAlreadyInit = true;
}

void initMPXstruct()
{
	mpx.numPorts = NUM_PORTS;
	mpx.lastTimeSeen = 0;
	mpx.ackReceived = false;
	mpx.ackIndex = 0;
	mpx.lastAckIndex = 0;
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

void initPP10Apins()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= CMDFH1 | CMDFH2 | CMDFH3 | CMDFH4 | CMDFL1 | CMDFL2 | CMDFL3 | CMDFL4;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(CMDFHL_PORT, &gpio_InitTypeDef);
}

void initMPXIgn()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= CMD_IGN1;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(CMD_IGN1_PORT, &gpio_InitTypeDef);

	GPIO_ResetBits(CMD_IGN1_PORT, CMD_IGN1);
}

void initMPXTempPin()
{
	GPIO_InitTypeDef	gpio_InitTypeDef;

	/* Enable clock GPIOC's peripheric  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* 	Initializes the High and Low side Push Pull 10A transistors' pins */
	gpio_InitTypeDef.GPIO_Pin	= PWM_NTC;
	gpio_InitTypeDef.GPIO_Mode	= GPIO_Mode_Out_PP;
	gpio_InitTypeDef.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(PWM_NTC_PORT, &gpio_InitTypeDef);

	GPIO_ResetBits(PWM_NTC_PORT, PWM_NTC);
}

/* ---------------------------------------------------------------------------*/
/* Functions to enable or disable pins relative to the MPX device ------------*/
/* ---------------------------------------------------------------------------*/

/* Active ID ports -----------------------------------------------------------*/
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

/* Active MPX ports ----------------------------------------------------------*/
void activeMPXports(uint8_t portx, FunctionalState_MPXports state)
{
	if ((portx >= 0) && (portx < 4))
		activePP10Aports(portx, state);
	else if ((portx >= 4) && (portx <= 27))
		activeBIDIports(portx, state);
	else if ((portx >= 28) && (portx <= 35))
		activeLODINports(portx, state);
}

/* Active push pull 10A ports ------------------------------------------------*/
inline void activePP10Aports(uint8_t portx, FunctionalState_MPXports state)
{
	if (portx == 0)
		activePP10AportX(CMDFH1, CMDFL1, portx, state);

	else if (portx == 1)
		activePP10AportX(CMDFH2, CMDFL2, portx, state);

	else if (portx == 2)
		activePP10AportX(CMDFH3, CMDFL3, portx, state);

	else if (portx == 3)
		activePP10AportX(CMDFH4, CMDFL4, portx, state);
}

inline void activePP10AportX(uint8_t pinH, uint8_t pinL, uint8_t portx, FunctionalState_MPXports state)
{
	/* This command turn high MPX device port */
	uint8_t highMPXdeviceport[8]={0x03, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	/* This command turn low MPX device port */
	uint8_t lowMPXdeviceport[8]={0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	/* This command turn off MPX device port */
	uint8_t offMPXdeviceport[8]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (state == PORT_HIGH)
	{
		GPIO_SetBits(CMDFHL_PORT, pinL);
		GPIO_ResetBits(CMDFHL_PORT, pinH);
		memcpy(&mpx.portOutput[portx], highMPXdeviceport, 8);
	}

	else if (state == PORT_LOW)
	{
		GPIO_ResetBits(CMDFHL_PORT, pinL);
		GPIO_SetBits(CMDFHL_PORT, pinH);
		memcpy(&mpx.portOutput[portx], lowMPXdeviceport, 8);
	}

	else if (state == PORT_OFF)
	{
		GPIO_ResetBits(CMDFHL_PORT, pinL);
		GPIO_ResetBits(CMDFHL_PORT, pinH);
		memcpy(&mpx.portOutput[portx], offMPXdeviceport, 8);
	}
	else if (state == PORT_OFF_GIGA_ON)
	{
		GPIO_SetBits(CMDFHL_PORT, pinL);
		GPIO_ResetBits(CMDFHL_PORT, pinH);
		memcpy(&mpx.portOutput[portx], offMPXdeviceport, 8);
	}
	else
	{
		return;
	}

	mpx.outputChanged[portx] = true;
}

/* Active BIDI ports ---------------------------------------------------------*/
inline void activeBIDIports(uint8_t portx, FunctionalState_MPXports state)
{
    uint32_t position=0x00000001;
    /* This command turn high MPX device port */
    uint8_t highMPXdeviceport[8]={0x03, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    /* This command turn off MPX device port */
    uint8_t offMPXdeviceport[8]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (state == PORT_OFF)
    {
    	memcpy(&mpx.portOutput[portx], offMPXdeviceport, 8);
    	mpx.outputChanged[portx] = true;
    	sendDataToSR(0x0);
    }

    else if (state == PORT_HIGH)
    {
    	/* Return int32 position which specify data to turn on port X */
    	position<<=8*((portx-4)/8)+(7-(portx-4)%8);
    	/* Send int32 position to update output shift registers pins */
    	sendDataToSR(position);
    	memcpy(&mpx.portOutput[portx], highMPXdeviceport, 8);
    	mpx.outputChanged[portx] = true;
    }
    else if (state == PORT_OFF_GIGA_ON)
	{
		/* Return int32 position which specify data to turn on port X */
		position<<=8*((portx-4)/8)+(7-(portx-4)%8);
		/* Send int32 position to update output shift registers pins */
		sendDataToSR(position);
		memcpy(&mpx.portOutput[portx], offMPXdeviceport, 8);
		mpx.outputChanged[portx] = true;
	}

    else
    	return;
}

/* Active LODIN ports --------------------------------------------------------*/
inline void activeLODINports(uint8_t portx, FunctionalState_MPXports state)
{
	uint32_t position=0x00000001;

    if (state == PORT_OFF)
    	sendDataToSR(0x0);

    else if (state == PORT_LOW)
    {
    	/* Return int32 position which specify data to turn on port X */
    	position<<=8*((portx-4)/8)+(7-(portx-4)%8);
    	/* Send int32 position to update output shift registers pins */
    	sendDataToSR(position);
    }

    else
    	return;
}

uint8_t getPortStatus(uint8_t portx)
{
	uint8_t offset_port;

	if ((portx >= 0) && (portx <= 7))
		offset_port = 0x07;

	else if ((portx >= 8) && (portx <= 15))
		offset_port = 0x17;

	else if ((portx >= 16) && (portx <= 23))
		offset_port = 0x27;

	else if ((portx >= 24) && (portx <= 28))
		offset_port = 0x37;
	else
		return;

	if (getSRBitStatus(offset_port-portx))
		return 1;
	else
		return 0;
}

void activeMPXIgnition(FunctionalState_MPXports state)
{
	if (state == PORT_HIGH)
		GPIO_SetBits(CMD_IGN1_PORT, CMD_IGN1);
	else if (state == PORT_LOW)
		GPIO_ResetBits(CMD_IGN1_PORT, CMD_IGN1);
	else
		return;
}

void toogleMPXNTC(void)
{
	if(GPIO_ReadOutputDataBit(PWM_NTC_PORT, PWM_NTC))
		GPIO_ResetBits(PWM_NTC_PORT, PWM_NTC);
	else
		GPIO_SetBits(PWM_NTC_PORT, PWM_NTC);
}
/* ---------------------------------------------------------------------------*/
/* Active or deactive MPX device ports ---------------------------------------*/
/* ---------------------------------------------------------------------------*/
void turnOffMpxPorts(void)
{
	int i;

	if(mpx.MpxAlreadyInit)
		sendDataToSR(0x0);

	/* Disable all ports that might be high or low */
	for(i=0; i<NUM_PORTS; i++)
		if((mpx.portOutput[i].mode == 0x01) || (mpx.portOutput[i].mode == 0x03))
			activeMPXports(i, PORT_OFF);
}

void sendChangedOutputsToMPXs(){
	static uint8_t lastPortSent;
	static uint64_t lastTimeSent;
	static uint8_t retries;

	const uint8_t NUM_RETRIES=3;
	const uint8_t TIME_TO_RETRY=2;

	uint8_t offMPXdeviceport[8]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	uint8_t mpxIndex,portIndex;

	/* This if prevents any port be active more than MAX_TIME_TO_PORT_BE_ACTIVE time */
	//if ((sysTickTimer - lastTimeSent) > MAX_TIME_TO_PORT_BE_ACTIVE)
	//{
	//	if ( (mpx.portOutput[lastPortSent].mode == 0x01) || (mpx.portOutput[lastPortSent].mode == 0x03))
	//	{
	//		CAN_writePort(CAN1, mpx.mpxId, lastPortSent, &offMPXdeviceport);
	//		activeMPXports(lastPortSent, PORT_OFF);
	//		lastTimeSent = sysTickTimer;
	//	}
	//}

	for(portIndex=0; portIndex<mpx.numPorts; portIndex++)
	{
		/* If port is marked to send */
		if (mpx.outputChanged[portIndex])
		{
			/* If port already sent at least once */
			if( ((sysTickTimer-lastTimeSent)>TIME_TO_RETRY) && (portIndex==lastPortSent) )
			{
				retries++;
			}

			/* Send port for first time */
			else
			{
				lastPortSent = portIndex;
				retries = 0;
			}

			CAN_writePort(CAN1, mpx.mpxId, portIndex, &mpx.portOutput[portIndex]);
			lastTimeSent = sysTickTimer;

			/* Mark output as sent */
			if (retries>=NUM_RETRIES){
				mpx.outputChanged[portIndex] = false;
				retries=0;
			}

			/* Send only first "outputChaged" from each MPX */
			break;
		}
	}
}

void turningOffMpxEmergencyMode()
{
	uint8_t data[8]={0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	sendCanPacket(CAN1, CAN_COMMAND_BROADCAST, CAN_BROADCAST_KEY_STATE_MASK, 0x20, 0x00, &data, 8);
}






