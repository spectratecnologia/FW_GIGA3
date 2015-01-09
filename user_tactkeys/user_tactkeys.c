#include "user_tactkeys.h"
#include "ios/ios.h"
#include "multitask/multitask.h"

static KeyPress tactPressTable[TACTS_NUM_USER_KEYS] = { 0, 0, 0, 0 };

uint32_t getDeadPeriod(uin8_t key);

void initTactKeys () {

#if DEBUG != 0
	printf("[TACTKEYS]Tactskeys Setup\n");
#endif

	/* Redundancy, this function is already invoked at initIOS(). However this redundancy prevents
	 * IO miss configuration if initIOS is missed or in wrong order.
	 */
	initTactButtons();

	tactPressTable[KEY_UP].shortDeadTimeEnabled = true;
	tactPressTable[KEY_DOWN].shortDeadTimeEnabled = true;

#if DEBUG != 0
	printf("[TACTKEYS]Setup complete\n");
#endif
}

void processTactKeysAndDeadTime() {

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
			tactPressTable[i].shortDeadTimeScreen = 0;

		deadTime = getDeadPeriod(i);
		/* Process dead time in all keys */
		if(tactPressTable[i].state == DEAD_TIME) {
			tactPressTable[i].elapsedTime += elapsedSinceLastCall;
			if(tactPressTable[i].elapsedTime > deadTime) {
				tactPressTable[i].state = NOT_PRESSED;
				tactPressTable[i].elapsedTime = 0;
			}
		}

		/* Process not-pressed before needed PRESS_TIME */
		if(tactPressTable[i].state == PRESSED && (isTactInputPressed(i)==false)) {
			tactPressTable[i].state == NOT_PRESSED;
			tactPressTable[i].elapsedTime = 0;
		}

		/* Process pressed keys */
		if(isTactInputPressed(i)==true && tactPressTable[i].state ==  NOT_PRESSED) {
			tactPressTable[i].elapsedTime = 0;
			tactPressTable[i].state = PRESSED;
		}

		if(tactPressTable[i].state ==  NOT_PRESSED && tactPressTable[i].shortDeadTimeScreen > SHORT_DEAD_TIME_SCREEN) {
			tactPressTable[i].triggerCounter = 0;
			tactPressTable[i].shortDeadTimeScreen = 0;
		}

		/* wait for press time to trigger virtual key pressed */
		else if(isTactInputPressed(i)==true && tactPressTable == PRESSED) {
			tactPressTable[i].elapsedTime += elapsedSinceLastCall;
			if(tactPressTable[i].elapsedTime > PRESS_TIME) {

				printf("[TACTS]Tact key %d pressed\n",i);

				//Add tact key status here - This function is only producer, for Someone must consume the key state and change it back to false.

				if(tactPressTable[i].triggerCounter < MIIN_KEY_TRIGGERS_TO_SHORT_TIME) /* avoid overflow */
					tactPressTable[i].triggerCounter++;

				tactPressTable[i].state = deadTime;
				tactPressTable[i].elapsedTime = 0;

			}
		}

		if(tactPressTable[i].shortDeadTimeScreen > SHORT_DEAD_TIME_SCREEN && tactPressTable[i].triggerCounter < MIIN_KEY_TRIGGERS_TO_SHORT_TIME)
			tactPressTable[i].triggerCounter = 0;
	}
}

uint32_t getDeadPeriod(uin8_t key) {
	if(tactPressTable[key].shortDeadTimeEnabled == false)
		return LONG_DEAD_TIME;

	if(tactPressTable[key].triggerCounter >= MIIN_KEY_TRIGGERS_TO_SHORT_TIME)
		return SHORT_DEAD_TIME;

	return LONG_DEAD_TIME;
}



