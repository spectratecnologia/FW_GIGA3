#include "lcd_screen.h"
#include "ios/ios.h"
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

static volatile bool showLCDAsTurnedOff=true;

static bool inDebugMode=false;

static volatile int ptcTemperature = 0;

bool isUSBHostMode=false;
uint8_t usbSaveMode=0;

static inline bool msgTimoutReached (uint32_t timeout);

void LCD_vMainScreen();
void LCD_vAdjustTime();
void LCD_vEditBusName();

void LCD_vAdjustTemperature0();
void LCD_vAdjustTemperature1();
void LCD_vAdjustTemperature(int temp_num);

void LCD_vAdjustUSBMode();
void LCD_vLogUploadMenu();

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
/*Current state		Event			Next state					callback */
{ST_MAIN			,EV_REFRESH       ,ST_MAIN           		,&LCD_vMainScreen			},
{ST_MAIN			,EV_LINE1         ,ST_ADJUST_TIME    		,&LCD_vAdjustTime			},

};


/*
{ST_ADJUST_TIME			,EV_REFRESH         ,ST_ADJUST_TIME    ,&LCD_vAdjustTime	},
{ST_ADJUST_TIME			,EV_PREVIOUS_FIELD  ,ST_ADJUST_TIME    ,&LCD_vAdjustTime	},
{ST_ADJUST_TIME			,EV_NEXT_FIELD      ,ST_ADJUST_TIME    ,&LCD_vAdjustTime	},
{ST_ADJUST_TIME			,EV_UP      		,ST_ADJUST_TIME    ,&LCD_vAdjustTime	},
{ST_ADJUST_TIME			,EV_DOWN     		,ST_ADJUST_TIME    ,&LCD_vAdjustTime	},
{ST_ADJUST_TIME			,EV_BACK_TO_MAIN    ,ST_MAIN    	   ,&LCD_vMainScreen	},

{ST_MENU_DEBUG			,EV_LINE1       ,ST_PAGE1           ,&LCD_vManualMode		},

//Page1
{ST_PAGE1			,EV_REFRESH         ,ST_PAGE1    	,&LCD_vManualMode	},
{ST_PAGE1			,EV_PREVIOUS_FIELD  ,ST_PAGE1    	,&LCD_vManualMode	},
{ST_PAGE1			,EV_NEXT_FIELD      ,ST_PAGE1    	,&LCD_vManualMode	},
{ST_PAGE1			,EV_UP      		,ST_PAGE1    	,&LCD_vManualMode	},
{ST_PAGE1			,EV_DOWN     		,ST_PAGE1		,&LCD_vManualMode	},
{ST_PAGE1			,EV_BACK_TO_MAIN    ,ST_MENU_DEBUG  ,&LCD_vMenuDebug	},

//Page2
{ST_PAGE2	     	,EV_REFRESH   	 ,ST_PAGE2       		,&LCD_vReboot	    	},//MENU
{ST_PAGE2		 	,EV_KBD_CANCEL	 ,ST_MENU_DEBUG			,&LCD_vMenuDebug  		},
};
*/

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

struct{
	union{
		struct {
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

/* Display message if no message is displayed. Otherwise, display only if it has more displayTime and higher priority than current message */
void displayErrorMessage(char *firstLine, char *secondLine, uint32_t displayTime, uint8_t priority){

	/* For security reasons it applies WARNING_MSG_PRIORITY_MASK */
	uint8_t priorityMask = priority & WARNING_MSG_PRIORITY_MASK;

	if (hasErrorToDisplay()){
		/* Verify Priority and displayTime */
		if(((lastWarningMessageConfigExecuted & WARNING_MSG_PRIORITY_MASK) < priorityMask)) {
			return;
		}

		if(((lastWarningMessageConfigExecuted & WARNING_MSG_PRIORITY_MASK) == priorityMask)) {
			if ((displayTime<errorStringTimeToDisplay)) {
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

bool hasErrorToDisplay(){
	return ( sysTickTimer - errorStringStartTime < errorStringTimeToDisplay);
}


StEvents LCD_vGetNextEvent(void){
	//Eventos gerados pelas Key.
	if(getVirtualKeyState(KEY_CANCEL)){
		setVirtualKeyState(KEY_CANCEL,0); //consumer

		LCD_vSetNextEvent(EV_KBD_CANCEL);

		if(sm.state == ST_ADJUST_TIME || (sm.state == ST_PAGE1) || (sm.state == ST_ADJUST_TEMPERATURE_0) || (sm.state == ST_ADJUST_TEMPERATURE_1) || (sm.state == ST_SAVE_LOG) ||
				(sm.state == ST_PAGE9)){
			LCD_vSetNextEvent(EV_PREVIOUS_FIELD);
		}
	}
	else if(getVirtualKeyState(KEY_DOWN)){
		setVirtualKeyState(KEY_DOWN,0); //consumer

		if((sm.state == ST_MAIN) && (lcd.sbLine == LINE1_TIME) &&  (multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] > 0))
			lcd.sbLine++; //If #temp_sensors > 0 and lcd is showing line 1 (Time) - go to Temperature 1 exhibition
		else if ((sm.state == ST_MAIN) && (lcd.sbLine == LINE1_TIME) &&  (multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] < 1))
			lcd.sbLine+=3; //If #temp_sensors == 0 and lcd is showing line 1 (Time) - skip temperature 1 and 2 exhibition
		else if((sm.state == ST_MAIN) && (lcd.sbLine == LINE2_TEMP_0) &&  (multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] > 1) &&
				!(multiplex.currentPtc.memory[MEMORY_INDEX_COUPLED_TEMPERATURE]))
			lcd.sbLine++; //if #temp_sensor > 1 and lcd is showing line 2 (Temp Calef 1) - go to Temperature 2 exhibition
		else if((sm.state == ST_MAIN) && (lcd.sbLine == LINE2_TEMP_0) &&  ((multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] < 2) ||
						(multiplex.currentPtc.memory[MEMORY_INDEX_COUPLED_TEMPERATURE])))
			lcd.sbLine+=2; //if lcd is showing line 2 (Temp Calef 1) and (#temp_sensor < 2 or temps are coupled) - skip temperature 2 exhibition
		else
			lcd.sbLine++; //Handles others state transitions.

		if(lcd.sbLine > lcd.sbLineMax ) (lcd.sbLine = lcd.sbLineMin);

		LCD_vSetNextEvent(EV_REFRESH);

		if(sm.state == ST_ADJUST_TIME || (sm.state == ST_PAGE1) || (sm.state == ST_ADJUST_TEMPERATURE_0) || (sm.state == ST_ADJUST_TEMPERATURE_1)
				|| (sm.state == ST_PAGE8) || (sm.state == ST_SAVE_LOG) || (sm.state == ST_PAGE9)){
			LCD_vSetNextEvent(EV_DOWN);
		}
	}
	else if(getVirtualKeyState(KEY_UP)){
		setVirtualKeyState(KEY_UP,0); //consumer

		//lcd.sbLine--; //só roda linha quando o x for zero.

		if((sm.state == ST_MAIN) && (lcd.sbLine == LINE4_FIRMWARE) &&  (multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] >= 2)
				&& (!multiplex.currentPtc.memory[MEMORY_INDEX_COUPLED_TEMPERATURE]))
					lcd.sbLine--;
		else if ((sm.state == ST_MAIN) && (lcd.sbLine == LINE4_FIRMWARE) &&  ((multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] == 1)
				|| (multiplex.currentPtc.memory[MEMORY_INDEX_COUPLED_TEMPERATURE] && (multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] != 0) )))
			lcd.sbLine-=2;
		else if((sm.state == ST_MAIN) && (lcd.sbLine == LINE4_FIRMWARE) &&  (multiplex.currentPtc.memory[MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY] <= 0))
			lcd.sbLine-=3;
		else
			lcd.sbLine--;

		if (lcd.sbLine < lcd.sbLineMin) (lcd.sbLine = lcd.sbLineMax);

		LCD_vSetNextEvent(EV_REFRESH);

		if(sm.state == ST_ADJUST_TIME || (sm.state == ST_PAGE1) || (sm.state == ST_ADJUST_TEMPERATURE_0) || (sm.state == ST_ADJUST_TEMPERATURE_1)
				|| (sm.state == ST_PAGE8) || (sm.state == ST_SAVE_LOG) || (sm.state == ST_PAGE9)){
			LCD_vSetNextEvent(EV_UP);
		}
	}
	else if(getVirtualKeyState(KEY_ENTER)){
		setVirtualKeyState(KEY_ENTER,0); //Consumer

		/* Allow enter only on Main, Debug menu or UserSettings */
		if ((sm.state == ST_MAIN) || (sm.state == ST_MENU_DEBUG)||(sm.state == ST_PAGE2)){
			LCD_vSetNextEvent(lcd.sbLine);
		}

		if(sm.state == ST_ADJUST_TIME || (sm.state == ST_PAGE1) || (sm.state == ST_ADJUST_TEMPERATURE_0) || (sm.state == ST_ADJUST_TEMPERATURE_1) || (sm.state == ST_SAVE_LOG) ||
				(sm.state == ST_PAGE9)){
			LCD_vSetNextEvent(EV_NEXT_FIELD);
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
			((lastWarningMessageConfigExecuted & WARNING_MSG_PRIORITY_MASK)!=PRIORITY_6_IGNORE_ALL_NO_ERROR_MESSAGES) ){
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
			if ((sm.state == sm.trans[i].state) || (ST_ANY == sm.trans[i].state)) {
				if (((sm.event == sm.trans[i].event) || (EV_ANY == sm.trans[i].event)))	{
	                sm.state = sm.trans[i].next;

	                if (sm.trans[i].fn)
	                    (sm.trans[i].fn)();
	                break;
				}
			}
		}

	}
}

void LCD_vMainScreen(void){
	char lines[][LINE_SIZE]={"      GIGA 3     "
			                ,"      __:__      "
			                ,"    TESTE MPX    "
			                ,"   TESTE PTC 24  "
							,"   TESTE PTC 16  "};

	uint8_t numLines = sizeof(lines)/LINE_SIZE;
	int n_sensors;

	uint32_t *id = getUniqueID();

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

	time_t rawtime;
	time ( &rawtime );

	currentTime.currentHour = getHours(rawtime);
	currentTime.currentMinute = getMinutes(rawtime);
	currentTime.currentDay = getDay(rawtime);
	currentTime.currentMonth = getMonth()(rawtime);
	currentTime.currentYear = getYear(rawtime);

	char timerSeparationChar=' ';
	/* 500 ms blink time */
	if ((sysTickTimer%1000)>499){
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

	static uint8_t maxFieldValues[5]={60,24,99,13,32};
	static uint8_t cursorPosition[5]={0x4F, 0x4C, 0x49, 0x44, 0x41};

	uint8_t numFields=sizeof(maxFieldValues) - 1;
	static int8_t currentField=0;

	LCD_vSetNextEvent(EV_REFRESH);

	struct tm oldTimestamp, newTime;
	time_t rawtime;

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

	if (sm.event == EV_UP){
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

	snprintf(lines[1],LINE_SIZE,"%02d/%02d/%04d %02d:%02d ", currentTime.currentDay, currentTime.currentMonth, currentTime.currentYear+1900,
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
void LCD_vSetProgressBar (uint8_t progress) {

	char line [1][LINE_SIZE];
	static const char progressBar[LINE_SIZE] = "****************";
	static const uint8_t percentages[] = {1,2,3,5,7,8,10,12,14,15,15};
	char *progressStart;

	if(progress > 10)
		progress = 10;

	if(progress <= 10) {
		progressStart = &progressBar[LINE_SIZE-1-percentages[progress]];
		if(currentLogSaveMsg == LCD_USB_MSG_SAVING) {
			sniprintf(line[0], LINE_SIZE, "%s                 ", progressStart);
		}
			LCD_printLine(1,line[0]);
	}
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

void LCD_vADCScreen(void){
	char lines[][LINE_SIZE]={"    Menu ADC    ",
			                        "Vbb:            ",
			                        "Temp:           ", };
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

	snprintf(lines[1],LINE_SIZE,"Vbb:%.3f           ",getVbbVoltage());
	snprintf(lines[2],LINE_SIZE,"Temp:%.3f          ",getTemperatureVoltage());

	/* Print currentLine */s
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[currentLine]);

	LCD_vSetNextEvent(EV_REFRESH);
}


void LCD_vCANScreen(void){
	static char lines[][LINE_SIZE]={"    Menu CAN    ",
									"Error code:     ",
									"R Pkts:         ",
									"T Errors:       "};
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


	snprintf(lines[1],LINE_SIZE,"Error code:0x%x    ",CAN_GetLastErrorCode(CAN1));

	/* Print currentLine */
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[currentLine]);

	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vRTCScreen(void){
	char lines[][LINE_SIZE]={"    Menu RTC    ",
									"RTC:            "};
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


	snprintf(lines[1],LINE_SIZE,"RTC:%lus          ",RTC_GetCounter());

	/* Print currentLine */
	LCD_Clear();
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[currentLine]);

	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vIOsScreen(void){
	char lines[][LINE_SIZE]={"    Entradas    ",
									"+15:            ",
									"User Tact:      ",
									"TACO:           ",
									"ODO:            "};
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

	snprintf(lines[1],LINE_SIZE,"+15: %d          ",isInginitionHigh());
	snprintf(lines[2],LINE_SIZE,"User Tact: %d    ",isTactButtonPressed());
	snprintf(lines[3],LINE_SIZE,"TACO: %d         ",isTacoHigh());
	snprintf(lines[4],LINE_SIZE,"ODO: %d          ",isOdoHigh());

	/* Print currentLine */
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[currentLine]);

	LCD_vSetNextEvent(EV_REFRESH);
}

void LCD_vTimingScreen(void){
	char lines[2][LINE_SIZE]={" Tempo tarefas  "};
	uint8_t numLines = MAX_VIRTUAL_TIMERS+1;

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


	snprintf(lines[1],LINE_SIZE,"Tarefa %d: %lums    ",currentLine-1,getMaxExecutionTime(currentLine-1));

	/* Print currentLine */
	LCD_printLine(0,lines[0]);
	LCD_printLine(1,lines[1]);

	LCD_vSetNextEvent(EV_REFRESH);
}


#define NO_MPX_SELECTED -1


void LCD_vManualMode(void){
	static int8_t selectedMpx=NO_MPX_SELECTED;

	LCD_vSetNextEvent(EV_REFRESH);

	/* If no MPX selected */
	if (selectedMpx==NO_MPX_SELECTED){
		uint8_t numMpx=multiplex.numberOfConfiguredMpx;
		static int8_t currentMpx=0;

		if(sm.event == EV_NEXT_FIELD){
			selectedMpx=currentMpx;
		}


		if(sm.event == EV_PREVIOUS_FIELD){
				LCD_vSetNextEvent(EV_BACK_TO_MAIN);
		}

		if (sm.event == EV_UP){
			currentMpx--;
			if (currentMpx<0){
				currentMpx=numMpx-1;
			}
		}

		if (sm.event == EV_DOWN){
			currentMpx++;
			if (currentMpx>=numMpx){
				currentMpx=0;
			}
		}

		static char mpxName[LINE_SIZE];
		snprintf(mpxName,LINE_SIZE,"%s                ",multiplex.mpx[currentMpx].mpxName);

		/* Print currentLine */
		LCD_printLine(0,"  Diag. Manual  ");
		LCD_printLine(1,mpxName);
	}
	else{
		uint8_t numPorts=multiplex.mpx[selectedMpx].numPorts;
		static char lines[NUM_PORTS+1][LINE_SIZE]={"MPX:            "};
		static int8_t currentPort=0;

		snprintf(lines[0],LINE_SIZE,"MPX:%s            ",multiplex.mpx[selectedMpx].mpxName);

		int i;
		for(i=0;i<numPorts;i++){

				char cPortState='_';
				char iPortState='_';
				char sPortState='_';
				if (multiplex.mpx[selectedMpx].portOutput[i].mode==3){
					cPortState='H';
				}
				else if (multiplex.mpx[selectedMpx].portOutput[i].mode==1) {
					cPortState='L';
				}
				if(multiplex.mpx[selectedMpx].portType[i] == PORT_TYPE_LO_DIGITAL_INPUT) {
					if(multiplex.mpx[selectedMpx].portInput[i] & INPUT_LOW_MASK){
						iPortState='L';
					}
					else{
						iPortState='H';
					}
				}
				else {
					if(multiplex.mpx[selectedMpx].portInput[i] & INPUT_HIGH_MASK){
						iPortState='H';
					}
					else{
						iPortState='L';
					}
				}

				bool shortFlag = multiplex.mpx[selectedMpx].portInput[i] & INPUT_SHORT_MASK;
				bool openFlag = multiplex.mpx[selectedMpx].portInput[i] & INPUT_OPEN_MASK;


				if(shortFlag && openFlag){
					sPortState='E';
				}
				else if (shortFlag){
					sPortState='S';
				}
				else if (openFlag){
					sPortState='O';
				}

				snprintf(lines[i+1],LINE_SIZE,"%-13.13s%c%c%c",
						multiplex.mpx[selectedMpx].portName[i],
						sPortState,
						iPortState,
						cPortState);
			
		}

		if(sm.event == EV_NEXT_FIELD){

			uint8_t portType = multiplex.mpx[selectedMpx].portType[currentPort];
			uint8_t currentMode = multiplex.mpx[selectedMpx].portOutput[currentPort].mode;

			if(portType == PORT_TYPE_PUSH_PULL_10A){
				currentMode++;
				if (currentMode>=4){
					currentMode = 0;
				}


				/* On and off intercalated */
				if(currentMode%2){
					multiplex.mpx[selectedMpx].portOutput[currentPort].duty = 10;

					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftDown = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].ton = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].toff = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftDown = 0;
				}
				else{
					multiplex.mpx[selectedMpx].portOutput[currentPort].duty = 0;

					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftDown = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].ton = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].toff = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftDown = 0;
				}

				multiplex.mpx[selectedMpx].portOutput[currentPort].mode = currentMode;


				multiplex.mpx[selectedMpx].outputChanged[currentPort] = true;
			}
			else if ((portType == PORT_TYPE_BIDI_10A) || (portType == PORT_TYPE_BIDI_2A)){
				currentMode+=3;
				if (currentMode>=4){
					currentMode = 0;
				}

				multiplex.mpx[selectedMpx].portOutput[currentPort].mode = currentMode;

				/* On and off intercalated */
				if (currentMode==3){
					multiplex.mpx[selectedMpx].portOutput[currentPort].duty = 10;

					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftDown = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].ton = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].toff = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftDown = 0;
				}
				else{
					multiplex.mpx[selectedMpx].portOutput[currentPort].duty = 0;

					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].fastSoftDown = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].ton = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].toff = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftUp = 0;
					multiplex.mpx[selectedMpx].portOutput[currentPort].slowSoftDown = 0;
				}

				multiplex.mpx[selectedMpx].outputChanged[currentPort] = true;
			}
			else if (portType == PORT_TYPE_LO_DIGITAL_INPUT){

			}

		}

		if(sm.event == EV_PREVIOUS_FIELD){
				selectedMpx=NO_MPX_SELECTED;
				currentPort=0;
		}

		if (sm.event == EV_UP){
			currentPort--;
			if (currentPort<0){
				currentPort=numPorts-1;
			}
		}

		if (sm.event == EV_DOWN){
			currentPort++;
			if (currentPort>=numPorts){
				currentPort=0;
			}
		}

		/* Print currentLine */
		LCD_printLine(0,lines[0]);
		LCD_printLine(1,lines[currentPort+1]);
	}
}


void LCD_vReboot(void){
	rebootPTC();
}

void LCD_vEditBusName(void) {

	char *busName = multiplex.busName;
	char line[LINE_SIZE];
	static int8_t currentField=0;
	uint8_t numFields = 11;
	char maxChar = 'Z', minChar = ' ';
	char emptyFields[] = {"____________"};

	LCD_vSetNextEvent(EV_REFRESH);

	if(sm.event == EV_PREVIOUS_FIELD){
		currentField--;
		if (currentField<0){
			currentField=11;
			LCD_vSetNextEvent(EV_BACK_TO_MAIN);
			saveBusNameFromRamToFlash(&multiplex);
		}
	}

	if(sm.event == EV_NEXT_FIELD){
		currentField++;
		if (currentField>numFields){
			currentField=0;
			LCD_vSetNextEvent(EV_BACK_TO_MAIN);
			saveBusNameFromRamToFlash(&multiplex);
		}
	}

	if (sm.event == EV_UP){
		busName[currentField]++;

		if (busName[currentField]>=maxChar){
			busName[currentField] = minChar;
		}

		if (busName[currentField]<=minChar){
			busName[currentField] = ' ';
		}

		if(busName[currentField] == ('9' + 1)) { //ASCII('9') + 1
			busName[currentField] = 'A';
		}
		if(busName[currentField] == (' ' + 1)) {
			busName[currentField] = '0';
		}
	}

	if (sm.event == EV_DOWN){
		if (busName[currentField]<=minChar){
			busName[currentField] = maxChar + 1;
		}

		if(busName[currentField] == '0') {
			busName[currentField] = (' ' + 1);
		}
		if(busName[currentField] == 'A') { //ASCII - A
			busName[currentField] = ('9' + 1);
		}
		busName[currentField]--;
	}

	uint8_t busNameLen = strlen(busName);
	uint8_t i=0;
	emptyFields[12 -  busNameLen] = '\0';

	/* Prevents trash after \0 character */
	if(currentField > busNameLen - 1) {
		busName[currentField] = '\0';
	}

	/* Erase white spaces after last letter */
	for(i=busNameLen-1; i>=0; i--) {
		if(busName[i] == ' ')
			busName[i] = '\0';
		else
			break;
	}

	snprintf(line, LINE_SIZE, "  %s%s   ", busName, emptyFields);

	LCD_CursorBlink(true);
	LCD_printLine(0," NOME DO ONIBUS  ");
	LCD_printLine(1,line);
	LCD_MoveCursor(currentField + 0x42);
}

void setLCDAsTurnedOff(bool value){
	showLCDAsTurnedOff = value;
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
