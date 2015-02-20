#include "can.h"
#include "multitask/multitask.h"
#include "LCD/lcd_screen.h"
#include "ios.h"

void initCAN1();
void initCAN2();

static uint32_t receivedPackets=0;
static uint32_t transmitErrors=0;

void initCANs(){
	printf("[CANs]Starting Setup\n");

	initCAN1();
	initCAN2();

	printf("[CANs]Setup complete\n");
}

void initCAN1(){
	GPIO_InitTypeDef  		GPIO_InitStructure;
	CAN_InitTypeDef        	CAN_InitStructure;
	CAN_FilterInitTypeDef  	CAN_FilterInitStructure;
	NVIC_InitTypeDef  		NVIC_InitStructure;

	/* Enable CAN1 clocks  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	/* Configure CAN1 pin: RX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure CAN1 pin: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinRemapConfig(GPIO_Remap1_CAN1,ENABLE);

	/* CAN register init */
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

	/* CAN Baudrate = 250KBps*/
	///baudrate=1/(1tq + BS1tq + BS2tq) // Prescaler*2 = tq/ (1/72Mhz). (9=>250K, 18=> 150K). Leandro
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
	CAN_InitStructure.CAN_Prescaler = 9;
	CAN_Init(CAN1, &CAN_InitStructure);

	/* CAN filter init */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);


	/* CAN reception interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* CAN Status change error */
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* IT Configuration for CAN1 fifo*/
	CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_BOF | CAN_IT_ERR, ENABLE);
}

void initCAN2(){
	GPIO_InitTypeDef 	 GPIO_InitStructure;
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/* Enable CAN2 clocks  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

	/* Configure CAN2 pin: RX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure CAN2 pin: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinRemapConfig(GPIO_Remap_CAN2,ENABLE);

	/* CAN register init */
	CAN_DeInit(CAN2);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

	/* CAN Baudrate = 250KBps*/
	///baudrate=1/(1tq + BS1tq + BS2tq) // Prescaler*2 = tq/ (1/72Mhz). (9=>250K, 18=> 150K). Leandro
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
	CAN_InitStructure.CAN_Prescaler = 9;

	if(CAN_Init(CAN2, &CAN_InitStructure) == CAN_InitStatus_Failed) {
		printf("CAN2 Init Failed\n");
	}

	/* CAN filter init */
	CAN_FilterInitStructure.CAN_FilterNumber = 14;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

	/* Define interrupt priority */
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* CAN Status change error */
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_SCE_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* IT Configuration for CAN2 fifo*/
	CAN_ITConfig(CAN2, CAN_IT_FMP0 | CAN_IT_BOF | CAN_IT_ERR, ENABLE);
}

void CAN1_RX0_IRQHandler(void)
{
	onCAN1ReceiveInterrupt();
}

void CAN2_RX0_IRQHandler(void)
{
	onCAN2ReceiveInterrupt();
}

void CAN1_SCE_IRQHandler(void){
	onCAN1StatusChangeError();
}

void CAN2_SCE_IRQHandler(void){
	onCAN2StatusChangeError();
}


inline void onCAN1ReceiveInterrupt(){
	static CanRxMsg rxMessage;
	static MsgId msgId;
	static int i;

	CAN_Receive(CAN1, CAN_FIFO0, &rxMessage);

	msgId.extId = rxMessage.ExtId;

	if (msgId.command == CAN_COMMAND_WRITE_ACK)
	{
		ackWriteReceived.extId = msgId.extId;
		return;
	}

	receivedPackets++;
}

inline void onCAN2ReceiveInterrupt(){
	static CanRxMsg rxMessage;
	static MsgId msgId;
	static int i;

	uint8_t data[8]={0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

	CAN_Receive(CAN2, CAN_FIFO0, &rxMessage);

	msgId.extId = rxMessage.ExtId;

	sendCanPacket(CAN2, CAN_COMMAND_BROADCAST, 0xAB, MY_ID, 0xCC, &data, 8);

	receivedPackets++;
}

inline void onCAN1StatusChangeError(){
    if(CAN_GetFlagStatus(CAN1,CAN_FLAG_BOF) == SET){
		displayErrorMessage("Falha de","comunicacao(bof)",5000, PRIORITY_3);
		initCAN1();
		CAN_ClearITPendingBit(CAN1,CAN_IT_BOF);
    }
}

inline void onCAN2StatusChangeError(){
    if(CAN_GetFlagStatus(CAN2,CAN_FLAG_BOF) == SET){
		displayErrorMessage("Falha de","comunicacao(bof)",5000, PRIORITY_3);
		initCAN2();
		CAN_ClearITPendingBit(CAN2,CAN_IT_BOF);
    }
}

uint32_t transmitErrorsCount(){
	return transmitErrors;
}

uint32_t receivedCanPacketsCount(){
	return receivedPackets;
}


void sendCanPacket(CAN_TypeDef* CANx, uint8_t command,uint8_t index, uint8_t source, uint8_t dest, uint8_t *data, uint8_t numBytes ){
	const uint32_t CAN_TRANSMIT_TIMEOUT=3;
	CanTxMsg txMessage;

	/* Build  ExtId */
	MsgId msgId;
	msgId.command = command;
	msgId.index = index;
	msgId.SA = source;
	msgId.DA = dest;
	txMessage.ExtId = msgId.extId;

	/* Copy data */
	memcpy(txMessage.Data,data,numBytes);

	/* Set parameters */
	txMessage.RTR = CAN_RTR_DATA;
	txMessage.IDE = CAN_ID_EXT;
	txMessage.DLC = numBytes;

	/* Send Packet */
	uint8_t transmitMailbox= CAN_Transmit(CANx, &txMessage);

	uint32_t startTime=sysTickTimer;
	/* Wait to transmit packet */
	while(CAN_TransmitStatus(CANx,transmitMailbox) == CAN_TxStatus_Pending){
		if (achievedTimeout(startTime,CAN_TRANSMIT_TIMEOUT)){
			/* Display message with error */
			//printExtId(txMessage.ExtId);
			transmitErrors++;
			break;
		}
	}
}

void CAN_writePort(CAN_TypeDef* CANx, uint8_t mpxId, uint8_t port, PortParameter *portParameter)
{
	sendCanPacket(CANx, CAN_COMMAND_WRITE, port, MY_ID, mpxId, (uint8_t*)portParameter, 8);
}
