/* Includes ------------------------------------------------------------------*/
#include "tests/ptc24_statemachine.h"

/* Local functions declaration #1 --------------------------------------------*/
void ptc24_vUpdateTests(void);
void ptc24_vResetTests(void);
void ptc24_vIdle(void);
void ptc24_vExecute(void);
void ptc24_vWait(void);
void ptc24_vAnalyse(void);
void ptc24_vFinalize(void);
void ptc24_vPrint(void);
void ptc24_vFinish(void);

/* Local functions declaration #2 --------------------------------------------*/
void ptc24_vAnalyse_CAN1(void);
void ptc24_vAnalyse_Ign(void);
void ptc24_vAnalyse_Taco(void);
void ptc24_vAnalyse_Odo(void);
void ptc24_vAnalyse_Buzzer(void);
void ptc24_vAnalyse_KeysOn(void);
void ptc24_vAnalyse_KeysOff(void);
void ptc24_vAnalyse_WarningLeds(void);
void ptc24_vAnalyse_EndTest(void);

/* Local functions declaration #3 --------------------------------------------*/
void (*printTestResult)(void);
void ptc24printTestMessage(char*, char*, uint8_t);
void ptc24print_CAN1Error(void);
void ptc24print_IgnError(void);
void ptc24print_TacoError(void);
void ptc24print_OdoError(void);
void ptc24print_Buzzer(void);
void ptc24print_PressKeyOn(void);
void ptc24print_PressKeyOff(void);
void ptc24print_AllLedsKeyOn(void);
void ptc24print_AllLedsKeyOff(void);
void ptc24print_AllWarningLedsOn(void);
void ptc24print_AllWarningLedsOff(void);
void ptc24print_EndTestOk(void);
void ptc24print_WaitMessage(void);

void ptc24print_OnGoing(void);

//AAAAAAAAAAAAAAAAAAAAAAAAA
//sendCanPacket(CAN1, 0x00, Ptc24Tests.testOk, Ptc24Tests.testFinished, Ptc24Tests.currentTest, 0, 0);

/* Local Variables -----------------------------------------------------------*/
StateMachine Ptc24StateMachine;
StructPtc24Test Ptc24Tests;
char message[LINE_SIZE];
static volatile int8_t nextEvent;

const Transition Ptc24SMTrans[] =  		//TABELA DE ESTADOS
{
/*Current state			Event				Next state				callback */
{PTC24_ST_IDLE			,PTC24_EV_REFRESH   ,PTC24_ST_IDLE     		,&ptc24_vIdle		},
{PTC24_ST_IDLE			,PTC24_EV_EXECUTE   ,PTC24_ST_EXECUTE  		,&ptc24_vExecute	},
{PTC24_ST_IDLE			,PTC24_EV_FINISH    ,PTC24_ST_FINISH    	,&ptc24_vFinish		},

{PTC24_ST_EXECUTE		,PTC24_EV_REFRESH  	,PTC24_ST_EXECUTE  		,&ptc24_vExecute	},
{PTC24_ST_EXECUTE		,PTC24_EV_PRINT    	,PTC24_ST_PRINT    		,&ptc24_vPrint 		},
{PTC24_ST_EXECUTE		,PTC24_EV_WAIT    	,PTC24_ST_WAIT       	,&ptc24_vWait 		},
{PTC24_ST_EXECUTE		,PTC24_EV_FINISH   	,PTC24_ST_FINISH    	,&ptc24_vFinish		},

{PTC24_ST_WAIT			,PTC24_EV_REFRESH  	,PTC24_ST_WAIT       	,&ptc24_vWait		},
{PTC24_ST_WAIT			,PTC24_EV_PRINT	 	,PTC24_ST_PRINT_WAIT	,&ptc24_vPrint		},
{PTC24_ST_WAIT			,PTC24_EV_ANALYSE  	,PTC24_ST_ANALYSE    	,&ptc24_vAnalyse 	},
{PTC24_ST_WAIT			,PTC24_EV_FINISH   	,PTC24_ST_FINISH    	,&ptc24_vFinish		},

{PTC24_ST_ANALYSE		,PTC24_EV_REFRESH  	,PTC24_ST_ANALYSE    	,&ptc24_vAnalyse	},
{PTC24_ST_ANALYSE		,PTC24_EV_EXECUTE  	,PTC24_ST_EXECUTE    	,&ptc24_vExecute	},
{PTC24_ST_ANALYSE		,PTC24_EV_FINALIZE 	,PTC24_ST_FINALIZE   	,&ptc24_vFinalize	},
{PTC24_ST_ANALYSE		,PTC24_EV_FINISH   	,PTC24_ST_FINISH    	,&ptc24_vFinish		},

{PTC24_ST_FINALIZE		,PTC24_EV_REFRESH  	,PTC24_ST_FINALIZE   	,&ptc24_vFinalize	},
{PTC24_ST_FINALIZE		,PTC24_EV_IDLE    	,PTC24_ST_IDLE    		,&ptc24_vIdle 		},
{PTC24_ST_FINALIZE		,PTC24_EV_PRINT    	,PTC24_ST_PRINT    		,&ptc24_vPrint 		},
{PTC24_ST_FINALIZE		,PTC24_EV_FINISH   	,PTC24_ST_FINISH    	,&ptc24_vFinish		},

{PTC24_ST_PRINT_WAIT	,PTC24_EV_REFRESH  	,PTC24_ST_PRINT_WAIT 	,&ptc24_vPrint    	},
{PTC24_ST_PRINT_WAIT	,PTC24_EV_WAIT	   	,PTC24_ST_WAIT     		,&ptc24_vWait 		},
{PTC24_ST_PRINT_WAIT	,PTC24_EV_FINISH   	,PTC24_ST_FINISH    	,&ptc24_vFinish		},

{PTC24_ST_PRINT			,PTC24_EV_REFRESH  	,PTC24_ST_PRINT      	,&ptc24_vPrint		},
{PTC24_ST_PRINT			,PTC24_EV_IDLE	   	,PTC24_ST_IDLE      	,&ptc24_vIdle		},
{PTC24_ST_PRINT			,PTC24_EV_FINISH   	,PTC24_ST_FINISH    	,&ptc24_vFinish		},

{PTC24_ST_FINISH		,PTC24_EV_REFRESH  	,PTC24_ST_FINISH     	,&ptc24_vFinish		},
{PTC24_ST_FINISH		,PTC24_EV_IDLE 	  	,PTC24_ST_IDLE     		,&ptc24_vIdle		},
};

#define TRANS_COUNT (sizeof(Ptc24SMTrans)/sizeof(*Ptc24SMTrans))

/* ---------------------------------------------------------------------------*/
/* General state machine functions -------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void ptc24Test_vStateMachineLoop(void)
{
	int i;
	Ptc24StateMachine.event = nextEvent;

	/* Execute test state machine */
	for (i = 0; i < TRANS_COUNT; i++)
	{
		if ( (Ptc24StateMachine.state == Ptc24StateMachine.trans[i].state) ||
			 (PTC24_ST_ANY == Ptc24StateMachine.trans[i].state) )
		{
			if ( (Ptc24StateMachine.event == Ptc24StateMachine.trans[i].event) ||
				 (PTC24_EV_ANY == Ptc24StateMachine.trans[i].event) )
			{
				Ptc24StateMachine.state = Ptc24StateMachine.trans[i].next;

                if (Ptc24StateMachine.trans[i].fn)
                {
                    (Ptc24StateMachine.trans[i].fn)();
                }
                break;
			}
		}
	}
}

void ptc24Test_vStateMachineInit(void)
{
	/* Start or reset mpx test state machine */
	Ptc24StateMachine.state = PTC24_ST_IDLE;
	Ptc24StateMachine.trans = &Ptc24SMTrans;
	Ptc24StateMachine.transCount = TRANS_COUNT;
	ptc24_vSetNextEvent(PTC24_EV_REFRESH);

	ptc24_vResetTests();
}

void ptc24_vSetNextEvent(Ptc24TestEvents event)
{
	nextEvent = event;
}

void ptc24Test_vSetTest(Ptc24TestList test)
{
	Ptc24Tests.currentTest = test;
}

void ptc24Test_vTestOk(void)
{
	Ptc24Tests.testOk = true;
}

void ptc24Test_vTestFail(void)
{
	Ptc24Tests.testFail = true;
}

void ptc24Test_vFinishTest()
{
	ptc24_vSetNextEvent(PTC24_EV_FINISH);
}

/* ---------------------------------------------------------------------------*/
/* State machine main functions ----------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void ptc24_vResetTests(void)
{
	int i;

	Ptc24Tests.currentTest = PTC24_TEST_NOTHING;
	Ptc24Tests.testOk = false;
	Ptc24Tests.testFail = false;
	Ptc24Tests.testFinished = false;
	Ptc24Tests.finishedTestBeep = false;
	Ptc24Tests.testError = false;
	Ptc24Tests.seriousError = false;
	enableToogleOdo(false);
	enableToogleTaco(false);
	ptc24.outputCommandReceived = false;

	for (i=0; i<NUM_PTC24_KEYS; i++)
	{
		/* Menu keys. */
		if (i>=1 && i<=4)
		{
			Ptc24Tests.keysOn[i]=1;
			Ptc24Tests.keysOff[i]=1;
		}
		else
		{
			Ptc24Tests.keysOn[i]=0;
			Ptc24Tests.keysOff[i]=0;
		}
	}
}

void ptc24_vUpdateTests(void)
{
	if (Ptc24Tests.currentTest == PTC24_TEST_START)
		/* Do first test: next test from PTC24_TEST_START. */
		Ptc24Tests.currentTest = PTC24_TEST_START + 1;

	else if (Ptc24Tests.currentTest == PTC24_TEST_IGN)
	{
		if (Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_TACO_OFF;
			Ptc24Tests.testFinished = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_TACO_OFF)
	{
		if (Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_TACO_ON;
			Ptc24Tests.testFinished = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_TACO_ON)
	{
		if (Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_ODO_OFF;
			Ptc24Tests.testFinished = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_ODO_OFF)
	{
		if (Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_ODO_ON;
			Ptc24Tests.testFinished = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_ODO_ON)
	{
		if (Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_KEYS_ON;
			Ptc24Tests.testFinished = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_KEYS_ON)
	{
		if (Ptc24Tests.testOk && Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_BUZZER;
			Ptc24Tests.testFinished = false;
			Ptc24Tests.testOk = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_BUZZER)
	{
		if (Ptc24Tests.testOk && Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_WARNINGLEDS_ON;
			Ptc24Tests.testFinished = false;
			Ptc24Tests.testOk = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_WARNINGLEDS_ON)
	{
		if (Ptc24Tests.testOk && Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_KEYS_OFF;
			Ptc24Tests.testFinished = false;
			Ptc24Tests.testOk = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_KEYS_OFF)
	{
		if (Ptc24Tests.testOk && Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_WARNINGLEDS_OFF;
			Ptc24Tests.testFinished = false;
			Ptc24Tests.testOk = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_WARNINGLEDS_OFF)
	{
		if (Ptc24Tests.testOk && Ptc24Tests.testFinished)
		{
			Ptc24Tests.currentTest = PTC24_TEST_END;
			Ptc24Tests.testFinished = false;
			Ptc24Tests.testOk = false;
		}
	}

	else if (Ptc24Tests.currentTest == PTC24_TEST_END)
	{
		if (Ptc24Tests.testOk && Ptc24Tests.testFinished)
		{
			ptc24_vSetNextEvent(PTC24_EV_FINISH);
			Ptc24Tests.testFinished = false;
			Ptc24Tests.testOk = false;
		}
	}
}

/* Idle ----------------------------------------------------------------------*/
void ptc24_vIdle(void)
{
	/* Update Ptc24Tests.currentTest */
	ptc24_vUpdateTests();

	if (nextEvent == PTC24_EV_FINISH)
		return;

	if ((Ptc24Tests.currentTest > PTC24_TEST_START) && (Ptc24Tests.currentTest <= PTC24_TEST_END))
		ptc24_vSetNextEvent(PTC24_EV_EXECUTE);

	else
		ptc24_vSetNextEvent(PTC24_EV_REFRESH);
}

/* Execute -------------------------------------------------------------------*/
void ptc24_vExecute(void)
{
	Ptc24Tests.statedTestTime = sysTickTimer;

	if (Ptc24Tests.currentTest == PTC24_TEST_IGN)
		tooglePTC24Ign();

	if (Ptc24Tests.currentTest == PTC24_TEST_ODO_ON)
		enableToogleOdo(true);

	if (Ptc24Tests.currentTest == PTC24_TEST_TACO_ON)
		enableToogleTaco(true);

	ptc24_vSetNextEvent(PTC24_EV_WAIT);
}


/* Wait ---------------------------------------------------------------------*/
void ptc24_vWait(void)
{
	if ( (Ptc24Tests.currentTest == PTC24_TEST_ODO_ON) &&
		 (sysTickTimer - Ptc24Tests.statedTestTime < DELAY_FAST_TESTS) )
		ptc24_vSetNextEvent(PTC24_EV_REFRESH);

	else if ( (Ptc24Tests.currentTest == PTC24_TEST_TACO_ON) &&
			  (sysTickTimer - Ptc24Tests.statedTestTime < DELAY_FAST_TESTS) )
		ptc24_vSetNextEvent(PTC24_EV_REFRESH);

	else
		ptc24_vSetNextEvent(PTC24_EV_ANALYSE);
}

/* Analysis ------------------------------------------------------------------*/
void ptc24_vAnalyse(void)
{
	ptc24_vSetNextEvent(PTC24_EV_FINALIZE);

	ptc24_vAnalyse_CAN1();

	if ((Ptc24Tests.currentTest == PTC24_TEST_IGN) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_Ign();

	if ((Ptc24Tests.currentTest == PTC24_TEST_TACO_OFF) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_Taco();

	if ((Ptc24Tests.currentTest == PTC24_TEST_TACO_ON) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_Taco();

	if ((Ptc24Tests.currentTest == PTC24_TEST_ODO_OFF) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_Odo();

	if ((Ptc24Tests.currentTest == PTC24_TEST_ODO_ON) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_Odo();

	if ((Ptc24Tests.currentTest == PTC24_TEST_KEYS_ON) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_KeysOn();

	if ((Ptc24Tests.currentTest == PTC24_TEST_BUZZER) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_Buzzer();

	if ((Ptc24Tests.currentTest == PTC24_TEST_WARNINGLEDS_ON) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_WarningLeds();

	if ((Ptc24Tests.currentTest == PTC24_TEST_KEYS_OFF) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_KeysOff();

	if ((Ptc24Tests.currentTest == PTC24_TEST_WARNINGLEDS_OFF) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_WarningLeds();

	if ((Ptc24Tests.currentTest == PTC24_TEST_END) && (!Ptc24Tests.testFinished))
		ptc24_vAnalyse_EndTest();
}

/* Finalize ------------------------------------------------------------------*/
void ptc24_vFinalize(void)
{
	ptc24_vSetNextEvent(PTC24_EV_PRINT);
}

/* Print ---------------------------------------------------------------------*/
void ptc24_vPrint(void)
{
	if (printTestResult)
		printTestResult();

	ptc24_vSetNextEvent(PTC24_EV_IDLE);

	if (Ptc24Tests.testError)
		ptc24_vSetNextEvent(PTC24_EV_REFRESH);
}

/* Finish test----------------------------------------------------------------*/
void ptc24_vFinish(void)
{
	ptc24_vResetTests();
	LCD_vSetNextEvent(EV_RETURN);
	ptc24_vSetNextEvent(PTC24_EV_IDLE);
}

/* ---------------------------------------------------------------------------*/
/* State machine secondary functions -----------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* Execute -------------------------------------------------------------------*/


/* Analysis ------------------------------------------------------------------*/
void ptc24_vAnalyse_CAN1()
{
	if (sysTickTimer - ptc24.lastTimeSeen > 100)
	{
		Ptc24Tests.testError = true;
		Ptc24Tests.testFinished = true;
		setBeep(1, 2000);
		printTestResult = ptc24print_CAN1Error;
	}
}

void ptc24_vAnalyse_Ign()
{
	if (ptc24.outputCommandReceived)
	{
		Ptc24Tests.testError = false;
		Ptc24Tests.testFinished = true;
	}

	else
	{
		Ptc24Tests.testError = true;
		Ptc24Tests.testFinished = true;
		setBeep(1, 2000);
		printTestResult = ptc24print_IgnError;
	}

}
void ptc24_vAnalyse_Taco()
{
	int i, num_tries = 5;

	for (i=0; i<num_tries; i++)
	{
		if (Ptc24Tests.currentTest == PTC24_TEST_TACO_OFF)
		{
			if (!ptc24.tacoReceivedCommand)
			{
				Ptc24Tests.testError = false;
				Ptc24Tests.testFinished = true;
				break;
			}

			else
			{
				Ptc24Tests.testError = true;
				Ptc24Tests.testFinished = true;
				setBeep(1, 2000);
				printTestResult = ptc24print_TacoError;
			}
		}

		else if (Ptc24Tests.currentTest == PTC24_TEST_TACO_ON)
		{
			if (ptc24.tacoReceivedCommand)
			{
				Ptc24Tests.testError = false;
				Ptc24Tests.testFinished = true;
				break;
			}

			else
			{
				Ptc24Tests.testError = true;
				Ptc24Tests.testFinished = true;
				setBeep(1, 2000);
				printTestResult = ptc24print_TacoError;
				enableToogleTaco(false);
			}
		}
	}
}

void ptc24_vAnalyse_Odo()
{
	int i, num_tries = 5;

	for (i=0; i<num_tries; i++)
	{
		if (Ptc24Tests.currentTest == PTC24_TEST_ODO_OFF)
		{
			if (!ptc24.odoReceivedCommand)
			{
				Ptc24Tests.testError = false;
				Ptc24Tests.testFinished = true;
				break;
			}

			else
			{
				Ptc24Tests.testError = true;
				Ptc24Tests.testFinished = true;
				setBeep(1, 2000);
				printTestResult = ptc24print_OdoError;
			}
		}

		else if (Ptc24Tests.currentTest == PTC24_TEST_ODO_ON)
		{
			if (ptc24.odoReceivedCommand)
			{
				Ptc24Tests.testError = false;
				Ptc24Tests.testFinished = true;
				break;
			}

			else
			{
				Ptc24Tests.testError = true;
				Ptc24Tests.testFinished = true;
				setBeep(1, 2000);
				printTestResult = ptc24print_OdoError;
				enableToogleOdo(false);
			}
		}
	}
}

void ptc24_vAnalyse_Buzzer()
{
	printTestResult = ptc24print_Buzzer;
	setBeep(3, 100);
	Ptc24Tests.testFinished = true;
}

void ptc24_vAnalyse_KeysOn()
{
	int i;

	printTestResult = ptc24print_PressKeyOn;

	for (i=NUM_PTC24_KEYS-1; i>=0; i--)
		if ((Ptc24Tests.keysOn[i] == 0) && (ptc24.keyState[i] == 1))
		{
			Ptc24Tests.keysOn[i] = 1;
			printTestResult = ptc24print_PressKeyOn;
		}

	for (i=0; i<NUM_PTC24_KEYS; i++)
	{
		if(Ptc24Tests.keysOn[i] == 0)
			break;
		/* All keys was analysed and all was pressed. */
		if((Ptc24Tests.keysOn[23] == 1) && (i == NUM_PTC24_KEYS-1))
		{
			setBeep(3, 100);
			printTestResult = ptc24print_AllLedsKeyOn;
			Ptc24Tests.testFinished = true;
		}
	}
}

void ptc24_vAnalyse_KeysOff()
{
	int i;

	printTestResult = ptc24print_PressKeyOff;

	for (i=NUM_PTC24_KEYS-1; i>=0; i--)
		if ((Ptc24Tests.keysOff[i] == 0) && (ptc24.keyState[i] == 0))
		{
			Ptc24Tests.keysOff[i] = 1;
			printTestResult = ptc24print_PressKeyOff;
		}

	for (i=0; i<NUM_PTC24_KEYS; i++)
	{
		if(Ptc24Tests.keysOff[i] == 0)
			break;
		/* All keys was analysed and all was pressed. */
		if((Ptc24Tests.keysOff[23] == 1) && (i == NUM_PTC24_KEYS-1))
		{
			setBeep(3, 100);
			printTestResult = ptc24print_AllLedsKeyOff;
			Ptc24Tests.testFinished = true;
		}
	}
}

void ptc24_vAnalyse_WarningLeds()
{
	if (Ptc24Tests.currentTest == PTC24_TEST_WARNINGLEDS_ON)
		printTestResult = ptc24print_AllWarningLedsOn;

	else if (Ptc24Tests.currentTest == PTC24_TEST_WARNINGLEDS_OFF)
		printTestResult = ptc24print_AllWarningLedsOff;

	setBeep(3, 100);
	Ptc24Tests.testFinished = true;
}

void ptc24_vAnalyse_EndTest()
{
	printTestResult = ptc24print_EndTestOk;
	setBeep(3, 100);
	Ptc24Tests.testFinished = true;
}

/* ---------------------------------------------------------------------------*/
/* Print LCD functions -------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void ptc24print_WaitMessage(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "    Aguarde!    ");
	sprintf(message, "  Executando");
	printTestMessage(TestMessages.lines[1], message, 3);
}

void ptc24print_CAN1Error(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "CAN1 Erro: Veri-");
	sprintf(message, "ficarCN1.5,12,13");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc24print_IgnError(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "IGN Erro: Veri- ");
	sprintf(message, "ficar CN1.9");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc24print_OdoError(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "ODO Erro: Veri- ");
	sprintf(message, "ficar CN1.4");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc24print_TacoError(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "TACO Erro: Veri-");
	sprintf(message, "ficar CN1.6");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc24print_Buzzer(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, "   Buzzer ok?   ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "(Primeira tecla)");
}

void ptc24print_PressKeyOn(void)
{
	int i;
	for (i=0; i<NUM_PTC24_KEYS; i++)
		if(Ptc24Tests.keysOn[i] == 0)
			break;

	sprintf(message, "Press. tecla %d", i+1);

	snprintf(TestMessages.lines[0], LINE_SIZE, "   Ligar LEDS   ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "%s              ", message);
}

void ptc24print_PressKeyOff(void)
{
	int i;
	for (i=0; i<NUM_PTC24_KEYS; i++)
		if(Ptc24Tests.keysOff[i] == 0)
			break;

	sprintf(message, "Press. tecla %d", i+1);
	snprintf(TestMessages.lines[0], LINE_SIZE, " Desligar LEDS  ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "%s              ", message);
}

void ptc24print_AllLedsKeyOn(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, " Leds das teclas ");
	snprintf(TestMessages.lines[1], LINE_SIZE, " todos ligados?  ");
}

void ptc24print_AllLedsKeyOff(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, " Leds das teclas");
	snprintf(TestMessages.lines[1], LINE_SIZE, "todos desligado?");
}

void ptc24print_AllWarningLedsOn(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, " Leds de avisos  ");
	snprintf(TestMessages.lines[1], LINE_SIZE, " todos ligados?  ");
}

void ptc24print_AllWarningLedsOff(void)
{
	snprintf(TestMessages.lines[0], LINE_SIZE, " Leds de avisos  ");
	snprintf(TestMessages.lines[1], LINE_SIZE, "todos desligado?");
}

void ptc24print_EndTestOk(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE," Teste PTC24: OK");
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc24print_OnGoing(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE," Em Construcao. ");
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void ptc24printTestMessage(char *line, char *sentence, uint8_t dots)
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
