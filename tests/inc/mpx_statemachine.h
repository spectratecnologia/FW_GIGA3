/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MPX_STATEMACHINE_H
#define __MPX_STATEMACHINE_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "mpx.h"
#include "LCD/lcd_screen.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DELAY_TO_RECEIVE_NEW_MEMORY_ACK 100
#define DELAY_TO_CAN_SEND_DATA 100
#define DELAY_TO_ANALISE_FLASH_TEST 5000
#define DELAY_TO_ANALISE_SLOW_TEST 1500
#define DELAY_TO_ANALISE_FAST_TEST 65
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
	MPX_ST_PRINT,
	MPX_ST_FINISH
} TestStStates;

typedef enum
{
	MPX_EV_ANY   = -1,
	MPX_EV_REFRESH =  0,

	MPX_EV_EXECUTE,
	MPX_EV_IDLE,
	MPX_EV_WAIT,
	MPX_EV_ANALYSE,
	MPX_EV_FINALIZE,
	MPX_EV_PRINT,
	MPX_EV_FINISH
} TestStEvents;

typedef enum
{
	TEST_AUTO,
	TEST_LOOP,
	TEST_NOTHING,
	TEST_FLASH,
	TEST_ID1,     TEST_ID2,     TEST_ID4,     TEST_ID0,
	TEST_IGN_L,   TEST_IGN_H,
	TEST_NTC,
	TEST_P0_L,    TEST_P1_L,    TEST_P2_L, 	  TEST_P3_L,
	TEST_P0_H,    TEST_P1_H,    TEST_P2_H,    TEST_P3_H,
	TEST_P4,      TEST_P5,      TEST_P6,      TEST_P7,
	TEST_P8,      TEST_P9,      TEST_P10,     TEST_P11,
	TEST_P12,     TEST_P13,     TEST_P14,     TEST_P15,
	TEST_P16,     TEST_P17,     TEST_P18,     TEST_P19,
	TEST_P20,     TEST_P21,     TEST_P22,     TEST_P23,
	TEST_P24,     TEST_P25,     TEST_P26,     TEST_P27,
	TEST_P28,     TEST_P29,     TEST_P30,     TEST_P31,
	TEST_P32,     TEST_P33,     TEST_P34,     TEST_P35,

	/* Test if Those ports are in short-circuit with BIDI and PUSH-PULL ports */
	TEST_P0_H_LDIN,    TEST_P1_H_LDIN,    TEST_P2_H_LDIN,    TEST_P3_H_LDIN,
	TEST_P4_LDIN,      TEST_P5_LDIN,      TEST_P6_LDIN,      TEST_P7_LDIN,
	TEST_P8_LDIN,      TEST_P9_LDIN,      TEST_P10_LDIN,     TEST_P11_LDIN,
	TEST_P12_LDIN,     TEST_P13_LDIN,     TEST_P14_LDIN,     TEST_P15_LDIN,
	TEST_P16_LDIN,     TEST_P17_LDIN,     TEST_P18_LDIN,     TEST_P19_LDIN,
	TEST_P20_LDIN,     TEST_P21_LDIN,     TEST_P22_LDIN,     TEST_P23_LDIN,
	TEST_P24_LDIN,     TEST_P25_LDIN,     TEST_P26_LDIN,     TEST_P27_LDIN,

	TEST_END
} TestList;

/* This struct handles data about MPX tests execution */
typedef struct
{
	int16_t currentTest;
	uint8_t switchPort;
	uint8_t shortCircuitPort;
	uint64_t statedTestTime;
	uint64_t startedMemoryAnalyseTime;
	bool boolIsAutoTest;
	bool boolIsLoopTest;
	bool testFinished;

	bool backToMainInIdle;
	/* This boolean shows if finished test beep occurred (true) or not (false). */
	bool finishedTestBeep;
	bool testError;
	bool seriousError;
	bool boolEraseFlash;
	uint16_t numberTestDone;
} Test;

/* Exported macro ------------------------------------------------------------*/

/* Exported declared types -------------------------------------------------- */

/* Exported functions ------------------------------------------------------- */
void mpxTest_vStateMachineLoop(void);
void mpxTest_vStateMachineInit(void);
void mpxTest_vSetTest(TestList);
void mpxTest_vContinueTest(void);
void mpxTest_vFinishTest(void);
int mpxTest_vGetTest(void);

#endif /* __SMTEST_H */
