/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MPX_STATEMACHINE_H
#define __MPX_STATEMACHINE_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "MPX/mpx.h"
#include "LCD/lcd_screen.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DELAY_TO_ANALISE_SLOW_TEST 1000
#define DELAY_TO_ANALISE_FAST_TEST 42
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

} TestStStates;

typedef enum
{
	MPX_EV_ANY   = -1,
	MPX_EV_REFRESH =  0,

	MPX_EV_IDLE,
	MPX_EV_EXECUTE,
	MPX_EV_WAIT,
	MPX_EV_ANALYSE,
	MPX_EV_FINALIZE,
	MPX_EV_PRINT

} TestStEvents;

typedef enum
{
	TEST_AUTO,
	TEST_LOOP,
	TEST_NOTHING,
	TEST_ID1, TEST_ID2,  TEST_ID4,  TEST_ID0,
	TEST_P0_L,  TEST_P1_L, TEST_P2_L, TEST_P3_L,
	TEST_P0_H,  TEST_P1_H, TEST_P2_H, TEST_P3_H,
	TEST_P4,    TEST_P5,   TEST_P6,   TEST_P7,
	TEST_END,
	TEST_P8,    TEST_P9,   TEST_P10,  TEST_P11,
	TEST_P12,   TEST_P13,  TEST_P14,  TEST_P15,
	TEST_P16,   TEST_P17,  TEST_P18,  TEST_P19,
	TEST_P20,   TEST_P21,  TEST_P22,  TEST_P23,
	TEST_P24,   TEST_P25,  TEST_P26,  TEST_P27

} TestList;

/* This struct handles data about MPX tests execution */
typedef struct
{
	int16_t currentTest;
	uint64_t statedTestTime;
	bool boolIsAutoTest;
	bool boolIsLoopTest;
	bool testError;
	uint16_t numberTestDone;
} Test;

/* Exported macro ------------------------------------------------------------*/

/* Exported declared types -------------------------------------------------- */

/* Exported functions ------------------------------------------------------- */
void mpxTest_vMpxStateMachineInit(void);
void mpxTest_vStateMachineInit(void);
void mpxTest_vSetTest(TestList);
void mpxTest_vResetTests(void);

#endif /* __SMTEST_H */
