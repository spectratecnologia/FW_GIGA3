#include "virtual_keyboard.h"

static uint8_t numOfConfiguredKeys = 0;

static VirtualKeyboard virtualKeyboard[MAX_NUM_KEYS];

void initVitualKeyboard () {

	int i = 0;

	initKeys();

	numOfConfiguredKeys = 4;

	/*
	 * Keys configured with two states. (pressed and not-pressed)
	 */
	for(i=0; i<numOfConfiguredKeys; i++) {
		virtualKeyboard[i].numStates = 1;
		virtualKeyboard[i].currentState = 0;
		virtualKeyboard[i].configParameter = 0;
	}
}

inline uint8_t getNumConfiguredKeys() {
	return numOfConfiguredKeys;
}

inline void setVirtualKeyState(uint8_t key, uint8_t state) {
	virtualKeyboard[key].currentState = state;
}

inline uint8_t getVirtualKeyState(uint8_t key) {
	return virtualKeyboard[key].currentState;
}

inline void nextVirtualKeyState(uint8_t key, bool beep) {
	if (virtualKeyboard[key].currentState==virtualKeyboard[key].numStates) {
		virtualKeyboard[key].currentState=0;
	}
	else {
		virtualKeyboard[key].currentState++;
	}
}
