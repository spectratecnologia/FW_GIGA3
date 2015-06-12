#include "lcd_screen.h"
#include "multitask/multitask.h"
#include "rtc/rtc.h"

#include <time.h>

struct{
	int8_t sbLine;
	int8_t sbLineMax;
	int8_t sbLineMin;
} lcd;

StateMachine sm;

static volatile int8_t nextEvent;

static volatile bool showLCDAsTurnedOff=false;

static bool inDebugMode=false;

static volatile int ptcTemperature = 0;

static idioma=SPANISH;

bool isUSBHostMode=false;
uint8_t usbSaveMode=0;

static inline bool msgTimoutReached (uint32_t timeout);

void LCD_vMainScreen();
void LCD_vAdjustTime();
void LCD_vChooseLanguage(void);

void LCD_vTestMPX();
void LCD_vTestMPXAuto();
void LCD_vTestMPXAutoStart();
void LCD_vTestMPXLoop();
void LCD_vTestMPXLoopStart();
void LCD_vTestMPXManual();
void LCD_vTestMPXManualStart();

void LCD_vTestPTC24();
void LCD_vTestPTC24Start(void);
void LCD_vTestPTC16();
void LCD_vTestPTC16Start(void);

void LCD_vDisplayTestMessage();

void LCD_vMenuDebug();
void LCD_vADCScreen();
void LCD_vCANScreen();
void LCD_vRTCScreen();
void LCD_vIOsScreen();
void LCD_vTimingScreen();
void LCD_vUserSettings();
void LCD_vManualMode();
void LCD_vReboot();
void LCD_vUIDScreen();

/* Rearrange test sequence between LCD display and MPX state machine tests. */
const int RearrangedTests[] = {TEST_FLASH,								   //Line1
							   TEST_ID1,  TEST_ID2,  TEST_ID4,  TEST_ID0,  //Line2
							   TEST_IGN_L,								   //Line6
							   TEST_P0_L, TEST_P0_H, TEST_P8,   TEST_P1_L, //Line7
							   TEST_P1_H, TEST_P9,   TEST_P28,  TEST_P10,  //Line11
							   TEST_P11,  TEST_P29,  TEST_P12,  TEST_P2_L, //Line15
							   TEST_P2_H, TEST_P13,  TEST_P3_L, TEST_P3_H, //...
							   TEST_P14,  TEST_P30,	 TEST_P15,  TEST_P16,
							   TEST_P31,  TEST_P17,  TEST_P4,   TEST_P18,
							   TEST_P5,   TEST_P19,  TEST_P32,  TEST_P20,
							   TEST_P21,  TEST_P33,  TEST_P22,  TEST_P6,
							   TEST_P23,  TEST_P7,   TEST_P24,  TEST_P34,
							   TEST_P25,  TEST_P26,  TEST_P35,  TEST_P27,
							   TEST_NTC};

const Transition smTrans[] =  		//TABELA DE ESTADOS
{
/*Current state			Event				Next state				callback */
{ST_MAIN				,EV_REFRESH       	,ST_MAIN           		,&LCD_vMainScreen			},
{ST_MAIN				,EV_LINE1         	,ST_ADJUST_TIME    		,&LCD_vAdjustTime			},
{ST_MAIN				,EV_LINE2		  	,ST_TEST_MPX          	,&LCD_vTestMPX				},
{ST_MAIN				,EV_LINE3		  	,ST_TEST_PTC24			,&LCD_vTestPTC24    		},
{ST_MAIN				,EV_LINE4		  	,ST_TEST_PTC16			,&LCD_vTestPTC16    		},
{ST_MAIN				,EV_LINE5		  	,ST_LANGUAGE			,&LCD_vChooseLanguage  		},
{ST_MAIN				,EV_KBD_CANCEL    	,ST_MAIN           		,&LCD_vMainScreen			},
{ST_MAIN				,EV_ANY		       	,ST_MAIN           		,&LCD_vMainScreen			},

{ST_ADJUST_TIME			,EV_REFRESH         ,ST_ADJUST_TIME	 	  	,&LCD_vAdjustTime			},
{ST_ADJUST_TIME			,EV_PREVIOUS_FIELD  ,ST_ADJUST_TIME    		,&LCD_vAdjustTime			},
{ST_ADJUST_TIME			,EV_NEXT_FIELD      ,ST_ADJUST_TIME    		,&LCD_vAdjustTime			},
{ST_ADJUST_TIME			,EV_UP      		,ST_ADJUST_TIME    		,&LCD_vAdjustTime			},
{ST_ADJUST_TIME			,EV_DOWN     		,ST_ADJUST_TIME    		,&LCD_vAdjustTime			},
{ST_ADJUST_TIME			,EV_BACK_TO_MAIN    ,ST_MAIN    	   		,&LCD_vMainScreen			},

{ST_LANGUAGE			,EV_REFRESH         ,ST_LANGUAGE	 	  	,&LCD_vChooseLanguage		},
{ST_LANGUAGE			,EV_KBD_ENTER		,ST_MAIN   				,&LCD_vMainScreen			},
{ST_LANGUAGE			,EV_KBD_CANCEL      ,ST_MAIN   				,&LCD_vMainScreen			},
{ST_LANGUAGE			,EV_UP      		,ST_LANGUAGE	   		,&LCD_vChooseLanguage		},
{ST_LANGUAGE			,EV_DOWN     		,ST_LANGUAGE    		,&LCD_vChooseLanguage		},
{ST_LANGUAGE			,EV_BACK_TO_MAIN    ,ST_MAIN    	   		,&LCD_vMainScreen			},

{ST_TEST_MPX 		  	,EV_REFRESH			,ST_TEST_MPX          	,&LCD_vTestMPX				},
{ST_TEST_MPX 		  	,EV_LINE1		  	,ST_TEST_MPX_AUTO      	,&LCD_vTestMPXAuto			},
{ST_TEST_MPX 		  	,EV_LINE2		  	,ST_TEST_MPX_LOOP    	,&LCD_vTestMPXLoop			},
{ST_TEST_MPX 		  	,EV_LINE3		  	,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManual		},
{ST_TEST_MPX 		  	,EV_KBD_CANCEL		,ST_MAIN    	   		,&LCD_vMainScreen	      	},
{ST_TEST_MPX 		  	,EV_BACK_TO_MAIN	,ST_MAIN    	   		,&LCD_vMainScreen	      	},
{ST_TEST_MPX 		  	,EV_ANY				,ST_TEST_MPX          	,&LCD_vTestMPX				},

{ST_TEST_MPX_AUTO 	  	,EV_REFRESH			,ST_TEST_MPX_AUTO      	,&LCD_vTestMPXAuto			},
{ST_TEST_MPX_AUTO	  	,EV_KBD_ENTER		,ST_TEST_MPX_AUTO		,&LCD_vTestMPXAutoStart	    },
{ST_TEST_MPX_AUTO		,EV_TEST_LOG		,ST_TEST_LOG_MPX		,&LCD_vDisplayTestMessage	},
{ST_TEST_MPX_AUTO	 	,EV_KBD_CANCEL		,ST_TEST_MPX  	     	,&LCD_vTestMPX				},
{ST_TEST_MPX_AUTO	 	,EV_BACK_TO_MAIN	,ST_MAIN		     	,&LCD_vMainScreen			},
{ST_TEST_MPX_AUTO 	  	,EV_ANY				,ST_TEST_MPX_AUTO      	,&LCD_vTestMPXAuto			},

{ST_TEST_MPX_LOOP 	  	,EV_REFRESH			,ST_TEST_MPX_LOOP      	,&LCD_vTestMPXLoop			},
{ST_TEST_MPX_LOOP	  	,EV_KBD_ENTER		,ST_TEST_MPX_LOOP		,&LCD_vTestMPXLoopStart	    },
{ST_TEST_MPX_LOOP	 	,EV_TEST_LOG		,ST_TEST_LOG_MPX		,&LCD_vDisplayTestMessage	},
{ST_TEST_MPX_LOOP 		,EV_KBD_CANCEL		,ST_TEST_MPX  	     	,&LCD_vTestMPX				},
{ST_TEST_MPX_LOOP	 	,EV_BACK_TO_MAIN	,ST_MAIN		     	,&LCD_vMainScreen			},
{ST_TEST_MPX_LOOP 	  	,EV_ANY				,ST_TEST_MPX_LOOP      	,&LCD_vTestMPXLoop			},

{ST_TEST_LOG_MPX		,EV_REFRESH		  	,ST_TEST_LOG_MPX   		,&LCD_vDisplayTestMessage	},
{ST_TEST_LOG_MPX		,EV_KBD_ENTER	  	,ST_TEST_MPX   			,&LCD_vTestMPX				},
{ST_TEST_LOG_MPX		,EV_KBD_CANCEL	  	,ST_TEST_MPX   			,&LCD_vTestMPX				},
{ST_TEST_LOG_MPX		,EV_ANY			  	,ST_TEST_LOG_MPX   		,&LCD_vDisplayTestMessage	},

{ST_TEST_MPX_MANUAL	  	,EV_REFRESH	  		,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManual		},
{ST_TEST_MPX_MANUAL	  	,EV_TEST_LOG	  	,ST_TEST_LOG_MPX_M		,&LCD_vDisplayTestMessage	},
{ST_TEST_MPX_MANUAL	  	,EV_KBD_CANCEL		,ST_TEST_MPX  	     	,&LCD_vTestMPX				},
// Memory tests
{ST_TEST_MPX_MANUAL	  	,EV_LINE1		  	,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManualStart	},
// ID tests
{ST_TEST_MPX_MANUAL	  	,EV_LINE2		  	,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE3		  	,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE4		  	,ST_TEST_MPX_MANUAL 	,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE5		  	,ST_TEST_MPX_MANUAL 	,&LCD_vTestMPXManualStart	},
//Ignition tests
{ST_TEST_MPX_MANUAL	  	,EV_LINE6		  	,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
//Push pull 10A tests
{ST_TEST_MPX_MANUAL	  	,EV_LINE7		  	,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE8		  	,ST_TEST_MPX_MANUAL 	,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE9		  	,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE10		  	,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE11	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE12	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE13	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE14	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// BIDI 10A tests
{ST_TEST_MPX_MANUAL	  	,EV_LINE15	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE16	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE17	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE18	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// BIDI 2A tests #1
{ST_TEST_MPX_MANUAL	  	,EV_LINE19	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE20	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE21	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE22	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// BIDI 2A tests #2
{ST_TEST_MPX_MANUAL	  	,EV_LINE23	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE24	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE25	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE26	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// BIDI 2A tests #3
{ST_TEST_MPX_MANUAL	  	,EV_LINE27	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE28	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE29	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE30	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// BIDI 2A tests #4
{ST_TEST_MPX_MANUAL	  	,EV_LINE31	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE32	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE33	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE34	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// BIDI 2A tests #5
{ST_TEST_MPX_MANUAL	  	,EV_LINE35	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE36	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE37	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE38	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// LODIN #1
{ST_TEST_MPX_MANUAL	  	,EV_LINE39	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE40	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL   	,EV_LINE41	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL		,EV_LINE42	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// LODIN #2
{ST_TEST_MPX_MANUAL	  	,EV_LINE43	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE44	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE45	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
{ST_TEST_MPX_MANUAL	  	,EV_LINE46	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// TESTE NTC
{ST_TEST_MPX_MANUAL	  	,EV_LINE47	  		,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManualStart	},
// Ev. ANY and Back to Main
{ST_TEST_MPX_MANUAL 	,EV_BACK_TO_MAIN	,ST_MAIN		     	,&LCD_vMainScreen			},
{ST_TEST_MPX_MANUAL	  	,EV_ANY		  		,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManual		},

{ST_TEST_LOG_MPX_M		,EV_REFRESH		  	,ST_TEST_LOG_MPX_M 		,&LCD_vDisplayTestMessage	},
{ST_TEST_LOG_MPX_M		,EV_KBD_ENTER	  	,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManual		},
{ST_TEST_LOG_MPX_M		,EV_KBD_CANCEL	  	,ST_TEST_MPX_MANUAL		,&LCD_vTestMPXManual		},
{ST_TEST_LOG_MPX_M		,EV_ANY			  	,ST_TEST_LOG_MPX_M 		,&LCD_vDisplayTestMessage	},

{ST_TEST_PTC24 			,EV_REFRESH		  	,ST_TEST_PTC24          ,&LCD_vTestPTC24			},
{ST_TEST_PTC24 			,EV_KBD_CANCEL	  	,ST_MAIN    	   		,&LCD_vMainScreen			},
{ST_TEST_PTC24 			,EV_KBD_ENTER	  	,ST_TEST_PTC24 			,&LCD_vTestPTC24Start		},
{ST_TEST_PTC24 			,EV_TEST_LOG	  	,ST_TEST_LOG_PTC24 		,&LCD_vDisplayTestMessage	},
{ST_TEST_PTC24		 	,EV_BACK_TO_MAIN	,ST_MAIN		     	,&LCD_vMainScreen			},
{ST_TEST_PTC24 			,EV_ANY			  	,ST_TEST_PTC24          ,&LCD_vTestPTC24			},

{ST_TEST_LOG_PTC24		,EV_REFRESH		  	,ST_TEST_LOG_PTC24      ,&LCD_vDisplayTestMessage	},
{ST_TEST_LOG_PTC24		,EV_KBD_ENTER	  	,ST_TEST_LOG_PTC24      ,&LCD_vDisplayTestMessage	},
{ST_TEST_LOG_PTC24		,EV_KBD_CANCEL	  	,ST_TEST_PTC24 		    ,&LCD_vTestPTC24			},
{ST_TEST_LOG_PTC24		,EV_RETURN		  	,ST_TEST_PTC24 		    ,&LCD_vTestPTC24			},
{ST_TEST_LOG_PTC24		,EV_ANY			  	,ST_TEST_LOG_PTC24      ,&LCD_vDisplayTestMessage	},

{ST_TEST_PTC16 			,EV_REFRESH		  	,ST_TEST_PTC16          ,&LCD_vTestPTC16			},
{ST_TEST_PTC16 			,EV_KBD_CANCEL	  	,ST_MAIN    	   		,&LCD_vMainScreen			},
{ST_TEST_PTC16 			,EV_KBD_ENTER	  	,ST_TEST_PTC16 			,&LCD_vTestPTC16Start		},
{ST_TEST_PTC16 			,EV_TEST_LOG	  	,ST_TEST_LOG_PTC16 		,&LCD_vDisplayTestMessage	},
{ST_TEST_PTC16		 	,EV_BACK_TO_MAIN	,ST_MAIN		     	,&LCD_vMainScreen			},
{ST_TEST_PTC16 			,EV_ANY			  	,ST_TEST_PTC16          ,&LCD_vTestPTC16			},

{ST_TEST_LOG_PTC16		,EV_REFRESH		  	,ST_TEST_LOG_PTC16      ,&LCD_vDisplayTestMessage	},
{ST_TEST_LOG_PTC16		,EV_KBD_ENTER	  	,ST_TEST_LOG_PTC16      ,&LCD_vDisplayTestMessage	},
{ST_TEST_LOG_PTC16		,EV_KBD_CANCEL	  	,ST_TEST_PTC16 		    ,&LCD_vTestPTC16			},
{ST_TEST_LOG_PTC16		,EV_RETURN		  	,ST_TEST_PTC16 		    ,&LCD_vTestPTC16			},
{ST_TEST_LOG_PTC16		,EV_ANY			  	,ST_TEST_LOG_PTC16      ,&LCD_vDisplayTestMessage	}
};

#define TRANS_COUNT (sizeof(smTrans)/sizeof(*smTrans))

void startDebugMode(){
	//enableFunction(FUNCTIONS_TASK_INDEX,false); //dont comment later
	sm.state = ST_MENU_DEBUG;
	nextEvent = EV_KBD_CANCEL;
	inDebugMode = true;
}

bool isInDebugMode(){
	return inDebugMode;
}

struct
{
	union
	{
		struct
		{
			uint32_t currentMinute;
			uint32_t currentHour;
			uint32_t currentYear;
			uint32_t currentMonth;
			uint32_t currentDay;
		};
		uint32_t fields[5];
	};
}currentTime;

static char errorStringFirstLine[LINE_SIZE];
static char errorStringSecondLine[LINE_SIZE];
static uint64_t errorStringStartTime=0;
static uint64_t errorStringTimeToDisplay=0;
static uint8_t lastWarningMessageConfigExecuted=0;

#define WARNING_MSG_PRIORITY_MASK 0xFF
#define IGNORE_ALL_NO_ERROR_MESSAGES 6

/* Display message if no message is displayed. Otherwise, display only if it has more displayTime and higher priority than current message */
void displayErrorMessage(char *firstLine, char *secondLine, uint32_t displayTime, uint8_t priority){

	/* For security reasons it applies WARNING_MSG_PRIORITY_MASK */
	uint8_t priorityMask = priority & WARNING_MSG_PRIORITY_MASK;

	if (hasErrorToDisplay()){
		/* Verify Priority and displayTime */
		if(((lastWarningMessageConfigExecuted & WARNING_MSG_PRIORITY_MASK) < priorityMask))
		{
			return;
		}

		if(((lastWarningMessageConfigExecuted & WARNING_MSG_PRIORITY_MASK) == priorityMask))
		{
			if ((displayTime<errorStringTimeToDisplay))
			{
				return;
			}
		}
	}

	/* Avoid any overflow */
	memcpy(errorStringFirstLine,firstLine,LINE_SIZE);
	memcpy(errorStringSecondLine,secondLine,LINE_SIZE);
	errorStringFirstLine[LINE_SIZE] = '\0';
	errorStringSecondLine[LINE_SIZE] = '\0';
	errorStringTimeToDisplay = displayTime;
	errorStringStartTime = sysTickTimer;
	lastWarningMessageConfigExecuted = priorityMask;
}

bool hasErrorToDisplay()
{
	return ( sysTickTimer - errorStringStartTime < errorStringTimeToDisplay);
}


StEvents LCD_vGetNextEvent(void){
	//Eventos gerados pelas Key.
	//LCD_vSetNextEvent(EV_REFRESH);

	if(getVirtualKeyState(KEY_CANCEL))
	{
		setBeep(1,0);

		setVirtualKeyState(KEY_CANCEL,0); //consumer

		LCD_vSetNextEvent(EV_KBD_CANCEL);

		if(sm.state == ST_ADJUST_TIME)
			LCD_vSetNextEvent(EV_PREVIOUS_FIELD);

		else if((sm.state == ST_TEST_LOG_MPX_M) || (sm.state == ST_TEST_LOG_MPX))
			mpxTest_vFinishTest();

		else if(sm.state == ST_TEST_LOG_PTC24)
		{
			ptc24Test_vFinishTest();
			deInitPTC24config();
		}

		else if(sm.state == ST_TEST_LOG_PTC16)
			ptc16Test_vFinishTest();
	}

	else if(getVirtualKeyState(KEY_DOWN)){
		setVirtualKeyState(KEY_DOWN,0); //consumer

		if (sm.state != ST_TEST_LOG_MPX)
			lcd.sbLine++; //Handles others state transitions.

		if(lcd.sbLine > lcd.sbLineMax )
		{
			lcd.sbLine = lcd.sbLineMin;
		}

		LCD_vSetNextEvent(EV_REFRESH);

		if (sm.state == ST_ADJUST_TIME)
		{
			LCD_vSetNextEvent(EV_DOWN);
		}
	}

	else if(getVirtualKeyState(KEY_UP)){
		setVirtualKeyState(KEY_UP,0); //consumer

		if (sm.state != ST_TEST_LOG_MPX)
			lcd.sbLine--;

		if (lcd.sbLine < lcd.sbLineMin)
		{
			lcd.sbLine = lcd.sbLineMax;
		}

		LCD_vSetNextEvent(EV_REFRESH);

		if (sm.state == ST_ADJUST_TIME)
		{
			LCD_vSetNextEvent(EV_UP);
		}

	}

	else if(getVirtualKeyState(KEY_ENTER))
	{
		setVirtualKeyState(KEY_ENTER,0); //Consumer

		/* Allow enter only on Main, Debug menu or UserSettings */
		if (sm.state == ST_MAIN)
			LCD_vSetNextEvent(lcd.sbLine);

		else if (sm.state == ST_ADJUST_TIME)
			LCD_vSetNextEvent(EV_NEXT_FIELD);

		else if (sm.state == ST_LANGUAGE)
		{
			LCD_vSetNextEvent(EV_KBD_ENTER);
			idioma = lcd.sbLine - 1 - PORTUGUESE;
		}

		else if (sm.state == ST_TEST_MPX)
			LCD_vSetNextEvent(lcd.sbLine);

		else if (sm.state == ST_TEST_MPX_AUTO)
			LCD_vSetNextEvent(EV_KBD_ENTER);

		else if (sm.state == ST_TEST_MPX_LOOP)
			LCD_vSetNextEvent(EV_KBD_ENTER);

		else if (sm.state == ST_TEST_MPX_MANUAL)
			LCD_vSetNextEvent(lcd.sbLine);

		else if (sm.state == ST_TEST_LOG_MPX)
		{
			if (mpxTest_vGetTest() == TEST_END)
			{
				LCD_vSetNextEvent(EV_KBD_ENTER);
				mpxTest_vFinishTest();
			}

			else
				mpxTest_vContinueTest();
		}

		else if (sm.state == ST_TEST_LOG_MPX_M)
		{
			LCD_vSetNextEvent(EV_KBD_ENTER);
			mpxTest_vFinishTest();
		}

		else if (sm.state == ST_TEST_PTC24)
			LCD_vSetNextEvent(EV_KBD_ENTER);

		else if (sm.state == ST_TEST_LOG_PTC24)
		{
			LCD_vSetNextEvent(EV_KBD_ENTER);
			ptc24Test_vTestOk();
		}

		else if (sm.state == ST_TEST_PTC16)
			LCD_vSetNextEvent(EV_KBD_ENTER);

		else if (sm.state == ST_TEST_LOG_PTC16)
		{
			LCD_vSetNextEvent(EV_KBD_ENTER);
			ptc16Test_vTestOk();
		}

	}

	return nextEvent;
}


void LCD_printLine(uint8_t line, const char *string){
	if(line==0){
		LCD_MoveToPosition(0x00);
	}
	else{
		LCD_MoveToPosition(0x40);
	}

	LCD_Print(string);
}


void  LCD_vSetNextEvent(StEvents event){
	nextEvent = event;
}

void LCD_vJumpToState(StStates state) {
	sm.state = state;
	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vStateMachineInit(void){
	sm.state = ST_MAIN;
	sm.trans = &smTrans;
	sm.transCount = TRANS_COUNT;

	/* Use event EV_KBD_CANCEL to update lcd.sbLineMax in LCD_vMenuDebug */
	LCD_vSetNextEvent(EV_KBD_CANCEL);
}

void  LCD_vStateMachineLoop(void){ // STATE MACHINE LOOP{

	if (showLCDAsTurnedOff){
		LCD_printLine(0,"                ");
		LCD_printLine(1,"                ");
	}
	/* Display messages if not in error state and if message is different from priority 6(ignore all no error messages) */
	else if(hasErrorToDisplay() && !inDebugMode &&
			((lastWarningMessageConfigExecuted & WARNING_MSG_PRIORITY_MASK)!=IGNORE_ALL_NO_ERROR_MESSAGES) )
	{
		static char lines[][LINE_SIZE]={"                "
									   ,"                "};
		snprintf(lines[0],LINE_SIZE,"%-16.16s",errorStringFirstLine);
		snprintf(lines[1],LINE_SIZE,"%-16.16s",errorStringSecondLine);
		LCD_printLine(0,lines[0]);
		LCD_printLine(1,lines[1]);
	}
	else{
		int i;
		sm.event = LCD_vGetNextEvent();

		/* Execute LCD state machine */
		for (i = 0; i < TRANS_COUNT; i++)
		{
			if ((sm.state == sm.trans[i].state) || (ST_ANY == sm.trans[i].state))
			{
				if (((sm.event == sm.trans[i].event) || (EV_ANY == sm.trans[i].event)))
				{
	                sm.state = sm.trans[i].next;

	                if (sm.trans[i].fn)
	                {
	                    (sm.trans[i].fn)();
	                }
	                break;
				}
			}
		}

	}
}

void LCD_vMainScreen(void)
{
	char lines[][LINE_SIZE]={"     GIGA 3     "
			                ,"      __:__     "
			                ,"    TESTE MPX   "
			                ,"  TESTE PTC 24  "
							,"  TESTE PTC 16  "
							,"     IDIOMA     "};

	char lines_spanish[][LINE_SIZE]={"     GIGA 3     "
									,"      __:__     "
									,"   PRUEBA MPX   "
									,"  PRUEBA PTC24  "
									,"  PRUEBA PTC16  "
									,"     IDIOMA     "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	/* Memorize current line */
	static uint8_t currentLine=1;

	mpx.MpxAlreadyInit = false;
	deInitPTC24config();
	deInitPTC16config();
	enableTactShortDeadTime(false);

	if(EV_REFRESH != sm.event)
	{
		lcd.sbLine = currentLine;
		lcd.sbLineMin = 1;
		lcd.sbLineMax = numLines-1;
	}
	else
	{
		currentLine = lcd.sbLine;
	}

	time_t rawtime;
	time ( &rawtime );

	currentTime.currentHour = getHours(rawtime);
	currentTime.currentMinute = getMinutes(rawtime);
	currentTime.currentDay = getDay(rawtime);
	currentTime.currentMonth = getMonth(rawtime);
	currentTime.currentYear = getYear(rawtime);

	char timerSeparationChar=' ';
	/* 500 ms blink time */
	if ((sysTickTimer%1000)>499)
	{
		timerSeparationChar=':';
	}



	LCD_CursorBlink(false);

	/* Print currentLine */
	if (LCD_languageChosen() == PORTUGUESE)
	{
		snprintf(lines[1],LINE_SIZE,"     %02d%c%02d      ",currentTime.currentHour,timerSeparationChar,currentTime.currentMinute);
		LCD_printLine(0,lines[0]);
		LCD_printLine(1,lines[currentLine]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		snprintf(lines_spanish[1],LINE_SIZE,"     %02d%c%02d      ",currentTime.currentHour,timerSeparationChar,currentTime.currentMinute);
		LCD_printLine(0,lines_spanish[0]);
		LCD_printLine(1,lines_spanish[currentLine]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

	/* Automatic device detection */
	if(sysTickTimer-mpx.lastTimeSeen<100)
		LCD_vSetNextEvent(EV_LINE2);

	else if(sysTickTimer-ptc24.lastTimeSeen<100)
		LCD_vSetNextEvent(EV_LINE3);

	else if(sysTickTimer-ptc16.lastTimeSeen<100)
		LCD_vSetNextEvent(EV_LINE4);
}

void LCD_vAdjustTime(void){
	char lines[][LINE_SIZE]={"AJUSTE DATA/HORA"
			                ,"DD/MM/YYYY __:__"};

	static uint8_t maxFieldValues[5]={60,24,130,13,32};
	static uint8_t cursorPosition[5]={0x4F, 0x4C, 0x49, 0x44, 0x41};

	uint8_t numFields=sizeof(maxFieldValues) - 1;
	static int8_t currentField=0;

	LCD_vSetNextEvent(EV_REFRESH);

	struct tm oldTimestamp, newTime;
	time_t rawtime;

	enableTactShortDeadTime(true);

	maxFieldValues[4] = maxDaysOnMoth(currentTime.currentMonth) + 1;

	if(sm.event == EV_NEXT_FIELD){
		currentField--;
		if (currentField<0){
			currentField=0;

			writeTime(currentTime.currentYear,currentTime.currentMonth,currentTime.currentDay,
					currentTime.currentHour,currentTime.currentMinute,0);

			LCD_vSetNextEvent(EV_BACK_TO_MAIN);
		}
	}

	if(sm.event == EV_PREVIOUS_FIELD){
		currentField++;
		if (currentField>numFields){
			currentField=numFields;

			writeTime(currentTime.currentYear,currentTime.currentMonth,currentTime.currentDay,
					currentTime.currentHour,currentTime.currentMinute,0);

			LCD_vSetNextEvent(EV_BACK_TO_MAIN);
		}
	}

	if (sm.event == EV_UP)
	{
		currentTime.fields[currentField]++;

		if (currentTime.fields[currentField]>=maxFieldValues[currentField]){
			if(currentField < 3)
				currentTime.fields[currentField]=0;
			else
				currentTime.fields[currentField]=1;
		}

	}

	if (sm.event == EV_DOWN){
		if(currentField < 3) {
			if (currentTime.fields[currentField]<=0){
				currentTime.fields[currentField]=maxFieldValues[currentField];
			}
		}
		else {
			if (currentTime.fields[currentField]<=1){
				currentTime.fields[currentField]=maxFieldValues[currentField];
			}
		}

		currentTime.fields[currentField]--;
	}

	snprintf(lines[1],LINE_SIZE,"%02d/%02d/%04d %02d:%02d ", currentTime.currentDay, currentTime.currentMonth, currentTime.currentYear,
															 currentTime.currentHour, currentTime.currentMinute);

	/* Print currentLine */
	LCD_CursorBlink(true);
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[1]);
	LCD_MoveCursor(cursorPosition[currentField]);
}

void LCD_vChooseLanguage()
{
	char lines[][LINE_SIZE]={"ESCOLHA O IDIOMA"
				            ,"   Portugues    "
				            ,"   Espanhol     "};

	char lines_spanish[][LINE_SIZE]={"ELIJA EL IDIOMA "
					        		,"   Portugues    "
					        		,"    Espanol     "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	/* Memorize current line */
	static uint8_t currentLine=1;

	if(EV_REFRESH != sm.event)
	{
		lcd.sbLine = currentLine;
		lcd.sbLineMin = 1;
		lcd.sbLineMax = numLines-1;
	}

	else
	{
		currentLine = lcd.sbLine;
	}

	if (LCD_languageChosen() == PORTUGUESE)
	{
		LCD_printLine(0,lines[0]);
		LCD_printLine(1,lines[lcd.sbLine]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		LCD_printLine(0,lines_spanish[0]);
		LCD_printLine(1,lines_spanish[lcd.sbLine]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

}

void LCD_vAdjustUSBMode() {

	LCD_vSetNextEvent(EV_REFRESH);

	char lines[][LINE_SIZE] = {"CONFIGURAR USB " ,
						  	  	  	  "MODE:          "};

	if (sm.event == EV_DOWN || sm.event == EV_UP){
		if(isUSBHostMode)
			isUSBHostMode =  false;
		else
			isUSBHostMode =  true;
	}

	snprintf(lines[1],LINE_SIZE,"MODE: %s      ", isUSBHostMode ? "HOST" : "DEVICE");

	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[1]);
}



void LCD_vMenuDebug(void){
	static const char lines[][LINE_SIZE]={"      Menu      ",
											"Diag. Manual    ",
											"Master/Slave    ",
											"Entradas        ",
											"CAN             ",
											"RTC             ",
											"ADC             ",
											"Tempo tarefas   ",
											"USB             ",
											"NOME DO ONIBUS  ",
											"UID             ",
											"Reiniciar PTC   "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	/* Memorize current line */
	static uint8_t currentLine=1;
	if(EV_REFRESH != sm.event){
		lcd.sbLine = currentLine;
		lcd.sbLineMin = 1;
		lcd.sbLineMax = numLines-1;
	}
	else{
		currentLine = lcd.sbLine;
	}

	/* Print currentLine */
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[currentLine]);

	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vUIDScreen(void) {
	char line [2][LINE_SIZE]={"    UID        "
							 ,"               "};
	uint8_t numLines = 3;
	static uint8_t currentLine=0;

	if(EV_REFRESH != sm.event){
		lcd.sbLine = currentLine;
		lcd.sbLineMin = 0;
		lcd.sbLineMax = numLines-1;
	}
	else{
		currentLine = lcd.sbLine;
	}

	uint32_t *id = getUniqueID();

	snprintf(line[0],LINE_SIZE,"      %s %d      ", "UID", currentLine+1);
	snprintf(line[1],LINE_SIZE,"    %08X         ", id[currentLine]);

	LCD_printLine(0,line[0]);
	LCD_printLine(1,line[1]);

	LCD_vSetNextEvent(EV_REFRESH);
}

/* ---------------------------------------------------------------------------*/
/* MPX tests prints ----------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void LCD_vTestMPX(void)
{
	char lines[][LINE_SIZE]={"   TESTE MPX    "
				            ,"Teste Automatico"
				            ," Teste em Loop  "
							,"  Teste Manual  "};

	char lines_spanish[][LINE_SIZE]={"   PRUEBA MPX   "
					        		,"PruebaAutomatica"
					        		," Prueba en Loop "
					        		," Prueba  Manual "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	initMPXconfig();
	enableTactShortDeadTime(false);

	static uint8_t currentLine=1;

	if(EV_REFRESH != sm.event)
	{
		lcd.sbLine = currentLine;
		lcd.sbLineMin = 1;
		lcd.sbLineMax = numLines-1;
	}
	else
	{
		currentLine = lcd.sbLine;
	}

	if (LCD_languageChosen() == PORTUGUESE)
	{
		LCD_printLine(0,lines[0]);
		LCD_printLine(1,lines[lcd.sbLine]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		LCD_printLine(0,lines_spanish[0]);
		LCD_printLine(1,lines_spanish[lcd.sbLine]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

	/* This if sequence determines the return to main screen if device connected
	 * does not belong to this test screen.
	 */
	if(sysTickTimer-mpx.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_REFRESH);
	}

	else if(sysTickTimer-ptc24.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}

	else if(sysTickTimer-ptc16.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}
}

/* ---------------------------------------------------------------------------*/
/* MPX automatic test prints -------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void LCD_vTestMPXAuto(void)
{
	char lines_spanish[][LINE_SIZE]={"PRUEBA MPX: AUTO"
   	                        		,"Conecte el MPX  "};

	char lines[][LINE_SIZE]={"TESTE MPX: AUTO "
	   	                    ,"Conecte o MPX   "};

	char finalpoint={' '};

	if ((sysTickTimer%2000)>999)
		finalpoint='.';

	if (LCD_languageChosen() == PORTUGUESE)
	{
		snprintf(lines[1],LINE_SIZE,"Iniciar o Teste%c",finalpoint);
		LCD_printLine(0, lines[0]);
		LCD_printLine(1, lines[1]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		snprintf(lines_spanish[1],LINE_SIZE,"Iniciar la Prueba%c",finalpoint);
		LCD_printLine(0, lines_spanish[0]);
		LCD_printLine(1, lines_spanish[1]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

	/* This if sequence determines the return to main screen if device connected
	 * does not belong to this test screen.
	 */
	if(sysTickTimer-mpx.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_REFRESH);
	}

	else if(sysTickTimer-ptc24.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}

	else if(sysTickTimer-ptc16.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}
}

void LCD_vTestMPXAutoStart(void)
{
	LCD_printLine(0, "                ");
	LCD_printLine(1, "                ");

	mpxTest_vSetTest(TEST_AUTO);

	LCD_vSetNextEvent(EV_TEST_LOG);
}

/* ---------------------------------------------------------------------------*/
/* MPX loop test prints ------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void LCD_vTestMPXLoop(void)
{
	char lines[][LINE_SIZE]={"TESTE MPX: LOOP "
   	                        ,"Conecte o MPX   "};

	char lines_spanish[][LINE_SIZE]={"PRUEBA MPX: LOOP"
   	                        		,"Conecte el MPX  "};

	char finalpoint={' '};

	if ((sysTickTimer%2000)>999)
		finalpoint='.';

	if (LCD_languageChosen() == PORTUGUESE)
	{
		snprintf(lines[1],LINE_SIZE,"Iniciar o Teste%c",finalpoint);
		LCD_printLine(0, lines[0]);
		LCD_printLine(1, lines[1]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		snprintf(lines_spanish[1],LINE_SIZE,"Iniciar la Prueba%c",finalpoint);
		LCD_printLine(0, lines_spanish[0]);
		LCD_printLine(1, lines_spanish[1]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

	/* This if sequence determines the return to main screen if device connected
	 * does not belong to this test screen.
	 */
	if(sysTickTimer-mpx.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_REFRESH);
	}

	else if(sysTickTimer-ptc24.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}

	else if(sysTickTimer-ptc16.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}
}

void LCD_vTestMPXLoopStart(void)
{
	LCD_printLine(0, "                ");
	LCD_printLine(1, "                ");

	mpxTest_vSetTest(TEST_LOOP);

	LCD_vSetNextEvent(EV_TEST_LOG);
}

/* ---------------------------------------------------------------------------*/
/* MPX manual test prints ----------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void LCD_vTestMPXManual(void)
{
	char lines[][LINE_SIZE]={"    TESTE MPX   "
		,"Teste 1: Memoria"
		,"Teste 2: ID1    " ,"Teste 3: ID2    "	,"Teste 4: ID4    "	,"Teste 5: ID0    "
		,"Teste 6: Ignicao"
		,"Teste 7: CN1.1L " ,"Teste 8: CN1.1H "	,"Teste 9: CN1.2  " ,"Teste 10: CN1.3L"
		,"Teste 11: CN1.3H" ,"Teste 12: CN1.4 "	,"Teste 13: CN1.5 " ,"Teste 14: CN1.6 "
		,"Teste 15: CN1.7 "	,"Teste 16: CN1.8 "	,"Teste 17: CN1.9 " ,"Teste 18: CN2.1L"
		,"Teste 19: CN2.1H"	,"Teste 20: CN2.2 " ,"Teste 21: CN2.3L"	,"Teste 22: CN2.3H"
		,"Teste 23: CN2.4 "	,"Teste 24: CN2.5 " ,"Teste 25: CN2.6 "	,"Teste 26: CN2.7 "
		,"Teste 27: CN2.8 "	,"Teste 28: CN2.9 " ,"Teste 29: CN3.1 "	,"Teste 30: CN3.2 "
		,"Teste 31: CN3.3 "	,"Teste 32: CN3.4 " ,"Teste 33: CN3.5 "	,"Teste 34: CN3.6 "
		,"Teste 35: CN3.7 "	,"Teste 36: CN3.8 " ,"Teste 37: CN3.9 "	,"Teste 38: CN4.1 "
		,"Teste 39: CN4.2 "	,"Teste 40: CN4.3 " ,"Teste 41: CN4.4 "	,"Teste 42: CN4.5 "
		,"Teste 43: CN4.6 "	,"Teste 44: CN4.7 " ,"Teste 45: CN4.8 "	,"Teste 46: CN4.9 "
		,"Teste 47: NTC   "};

	char lines_spanish[][LINE_SIZE]={"   PRUEBA MPX   "
		,"Prueba1: Memoria"
		,"Prueba2: ID1    " ,"Prueba3: ID2    "	,"Prueba4: ID4    "	,"Prueba5: ID0    "
		,"Prueba6:Ignicion"
		,"Prueba7: CN1.1L " ,"Prueba8: CN1.1H "	,"Prueba9: CN1.2  " ,"Prueba10: CN1.3L"
		,"Prueba11: CN1.3H" ,"Prueba12: CN1.4 "	,"Prueba13: CN1.5 " ,"Prueba14: CN1.6 "
		,"Prueba15: CN1.7 "	,"Prueba16: CN1.8 "	,"Prueba17: CN1.9 " ,"Prueba18: CN2.1L"
		,"Prueba19: CN2.1H"	,"Prueba20: CN2.2 " ,"Prueba21: CN2.3L"	,"Prueba22: CN2.3H"
		,"Prueba23: CN2.4 "	,"Prueba24: CN2.5 " ,"Prueba25: CN2.6 "	,"Prueba26: CN2.7 "
		,"Prueba27: CN2.8 "	,"Prueba28: CN2.9 " ,"Prueba29: CN3.1 "	,"Prueba30: CN3.2 "
		,"Prueba31: CN3.3 "	,"Prueba32: CN3.4 " ,"Prueba33: CN3.5 "	,"Prueba34: CN3.6 "
		,"Prueba35: CN3.7 "	,"Prueba36: CN3.8 " ,"Prueba37: CN3.9 "	,"Prueba38: CN4.1 "
		,"Prueba39: CN4.2 "	,"Prueba40: CN4.3 " ,"Prueba41: CN4.4 "	,"Prueba42: CN4.5 "
		,"Prueba43: CN4.6 "	,"Prueba44: CN4.7 " ,"Prueba45: CN4.8 "	,"Prueba46: CN4.9 "
		,"Prueba47: NTC   "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	static uint8_t currentLine=1;

	enableTactShortDeadTime(true);

	if(EV_REFRESH != sm.event)
	{
		lcd.sbLine = currentLine;
		lcd.sbLineMin = 1;
		lcd.sbLineMax = numLines-1;
	}
	else
	{
		currentLine = lcd.sbLine;
	}

	if (LCD_languageChosen() == PORTUGUESE)
	{
		LCD_printLine(0, lines[0]);
		LCD_printLine(1, lines[lcd.sbLine]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		LCD_printLine(0, lines_spanish[0]);
		LCD_printLine(1, lines_spanish[lcd.sbLine]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

	/* This if sequence determines the return to main screen if device connected
	 * does not belong to this test screen.
	 */
	if(sysTickTimer-mpx.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_REFRESH);
	}

	else if(sysTickTimer-ptc24.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}

	else if(sysTickTimer-ptc16.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}
}

void LCD_vTestMPXManualStart(void)
{
	LCD_printLine(0, "                ");
	LCD_printLine(1, "                ");

	mpxTest_vSetTest(RearrangedTests[sm.event - EV_LINE1]);

	LCD_vSetNextEvent(EV_TEST_LOG);
}

/* ---------------------------------------------------------------------------*/
/* PTC24 tests prints --------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void LCD_vTestPTC24(void)
{
	char lines[][LINE_SIZE]={"  TESTE PTC 24  "
   	                        ,"                "};

	char lines_spanish[][LINE_SIZE]={"  PRUEBA PTC24  "
   	                        		,"                "};

	char finalpoint={' '};

	if (!ptc24.PtcAlreadyInit)
		initPTC24config();

	if ((sysTickTimer%2000)>999)
		finalpoint='.';

	if (LCD_languageChosen() == PORTUGUESE)
	{
		snprintf(lines[1],LINE_SIZE,"Iniciar o Teste%c",finalpoint);
		LCD_printLine(0, lines[0]);
		LCD_printLine(1, lines[1]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		snprintf(lines_spanish[1],LINE_SIZE,"Iniciar Prueba%c ",finalpoint);
		LCD_printLine(0, lines_spanish[0]);
		LCD_printLine(1, lines_spanish[1]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

	/* This if sequence determines the return to main screen if device connected
	 * does not belong to this test screen.
	 */
	if(sysTickTimer-mpx.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}

	else if(sysTickTimer-ptc24.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_REFRESH);
	}

	else if(sysTickTimer-ptc16.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}
}

void LCD_vTestPTC24Start(void)
{
	LCD_printLine(0, "                ");
	LCD_printLine(1, "                ");
	snprintf(TestMessages.lines[0], LINE_SIZE, "                ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "                ");

	ptc24Test_vSetTest(PTC24_TEST_START);

	LCD_vSetNextEvent(EV_TEST_LOG);
}

/* ---------------------------------------------------------------------------*/
/* PTC16 tests prints --------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void LCD_vTestPTC16(void)
{
	char lines[][LINE_SIZE]={"  TESTE PTC 16  "
   	                        ,"                "};

	char lines_spanish[][LINE_SIZE]={"  PRUEBA PTC16  "
   	                        		,"                "};

	char finalpoint={' '};

	if (!ptc16.PtcAlreadyInit)
		initPTC16config();

	if ((sysTickTimer%2000)>999)
		finalpoint='.';

	if (LCD_languageChosen() == PORTUGUESE)
	{
		snprintf(lines[1],LINE_SIZE,"Iniciar o Teste%c",finalpoint);
		LCD_printLine(0, lines[0]);
		LCD_printLine(1, lines[1]);
	}

	else if (LCD_languageChosen() == SPANISH)
	{
		snprintf(lines_spanish[1],LINE_SIZE,"Iniciar Prueba%c ",finalpoint);
		LCD_printLine(0, lines_spanish[0]);
		LCD_printLine(1, lines_spanish[1]);
	}

	LCD_vSetNextEvent(EV_REFRESH);

	/* This if sequence determines the return to main screen if device connected
	 * does not belong to this test screen.
	 */
	if(sysTickTimer-mpx.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}

	else if(sysTickTimer-ptc24.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_BACK_TO_MAIN);
	}

	else if(sysTickTimer-ptc16.lastTimeSeen<100)
	{
		LCD_vSetNextEvent(EV_REFRESH);
	}
}

void LCD_vTestPTC16Start(void)
{
	LCD_printLine(0, "                ");
	LCD_printLine(1, "                ");

	ptc16Test_vSetTest(PTC16_TEST_START);

	LCD_vSetNextEvent(EV_TEST_LOG);
}

/* ---------------------------------------------------------------------------*/
/* Message tests log print ---------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
void LCD_vDisplayTestMessage(void)
{
	LCD_printLine(0,TestMessages.lines[0]);
	LCD_printLine(1,TestMessages.lines[1]);
	LCD_vSetNextEvent(EV_REFRESH);
}

languages LCD_languageChosen(void)
{
	return idioma;
}

void LCD_vADCScreen(void){

}


void LCD_vCANScreen(void){

}

void LCD_vRTCScreen(void){

}

void LCD_vIOsScreen(void){

}

void LCD_vTimingScreen(void){

}


#define NO_MPX_SELECTED -1


void LCD_vManualMode(void){

}


void LCD_vReboot(void){
	rebootPTC();
}

void LCD_vEditBusName(void) {

}

void setLCDAsTurnedOff(bool value){
	showLCDAsTurnedOff = value;
}

static inline bool msgTimoutReached (uint32_t timeout) {
	static int64_t msgTimeout=0;
	if(msgTimeout == 0)
		msgTimeout = sysTickTimer;
	if(sysTickTimer - msgTimeout > timeout) {
		msgTimeout = 0;
		return true;
	}
	return false;
}

/* progress must be between 0 and 10 */
void LCD_vSetProgressBar (uint8_t progress)
{

	char line [1][LINE_SIZE];
	static const char progressBar[LINE_SIZE] = "****************";
	static const uint8_t percentages[] = {1,2,3,5,7,8,10,12,14,15,15};
	char *progressStart;

	if(progress > 10)
		progress = 10;

	if(progress <= 10)
	{
		progressStart = &progressBar[LINE_SIZE-1-percentages[progress]];
		//if(currentLogSaveMsg == LCD_USB_MSG_SAVING) {
			sniprintf(line[0], LINE_SIZE, "%s                 ", progressStart);
		//}
			LCD_printLine(1,line[0]);
	}
}


/*******************************************************/
/* 					Interrupt Handler 				   */
/*													   */
/*******************************************************/
void TIM5_IRQHandler(void){
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	/* LCD loop */
	LCD_ClockTick();
}
