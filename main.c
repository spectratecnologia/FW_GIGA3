#include "stm32f10x.h"

#include "usart/usart.h"
#include "multitask/multitask.h"
#include "ios/ios.h"
#include "can/can.h"
#include "rtc/rtc.h"
#include "beep/beep.h"
#include "virtual_keyboard/virtual_keyboard.h"
#include "usb/user/usbd_cdc_vcp.h"
#include "MPX/mpx.h"


void teste () {

	uint8_t data[8]={0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

	activeMPXports(0, PP10A_HIGH);
	//toggleCPULED();
}

int main(void)
{
	/* Functions already working - Pedro */
	initUSART();
	initUSBVCP();
	//initRTC();
	initMultiTask();
	//initSPIs();
	//initMPXconfig();
	//initLCD();
	initIOs();
	//initVitualKeyboard();
	/* --------------------------------- */
	//initCANs();

    while(1)
    {
    	//executeEveryInterval(0, 1000, &toggleCPULED);

  //  	GPIO_WriteBit(IO_BEEP_PORT,IO_BEEP_PIN, 1);

 //   	executeEveryInterval(1, 1, &processKeysAndDeadTime);

    	//executeEveryInterval(1, 5000, &teste);


    	//executeEveryInterval(3, 1000, &teste);
    }
}
