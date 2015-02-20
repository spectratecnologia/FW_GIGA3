/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SMTEST_H
#define __SMTEST_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
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
	ST_ANY   = -1,
	ST_IDLE = 0,

	/* MPX's states */
	ST_MPX_TEST,
	ST_MPX_TEST_ID0,
	ST_MPX_TEST_ID1,
	ST_MPX_TEST_ID2,
	ST_MPX_TEST_ID4,
	ST_MPX_TEST_END,

	/* PTC24's states */
	ST_PTC24_TEST
} TestsStStates;

typedef enum {
	EV_ANY   = -1,
	EV_REFRESH =  0,
	EV_TEST_END,
	EV_AUTOMATIC,
	EV_LOOP,
	EV_MANUAL,

	/* MPX's events */
	EV_MPX_TEST,
	EV_TEST_ID0,
	EV_TEST_ID1,
	EV_TEST_ID2,
	EV_TEST_ID4,

	/* PTC24's events */
	EV_PTC24_TEST

} TestsStEvents;

/* Exported functions ------------------------------------------------------- */
void  test_vStateMachineLoop(void);
void  test_vSetNextEvent(TestsStEvents);

#endif /* __SMTEST_H */
