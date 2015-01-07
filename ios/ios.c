#include "ios.h"
#include "multitask/multitask.h"
//#include "functions/functions.h"
//#include "functions/functions_config.h"


/* Struct to store ADC conversions using DMA.
 * The  Rank parameter of ADC_RegularChannelConfig  **MUST** be
 * the same sequence of declared struct members
 */
volatile static struct {
	uint16_t v_vbb;
	uint16_t v_temp;
}adcValues;

static uint8_t tactInputStatus[4];

void initADCs();
void initStatusLeds();
void initIgnitionCmd();
void initTactButtonsStatus();
void initBeep();

void initIOs(){
#if DEBUG != 0
	printf("[IOs]Starting Setup\n");
#endif
	initStatusLeds();

	initBeep();

	initIgnitionCmd();

	initTactButtonsStatus();

	initADCs();
#if DEBUG != 0
	printf("[IOs]Setup complete\n");
#endif
}

void initStatusLeds(){
	GPIO_InitTypeDef 		gpio_InitTypeDef;
	
	/* Enable GPIOs clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	/* Configure LEDs PD0 (CPU), PD1(LD1) and PD2(LD2) in output pushpull mode */
	gpio_InitTypeDef.GPIO_Pin = IO_STATUS_LED_CPU_PIN;
	gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(IO_STATUS_LED_CPU_PORT, &gpio_InitTypeDef);
}

void initBeep(){
	GPIO_InitTypeDef 		gpio_InitTypeDef;

	/* Enable GPIOs clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	/* PD3(BEEPH) in output pushpull mode */
	gpio_InitTypeDef.GPIO_Pin =  IO_BEEP_PIN;
	gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(IO_BEEP_PORT, &gpio_InitTypeDef);
}

void initIgnitionCmd(){
	GPIO_InitTypeDef 		gpio_InitTypeDef;

	/* Enable GPIOs clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure PB11 (IGNI_CPU)  in output push-pull mode */
	gpio_InitTypeDef.GPIO_Pin =  IO_IGNITION_CMD1_PIN | IO_IGNITION_CMD2_PIN;
	gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(IO_IGNITION_CMD_PORT, &gpio_InitTypeDef);
}

void initTactButtonsStatus(){
	GPIO_InitTypeDef 		gpio_InitTypeDef;

	/* Enable GPIOs clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	/* Configure PD4 (TACTL), input with Pull-Up */
	gpio_InitTypeDef.GPIO_Pin =  IO_TACT_UP_PIN | IO_TACT_DOWN_PIN | IO_TACT_ENTER_PIN | IO_TACT_CANCEL_PIN;
	gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(IO_TACT_PORT, &gpio_InitTypeDef);
}

void initADCs(){
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	/* Enable DMA RCC clock  */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adcValues;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	/* Set ADC clock preescaler */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);

	GPIO_InitTypeDef 		gpio_InitTypeDef;

	/* Enable GPIOs clock  */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure PD4 (TACTL), input with Pull-Up */
	gpio_InitTypeDef.GPIO_Pin = IO_ADC_VBB_PIN | IO_ADC_TEMP_PIN;
	gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(IO_TACT_PORT, &gpio_InitTypeDef);

	/* Enable ADC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular ADC_Channel_2, ADC_Channel_3, ADC_Channel_8 and ADC_Channel_9.
	 * Value are stored in adcValues struct
	 * The  Rank parameter of ADC_RegularChannelConfig  **MUST** be the same sequence of declared struct members*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_28Cycles5); /* v_vbb */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_28Cycles5); /* v_temp */

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable ADC1 reset calibration register */
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));

	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

void setStatusLed(){
	GPIO_WriteBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN,Bit_SET);
}

void resetStatusLed(){
	GPIO_WriteBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN,Bit_RESET);
}

bool getCPULEDstatus(){
	return GPIO_ReadInputDataBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN);
}

void toggleStatusLed(){
	GPIO_WriteBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN,!GPIO_ReadInputDataBit(IO_STATUS_LED_CPU_PORT,IO_STATUS_LED_CPU_PIN));
}

void readCurrentTactsStatus () {
	 for(i=IO_TACT_UP; i<=IO_TACT_CANCEL; i++) {
		 tactInputStatus[i] =  GPIO_ReadInputDataBit(IO_TACT_PORT, IO_TACT_UP_PIN << i);
	 }
}

/** Test Functions */
void toggleBeep () {
	setBeepOutput(!GPIO_ReadInputDataBit(IO_BEEP_PORT, IO_BEEP_PIN));
}

void toggleCPULED(){
	toggleStatusLed();
}

void setBeepOutput(bool beepOn){
	GPIO_WriteBit(IO_BEEP_PORT,IO_BEEP_PIN,beepOn);
}

float convert12bitsAdcTo3_3(uint16_t adcValue){
	return ((float)adcValue) * (3.3/4096.0);
}

float getTemperatureVoltage(){
	return convert12bitsAdcTo3_3(adcValues.v_temp);
}

float getVbbVoltage(){
	return convert12bitsAdcTo3_3(adcValues.v_vbb);
}

float getVvext1Voltage(){
	return convert12bitsAdcTo3_3(adcValues.v_vext1);
}

float getVvext2Voltage(){
	return convert12bitsAdcTo3_3(adcValues.v_vext2);
}

bool isTactInputPressed(uint8_t tactNumber) {
	/* Active Low Input*/
	return tactInputStatus[tactNumber] == 0 ? true : false;
}

