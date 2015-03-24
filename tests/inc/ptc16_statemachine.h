/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PTC16_STATEMACHINE_H
#define __PTC16_STATEMACHINE_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "ptc16.h"
#include "LCD/lcd_screen.h"

/* Exported types ------------------------------------------------------------*/
#define NUM_PTC16_KEYS 12
#define DELAY_FAST_TESTS 50
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

typedef enum {
	PTC16_ST_ANY   = -1,
	PTC16_ST_IDLE = 0,

	/* MPX's states */
	PTC16_ST_EXECUTE,
	PTC16_ST_WAIT,
	PTC16_ST_PRINT_WAIT,
	PTC16_ST_ANALYSE,
	PTC16_ST_FINALIZE,
	PTC16_ST_PRINT,
	PTC16_ST_FINISH
} Ptc16TestStates;

typedef enum
{
	PTC16_EV_ANY   = -1,
	PTC16_EV_REFRESH =  0,

	PTC16_EV_IDLE,
	PTC16_EV_EXECUTE,
	PTC16_EV_WAIT,
	PTC16_EV_ANALYSE,
	PTC16_EV_FINALIZE,
	PTC16_EV_PRINT,
	PTC16_EV_FINISH
} Ptc16TestEvents;

typedef enum
{
	PTC16_TEST_NOTHING,
	PTC16_TEST_START,
	PTC16_TEST_IGN,
	PTC16_TEST_TACO_OFF, PTC16_TEST_TACO_ON,
	PTC16_TEST_ODO_OFF, PTC16_TEST_ODO_ON,
	PTC16_TEST_KEYS_ON,
	PTC16_TEST_WARNINGLEDS_ON,
	PTC16_TEST_BUZZER,
	PTC16_TEST_KEYS_OFF,
	PTC16_TEST_WARNINGLEDS_OFF,
	PTC16_TEST_END
} Ptc16TestList;

/* This struct handles data about MPX tests execution */
typedef struct
{
	uint64_t statedTestTime;

	uint8_t currentTest;

	uint8_t keysOn[NUM_PTC16_KEYS];
	uint8_t keysOff[NUM_PTC16_KEYS];

	bool testOk;
	bool testFail;

	bool testFinished;
	bool testError;
	bool seriousError;
	/* This boolean shows if finished test beep occurred (true) or not (false). */
	bool finishedTestBeep;
} StructPtc16Test;

/* Exported macro ------------------------------------------------------------*/

/* Exported declared types -------------------------------------------------- */

/* Exported functions ------------------------------------------------------- */
void ptc16Test_vStateMachineLoop(void);
void ptc16Test_vStateMachineInit(void);
void ptc16Test_vSetTest(Ptc16TestList);
void ptc16Test_vTestOk(void);
void ptc16Test_vTestFail(void);
void ptc16Test_vFinishTest(void);

#endif /* PTC16_STATEMACHINE_H */
