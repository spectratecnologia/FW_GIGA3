/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_SCREEN_H
#define __LCD_SCREEN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "HD44780.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
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

typedef enum {
	ST_ANY   = -1,
	ST_MENU_DEBUG,
	ST_ADJUST_TIME,
	ST_ADJUST_TEMPERATURE_0,
	ST_ADJUST_TEMPERATURE_1,
	ST_ADJUST_USB_MODE,
	ST_SAVE_LOG,
	ST_MAIN,
	ST_IDLE,
	ST_PAGE1,
	ST_PAGE2,
	ST_PAGE3,
	ST_PAGE4,
	ST_PAGE5,
	ST_PAGE6,
	ST_PAGE7,
	ST_PAGE8,
	ST_PAGE9,
	ST_PAGE10,
	ST_PAGE11,

}StStates;

typedef enum {
	EV_ANY   = -1,
	EV_REFRESH =  0,

	EV_LINE1 = 1, //On enter events
	EV_LINE2 = 2,
	EV_LINE3 = 3,
	EV_LINE4 = 4,
	EV_LINE5 = 5,
	EV_LINE6 = 6,
	EV_LINE7 = 7,
	EV_LINE8 = 8,
	EV_LINE9 = 9,
	EV_LINE10 = 10,
	EV_LINE11 = 11,

	EV_LINE1_TEMPERATURE,
	EV_BACK_TO_MAIN,
	EV_NEXT_FIELD,
	EV_PREVIOUS_FIELD,
	EV_UP,
	EV_DOWN,

	EV_KBD_DOWN,
	EV_KBD_CANCEL,
	EV_KBD_UP
}StEvents;

typedef enum {
	LINE0_TITLE = 0,
	LINE1_TIME = 1,
	LINE2_TEMP_0 = 2,
	LINE3_TEMP_1 = 3,
	LINE4_FIRMWARE = 4
};

#define LINE_SIZE 17

/* Menu Keys */
#define KEY_CANCEL 12
#define KEY_UP 13
#define KEY_DOWN 14
#define KEY_ENTER 15

typedef enum {
	USB_SAVE_MODE_NOT_CHOSEN = 0,
	USB_SAVE_NEW = 1,
	USB_SAVE_ALL = 2
};

/* Exported functions ------------------------------------------------------- */

void startDebugMode();
bool isInDebugMode();

void LCD_vStateMachineInit();
void LCD_vStateMachineLoop();

void LCD_vSetNextEvent(StEvents event);
void LCD_vJumpToState(StStates state);

void LCD_printLine(uint8_t line, const char *string);

bool hasErrorToDisplay();
void displayErrorMessage(char *firstLine, char *secondLine, uint32_t displayTime, uint8_t priority);

void setLCDTemperature(int temperature);

/* Exported functions related to save log menu  ------------------------------*/

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
#define LCD_USB_MSG_ERROR_CANT_OPEN_FILE     10
#define LCD_USB_MSG_RM_ERROR 				11
/* Try to keep this error message with highest value */
#define LCD_USB_MSG_ERROR_CANT_CLOSE		12
/* End of error messages number definition */

/* Error Msg timeout */
#define MSG_SAVING_TIMEOUT_TO_ERROR 300000
#define MSG_DONT_DISCONNECT_TIMEOUT 2000
#define MSG_FINISHED_TIMEOUT		2000
#define MSG_ERROR_TIMEOUT			2000

void LCD_vLogSaveStart();
void LCD_vLogSaveDone();
void LCD_vLogSaveInterrupted(uint8_t reason);
void LCD_vSetProgressBar (uint8_t progress);
bool LCD_vLogSaveCanceled ();

/* IRQ  ----------------------------------------------------------*/

void TIM2_IRQHandler(void);

#endif /* __LCD_SCREEN_H */
