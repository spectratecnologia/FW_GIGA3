#include <stdio.h>
#include <stdbool.h>

#include "user_tactkeys.h"
//#include "ios/ios.h"
#include "multitask.h"
#include "virtual_keyboard.h"

void readCurrentTactsStatus(void);

static KeyPress tactPressTable[TACTS_NUM_USER_KEYS] = { {0, 0, false, 0, 0},
														{0, 0, false, 0, 0},
														{0, 0, false, 0, 0},
														{0, 0, false, 0, 0},
													  };
static uint8_t tactInputStatus[4];
uint32_t getDeadTime(uint8_t);
bool isTactInputPressed(uint8_t);
void initTactButtons();

void initKeys ()
{
	/* Redundancy, this function is already invoked at initIOS(). However this redundancy prevents
	 * IO miss configuration if initIOS is missed or in wrong order.
	 */
	initTactButtons();
	enableTactShortDeadTime(false);

}

void initTactButtons()
{
    GPIO_InitTypeDef        gpio_InitTypeDef;

    /* Enable GPIOs clock  */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    /* Configure PD4 (TACTL), input with Pull-Up */
    gpio_InitTypeDef.GPIO_Pin =  IO_TACT_UP_PIN | IO_TACT_DOWN_PIN | IO_TACT_ENTER_PIN | IO_TACT_CANCEL_PIN;
    gpio_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_InitTypeDef.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IO_TACT_PORT, &gpio_InitTypeDef);
}

void enableTactShortDeadTime(bool state)
{
	tactPressTable[KEY_UP].shortDeadTimeEnabled = state;
	tactPressTable[KEY_DOWN].shortDeadTimeEnabled = state;
}

void processKeysAndDeadTime() {

	static uint64_t lastCall=0;

	int8_t i;

	uint64_t elapsedSinceLastCall=sysTickTimer-lastCall;

	lastCall = sysTickTimer;

	readCurrentTactsStatus();

	uint32_t deadTime;

	for(i=0; i<TACTS_NUM_USER_KEYS; i++) {

		tactPressTable[i].shortDeadTimeScreen+=elapsedSinceLastCall;

		/*
		 * stuck timescreen counter in zero if no key has been
		 * pressed
		 */

		if(tactPressTable[i].triggerCounter == 0)
		{
			tactPressTable[i].shortDeadTimeScreen = 0;
			//printf("K %d - Scr %d\n", tactPressTable[i].shortDeadTimeScreen);
		}

		deadTime = getDeadTime(i);

		/* Process dead time in all keys */
		if(tactPressTable[i].state == DEAD_TIME)
		{
			tactPressTable[i].elapsedTime += elapsedSinceLastCall;
			if(tactPressTable[i].elapsedTime > deadTime)
			{
				//printf("K: %d - t: %d\n", i, tactPressTable[i].elapsedTime);
				tactPressTable[i].state = NOT_PRESSED;
				tactPressTable[i].elapsedTime = 0;
			}
		}

		/* Process not-pressed before needed PRESS_TIME */
		if(tactPressTable[i].state == PRESSED && (isTactInputPressed(i)==false)) {
			tactPressTable[i].state = NOT_PRESSED;
			tactPressTable[i].elapsedTime = 0;
			//printf("K %d - Not Pre\n", i);
		}

		/* Process pressed keys */
		if(isTactInputPressed(i)==true && tactPressTable[i].state ==  NOT_PRESSED) {
			tactPressTable[i].elapsedTime = 0;
			tactPressTable[i].state = PRESSED;
			//printf("K %d - Pr\n", i);
		}

		if(tactPressTable[i].state ==  NOT_PRESSED && (tactPressTable[i].shortDeadTimeScreen > SHORT_DEAD_TIME_SCREEN)) {
			tactPressTable[i].triggerCounter = 0;
			tactPressTable[i].shortDeadTimeScreen = 0;
			//printf("rst sdt\n");
		}

		/* wait for press time to trigger virtual key pressed */
		else if(isTactInputPressed(i)==true && tactPressTable[i].state == PRESSED) {
			tactPressTable[i].elapsedTime += elapsedSinceLastCall;
			if(tactPressTable[i].elapsedTime > PRESS_TIME) {

				//printf("[TACTS]Tact key %d pressed\n",i);

				//Add tact key status here - This function is only producer, for Someone must consume the key state and change it back to false.
				setVirtualKeyState(i, 1);

				if(tactPressTable[i].triggerCounter < MIN_KEY_TRIGGERS_TO_SHORT_TIME) /* avoid overflow */
					tactPressTable[i].triggerCounter++;

				tactPressTable[i].state = DEAD_TIME;
				//tactPressTable[i].shortDeadTimeScreen = 0;
				tactPressTable[i].elapsedTime = 0;
			}
		}

		if(tactPressTable[i].shortDeadTimeScreen > SHORT_DEAD_TIME_SCREEN && tactPressTable[i].triggerCounter < MIN_KEY_TRIGGERS_TO_SHORT_TIME)
			tactPressTable[i].triggerCounter = 0;
	}
}

uint32_t getDeadTime(uint8_t key) {
	if(tactPressTable[key].shortDeadTimeEnabled == false)
		return LONG_DEAD_TIME;

	if(tactPressTable[key].triggerCounter >= MIN_KEY_TRIGGERS_TO_SHORT_TIME)
		return SHORT_DEAD_TIME;

	return LONG_DEAD_TIME;
}

bool isTactInputPressed(uint8_t tactNumber)
{
	/* Active Low Input*/
	return tactInputStatus[tactNumber] == 0 ? true : false;
}

void readCurrentTactsStatus ()
{
    uint32_t i;
    for(i=KEY_UP; i<=KEY_ENTER; i++) {
         tactInputStatus[i] =  GPIO_ReadInputDataBit(IO_TACT_PORT, IO_TACT_UP_PIN << i);
         //printf("K %d - St: %d\n", i, tactInputStatus[i]);
     }
}



