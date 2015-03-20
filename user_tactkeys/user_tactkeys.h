#ifndef __TACTKEYS_H__
#define __TACTKEYS_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


#define TACTS_NUM_USER_KEYS 4

#define PRESS_TIME 10
#define LONG_DEAD_TIME 500
#define SHORT_DEAD_TIME 100

#define SHORT_DEAD_TIME_SCREEN LONG_DEAD_TIME*3+50
#define MIN_KEY_TRIGGERS_TO_SHORT_TIME 3

#define IO_TACT_PORT GPIOD
#define IO_TACT_UP_PIN GPIO_Pin_11
#define IO_TACT_DOWN_PIN GPIO_Pin_12
#define IO_TACT_CANCEL_PIN GPIO_Pin_13
#define IO_TACT_ENTER_PIN GPIO_Pin_14

#define NOT_PRESSED 0
#define PRESSED 1
#define DEAD_TIME 2

typedef struct {
	uint8_t state;
	uint32_t elapsedTime;
	bool shortDeadTimeEnabled;
	uint32_t shortDeadTimeScreen;
	uint8_t triggerCounter;
}KeyPress;

enum KEYS {
	KEY_UP = 0,
	KEY_DOWN = 1,
	KEY_ENTER = 2,
	KEY_CANCEL = 3
};

void initKeys();
void enableTactShortDeadTime(bool);
void processKeysAndDeadTime ();

#endif
