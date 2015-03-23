/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PTC24_STATEMACHINE_H
#define __PTC24_STATEMACHINE_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "devices/PTC24/ptc24.h"
#include "LCD/lcd_screen.h"

/* Exported types ------------------------------------------------------------*/
#define NUM_PTC24_KEYS 24
#define DELAY_FAST_TESTS 50
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

typedef enum {
	PTC24_ST_ANY   = -1,
	PTC24_ST_IDLE = 0,

	/* MPX's states */
	PTC24_ST_EXECUTE,
	PTC24_ST_WAIT,
	PTC24_ST_PRINT_WAIT,
	PTC24_ST_ANALYSE,
	PTC24_ST_FINALIZE,
	PTC24_ST_PRINT,
	PTC24_ST_FINISH
} Ptc24TestStates;

typedef enum
{
	PTC24_EV_ANY   = -1,
	PTC24_EV_REFRESH =  0,

	PTC24_EV_IDLE,
	PTC24_EV_EXECUTE,
	PTC24_EV_WAIT,
	PTC24_EV_ANALYSE,
	PTC24_EV_FINALIZE,
	PTC24_EV_PRINT,
	PTC24_EV_FINISH
} Ptc24TestEvents;

typedef enum
{
	PTC24_TEST_NOTHING,
	PTC24_TEST_START,
	PTC24_TEST_IGN,
	PTC24_TEST_TACO_OFF, PTC24_TEST_TACO_ON,
	PTC24_TEST_ODO_OFF, PTC24_TEST_ODO_ON,
	PTC24_TEST_KEYS_ON,
	PTC24_TEST_WARNINGLEDS_ON,
	PTC24_TEST_BUZZER,
	PTC24_TEST_KEYS_OFF,
	PTC24_TEST_WARNINGLEDS_OFF,
	PTC24_TEST_END
} Ptc24TestList;

/* This struct handles data about MPX tests execution */
typedef struct
{
	uint64_t statedTestTime;

	uint8_t currentTest;

	uint8_t keysOn[NUM_PTC24_KEYS];
	uint8_t keysOff[NUM_PTC24_KEYS];

	bool testOk;
	bool testFail;

	bool testFinished;
	bool testError;
	bool seriousError;
	/* This boolean shows if finished test beep occurred (true) or not (false). */
	bool finishedTestBeep;
} StructPtc24Test;

/* Exported macro ------------------------------------------------------------*/

/* Exported declared types -------------------------------------------------- */

/* Exported functions ------------------------------------------------------- */
void ptc24Test_vStateMachineLoop(void);
void ptc24Test_vStateMachineInit(void);
void ptc24Test_vSetTest(Ptc24TestList);
void ptc24Test_vTestOk(void);
void ptc24Test_vTestFail(void);
void ptc24Test_vFinishTest(void);

#endif /* __SMTEST_H */
