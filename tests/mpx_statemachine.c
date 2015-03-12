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
void mpxTest_vExecute_ID(StStates);
void mpxTest_vExecute_PP10A(StStates);
void mpxTest_vExecute_BIDI(StStates);
void mpxTest_vAnalyse_ID(StStates);
void mpxTest_vAnalyse_PP10A(StStates);
void mpxTest_vAnalyse_BIDI(StStates);

/* Local functions declaration #3 --------------------------------------------*/
void (*printTestResult)(StStates);
void printTestMessage(char*, char*, uint8_t);
void print_WaitMessage(StStates);
void print_AutoTest_OK(StStates);
void print_IDTest_OK(StStates);
void print_IDTest_erro(StStates);
void print_PortTest_OK(StStates);
void print_PortTest_erro(StStates);

char CN[NUM_PORTS+4][4] = {"1.1", "1.3", "2.1", "2.3"
						  ,"1.1", "1.3", "2.1", "2.3"
						  ,"3.1", "3.3", "4.1", "4.3"};

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
		if ((MpxSM.state == MpxSM.trans[i].state) || (MPX_ST_ANY == MpxSM.trans[i].state))
		{
			if (((MpxSM.event == MpxSM.trans[i].event) || (MPX_EV_ANY == MpxSM.trans[i].event)))
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
	MpxSM.state = MPX_ST_IDLE;
	MpxSM.trans = &MpxSMTrans;
	MpxSM.transCount = TRANS_COUNT;
	test_vSetNextEvent(EV_REFRESH);
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
	StStates test = test_vGetState();

	Mpxtests.testError = false;

	if ((test >= ST_TEST_MPX_ID1) && (test <= ST_TEST_MPX_END))
	{
		mpxTest_vSetNextEvent(MPX_EV_EXECUTE);
	}
	else
		mpxTest_vSetNextEvent(MPX_EV_REFRESH);
}

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute(void)
{
	StStates test = test_vGetState();

	Mpxtests.statedTestTime = sysTickTimer;
	mpxTest_vSetNextEvent(MPX_EV_WAIT);

	/* ID test */
	if ((test >= ST_TEST_MPX_ID1) && (test <= ST_TEST_MPX_ID0))
		mpxTest_vExecute_ID(test);

	/* Push Pull 10A test */
	else if ((test >= ST_TEST_MPX_P0_L) && (test <= ST_TEST_MPX_P3_H ))
		mpxTest_vExecute_PP10A(test);

	else if ((test >= ST_TEST_MPX_P4) && (test <= ST_TEST_MPX_P7 ))
		mpxTest_vExecute_BIDI(test);

	/* This state is only reached by automatic test when all tests is working. */
	else if (test == ST_TEST_MPX_END)
	{
		printTestResult = print_AutoTest_OK;
		mpxTest_vSetNextEvent(MPX_EV_PRINT);
	}
}


/* Wait ---------------------------------------------------------------------*/
void mpxTest_vWait(void)
{
	StStates test = test_vGetState();
	char message[LINE_SIZE];

	mpxTest_vSetNextEvent(MPX_EV_REFRESH);

	/* This delay is used to do GIGA3 waits until the MPX be prepared to be analyzed.
	 * Each kind of test (manual, automatic or loop test) has a message to be shown
	 * while the delay time is not reached.
	 */
	/* ID tests is slow, therefore it will be used a greater delay than other tests. */
	if ( (test >= ST_TEST_MPX_ID1) && (test <= ST_TEST_MPX_ID0) &&
	   ( (sysTickTimer - Mpxtests.statedTestTime) > DELAY_TO_ANALISE_SLOW_TEST) )
	{
		mpxTest_vSetNextEvent(MPX_EV_ANALYSE);
	}

	/* Other tests, that is slow, is used a less delay. */
	else if ( (test > ST_TEST_MPX_ID0) && (test <= ST_TEST_MPX_END) &&
			( (sysTickTimer - Mpxtests.statedTestTime) > DELAY_TO_ANALISE_FAST_TEST) )
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
	StStates test = test_vGetState();

	/* ID test */
	if ((test >= ST_TEST_MPX_ID1) && (test <= ST_TEST_MPX_ID0 ))
		mpxTest_vAnalyse_ID(test);

	/* Push Pull 10A test */
	else if ((test >= ST_TEST_MPX_P0_L) && (test <= ST_TEST_MPX_P3_H ))
		mpxTest_vAnalyse_PP10A(test);

	else if ((test >= ST_TEST_MPX_P4) && (test <= ST_TEST_MPX_P7 ))
		mpxTest_vAnalyse_BIDI(test);

	mpxTest_vSetNextEvent(MPX_EV_FINALIZE);
}

/* Finalize ------------------------------------------------------------------*/
void mpxTest_vFinalize(void)
{
	mpxTest_vSetNextEvent(MPX_EV_PRINT);

	turnOffMpxPorts();

	if (!Mpxtests.testError)
	{
		if (Mpxtests.boolIsAutoTest)
		{
			test_vSetNextEvent(EV_AUTOMATIC);
			mpxTest_vSetNextEvent(MPX_EV_IDLE);
		}

		else if (Mpxtests.boolIsLoopTest)
		{
			test_vSetNextEvent(EV_LOOP);
			mpxTest_vSetNextEvent(MPX_EV_IDLE);
		}
	}
}

/* Print ---------------------------------------------------------------------*/
void mpxTest_vPrint(void)
{
	StStates test = test_vGetState();
	char message[LINE_SIZE];

	if(printTestResult)
		printTestResult(test);

	/* This function may be called by wait or analyse stage. If was called by former,
	 * state machine state should be back to wait, else if was called by latter, it
	 * analyse if should return to idle state to wait for new tests or continue (loop
	 * or auto tests) or keep this state (manual and end auto test) until user press
	 * ENTER.
	 */

	if (MpxSM.state == MPX_ST_PRINT_WAIT)
		mpxTest_vSetNextEvent(MPX_EV_WAIT);

	else if (MpxSM.state == MPX_ST_PRINT)
	{
		if (test_vGetState() == MPX_ST_IDLE)
			mpxTest_vSetNextEvent(MPX_EV_IDLE);
		else
			mpxTest_vSetNextEvent(MPX_EV_REFRESH);
	}
}

/* ---------------------------------------------------------------------------*/
/* State machine secondary functions -----------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute_ID(StStates test)
{
	if (test == ST_TEST_MPX_ID1)
		Mpxtests.numberTestDone++;

	setMPXIDports(ID1 + (test-ST_TEST_MPX_ID1) );
}

void mpxTest_vExecute_PP10A(StStates test)
{
	if ((test >= ST_TEST_MPX_P0_L) && (test <= ST_TEST_MPX_P3_L))
		activeMPXports((test - ST_TEST_MPX_P0_L), PORT_LOW);

	else if ((test >= ST_TEST_MPX_P0_H) && (test <= ST_TEST_MPX_P3_H))
		activeMPXports((test - ST_TEST_MPX_P0_H), PORT_HIGH);
}

void mpxTest_vExecute_BIDI(StStates test)
{
	if ((test >= ST_TEST_MPX_P4) && (test <= ST_TEST_MPX_P7))
		activeMPXports((test - ST_TEST_MPX_P0_H), PORT_HIGH);
}

/* Analysis ------------------------------------------------------------------*/
void mpxTest_vAnalyse_ID(StStates test)
{
	if ( ( (test == ST_TEST_MPX_ID1) && (mpx.mpxId == 0x81) ) ||
		 ( (test == ST_TEST_MPX_ID2) && (mpx.mpxId == 0x82) ) ||
		 ( (test == ST_TEST_MPX_ID4) && (mpx.mpxId == 0x84) ) ||
		 ( (test == ST_TEST_MPX_ID0) && (mpx.mpxId == 0x80) ) )
	{
		if (Mpxtests.boolIsManualTest)
			printTestResult = print_IDTest_OK;
	}

	else
	{
		printTestResult = print_IDTest_erro;
		Mpxtests.testError = true;
	}
}

void mpxTest_vAnalyse_PP10A(StStates test)
{
	bool isMpxPortLow;
	bool isMpxPortHigh;
	bool isGiga3HighSideMosfetDrivingCurrent;

	if ( (test >= ST_TEST_MPX_P0_L) && (test <= ST_TEST_MPX_P3_L) )
	{
		/* Is MPX device port low? If port is low, MPX device low side portx is working when low tests. */
		isMpxPortLow = !getPortStatus(test-ST_TEST_MPX_P0_L);
		/* If this bool is true, then giga3 mosfet is driving current. */
		isGiga3HighSideMosfetDrivingCurrent = getSRBitStatus(ST_TEST_MPX_P0_L+27-test);
	}

	else
	{
		/* Is MPX device port high? If port is high, MPX device low side portx is working when high tests. */
		isMpxPortHigh = getPortStatus(test-ST_TEST_MPX_P0_H);
	}



	/* Is a LOW TEST? */
	if ( (test >= ST_TEST_MPX_P0_L) && (test <= ST_TEST_MPX_P3_L) )
	{
		/* is MPX port low? */
		if (isMpxPortLow)
		{
			/* GIGA3 high side mosfet is driving current? */
			if (isGiga3HighSideMosfetDrivingCurrent)
			{
				Mpxtests.testError = false;
				printTestResult = print_PortTest_OK;
			}
			/* GIGA3 high side mosfet is not driving current? */
			else
			{
				Mpxtests.testError = true;
				printTestResult = print_PortTest_erro;
			}
		}

		/* is MPX port high? */
		else
		{
			/* GIGA3 high side mosfet is driving current? */
			if (isGiga3HighSideMosfetDrivingCurrent)
			{
				Mpxtests.testError = true;
				printTestResult = print_PortTest_erro;
			}
			/* GIGA3 high side mosfet is not driving current? */
			else
			{
				Mpxtests.testError = true;
				printTestResult = print_PortTest_erro;
			}
		}
	}

	/* This condition ensures a HIGH TEST */
	else
	{
		/* If mpx.portInput[X] is not "0x01", then there is some problem with the port X. */
		if (mpx.portInput[test-ST_TEST_MPX_P0_H] == 0x01)
		{
			Mpxtests.testError = false;
			printTestResult = print_PortTest_OK;
		}

		else
		{
			Mpxtests.testError = true;
			printTestResult = print_PortTest_erro;
		}
	}
}

void mpxTest_vAnalyse_BIDI(StStates test)
{
	/* If mpx.portInput[X] is not "0x01", then there is some problem with the port X. */
	if (mpx.portInput[test-ST_TEST_MPX_P0_H] == 0x01)
	{
		Mpxtests.testError = false;
		printTestResult = print_PortTest_OK;
	}

	else
	{
		Mpxtests.testError = true;
		printTestResult = print_PortTest_erro;
	}
}
/* ---------------------------------------------------------------------------*/
/* Print LCD functions -------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void print_WaitMessage(StStates test)
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

void print_AutoTest_OK(StStates test)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste Auto: OK  ");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

/* ID test print functions ---------------------------------------------------*/
void print_IDTest_OK(StStates test)
{
	char ID;

	if (test == ST_TEST_MPX_ID0)
		ID = '0';
	else if (test == ST_TEST_MPX_ID1)
		ID = '1';
	else if (test == ST_TEST_MPX_ID2)
		ID = '2';
	else if (test == ST_TEST_MPX_ID4)
		ID = '4';

	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste ID%c: OK     ", ID);
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_IDTest_erro(StStates test)
{
	char ID;

	if (test == ST_TEST_MPX_ID0)
		ID = '0';
	else if (test == ST_TEST_MPX_ID1)
		ID = '1';
	else if (test == ST_TEST_MPX_ID2)
		ID = '2';
	else if (test == ST_TEST_MPX_ID4)
		ID = '4';

	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste ID%c: FALHA ", ID);
	snprintf(TestMessages.lines[1],LINE_SIZE,"Erro COM6 8 ou 9");
}

/* ID test print functions ---------------------------------------------------*/
void print_PortTest_OK(StStates test)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste CN%s: OK  ", CN[test - ST_TEST_MPX_P0_L]);
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_PortTest_erro(StStates test)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"  Erro CN%s E  ", CN[test - ST_TEST_MPX_P0_L]);
	sprintf(message, "agora? Continue", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
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
