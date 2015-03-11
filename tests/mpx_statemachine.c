/* Includes ------------------------------------------------------------------*/
#include "tests/mpx_statemachine.h"

/* Local functions declaration #1 --------------------------------------------*/
void mpxTest_vIdle(void);
void mpxTest_vExecute(void);
void mpxTest_vWait(void);
void mpxTest_vAnalyse(void);
void mpxTest_vFinalize(void);
void mpxTest_vPrint(void);

/* Local functions declaration #2 --------------------------------------------*/
void mpxTest_vExecute_ID(TestsStStates);
void mpxTest_vAnalyse_ID(TestsStStates);

/* Local functions declaration #3 --------------------------------------------*/
void (*printTestResult)(TestsStStates);
void printTestMessage(char*, char*, uint8_t);
void print_WaitMessage(TestsStStates);
void print_AutoTest_OK(TestsStStates);
void print_IDTest_OK(TestsStStates);
void print_IDTest_erro(TestsStStates);

/* Local Variables -----------------------------------------------------------*/
StateMachine MpxSM;

char message[LINE_SIZE];

static volatile int8_t nextEvent;

const Transition MpxSMTrans[] =  		//TABELA DE ESTADOS
{
/*Current state		Event				Next state			callback */
{MPX_ST_IDLE		,MPX_EV_REFRESH   	,MPX_ST_IDLE       	,&mpxTest_vIdle		},
{MPX_ST_IDLE		,MPX_EV_EXECUTE    	,MPX_ST_EXECUTE    	,&mpxTest_vExecute	},

{MPX_ST_EXECUTE		,MPX_EV_REFRESH   	,MPX_ST_EXECUTE    	,&mpxTest_vExecute	},
{MPX_ST_EXECUTE		,MPX_EV_PRINT    	,MPX_ST_PRINT    	,&mpxTest_vPrint 	},
{MPX_ST_EXECUTE		,MPX_EV_WAIT    	,MPX_ST_WAIT       	,&mpxTest_vWait 	},

{MPX_ST_WAIT		,MPX_EV_REFRESH   	,MPX_ST_WAIT       	,&mpxTest_vWait		},
{MPX_ST_WAIT		,MPX_EV_PRINT	 	,MPX_ST_PRINT_WAIT  ,&mpxTest_vPrint	},
{MPX_ST_WAIT		,MPX_EV_ANALYSE    	,MPX_ST_ANALYSE    	,&mpxTest_vAnalyse 	},

{MPX_ST_ANALYSE		,MPX_EV_REFRESH   	,MPX_ST_ANALYSE    	,&mpxTest_vAnalyse	},
{MPX_ST_ANALYSE		,MPX_EV_FINALIZE   	,MPX_ST_FINALIZE   	,&mpxTest_vFinalize	},

{MPX_ST_FINALIZE	,MPX_EV_REFRESH   	,MPX_ST_FINALIZE   	,&mpxTest_vFinalize	},
{MPX_ST_FINALIZE	,MPX_EV_IDLE    	,MPX_ST_IDLE    	,&mpxTest_vIdle 	},
{MPX_ST_FINALIZE	,MPX_EV_PRINT    	,MPX_ST_PRINT    	,&mpxTest_vPrint 	},

{MPX_ST_PRINT_WAIT	,MPX_EV_REFRESH   	,MPX_ST_PRINT_WAIT 	,&mpxTest_vPrint    },
{MPX_ST_PRINT_WAIT	,MPX_EV_WAIT	   	,MPX_ST_WAIT     	,&mpxTest_vWait 	},

{MPX_ST_PRINT		,MPX_EV_REFRESH   	,MPX_ST_PRINT      	,&mpxTest_vPrint	},
{MPX_ST_PRINT		,MPX_EV_IDLE	   	,MPX_ST_IDLE      	,&mpxTest_vIdle		}
};


#define TRANS_COUNT (sizeof(MpxSMTrans)/sizeof(*MpxSMTrans))

/* ---------------------------------------------------------------------------*/
/* General state machine functions -------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void  mpxTest_vStateMachineLoop(void)
{
	int i;
	MpxSM.event = nextEvent;

	/* Execute test state machine */
	for (i = 0; i < TRANS_COUNT; i++)
	{
		if ((MpxSM.state == MpxSM.trans[i].state) || (TST_ANY == MpxSM.trans[i].state))
		{
			if (((MpxSM.event == MpxSM.trans[i].event) || (TEV_ANY == MpxSM.trans[i].event)))
			{
				MpxSM.state = MpxSM.trans[i].next;

                if (MpxSM.trans[i].fn)
                {
                    (MpxSM.trans[i].fn)();
                }
                break;
			}
		}
	}
}

void mpxTest_vStateMachineInit(void)
{
	MpxSM.state = TST_IDLE;
	MpxSM.trans = &MpxSMTrans;
	MpxSM.transCount = TRANS_COUNT;
	test_vSetNextEvent(TEV_REFRESH);
}

void  mpxTest_vSetNextEvent(MpxTestsStEvents event)
{
	nextEvent = event;
}

void mpxTest_vJumpToState(MpxTestsStStates state)
{
	MpxSM.state = state;
	mpxTest_vSetNextEvent(MPX_EV_REFRESH);
}

/* ---------------------------------------------------------------------------*/
/* State machine main functions ----------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* Idle ----------------------------------------------------------------------*/
void mpxTest_vIdle(void)
{
	TestsStStates state = test_vGetState();

	Mpxtests.testError = false;

	sendCanPacket(CAN1, 0x00, state, 0xAA, 0XFF, 0, 0);

	if ((state >= TST_MPX_TEST_ID1) && (state <= TST_MPX_TEST_END))
		mpxTest_vSetNextEvent(MPX_EV_EXECUTE);
	else
		mpxTest_vSetNextEvent(MPX_EV_REFRESH);
}

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute(void)
{
	TestsStStates state = test_vGetState();

	Mpxtests.statedTestTime = sysTickTimer;
	sendCanPacket(CAN1, 0x00, state, 0xCC, 0XFF, 0, 0);
	mpxTest_vSetNextEvent(MPX_EV_WAIT);

	/* ID test */
	if ((state >= TST_MPX_TEST_ID1) && (state <= TST_MPX_TEST_ID0))
		mpxTest_vExecute_ID(state);

	/* Push Pull 10A test */
	//else if ((state >= TST_MPX_TEST_P0_L) && (state <= TST_MPX_TEST_P3_H ))
		//executePP10ATest_MPX(state);

	/* This state is only reached by automatic test when all tests is working. */
	else if (state == TST_MPX_TEST_END)
	{
		printTestResult = print_AutoTest_OK;
		mpxTest_vSetNextEvent(MPX_EV_PRINT);
	}
}


/* Wait ---------------------------------------------------------------------*/
void mpxTest_vWait(void)
{
	TestsStStates state = test_vGetState();
	char message[LINE_SIZE];

	mpxTest_vSetNextEvent(MPX_EV_REFRESH);

	/* This delay is used to do GIGA3 waits until the MPX be prepared to be analyzed.
	 * Each kind of test (manual, automatic or loop test) has a message to be shown
	 * while the delay time is not reached.
	 */
	/* ID tests is slow, therefore it will be used a greater delay than other tests. */
	if ( (state >= TST_MPX_TEST_ID1) && (state <= TST_MPX_TEST_ID0) &&
	   ( (sysTickTimer - Mpxtests.statedTestTime) > DELAY_TO_ANALISE_SLOW_TEST) )
	{
		mpxTest_vSetNextEvent(MPX_EV_ANALYSE);
	}

	/* Other tests, that is slow, is used a less delay. */
	else if (sysTickTimer - Mpxtests.statedTestTime > DELAY_TO_ANALISE_FAST_TEST)
		mpxTest_vSetNextEvent(MPX_EV_ANALYSE);

	else
	{
		printTestResult = print_WaitMessage;
		mpxTest_vSetNextEvent(MPX_EV_PRINT);
	}
}

/* Analysis ------------------------------------------------------------------*/
void mpxTest_vAnalyse(void)
{
	TestsStStates state = test_vGetState();

	/* ID test */
	if ((state >= TST_MPX_TEST_ID1) && (state <= TST_MPX_TEST_ID0 ))
		mpxTest_vAnalyse_ID(state);

	/* Push Pull 10A test */
	//else if ((state >= TST_MPX_TEST_P0_L) && (state <= TST_MPX_TEST_P3_H ))
	//	analysisPP10ATest_MPX(state);

	mpxTest_vSetNextEvent(MPX_EV_FINALIZE);
}

/* Finalize ------------------------------------------------------------------*/
void mpxTest_vFinalize(void)
{
	mpxTest_vSetNextEvent(MPX_EV_PRINT);

	if (!Mpxtests.testError)
	{
		if (Mpxtests.boolIsAutoTest)
		{
			test_vSetNextEvent(TEV_AUTOMATIC);
			mpxTest_vSetNextEvent(MPX_EV_IDLE);
		}

		else if (Mpxtests.boolIsLoopTest)
		{
			test_vSetNextEvent(TEV_LOOP);
			mpxTest_vSetNextEvent(MPX_EV_IDLE);
		}
	}
}

/* Print ---------------------------------------------------------------------*/
void mpxTest_vPrint(void)
{
	TestsStStates state = test_vGetState();
	char message[LINE_SIZE];

	if(printTestResult)
		printTestResult(state);

	/* This function may be called by wait or analyse stage. If was called by former,
	 * state machine state should be back to wait, else if was called by latter, it
	 * analyse if should return to idle state to wait for new tests or continue (loop
	 * or auto tests) or keep this state (manual and end auto test) until user press
	 * ENTER.
	 */

	sendCanPacket(CAN1, 0x00, state, 0xBB, 0XFF, 0, 0);

	if (MpxSM.state == MPX_ST_PRINT_WAIT)
		mpxTest_vSetNextEvent(MPX_EV_WAIT);

	else if (MpxSM.state == MPX_ST_PRINT)
	{
		if (test_vGetState() == TST_IDLE)
			mpxTest_vSetNextEvent(MPX_EV_IDLE);
		else
			mpxTest_vSetNextEvent(MPX_EV_REFRESH);
	}
}

/* ---------------------------------------------------------------------------*/
/* State machine secondary functions -----------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute_ID(TestsStStates state)
{
	if (state == TST_MPX_TEST_ID1)
		Mpxtests.numberTestDone++;

	setMPXIDports(ID1 + (state-TST_MPX_TEST_ID1) );
}

/* Analysis ------------------------------------------------------------------*/
void mpxTest_vAnalyse_ID(TestsStStates state)
{
	if ( ( (state == TST_MPX_TEST_ID1) && (mpx.mpxId == 0x81) ) ||
		 ( (state == TST_MPX_TEST_ID2) && (mpx.mpxId == 0x82) ) ||
		 ( (state == TST_MPX_TEST_ID4) && (mpx.mpxId == 0x84) ) ||
		 ( (state == TST_MPX_TEST_ID0) && (mpx.mpxId == 0x80) ) )
	{
		if (Mpxtests.boolIsManualTest)
		{
			printTestResult = print_IDTest_OK;
			sendCanPacket(CAN1, 0x00, 0x77, 0x77, 0XFF, 0, 0);
		}
	}

	else
	{
		printTestResult = print_IDTest_erro;
		Mpxtests.testError = true;
	}
}

/* ---------------------------------------------------------------------------*/
/* Print LCD functions -------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void print_WaitMessage(TestsStStates state)
{
	if (Mpxtests.boolIsAutoTest || Mpxtests.boolIsManualTest)
	{
		sprintf(message, "Aguarde");
		printTestMessage(TestMessages.lines[0], message, 3);
		sniprintf(TestMessages.lines[1], LINE_SIZE, "                ");
	}

	else if (Mpxtests.boolIsLoopTest)
	{
		snprintf(TestMessages.lines[0],LINE_SIZE," Teste em Loop  ");
		if (Mpxtests.numberTestDone < 10)
			sprintf(message, "  Contagem: %d    ", Mpxtests.numberTestDone);
		else
			sprintf(message, " Contagem: %d     ", Mpxtests.numberTestDone);
		printTestMessage(TestMessages.lines[1], message, 0);
	}
}

void print_AutoTest_OK(TestsStStates state)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste Auto: OK  ");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

/* ID test print functions ---------------------------------------------------*/
void print_IDTest_OK(TestsStStates State)
{
	char ID;

	if (State == TST_MPX_TEST_ID0)
		ID = '0';
	else if (State == TST_MPX_TEST_ID1)
		ID = '1';
	else if (State == TST_MPX_TEST_ID2)
		ID = '2';
	else if (State == TST_MPX_TEST_ID4)
		ID = '4';

	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste ID%c: OK     ", ID);
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_IDTest_erro(TestsStStates State)
{
	char ID;

	if (State == TST_MPX_TEST_ID0)
		ID = '0';
	else if (State == TST_MPX_TEST_ID1)
		ID = '1';
	else if (State == TST_MPX_TEST_ID2)
		ID = '2';
	else if (State == TST_MPX_TEST_ID4)
		ID = '4';

	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste ID%c: FALHA ", ID);
	snprintf(TestMessages.lines[1],LINE_SIZE,"Erro COM6 8 ou 9");
}

void printTestMessage(char *line, char *sentence, uint8_t dots)
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

		snprintf(line, LINE_SIZE, "%s%c%c%c", sentence, finalpoint[0], finalpoint[1], finalpoint[2]);
	}

	else if (dots == 1)
	{
		if ((sysTickTimer%2000)>999)
			finalpoint[0]='.';

		snprintf(line, LINE_SIZE, "%s%c", sentence, finalpoint[0]);
	}

	else
		snprintf(line, LINE_SIZE, "%s", sentence);
}

/* ---------------------------------------------------------------------------*/
/* Auxiliary functions -------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void isManualTest(void)
{
	Mpxtests.boolIsManualTest = true;
	Mpxtests.boolIsAutoTest = false;
	Mpxtests.boolIsLoopTest = false;
}

void isAutoTest(void)
{
	Mpxtests.boolIsManualTest = false;
	Mpxtests.boolIsAutoTest = true;
	Mpxtests.boolIsLoopTest = false;
}

void isLoopTest(void)
{
	Mpxtests.boolIsManualTest = false;
	Mpxtests.boolIsAutoTest = false;
	Mpxtests.boolIsLoopTest = true;
}
