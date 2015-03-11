/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SMTEST_H
#define __SMTEST_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "MPX/mpx.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#ifndef SM_PARAM
#define SM_PARAM

typedef struct {
	int state;
	int event;
	int next;
	void (*fn)(void);

} Transition;

typedef struct {
	int state;
	int event;
	int reset;

  Transition* trans;

  int transCount;

} StateMachine;

#endif /* SM_PARAM */

typedef enum {
	TST_ANY   = -1,
	TST_IDLE = 0,

	/* MPX's states */
	TST_MPX_TEST,
	TST_MPX_TEST_ID1, 	TST_MPX_TEST_ID2, 	TST_MPX_TEST_ID4, 	TST_MPX_TEST_ID0,
	TST_MPX_TEST_P0_L, 	TST_MPX_TEST_P1_L,	TST_MPX_TEST_P2_L,  TST_MPX_TEST_P3_L,
	TST_MPX_TEST_P0_H, 	TST_MPX_TEST_P1_H,	TST_MPX_TEST_P2_H,	TST_MPX_TEST_P3_H,
	TST_MPX_TEST_END,

	/* PTC24's states */
	TST_PTC24_TEST
} TestsStStates;

typedef enum {
	TEV_ANY   = -1,
	TEV_REFRESH =  0,
	TEV_TEST_END,
	TEV_AUTOMATIC,
	TEV_LOOP,

	/* MPX's events */
	TEV_MPX_TEST,
	TEV_TEST_ID1,
	TEV_TEST_ID2,
	TEV_TEST_ID4,
	TEV_TEST_ID0,

	/* PTC24's events */
	TEV_PTC24_TEST

} TestsStEvents;

/* Exported functions ------------------------------------------------------- */
void test_vStateMachineInit(void);
void test_vStateMachineLoop(void);
void test_vSetNextEvent(TestsStEvents);
void test_vJumpToState(TestsStStates);
TestsStStates test_vGetState(void);

#endif /* __SMTEST_H */
