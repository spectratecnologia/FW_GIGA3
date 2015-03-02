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

bool isUSBHostMode=false;
uint8_t usbSaveMode=0;

static inline bool msgTimoutReached (uint32_t timeout);

void LCD_vMainScreen();
void LCD_vAdjustTime();


void LCD_vTestMPX();
void LCD_vTestMPXAuto();
void LCD_vTestMPXAutoStarted();
void LCD_vTestMPXLoop();

void LCD_vTestMPXManual();
void LCD_vTestMPXManualStarted();


void LCD_vTestPTC24();
void LCD_vTestPTC16();

void displayTestMessage();

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

const Transition smTrans[] =  		//TABELA DE ESTADOS
{
/*Current state		Event				Next state				callback */
{ST_MAIN			,EV_REFRESH       	,ST_MAIN           		,&LCD_vMainScreen	},
{ST_MAIN			,EV_LINE1         	,ST_ADJUST_TIME    		,&LCD_vAdjustTime	},
{ST_MAIN			,EV_LINE2		  	,ST_TEST_MPX          	,&LCD_vTestMPX		},
{ST_MAIN			,EV_LINE3		  	,ST_TEST_PTC24			,&LCD_vTestPTC24    },
{ST_MAIN			,EV_LINE4		  	,ST_TEST_PTC16			,&LCD_vTestPTC16    },
{ST_MAIN			,EV_KBD_CANCEL    	,ST_MAIN           		,&LCD_vMainScreen	},

{ST_ADJUST_TIME		,EV_REFRESH         ,ST_ADJUST_TIME	 	  	,&LCD_vAdjustTime	},
{ST_ADJUST_TIME		,EV_PREVIOUS_FIELD  ,ST_ADJUST_TIME    		,&LCD_vAdjustTime	},
{ST_ADJUST_TIME		,EV_NEXT_FIELD      ,ST_ADJUST_TIME    		,&LCD_vAdjustTime	},
{ST_ADJUST_TIME		,EV_UP      		,ST_ADJUST_TIME    		,&LCD_vAdjustTime	},
{ST_ADJUST_TIME		,EV_DOWN     		,ST_ADJUST_TIME    		,&LCD_vAdjustTime	},
{ST_ADJUST_TIME		,EV_BACK_TO_MAIN    ,ST_MAIN    	   		,&LCD_vMainScreen	},

{ST_TEST_MPX 		,EV_REFRESH		  	,ST_TEST_MPX          	,&LCD_vTestMPX		},
{ST_TEST_MPX 		,EV_LINE1		  	,ST_TEST_MPX_AUTO      	,&LCD_vTestMPXAuto	},
{ST_TEST_MPX 		,EV_LINE2		  	,ST_TEST_MPX_LOOP    	,&LCD_vTestMPXLoop	},
{ST_TEST_MPX 		,EV_LINE3		  	,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManual},
{ST_TEST_MPX 		,EV_KBD_CANCEL	  	,ST_MAIN    	   		,&LCD_vMainScreen	},

{ST_TEST_MPX_AUTO 	,EV_REFRESH		  	,ST_TEST_MPX_AUTO      		,&LCD_vTestMPXAuto			},
{ST_TEST_MPX_AUTO	,EV_START			,ST_TEST_MPX_AUTO_STARTED	,&LCD_vTestMPXAutoStarted	},
{ST_TEST_MPX_AUTO 	,EV_KBD_CANCEL	  	,ST_TEST_MPX  	     		,&LCD_vTestMPX				},

{ST_TEST_MPX_AUTO_STARTED	,EV_REFRESH		,ST_TEST_MPX_AUTO_STARTED	,&LCD_vTestMPXAutoStarted	},
{ST_TEST_MPX_AUTO_STARTED	,EV_KBD_CANCEL	,ST_TEST_MPX				,&LCD_vTestMPX				},

{ST_TEST_MPX_MANUAL	,EV_REFRESH		  	,ST_TEST_MPX_MANUAL    	,&LCD_vTestMPXManual		},
{ST_TEST_MPX_MANUAL	,EV_LINE1		  	,ST_TEST_MPX_ID1    	,&LCD_vTestMPXManualStarted	},
{ST_TEST_MPX_MANUAL	,EV_LINE2		  	,ST_TEST_MPX_ID2    	,&LCD_vTestMPXManualStarted	},
{ST_TEST_MPX_MANUAL	,EV_LINE3		  	,ST_TEST_MPX_ID4    	,&LCD_vTestMPXManualStarted	},
{ST_TEST_MPX_MANUAL	,EV_LINE4		  	,ST_TEST_MPX_ID0    	,&LCD_vTestMPXManualStarted	},
{ST_TEST_MPX_MANUAL	,EV_KBD_CANCEL	  	,ST_TEST_MPX  	     	,&LCD_vTestMPX				},

{ST_TEST_MPX_ID1	,EV_REFRESH		  	,ST_TEST_MPX_ID1    	,&LCD_vTestMPXManualStarted	},

{ST_TEST_MPX_ID2	,EV_REFRESH		  	,ST_TEST_MPX_ID2    	,&LCD_vTestMPXManualStarted	},

{ST_TEST_MPX_ID4	,EV_REFRESH		  	,ST_TEST_MPX_ID4    	,&LCD_vTestMPXManualStarted	},

{ST_TEST_MPX_ID0	,EV_REFRESH		  	,ST_TEST_MPX_ID0    	,&LCD_vTestMPXManualStarted	},

{ST_TEST_PTC24 		,EV_REFRESH		  	,ST_TEST_PTC24          ,&LCD_vTestPTC24	},
{ST_TEST_PTC24 		,EV_KBD_CANCEL	  	,ST_MAIN    	   		,&LCD_vMainScreen	},

{ST_TEST_PTC16 		,EV_REFRESH		  	,ST_TEST_PTC16          ,&LCD_vTestPTC16	},
{ST_TEST_PTC16 		,EV_KBD_CANCEL	  	,ST_MAIN    	   		,&LCD_vMainScreen	},

{ST_TEST_LOG 		,EV_REFRESH		  	,ST_TEST_LOG   	   		,&displayTestMessage}
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
		setVirtualKeyState(KEY_CANCEL,0); //consumer

		LCD_vSetNextEvent(EV_KBD_CANCEL);

		if(sm.state == ST_ADJUST_TIME)
		{
			LCD_vSetNextEvent(EV_PREVIOUS_FIELD);
		}
	}

	else if(getVirtualKeyState(KEY_DOWN)){
		setVirtualKeyState(KEY_DOWN,0); //consumer

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
		if ( sm.state == ST_TEST_MPX_MANUAL )
		{
			TestMessages.lastLCDState = sm.state;
			TestMessages.lastTestState = test_vGetState();
			LCD_vSetNextEvent(lcd.sbLine);
		}

		if ( sm.state == ST_TEST_MPX_AUTO )
		{
			TestMessages.lastLCDState = sm.state;
			TestMessages.lastTestState = test_vGetState();
			LCD_vSetNextEvent(EV_START);
		}

		else if (sm.state == ST_MAIN || sm.state == ST_TEST_MPX )
		{
			LCD_vSetNextEvent(lcd.sbLine);
		}

		else if (sm.state == ST_ADJUST_TIME)
		{
			LCD_vSetNextEvent(EV_NEXT_FIELD);
		}

		else if (sm.state == ST_TEST_LOG)
		{
			LCD_vJumpToState(TestMessages.lastLCDState);
			test_vJumpToState(TestMessages.lastTestState);
			closeMpxTests();
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
							,"  TESTE PTC 16  "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	/* Memorize current line */
	static uint8_t currentLine=1;

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

	snprintf(lines[1],LINE_SIZE,"     %02d%c%02d      ",currentTime.currentHour,timerSeparationChar,
							currentTime.currentMinute);

	/* Print currentLine */
	LCD_CursorBlink(false);
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[currentLine]);

	LCD_vSetNextEvent(EV_REFRESH);
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

void LCD_vTestMPX(void)
{
	char lines[][LINE_SIZE]={"   TESTE MPX    "
				            ,"Teste Automatico"
				            ," Teste em Loop  "
							,"  Teste Manual  "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	initMPXconfig();

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

	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[lcd.sbLine]);

	test_vJumpToState(TST_MPX_TEST);
	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vTestMPXAuto(void)
{
	char lines[][LINE_SIZE]={"TESTE MPX: AUTO "
   	                        ,"Conecte o MPX   "};

	char finalpoint={' '};

	closeMpxTests();
	isAutoTest();

	if ((sysTickTimer%2000)>999)
		finalpoint='.';

	snprintf(lines[1],LINE_SIZE,"Iniciar o Teste%c",finalpoint);

	LCD_printLine(0, lines[0]);
	LCD_printLine(1, lines[1]);

	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vTestMPXAutoStarted(void)
{
	LCD_printLine(0, "                ");
	LCD_printLine(1, "                ");

	test_vSetNextEvent(TEV_AUTOMATIC);

	LCD_vSetNextEvent(EV_REFRESH);
	LCD_vJumpToState(ST_TEST_LOG);
}

void LCD_vTestMPXLoop(void)
{
	isLoopTest();
}

void LCD_vTestMPXManual(void)
{
	char lines[][LINE_SIZE]={"TESTE MPX Manual"
	               	   	   	,"Teste 1: ID1    "
							,"Teste 2: ID2    "
							,"Teste 3: ID4    "
							,"Teste 4: ID0    "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;

	static uint8_t currentLine=1;

	closeMpxTests();
	isManualTest();

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

	LCD_printLine(0, lines[0]);
	LCD_printLine(1, lines[lcd.sbLine]);

	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vTestMPXManualStarted(void)
{
	LCD_printLine(0, "                ");
	LCD_printLine(1, "                ");

	if (sm.state == ST_TEST_MPX_ID1 )
		test_vJumpToState(TST_MPX_TEST_ID1);

	else if (sm.state == ST_TEST_MPX_ID2)
		test_vJumpToState(TST_MPX_TEST_ID2);

	else if (sm.state == ST_TEST_MPX_ID4)
		test_vJumpToState(TST_MPX_TEST_ID4);

	else if (sm.state == ST_TEST_MPX_ID0)
		test_vJumpToState(TST_MPX_TEST_ID0);

	LCD_vSetNextEvent(EV_REFRESH);
	LCD_vJumpToState(ST_TEST_LOG);
}

void LCD_vTestPTC24(void)
{
	LCD_printLine(0,"  TESTE PTC 24  ");
	LCD_printLine(1,"Em construcao...");

	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vTestPTC16(void)
{
	LCD_printLine(0,"  TESTE PTC 16  ");
	LCD_printLine(1,"Em construcao...");

	LCD_vSetNextEvent(EV_REFRESH);
}

void displayTestMessage(void)
{
	LCD_printLine(0,TestMessages.lines[0]);
	LCD_printLine(1,TestMessages.lines[1]);
	LCD_vSetNextEvent(EV_REFRESH);
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
