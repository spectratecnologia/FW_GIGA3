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

int i=0;

void teste()
{
	uint32_t data = 0x0;
	uint8_t data2[4];
	uint8_t data3;

	//data = readDataFromSR();
	//data2[0] = (uint8_t)(data);
	//data2[1] = (uint8_t)(data<<8);
	//data2[2] = (uint8_t)(data<<16);
	//data2[3] = (uint8_t)(data<<24);

	for (i=0; i<32; i++)
	{
		data3=getPortStatus(i);
		sendCanPacket(CAN1, 0x00, 0x00, i, 0x00, &data3, 1);
	}
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
	mpxTest_vStateMachineInit();
	initCANs();

	//teste();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    	executeEveryInterval(1, 10, &processBeeps);

    	executeEveryInterval(2, 50, &processKeysAndDeadTime);

    	executeEveryInterval(3, 100, &processLCD);

    	executeEveryInterval(4, 10, &processTest);

    	if (mpx.MpxAlreadyInit)
    	{
    		executeEveryInterval(5, 80, &processTurningOffMpxEmergencyMode);
    		executeEveryInterval(6, 5, &sendChangedOutputsToMPXs);
    		//executeEveryInterval(8, 50, &teste);
    	}


    }
}

void processLCD()
{
	LCD_vStateMachineLoop();
}

void processTest()
{
	mpxTest_vStateMachineLoop();
}

void processTurningOffMpxEmergencyMode()
{
	turningOffMpxEmergencyMode();
}


