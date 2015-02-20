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

void teste () {
	//uint8_t highMPXdeviceports[8]={0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//deactiveallMPXports();
	//sendCanPacket(CAN1, CAN_COMMAND_WRITE, 1, MY_ID, 0x82, &highMPXdeviceports, 8);
	activeMPXports(4, PORT_HIGH);

	}

int main(void)
{
	/* Functions already working - Pedro */
	initUSART();
	initUSBVCP();
	initRTC();
	initMultiTask();
	initSPIs();
	initMPXconfig();
	initLCD();
	LCD_vStateMachineInit();
	initIOs();
	initVitualKeyboard();

	/* --------------------------------- */
	initCANs();
	initBeepIO();


	//activeMPXdeviceports(0, PORT_HIGH);

	//processBeeps();

    while(1)
    {
    	executeEveryInterval(0, 1000, &toggleCPULED);

    	executeEveryInterval(1, 10, &processBeeps);

    	executeEveryInterval(2, 50, &processKeysAndDeadTime);

    	executeEveryInterval(3, 50, &processLCD);

    	//executeEveryInterval(1, 60000, &teste);


    	//executeEveryInterval(3, 1000, &teste);
    }
}

void processLCD()
{
	LCD_vStateMachineLoop();
}
