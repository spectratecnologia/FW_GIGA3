#include "usart.h"

//#include "usb_dual/usr/usbd_cdc_vcp.h"
#include "rtc/rtc.h"

static void downloadDateTime(char byte);

static uint8_t startTimeDateDownload=0;
static uint32_t bytesTimeDateDownloaded=0;

/* Change This to the unix-time format */
static char timeDateTemp[6];

static inline void checkForResetToBootloader(char c);
static inline void checkForUpdateTime(char c);

void initUSART(){

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);

	/* Enable the USART2 Pins Software Remapping */
	GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* USART configuration */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	/* COMMENTED TO DISABLE INTERRUPTION - WHEN RX PIN IS FLOATING, CAUSES ERRORS DUE INTERRUPTION*/
	/* Enable USART2 Receive and Transmit interrupts */
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	/* Enable USART */
	USART_Cmd(USART2, ENABLE);

	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}


bool isAllDateTimeDownloaded(){
	return bytesTimeDateDownloaded==sizeof(timeDateTemp);
}

inline void onUSART2Interrupt(){
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		/* Read one byte from the receive data register */
		processReceivedChar(USART_ReceiveData(USART2));
	}
}

inline void processReceivedChar(char c){
	if(startTimeDateDownload) {
		downloadDateTime(c);
		bytesTimeDateDownloaded++;
		if(isAllDateTimeDownloaded()) {

			/* Write time on the RTC */

			//writeTime((uint8_t)timeDateTemp[0], (uint8_t)timeDateTemp[1], (uint8_t)timeDateTemp[2],
			//		  (uint8_t)timeDateTemp[3], (uint8_t)timeDateTemp[4], (uint8_t)timeDateTemp[5]);

			finishDateTimeDownload();
		}
	}
	else{
		checkForUpdateTime(c);
		checkForResetToBootloader(c);
	}
}

static inline void checkForResetToBootloader(char c){
	const char stringToMatch[] = "##BOOTLOADER_GIGA3##";
	static uint8_t currentCharMatched=0;
	if ( c == stringToMatch[currentCharMatched] ){
		currentCharMatched++;
		if ( currentCharMatched == strlen(stringToMatch) ){
			currentCharMatched=0;
			rebootToBootloader();
		}
	}
	else{
		currentCharMatched=0;
		if ( c == stringToMatch[currentCharMatched] ){
			currentCharMatched++;
		}
	}
}

static inline void checkForUpdateTime(char c){
	const char stringToMatch[] = "##DATETIME_GIGA3##";
	static uint8_t currentCharMatched=0;
	if ( c == stringToMatch[currentCharMatched] ){
		currentCharMatched++;
		if ( currentCharMatched == strlen(stringToMatch) ){
			currentCharMatched=0;
			startDownloadDateTime();
		}
	}
	else{
		currentCharMatched=0;
		if ( c == stringToMatch[currentCharMatched] ){
			currentCharMatched++;
		}
	}
}

static void downloadDateTime(char byte){
	/*prevents ISR to overflow of the time_t variable */
	if(!isAllDateTimeDownloaded())
		*(((char*)(&timeDateTemp))+bytesTimeDateDownloaded) = byte;
}

bool isUSARTAvailableToWrite(){
	return true;
}

bool isDownloadDateTimeDetected(){
	return startTimeDateDownload;
}

void startDownloadDateTime() {
	bytesTimeDateDownloaded=0;
	startTimeDateDownload=true;
}

void finishDateTimeDownload(){
	startTimeDateDownload=false;
}

uint32_t writeBytesToUSART(char *data, uint32_t len){
	int i=0;
	while (i < len) {
		/* take this comment out when VCP is implemented */
		//VCP_DataTx((uint8_t*)(data+i),1);
		USART_SendData(USART2, (uint8_t) data[i++]);

		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	}
	return i;
}

/* Reboot to bootloader */
void rebootToBootloader(void){
	printf("[BOOTLOADER]Rebooting to bootloader..\n");
	/* This magic is explained in the startup_assembly */
	*((unsigned long *)0x2000FFF0) = 0xDEADBEEF; // STM32F407

  NVIC_SystemReset();
}

/* Reboot PTC */
void rebootPTC(){
	printf("[BOOTLOADER]Rebooting PTC..\n");
	LCD_printLine(0,"Reiniciando..   ");
	LCD_printLine(1,"                ");
	delayMsUsingSysTick(1000);
	NVIC_SystemReset();
}

/**
  * @brief  This function handles USART2 global interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void){
	onUSART2Interrupt();
}
