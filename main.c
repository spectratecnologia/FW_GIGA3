#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"
#include "can/can.h"
#include "rtc/rtc.h"
#include "LCD/lcd_screen.h"
#include "beep/beep.h"
#include "virtual_keyboard/virtual_keyboard.h"
#include "usb/user/usbd_cdc_vcp.h"
#include "MPX/mpx.h"

void processLCD();
void processTurningOffMpxEmergencyMode();
void processTest();
void processAnalysisTest();

void teste()
{
	uint32_t data=0xFEDCBA98;
	uint16_t data2;
	uint8_t candata[8]={0,0,0,0,0,0,0,0};
	uint8_t candata1[8]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8_t candata2[8]={0,0,0,0,0,0,0,0};
	uint8_t portx;

	initMPXconfig();


	portx=22;

	//sendDataToSR(0x11);
	activeMPXports(portx, PORT_OFF);

	//sendDataToSR(0xFFFFFFFF);
    data = readDataFromSR();

	candata[4]=(uint8_t)(data>>24);
	candata[5]=(uint8_t)(data>>16);
	candata[6]=(uint8_t)(data>>8);
	candata[7]=(uint8_t)(data>>0);

	sendCanPacket(CAN1, 0xAA, 0xBB, MY_ID, 0x00, &candata, 8);

	if (getPortStatus(portx))
		sendCanPacket(CAN1, 0xAA, 0xBC, MY_ID, 0x00, &candata1, 8);
	else
		sendCanPacket(CAN1, 0xAA, 0xBC, MY_ID, 0x00, &candata2, 8);
}

int main(void)
{
	/* Functions already working - Pedro */
	initUSART();
	initUSBVCP();
	initRTC();
	initMultiTask();
	initBeepIO();
	initLCD();
	initIOs();
	LCD_vStateMachineInit();
	initVitualKeyboard();
	test_vStateMachineInit();
	initCANs();

	//teste();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    	executeEveryInterval(1, 10, &processBeeps);

    	executeEveryInterval(2, 50, &processKeysAndDeadTime);

    	executeEveryInterval(3, 100, &processLCD);

    	executeEveryInterval(4, 10, &processTest);

    	executeEveryInterval(5, 10, &processAnalysisTest);

    	if (mpx.MpxAlreadyInit)
    	{
    		executeEveryInterval(6, 80, &processTurningOffMpxEmergencyMode);
    		executeEveryInterval(7, 40, &sendChangedOutputsToMPXs);
    	}

    	executeEveryInterval(8, 2000, &teste);
    }
}

void processLCD()
{
	LCD_vStateMachineLoop();
}

void processTurningOffMpxEmergencyMode()
{
	turningOffMpxEmergencyMode();
}

void processTest()
{
	test_vStateMachineLoop();

	if (mpx.MpxAlreadyInit == true)
		executeTest_MPX();
}

void processAnalysisTest()
{
	if (mpx.MpxAlreadyInit == true)
		analysisTest_MPX();
}


