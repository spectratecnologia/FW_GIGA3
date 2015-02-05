#ifndef __VIRTUAL_KEYBOARD_H__
#define __VIRTUAL_KEYBOARD_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "user_tactkeys/user_tactkeys.h"

#define MAX_NUM_KEYS 4

typedef struct {
	uint8_t configParameter;
	uint8_t numStates;
	uint8_t currentState;
}VirtualKeyboard;

inline void initVitualKeyboard();
inline uint8_t getNumConfiguredKeys();

inline void setVirtualKeyState(uint8_t key, uint8_t state);
inline uint8_t getVirtualKeyState(uint8_t key);

/*
 * Default interface functions to the low-level specific hardware functions.
 * These functions must be redefined elsewhere.
 */

extern void initKeys ();
extern void processKeysAndDeadTime ();



#endif
