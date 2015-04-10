/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PTC16_STATEMACHINE_H
#define __PTC16_STATEMACHINE_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "ptc16.h"
#include "LCD/lcd_screen.h"

/* Exported types ------------------------------------------------------------*/
#define NUM_PTC16_KEYS 12
#define PTC16_DELAY_SLOW_TESTS 200
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
	PTC16_TEST_IGN_OFF, PTC16_TEST_IGN_ON,
	PTC16_TEST_CAN2,
	PTC16_TEST_TACO_OFF, PTC16_TEST_TACO_ON,
	PTC16_TEST_ODO_OFF, PTC16_TEST_ODO_ON,
	PTC16_TEST_PENDRIVE,
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

/*LCD USB Message state number */
#define LCD_USB_MSG_NEW_LOGS 						0
#define LCD_USB_MSG_ALL_LOGS 						1
#define LCD_USB_MSG_DONT_DISCONNECT_THE_PENDRIVE 	2
#define LCD_USB_MSG_SAVING 							3
#define LCD_USB_MSG_FINISHED 						4

#define LCD_USB_MSG_SAVE_TIMEOUT_ERROR				5
#define LCD_USB_MSG_SAVE_ERROR						6

/* Error Message Number */
/* Try to keep this error message with lowest value */
#define LCD_USB_MSG_ERROR_CANT_MOUNT  		7
#define LCD_USB_MSG_ERROR_CANT_CREATE_FILE	8
#define LCD_USB_MSG_ERROR_CANT_WRITE 		9
#define LCD_USB_MSG_ERROR_CANT_OPEN_FILE    10
#define LCD_USB_MSG_RM_ERROR 				11
/* Try to keep this error message with highest value */
#define LCD_USB_MSG_ERROR_CANT_CLOSE		12

#define LCD_USB_WAIT_MESSAGE				0x20


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
