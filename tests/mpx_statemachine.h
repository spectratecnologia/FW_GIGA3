/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MPX_STATEMACHINE_H
#define __MPX_STATEMACHINE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "tests/tests_statemachine.h"
#include "LCD/lcd_screen.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DELAY_TO_ANALISE_SLOW_TEST 3000
#define DELAY_TO_ANALISE_FAST_TEST 50
/* Exported macro ------------------------------------------------------------*/

typedef enum {
	MPX_ST_ANY   = -1,
	MPX_ST_IDLE = 0,

	/* MPX's states */
	MPX_ST_EXECUTE,
	MPX_ST_WAIT,
	MPX_ST_PRINT_WAIT,
	MPX_ST_ANALYSE,
	MPX_ST_FINALIZE,
	MPX_ST_PRINT

} MpxTestsStStates;

typedef enum {
	MPX_EV_ANY   = -1,
	MPX_EV_REFRESH =  0,

	MPX_EV_IDLE,
	MPX_EV_EXECUTE,
	MPX_EV_WAIT,
	MPX_EV_ANALYSE,
	MPX_EV_FINALIZE,
	MPX_EV_PRINT

} MpxTestsStEvents;

/* This struct handles data about MPX tests execution */
typedef struct
{
	uint64_t statedTestTime;

	/* Currently MPX test */
	bool boolIsManualTest;
	bool boolIsAutoTest;
	bool boolIsLoopTest;

	bool testFinished;
	bool testError;

	/* Complete test number */
	uint16_t numberTestDone;
} MpxTests;

/* Exported macro ------------------------------------------------------------*/

/* Exported declared types -------------------------------------------------- */
MpxTests Mpxtests;

/* Exported functions ------------------------------------------------------- */
void test_vMpxStateMachineInit(void);
void mpxTest_vStateMachineLoop(void);
void mpxTest_vJumpToState(MpxTestsStStates);

void isManualTest(void);
void isAutoTest(void);
void isLoopTest(void);

#endif /* __SMTEST_H */
