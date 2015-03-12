/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SMTEST_H
#define __SMTEST_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "MPX/mpx.h"
#include "LCD/lcd_screen.h"
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

/* Exported functions ------------------------------------------------------- */
void test_vStateMachineInit(void);
void test_vStateMachineLoop(void);
void test_vSetNextEvent(StEvents);
void test_vJumpToState(StStates);
int test_vGetState(void);

#endif /* __SMTEST_H */
