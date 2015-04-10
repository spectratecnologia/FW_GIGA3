/* Includes ------------------------------------------------------------------*/
#include "ptc16_statemachine.h"

/* Local functions declaration #1 --------------------------------------------*/
void ptc16_vUpdateTests(void);
void ptc16_vResetTests(void);
void ptc16_vIdle(void);
void ptc16_vExecute(void);
void ptc16_vWait(void);
void ptc16_vAnalyse(void);
void ptc16_vFinalize(void);
void ptc16_vPrint(void);
void ptc16_vFinish(void);

/* Local functions declaration #2 --------------------------------------------*/
void ptc16_vExecute_Pendrive(void);
void ptc16_vAnalyse_CAN1(void);
void ptc16_vAnalyse_CAN2(void);
void ptc16_vAnalyse_IgnOn(void);
void ptc16_vAnalyse_Taco(void);
void ptc16_vAnalyse_Odo(void);
void ptc16_vAnalyse_Buzzer(void);
void ptc16_vAnalyse_KeysOn(void);
void ptc16_vAnalyse_KeysOff(void);
void ptc16_vAnalyse_WarningLeds(void);
void ptc16_vAnalyse_Pendrive(void);
void ptc16_vAnalyse_EndTest(void);

/* Local functions declaration #3 --------------------------------------------*/
void (*printTestResult)(void);
void ptc16printTestMessage(char*, char*, uint8_t);
void prc16print_ClearMessages(void);
void ptc16print_CAN1Error(void);
void ptc16print_CAN2Error(void);
void ptc16print_IgnError(void);
void ptc16print_TacoError(void);
void ptc16print_OdoError(void);
void ptc16print_Buzzer(void);
void ptc16print_PressKeyOn(void);
void ptc16print_PressKeyOff(void);
void ptc16print_AllLedsKeyOn(void);
void ptc16print_AllLedsKeyOff(void);
void ptc16print_AllWarningLedsOn(void);
void ptc16print_AllWarningLedsOff(void);
void ptc16print_USBTestMessage(void);
void ptc16print_EndTestOk(void);
void ptc16print_WaitMessage(void);

void ptc16print_OnGoing(void);

//AAAAAAAAAAAAAAAAAAAAAAAAA
//sendCanPacket(CAN1, 0x00, Ptc16Tests.testOk, Ptc16Tests.testFinished, Ptc16Tests.currentTest, 0, 0);

/* Local Variables -----------------------------------------------------------*/
StateMachine Ptc16StateMachine;
StructPtc16Test Ptc16Tests;
char message[LINE_SIZE];
static volatile int8_t nextEvent;
static volatile int PTC16_NUM_TACO_TRIES;
static volatile int PTC16_NUM_ODO_TRIES;
uint8_t CAN2DataReference[8] = {0x12, 0x45, 0x32, 0xA3, 0x7F, 0x90, 0xDC, 0x1E};

const Transition Ptc16SMTrans[] =  		//TABELA DE ESTADOS
{
/*Current state			Event				Next state				callback */
{PTC16_ST_IDLE			,PTC16_EV_REFRESH   ,PTC16_ST_IDLE     		,&ptc16_vIdle		},
{PTC16_ST_IDLE			,PTC16_EV_EXECUTE   ,PTC16_ST_EXECUTE  		,&ptc16_vExecute	},
{PTC16_ST_IDLE			,PTC16_EV_FINISH    ,PTC16_ST_FINISH    	,&ptc16_vFinish		},

{PTC16_ST_EXECUTE		,PTC16_EV_REFRESH  	,PTC16_ST_EXECUTE  		,&ptc16_vExecute	},
{PTC16_ST_EXECUTE		,PTC16_EV_PRINT    	,PTC16_ST_PRINT    		,&ptc16_vPrint 		},
{PTC16_ST_EXECUTE		,PTC16_EV_WAIT    	,PTC16_ST_WAIT       	,&ptc16_vWait 		},
{PTC16_ST_EXECUTE		,PTC16_EV_FINISH   	,PTC16_ST_FINISH    	,&ptc16_vFinish		},

{PTC16_ST_WAIT			,PTC16_EV_REFRESH  	,PTC16_ST_WAIT       	,&ptc16_vWait		},
{PTC16_ST_WAIT			,PTC16_EV_PRINT	 	,PTC16_ST_PRINT_WAIT	,&ptc16_vPrint		},
{PTC16_ST_WAIT			,PTC16_EV_ANALYSE  	,PTC16_ST_ANALYSE    	,&ptc16_vAnalyse 	},
{PTC16_ST_WAIT			,PTC16_EV_FINISH   	,PTC16_ST_FINISH    	,&ptc16_vFinish		},

{PTC16_ST_ANALYSE		,PTC16_EV_REFRESH  	,PTC16_ST_ANALYSE    	,&ptc16_vAnalyse	},
{PTC16_ST_ANALYSE		,PTC16_EV_EXECUTE  	,PTC16_ST_EXECUTE    	,&ptc16_vExecute	},
{PTC16_ST_ANALYSE		,PTC16_EV_FINALIZE 	,PTC16_ST_FINALIZE   	,&ptc16_vFinalize	},
{PTC16_ST_ANALYSE		,PTC16_EV_FINISH   	,PTC16_ST_FINISH    	,&ptc16_vFinish		},

{PTC16_ST_FINALIZE		,PTC16_EV_REFRESH  	,PTC16_ST_FINALIZE   	,&ptc16_vFinalize	},
{PTC16_ST_FINALIZE		,PTC16_EV_IDLE    	,PTC16_ST_IDLE    		,&ptc16_vIdle 		},
{PTC16_ST_FINALIZE		,PTC16_EV_PRINT    	,PTC16_ST_PRINT    		,&ptc16_vPrint 		},
{PTC16_ST_FINALIZE		,PTC16_EV_FINISH   	,PTC16_ST_FINISH    	,&ptc16_vFinish		},

{PTC16_ST_PRINT_WAIT	,PTC16_EV_REFRESH  	,PTC16_ST_PRINT_WAIT 	,&ptc16_vPrint    	},
{PTC16_ST_PRINT_WAIT	,PTC16_EV_WAIT	   	,PTC16_ST_WAIT     		,&ptc16_vWait 		},
{PTC16_ST_PRINT_WAIT	,PTC16_EV_FINISH   	,PTC16_ST_FINISH    	,&ptc16_vFinish		},

{PTC16_ST_PRINT			,PTC16_EV_REFRESH  	,PTC16_ST_PRINT      	,&ptc16_vPrint		},
{PTC16_ST_PRINT			,PTC16_EV_IDLE	   	,PTC16_ST_IDLE      	,&ptc16_vIdle		},
{PTC16_ST_PRINT			,PTC16_EV_FINISH   	,PTC16_ST_FINISH    	,&ptc16_vFinish		},

{PTC16_ST_FINISH		,PTC16_EV_REFRESH  	,PTC16_ST_FINISH     	,&ptc16_vFinish		},
{PTC16_ST_FINISH		,PTC16_EV_IDLE 	  	,PTC16_ST_IDLE     		,&ptc16_vIdle		},
};

#define TRANS_COUNT (sizeof(Ptc16SMTrans)/sizeof(*Ptc16SMTrans))

/* ---------------------------------------------------------------------------*/
/* General state machine functions -------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void ptc16Test_vStateMachineLoop(void)
{
	int i;
	Ptc16StateMachine.event = nextEvent;

	/* Execute test state machine */
	for (i = 0; i < TRANS_COUNT; i++)
	{
		if ( (Ptc16StateMachine.state == Ptc16StateMachine.trans[i].state) ||
			 (PTC16_ST_ANY == Ptc16StateMachine.trans[i].state) )
		{
			if ( (Ptc16StateMachine.event == Ptc16StateMachine.trans[i].event) ||
				 (PTC16_EV_ANY == Ptc16StateMachine.trans[i].event) )
			{
				Ptc16StateMachine.state = Ptc16StateMachine.trans[i].next;

                if (Ptc16StateMachine.trans[i].fn)
                {
                    (Ptc16StateMachine.trans[i].fn)();
                }
                break;
			}
		}
	}
}

void ptc16Test_vStateMachineInit(void)
{
	/* Start or reset mpx test state machine */
	Ptc16StateMachine.state = PTC16_ST_IDLE;
	Ptc16StateMachine.trans = &Ptc16SMTrans;
	Ptc16StateMachine.transCount = TRANS_COUNT;
	ptc16_vSetNextEvent(PTC16_EV_REFRESH);

	ptc16_vResetTests();
}

void ptc16_vSetNextEvent(Ptc16TestEvents event)
{
	nextEvent = event;
}

void ptc16Test_vSetTest(Ptc16TestList test)
{
	Ptc16Tests.currentTest = test;
}

void ptc16Test_vTestOk(void)
{
	Ptc16Tests.testOk = true;
}

void ptc16Test_vTestFail(void)
{
	Ptc16Tests.testFail = true;
}

void ptc16Test_vFinishTest()
{
	ptc16_vSetNextEvent(PTC16_EV_FINISH);
}

/* ---------------------------------------------------------------------------*/
/* State machine main functions ----------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void ptc16_vResetTests(void)
{
	Ptc16Tests.currentTest = PTC16_TEST_NOTHING;
	Ptc16Tests.testOk = false;
	Ptc16Tests.testFail = false;
	Ptc16Tests.testFinished = false;
	Ptc16Tests.finishedTestBeep = false;
	Ptc16Tests.testError = false;
	Ptc16Tests.seriousError = false;
	ptc16_enableToogleOdo(false);
	ptc16_enableToogleTaco(false);
	ptc16.outputCommandReceived = false;
	ptc16.pendriveTestLog = 0xFF;
	ptc16.pendriveTestStarted = false;
	ptc16print_ClearMessages();
	PTC16_NUM_TACO_TRIES=0;
	PTC16_NUM_ODO_TRIES=0;
}

void ptc16_vUpdateTests(void)
{
	if (Ptc16Tests.currentTest == PTC16_TEST_START)
		/* Do first test: next test from PTC16_TEST_START. */
		Ptc16Tests.currentTest = PTC16_TEST_START + 1;

	else if ((Ptc16Tests.currentTest >= PTC16_TEST_IGN_OFF) && (Ptc16Tests.currentTest <= PTC16_TEST_ODO_ON))
	{
		if (Ptc16Tests.testFinished)
		{
			Ptc16Tests.currentTest++;
			Ptc16Tests.testFinished = false;
		}
	}

	else if (Ptc16Tests.currentTest == PTC16_TEST_PENDRIVE)
	{
		if (Ptc16Tests.testOk)
		{
			Ptc16Tests.currentTest++;
			Ptc16Tests.testFinished = false;
			Ptc16Tests.testOk = false;
		}
	}

	else if ((Ptc16Tests.currentTest >= PTC16_TEST_KEYS_ON) && (Ptc16Tests.currentTest <= PTC16_TEST_WARNINGLEDS_OFF))
	{
		if (Ptc16Tests.testOk && Ptc16Tests.testFinished)
		{
			Ptc16Tests.currentTest++;
			Ptc16Tests.testFinished = false;
			Ptc16Tests.testOk = false;
		}
	}

	else if (Ptc16Tests.currentTest == PTC16_TEST_END)
	{
		if (Ptc16Tests.testOk && Ptc16Tests.testFinished)
		{
			ptc16_vSetNextEvent(PTC16_EV_FINISH);
			Ptc16Tests.testFinished = false;
			Ptc16Tests.testOk = false;
		}
	}
}

/* Idle ----------------------------------------------------------------------*/
void ptc16_vIdle(void)
{
	/* Update Ptc16Tests.currentTest */
	ptc16_vUpdateTests();

	if (nextEvent == PTC16_EV_FINISH)
		return;

	if ((Ptc16Tests.currentTest > PTC16_TEST_START) && (Ptc16Tests.currentTest <= PTC16_TEST_END))
		ptc16_vSetNextEvent(PTC16_EV_EXECUTE);

	else
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
}

/* Execute -------------------------------------------------------------------*/
void ptc16_vExecute(void)
{
	Ptc16Tests.statedTestTime = sysTickTimer;

	if (Ptc16Tests.currentTest == PTC16_TEST_IGN_OFF)
		activePTC16Ign(DISABLE);

	if (Ptc16Tests.currentTest == PTC16_TEST_IGN_ON)
		activePTC16Ign(ENABLE);

	if(Ptc16Tests.currentTest == PTC16_TEST_CAN2)
		sendCanPacket(CAN2, CAN_COMMAND_BROADCAST, 0x00, MY_ID, PTC16_DEVICE_ID, CAN2DataReference, 8);

	if (Ptc16Tests.currentTest == PTC16_TEST_ODO_ON)
		ptc16_enableToogleOdo(true);

	if (Ptc16Tests.currentTest == PTC16_TEST_TACO_ON)
		ptc16_enableToogleTaco(true);

	if(Ptc16Tests.currentTest == PTC16_TEST_PENDRIVE)
		if (!ptc16.pendriveTestStarted)
			ptc16_vExecute_Pendrive();

	ptc16_vSetNextEvent(PTC16_EV_WAIT);
}


/* Wait ---------------------------------------------------------------------*/
void ptc16_vWait(void)
{
	if ( (Ptc16Tests.currentTest == PTC16_TEST_ODO_OFF) &&
		 (sysTickTimer - Ptc16Tests.statedTestTime < PTC16_DELAY_SLOW_TESTS) )
	{
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
		printTestResult = ptc16print_WaitMessage;
	}

	else if ( (Ptc16Tests.currentTest == PTC16_TEST_ODO_ON) &&
		 (sysTickTimer - Ptc16Tests.statedTestTime < PTC16_DELAY_SLOW_TESTS) )
	{
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
		printTestResult = ptc16print_WaitMessage;
	}

	else if ( (Ptc16Tests.currentTest == PTC16_TEST_TACO_OFF) &&
			  (sysTickTimer - Ptc16Tests.statedTestTime < 2*PTC16_DELAY_SLOW_TESTS) )
	{
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
		printTestResult = ptc16print_WaitMessage;
	}

	else if ( (Ptc16Tests.currentTest == PTC16_TEST_TACO_ON) &&
			  (sysTickTimer - Ptc16Tests.statedTestTime < 2*PTC16_DELAY_SLOW_TESTS) )
	{
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
		printTestResult = ptc16print_WaitMessage;
	}

	else if ( (Ptc16Tests.currentTest == PTC16_TEST_IGN_OFF) &&
			  (sysTickTimer - Ptc16Tests.statedTestTime < PTC16_DELAY_SLOW_TESTS) )
	{
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
		printTestResult = ptc16print_WaitMessage;
	}

	else if ( (Ptc16Tests.currentTest == PTC16_TEST_IGN_ON) &&
			  (sysTickTimer - Ptc16Tests.statedTestTime < PTC16_DELAY_SLOW_TESTS) )
	{
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
		printTestResult = ptc16print_WaitMessage;
	}

	else
		ptc16_vSetNextEvent(PTC16_EV_ANALYSE);
}

/* Analysis ------------------------------------------------------------------*/
void ptc16_vAnalyse(void)
{
	ptc16_vSetNextEvent(PTC16_EV_FINALIZE);

	ptc16_vAnalyse_CAN1();

	if ((Ptc16Tests.currentTest == PTC16_TEST_IGN_OFF) && (!Ptc16Tests.testFinished))
		Ptc16Tests.testFinished = true;

	if ((Ptc16Tests.currentTest == PTC16_TEST_IGN_ON) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_IgnOn();

	if ((Ptc16Tests.currentTest == PTC16_TEST_CAN2) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_CAN2();

	if ((Ptc16Tests.currentTest == PTC16_TEST_TACO_OFF) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_Taco();

	if ((Ptc16Tests.currentTest == PTC16_TEST_TACO_ON) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_Taco();

	if ((Ptc16Tests.currentTest == PTC16_TEST_ODO_OFF) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_Odo();

	if ((Ptc16Tests.currentTest == PTC16_TEST_ODO_ON) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_Odo();

	if ((Ptc16Tests.currentTest == PTC16_TEST_KEYS_ON) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_KeysOn();

	if ((Ptc16Tests.currentTest == PTC16_TEST_BUZZER) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_Buzzer();

	if ((Ptc16Tests.currentTest == PTC16_TEST_WARNINGLEDS_ON) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_WarningLeds();

	if ((Ptc16Tests.currentTest == PTC16_TEST_KEYS_OFF) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_KeysOff();

	if ((Ptc16Tests.currentTest == PTC16_TEST_WARNINGLEDS_OFF) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_WarningLeds();

	if ((Ptc16Tests.currentTest == PTC16_TEST_PENDRIVE) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_Pendrive();

	if ((Ptc16Tests.currentTest == PTC16_TEST_END) && (!Ptc16Tests.testFinished))
		ptc16_vAnalyse_EndTest();
}

/* Finalize ------------------------------------------------------------------*/
void ptc16_vFinalize(void)
{
	ptc16_vSetNextEvent(PTC16_EV_PRINT);
}

/* Print ---------------------------------------------------------------------*/
void ptc16_vPrint(void)
{
	if (printTestResult)
		printTestResult();

	ptc16_vSetNextEvent(PTC16_EV_IDLE);

	if (Ptc16Tests.testError)
		ptc16_vSetNextEvent(PTC16_EV_REFRESH);
}

/* Finish test----------------------------------------------------------------*/
void ptc16_vFinish(void)
{
	ptc16_vResetTests();
	LCD_vSetNextEvent(EV_RETURN);
	ptc16_vSetNextEvent(PTC16_EV_IDLE);
}

/* ---------------------------------------------------------------------------*/
/* State machine secondary functions -----------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* Execute -------------------------------------------------------------------*/
void ptc16_vExecute_Pendrive()
{
	uint8_t activePendriveTest[8] = {0x0, 0x0, 0x0, MEMORY_INDEX_TEST_STEP, 0x0, 0x0, 0x0, 16};
	sendCanPacket(CAN1, CAN_COMMAND_WRITE, 0xE0, MY_ID, PTC16_DEVICE_ID, activePendriveTest, 8);
	ptc16.pendriveTestStarted = true;
}

/* Analysis ------------------------------------------------------------------*/
void ptc16_vAnalyse_CAN1()
{
	if ( (sysTickTimer - ptc16.lastTimeSeen > 200) && ((Ptc16Tests.currentTest > PTC16_TEST_PENDRIVE+1) || (Ptc16Tests.currentTest < PTC16_TEST_PENDRIVE-1)) )
	{
		Ptc16Tests.testError = true;
		Ptc16Tests.testFinished = true;
		setBeep(1, 2000);
		printTestResult = ptc16print_CAN1Error;
	}

	if ( (sysTickTimer - ptc16.lastTimeSeen > 7000) && (Ptc16Tests.currentTest <= PTC16_TEST_PENDRIVE+1) && (Ptc16Tests.currentTest >= PTC16_TEST_PENDRIVE-1) )
	{
		Ptc16Tests.testError = true;
		Ptc16Tests.testFinished = true;
		setBeep(1, 2000);
		printTestResult = ptc16print_CAN1Error;
	}
}

void ptc16_vAnalyse_CAN2()
{
	int i, j;

	for (i=0; i<8; i++)
	{
		if (CAN2DataReference[i] != ptc16.CAN2DataReceived[i])
		{
			Ptc16Tests.testError = true;
			Ptc16Tests.testFinished = true;
			setBeep(1, 2000);
			printTestResult = ptc16print_CAN2Error;
			/* Erase ptc16.CAN2DataReceived aiming doesn't leave residues for future tests. */
			for (j=0; j<8; j++)
				ptc16.CAN2DataReceived[j]=0;
			return;
		}
	}

	/* This functions is only reached if doesn't have any errors at CAN2's received data. */
	Ptc16Tests.testError = false;
	Ptc16Tests.testFinished = true;
}

void ptc16_vAnalyse_IgnOn()
{
	if (ptc16.outputCommandReceived)
	{
		Ptc16Tests.testError = false;
		Ptc16Tests.testFinished = true;
	}

	else
	{
		Ptc16Tests.testError = true;
		Ptc16Tests.testFinished = true;
		setBeep(1, 2000);
		printTestResult = ptc16print_IgnError;
	}

}

void ptc16_vAnalyse_Taco()
{
	int i, num_tries = 5;

	if (Ptc16Tests.currentTest == PTC16_TEST_TACO_OFF)
	{
		if (!ptc16.tacoReceivedCommand)
		{
			Ptc16Tests.testError = false;
			Ptc16Tests.testFinished = true;
			PTC16_NUM_TACO_TRIES=0;
			return;
		}

		/*
		else
		{
			Ptc16Tests.testError = true;
			Ptc16Tests.testFinished = true;
			setBeep(1, 2000);
			printTestResult = ptc16print_TacoError;
		}
		*/
	}

	else if (Ptc16Tests.currentTest == PTC16_TEST_TACO_ON)
	{
		if (ptc16.tacoReceivedCommand)
		{
			Ptc16Tests.testError = false;
			Ptc16Tests.testFinished = true;
			PTC16_NUM_TACO_TRIES=0;
			return;
		}

		/*
		else
		{
			Ptc16Tests.testError = true;
			Ptc16Tests.testFinished = true;
			setBeep(1, 2000);
			printTestResult = ptc16print_TacoError;
			ptc16_enableToogleTaco(false);
		}
		*/
	}

	if (PTC16_NUM_TACO_TRIES == num_tries)
	{
		Ptc16Tests.testError = true;
		Ptc16Tests.testFinished = true;
		setBeep(1, 2000);
		printTestResult = ptc16print_TacoError;
		ptc16_enableToogleTaco(false);
	}

	PTC16_NUM_TACO_TRIES++;
}

void ptc16_vAnalyse_Odo()
{
	int i, num_tries = 5;

	if (Ptc16Tests.currentTest == PTC16_TEST_ODO_OFF)
	{
		if (!ptc16.odoReceivedCommand)
		{
			Ptc16Tests.testError = false;
			Ptc16Tests.testFinished = true;
			PTC16_NUM_ODO_TRIES=0;
			return;
		}
	}

	else if (Ptc16Tests.currentTest == PTC16_TEST_ODO_ON)
	{
		if (ptc16.odoReceivedCommand)
		{
			Ptc16Tests.testError = false;
			Ptc16Tests.testFinished = true;
			PTC16_NUM_ODO_TRIES=0;
			return;
		}
	}

	if (PTC16_NUM_ODO_TRIES == num_tries)
	{
		Ptc16Tests.testError = true;
		Ptc16Tests.testFinished = true;
		setBeep(1, 2000);
		printTestResult = ptc16print_OdoError;
		ptc16_enableToogleOdo(false);
	}
	PTC16_NUM_ODO_TRIES++;
}

void ptc16_vAnalyse_Buzzer()
{
	printTestResult = ptc16print_Buzzer;
	setBeep(3, 100);
	Ptc16Tests.testFinished = true;
}

void ptc16_vAnalyse_KeysOn()
{
	int i,j;

	printTestResult = ptc16print_PressKeyOn;

	for (i=NUM_PTC16_KEYS-1; i>=0; i--)
		if ((Ptc16Tests.keysOn[i] == 0) && (ptc16.keyState[i] == 1))
		{
			Ptc16Tests.keysOn[i] = 1;
			printTestResult = ptc16print_PressKeyOn;
		}

	for (i=0; i<NUM_PTC16_KEYS; i++)
	{
		if(Ptc16Tests.keysOn[i] == 0)
			break;
		/* All keys was analysed and all was pressed. */
		if((Ptc16Tests.keysOn[NUM_PTC16_KEYS-1] == 1) && (i == NUM_PTC16_KEYS-1))
		{
			setBeep(3, 100);
			printTestResult = ptc16print_AllLedsKeyOn;
			for (j=0; j<NUM_PTC16_KEYS; j++)
				Ptc16Tests.keysOn[j] = 0;
			Ptc16Tests.testFinished = true;
		}
	}
}

void ptc16_vAnalyse_KeysOff()
{
	int i,j;

	printTestResult = ptc16print_PressKeyOff;

	for (i=NUM_PTC16_KEYS-1; i>=0; i--)
		if ((Ptc16Tests.keysOff[i] == 0) && (ptc16.keyState[i] == 0))
		{
			Ptc16Tests.keysOff[i] = 1;
			printTestResult = ptc16print_PressKeyOff;
		}

	for (i=0; i<NUM_PTC16_KEYS; i++)
	{
		if(Ptc16Tests.keysOff[i] == 0)
			break;
		/* All keys was analysed and all was pressed. */
		if((Ptc16Tests.keysOff[NUM_PTC16_KEYS-1] == 1) && (i == NUM_PTC16_KEYS-1))
		{
			setBeep(3, 100);
			printTestResult = ptc16print_AllLedsKeyOff;
			for (j=0; j<NUM_PTC16_KEYS; j++)
				Ptc16Tests.keysOff[j] = 0;
			Ptc16Tests.testFinished = true;
		}
	}
}

void ptc16_vAnalyse_WarningLeds()
{
	if (Ptc16Tests.currentTest == PTC16_TEST_WARNINGLEDS_ON)
		printTestResult = ptc16print_AllWarningLedsOn;

	else if (Ptc16Tests.currentTest == PTC16_TEST_WARNINGLEDS_OFF)
		printTestResult = ptc16print_AllWarningLedsOff;

	setBeep(3, 100);
	Ptc16Tests.testFinished = true;
}

void ptc16_vAnalyse_Pendrive()
{
	printTestResult = ptc16print_USBTestMessage;

	switch (ptc16.pendriveTestLog)
	{
		case LCD_USB_MSG_FINISHED:
			setBeep(3, 100);
			Ptc16Tests.testFinished = true;
			ptc16.pendriveTestStarted = false;
			break;

		case LCD_USB_MSG_SAVE_TIMEOUT_ERROR:
		case LCD_USB_MSG_SAVE_ERROR:
		case LCD_USB_MSG_ERROR_CANT_CLOSE:
		case LCD_USB_MSG_ERROR_CANT_MOUNT:
		case LCD_USB_MSG_ERROR_CANT_CREATE_FILE:
		case LCD_USB_MSG_ERROR_CANT_WRITE:
		case LCD_USB_MSG_RM_ERROR:
		case LCD_USB_MSG_ERROR_CANT_OPEN_FILE:
			Ptc16Tests.testError = true;
			Ptc16Tests.testFinished = true;
			setBeep(1, 2000);
			ptc16.pendriveTestStarted = false;
			break;

		default:
			/* If cases above wasn't called, it's because test is not done
			 * on PTC16 and it's necessary keep state machine in analyse loop.
			 */
			//ptc16_vSetNextEvent(PTC16_EV_REFRESH);
			break;
	}
}

void ptc16_vAnalyse_EndTest()
{
	printTestResult = ptc16print_EndTestOk;
	setBeep(3, 100);
	Ptc16Tests.testFinished = true;
}

/* ---------------------------------------------------------------------------*/
/* Print LCD functions -------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void ptc16print_ClearMessages(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"                  ");
	snprintf(TestMessages.lines[1],LINE_SIZE,"                  ");
}

void ptc16print_WaitMessage(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "    Aguarde!    ");
	sprintf(message, "  Executando");
	printTestMessage(TestMessages.lines[1], message, 3);
}

void ptc16print_CAN1Error(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "CAN1 Erro: Veri-");
	sprintf(message, "ficarCN1.5,12,13");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc16print_CAN2Error(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "CAN2 Erro: Veri-");
	sprintf(message, "ficarCN1.7,14,15");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc16print_IgnError(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "IGN Erro: Veri- ");
	sprintf(message, "ficar CN1.9");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc16print_OdoError(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "ODO Erro: Veri- ");
	sprintf(message, "ficar CN1.4");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc16print_TacoError(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "TACO Erro: Veri-");
	sprintf(message, "ficar CN1.6");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc16print_Buzzer(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "Buzzer ok?       ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "(Primeira tecla) ");
}

void ptc16print_PressKeyOn(void)
{
	int i;
	for (i=0; i<NUM_PTC16_KEYS; i++)
		if(Ptc16Tests.keysOn[i] == 0)
			break;

	sprintf(message, "Press. tecla %d", i+1);

	snprintf(TestMessages.lines[0], LINE_SIZE, "Ligar LEDS      ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "%s              ", message);
}

void ptc16print_PressKeyOff(void)
{
	int i;
	for (i=0; i<NUM_PTC16_KEYS; i++)
		if(Ptc16Tests.keysOff[i] == 0)
			break;

	sprintf(message, "Press. tecla %d", i+1);
	snprintf(TestMessages.lines[0], LINE_SIZE, "Desligar LEDS    ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "%s               ", message);
}

void ptc16print_AllLedsKeyOn(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "Leds das teclas  ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "todos ligados?   ");
}

void ptc16print_AllLedsKeyOff(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "Leds das teclas  ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "todos desligado? ");
}

void ptc16print_AllWarningLedsOn(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "Leds de avisos   ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "todos ligados?   ");
}

void ptc16print_AllWarningLedsOff(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "Leds de avisos   ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "todos desligado? ");
}

void ptc16print_USBTestMessage()
{
	/* This command makes message doesn't change at GIGA3 display if PTC sends
	 * other log message with pen drive test already finished. */
	//if (Ptc16Tests.testFinished)
		//return;

	switch (ptc16.pendriveTestLog)
	{
		case 0xFF:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Teste: Pendrive ");
			sprintf(message, "Insira pendrive");
			printTestMessage(TestMessages.lines[1], message, 3);
			break;

		case LCD_USB_WAIT_MESSAGE:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Teste: Pendrive ");
			sprintf(message, "Executando");
			printTestMessage(TestMessages.lines[1], message, 3);
			break;

		case LCD_USB_MSG_NEW_LOGS:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Testar Escrita/ ");
			snprintf(TestMessages.lines[1], LINE_SIZE, "Leitura Pendrive");
			break;

		case LCD_USB_MSG_ALL_LOGS:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Testar Escrita/ ");
			snprintf(TestMessages.lines[1], LINE_SIZE, "Leitura Pendrive");
			break;

		case LCD_USB_MSG_DONT_DISCONNECT_THE_PENDRIVE:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Nao desconecte  ");
			sprintf(message, "o pendrive");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_SAVING:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Testando        ");
			sprintf(message, "Aguarde");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_FINISHED:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Teste realizado ");
			sprintf(message, "com sucesso");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_SAVE_TIMEOUT_ERROR:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro: USB time- ");
			sprintf(message, "out");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_SAVE_ERROR:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro ao salvar ");
			sprintf(message, "USB");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_ERROR_CANT_CLOSE:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro ao fechar ");
			sprintf(message, "arquivo");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_ERROR_CANT_MOUNT:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro ao montar ");
			sprintf(message, "FAT-FS");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_ERROR_CANT_CREATE_FILE:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro ao tentar ");
			sprintf(message, "criar arquivo");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_ERROR_CANT_WRITE:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro ao tentar ");
			sprintf(message, "escrever arquivo");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_RM_ERROR:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro ao tentar ");
			sprintf(message, "remover arquivo");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		case LCD_USB_MSG_ERROR_CANT_OPEN_FILE:
			snprintf(TestMessages.lines[0], LINE_SIZE, "Erro ao tentar ");
			sprintf(message, "abrir arquivo");
			printTestMessage(TestMessages.lines[1], message, 1);
			break;

		default:
			break;
	}
}

void ptc16print_EndTestOk(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste PTC16: OK ");
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc16print_OnGoing(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE," Em Construcao. ");
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc16printTestMessage(char *line, char *sentence, uint8_t dots)
{
	char finalpoint[3]={' ', ' ', ' '};

	if (dots == 3)
	{
		if ((sysTickTimer%4000)>999)
			finalpoint[0]='.';

		if ((sysTickTimer%4000)>1999)
			finalpoint[1]='.';

		if ((sysTickTimer%4000)>2999)
			finalpoint[2]='.';

		snprintf(line, LINE_SIZE, "%s%c%c%c        ", sentence, finalpoint[0], finalpoint[1], finalpoint[2]);
	}

	else if (dots == 1)
	{
		if ((sysTickTimer%2000)>999)
			finalpoint[0]='.';

		snprintf(line, LINE_SIZE, "%s%c         ", sentence, finalpoint[0]);
	}

	else
		snprintf(line, LINE_SIZE, "%s", sentence);
}
