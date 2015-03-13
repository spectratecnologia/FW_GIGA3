/* Includes ------------------------------------------------------------------*/
#include "tests/mpx_statemachine.h"

/* Local functions declaration #1 --------------------------------------------*/
void mpxTest_vUpdateTests(void);
void mpxTest_vResetTests(void);
void mpxTest_vIdle(void);
void mpxTest_vExecute(void);
void mpxTest_vWait(void);
void mpxTest_vAnalyse(void);
void mpxTest_vFinalize(void);
void mpxTest_vPrint(void);
void mpxTest_vFinish(void);

/* Local functions declaration #2 --------------------------------------------*/
void mpxTest_vExecute_ID(void);
void mpxTest_vExecute_PP10A(void);
void mpxTest_vExecute_BIDI(void);
void mpxTest_vAnalyse_ID(void);
void mpxTest_vAnalyse_PP10A(void);
void mpxTest_vAnalyse_BIDI(void);

/* Local functions declaration #3 --------------------------------------------*/
void (*printTestResult)(void);
void printTestMessage(char*, char*, uint8_t);
void print_WaitMessage(void);
void print_AutoTest_OK(void);
void print_IDTest_OK(void);
void print_IDTest_erro(void);
void print_PortTest_OK(void);
void print_PortTest_erro(void);

char CN[NUM_PORTS+4][4] = {"1.1", "1.3", "2.1", "2.3"
						  ,"1.1", "1.3", "2.1", "2.3"
						  ,"3.1", "3.3", "4.1", "4.3"};

/* Local Variables -----------------------------------------------------------*/
StateMachine MpxStateMachine;
Test MpxTests;
char message[LINE_SIZE];
static volatile int8_t nextEvent;

const Transition MpxSMTrans[] =  		//TABELA DE ESTADOS
{
/*Current state		Event				Next state			callback */
{MPX_ST_IDLE		,MPX_EV_REFRESH   	,MPX_ST_IDLE       	,&mpxTest_vIdle		},
{MPX_ST_IDLE		,MPX_EV_EXECUTE    	,MPX_ST_EXECUTE    	,&mpxTest_vExecute	},
{MPX_ST_IDLE		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_EXECUTE		,MPX_EV_REFRESH   	,MPX_ST_EXECUTE    	,&mpxTest_vExecute	},
{MPX_ST_EXECUTE		,MPX_EV_PRINT    	,MPX_ST_PRINT    	,&mpxTest_vPrint 	},
{MPX_ST_EXECUTE		,MPX_EV_WAIT    	,MPX_ST_WAIT       	,&mpxTest_vWait 	},
{MPX_ST_EXECUTE		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_WAIT		,MPX_EV_REFRESH   	,MPX_ST_WAIT       	,&mpxTest_vWait		},
{MPX_ST_WAIT		,MPX_EV_PRINT	 	,MPX_ST_PRINT_WAIT  ,&mpxTest_vPrint	},
{MPX_ST_WAIT		,MPX_EV_ANALYSE    	,MPX_ST_ANALYSE    	,&mpxTest_vAnalyse 	},
{MPX_ST_WAIT		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_ANALYSE		,MPX_EV_REFRESH   	,MPX_ST_ANALYSE    	,&mpxTest_vAnalyse	},
{MPX_ST_ANALYSE		,MPX_EV_FINALIZE   	,MPX_ST_FINALIZE   	,&mpxTest_vFinalize	},
{MPX_ST_ANALYSE		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_FINALIZE	,MPX_EV_REFRESH   	,MPX_ST_FINALIZE   	,&mpxTest_vFinalize	},
{MPX_ST_FINALIZE	,MPX_EV_IDLE    	,MPX_ST_IDLE    	,&mpxTest_vIdle 	},
{MPX_ST_FINALIZE	,MPX_EV_PRINT    	,MPX_ST_PRINT    	,&mpxTest_vPrint 	},
{MPX_ST_FINALIZE	,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_PRINT_WAIT	,MPX_EV_REFRESH   	,MPX_ST_PRINT_WAIT 	,&mpxTest_vPrint    },
{MPX_ST_PRINT_WAIT	,MPX_EV_WAIT	   	,MPX_ST_WAIT     	,&mpxTest_vWait 	},
{MPX_ST_PRINT_WAIT	,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_PRINT		,MPX_EV_REFRESH   	,MPX_ST_PRINT      	,&mpxTest_vPrint	},
{MPX_ST_PRINT		,MPX_EV_IDLE	   	,MPX_ST_IDLE      	,&mpxTest_vIdle		},
{MPX_ST_PRINT		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_FINISH		,MPX_EV_REFRESH   	,MPX_ST_FINISH     	,&mpxTest_vFinish	},
{MPX_ST_FINISH		,MPX_EV_IDLE 	  	,MPX_ST_IDLE     	,&mpxTest_vIdle		},
};

#define TRANS_COUNT (sizeof(MpxSMTrans)/sizeof(*MpxSMTrans))

/* ---------------------------------------------------------------------------*/
/* General state machine functions -------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void mpxTest_vStateMachineLoop(void)
{
	int i;
	MpxStateMachine.event = nextEvent;

	/* Execute test state machine */
	for (i = 0; i < TRANS_COUNT; i++)
	{
		if ( (MpxStateMachine.state == MpxStateMachine.trans[i].state) ||
			 (MPX_ST_ANY == MpxStateMachine.trans[i].state) )
		{
			if ( (MpxStateMachine.event == MpxStateMachine.trans[i].event) ||
				 (MPX_EV_ANY == MpxStateMachine.trans[i].event) )
			{
				MpxStateMachine.state = MpxStateMachine.trans[i].next;

                if (MpxStateMachine.trans[i].fn)
                {
                    (MpxStateMachine.trans[i].fn)();
                }
                break;
			}
		}
	}
}

void mpxTest_vStateMachineInit(void)
{
	/* Start or reset mpx test state machine */
	MpxStateMachine.state = MPX_ST_IDLE;
	MpxStateMachine.trans = &MpxSMTrans;
	MpxStateMachine.transCount = TRANS_COUNT;
	mpxTest_vSetNextEvent(MPX_EV_REFRESH);

	mpxTest_vResetTests();
}

void mpxTest_vSetNextEvent(TestStEvents event)
{
	nextEvent = event;
}

void mpxTest_vSetTest(TestList test)
{
	MpxTests.currentTest = test;
}

void mpxTest_vFinishTest()
{
	mpxTest_vSetNextEvent(MPX_EV_FINISH);
}

/* ---------------------------------------------------------------------------*/
/* State machine main functions ----------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void mpxTest_vResetTests(void)
{
	/* Reset current test */
	MpxTests.currentTest = TEST_NOTHING;
	MpxTests.boolIsAutoTest = false;
	MpxTests.boolIsLoopTest = false;
	MpxTests.testError = false;
	MpxTests.numberTestDone = 0;
	turnOffMpxPorts();
}

void mpxTest_vUpdateTests(void)
{
	if (MpxTests.currentTest == TEST_NOTHING)
		return;

	else if (MpxTests.currentTest == TEST_AUTO)
		MpxTests.boolIsAutoTest = true;

	else if (MpxTests.currentTest == TEST_LOOP)
		MpxTests.boolIsLoopTest = true;

	if (MpxTests.boolIsAutoTest)
	{
		/* TEST_AUTO is set by LCD and if there is this condition, current test should
		 * be the first test: TEST_NOTHING + 1.
		 */
		if (MpxTests.currentTest == TEST_AUTO)
			MpxTests.currentTest = TEST_NOTHING + 1;
		/* If TEST_END is reached, tests auto must be stopped. */
		else if (MpxTests.currentTest != TEST_END)
			MpxTests.currentTest++;
	}

	else if (MpxTests.boolIsLoopTest)
	{
		/* TEST_LOOP is set by LCD and if there is this condition, current test should
		 * be the first test: TEST_NOTHING + 1.
		 */
		if (MpxTests.currentTest == TEST_LOOP)
			MpxTests.currentTest = TEST_NOTHING + 1;
		else
			MpxTests.currentTest++;

		/* If TEST_END is reached, tests loop must be restarted. */
		if (MpxTests.currentTest == TEST_END)
			MpxTests.currentTest = TEST_NOTHING + 1;
	}
}

/* Idle ----------------------------------------------------------------------*/
void mpxTest_vIdle(void)
{
	/* Update MpxTests.currentTest */
	mpxTest_vUpdateTests();

	sendCanPacket(CAN1, 0x0, MpxTests.currentTest, MpxTests.boolIsLoopTest, MpxTests.boolIsAutoTest, 0, 0);

	if ((MpxTests.currentTest >= TEST_ID1) && (MpxTests.currentTest <= TEST_END))
		mpxTest_vSetNextEvent(MPX_EV_EXECUTE);

	else
		mpxTest_vSetNextEvent(MPX_EV_REFRESH);
}

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute(void)
{
	MpxTests.statedTestTime = sysTickTimer;
	mpxTest_vSetNextEvent(MPX_EV_WAIT);

	/* ID test */
	if ((MpxTests.currentTest >= TEST_ID1) && (MpxTests.currentTest <= TEST_ID0))
		mpxTest_vExecute_ID();

	/* Push Pull 10A test */
	else if ((MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_H ))
		mpxTest_vExecute_PP10A();

	else if ((MpxTests.currentTest >= TEST_P4) && (MpxTests.currentTest <= TEST_P7 ))
		mpxTest_vExecute_BIDI();

	/* This state is only reached by automatic test when all tests is working. */
	else if (MpxTests.currentTest == TEST_END)
	{
		printTestResult = print_AutoTest_OK;
		if(MpxStateMachine.state != MPX_EV_FINISH)
			mpxTest_vSetNextEvent(MPX_EV_PRINT);
	}
}


/* Wait ---------------------------------------------------------------------*/
void mpxTest_vWait(void)
{
	char message[LINE_SIZE];

	mpxTest_vSetNextEvent(MPX_EV_REFRESH);

	/* This delay is used to do GIGA3 waits until the MPX be prepared to be analyzed.
	 * Each kind of test (manual, automatic or loop test) has a message to be shown
	 * while the delay time is not reached.
	 */
	/* ID tests is slow, therefore it will be used a greater delay than other tests. */
	if ( (MpxTests.currentTest >= TEST_ID1) && (MpxTests.currentTest <= TEST_ID0) &&
	   ( (sysTickTimer - MpxTests.statedTestTime) > DELAY_TO_ANALISE_SLOW_TEST) )
	{
		mpxTest_vSetNextEvent(MPX_EV_ANALYSE);
	}

	/* Other tests, that is slow, is used a less delay. */
	else if ( (MpxTests.currentTest > TEST_ID0) && (MpxTests.currentTest <= TEST_END) &&
			( (sysTickTimer - MpxTests.statedTestTime) > DELAY_TO_ANALISE_FAST_TEST) )
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
	/* ID test */
	if ((MpxTests.currentTest >= TEST_ID1) && (MpxTests.currentTest <= TEST_ID0 ))
		mpxTest_vAnalyse_ID();

	/* Push Pull 10A test */
	else if ((MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_H ))
		mpxTest_vAnalyse_PP10A();

	else if ((MpxTests.currentTest >= TEST_P4) && (MpxTests.currentTest <= TEST_P7 ))
		mpxTest_vAnalyse_BIDI();

	mpxTest_vSetNextEvent(MPX_EV_FINALIZE);
}

/* Finalize ------------------------------------------------------------------*/
void mpxTest_vFinalize(void)
{
	turnOffMpxPorts();

	mpxTest_vSetNextEvent(MPX_EV_PRINT);

	if(!MpxTests.testError && (MpxTests.boolIsAutoTest || MpxTests.boolIsLoopTest) )
		mpxTest_vSetNextEvent(MPX_EV_IDLE);
}

/* Print ---------------------------------------------------------------------*/
void mpxTest_vPrint(void)
{
	char message[LINE_SIZE];

	if(printTestResult)
		printTestResult();

	mpxTest_vSetNextEvent(MPX_EV_REFRESH);

	if (MpxStateMachine.state == MPX_ST_PRINT_WAIT)
		mpxTest_vSetNextEvent(MPX_EV_WAIT);
}

void mpxTest_vFinish(void)
{
	mpxTest_vResetTests();

	mpxTest_vSetNextEvent(MPX_EV_IDLE);
}

/* ---------------------------------------------------------------------------*/
/* State machine secondary functions -----------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute_ID(void)
{
	if (MpxTests.currentTest == TEST_ID1)
		MpxTests.numberTestDone++;

	setMPXIDports(ID1 + (MpxTests.currentTest-TEST_ID1) );
}

void mpxTest_vExecute_PP10A(void)
{
	if ((MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_L))
		activeMPXports((MpxTests.currentTest - TEST_P0_L), PORT_LOW);

	else if ((MpxTests.currentTest >= TEST_P0_H) && (MpxTests.currentTest <= TEST_P3_H))
		activeMPXports((MpxTests.currentTest - TEST_P0_H), PORT_HIGH);
}

void mpxTest_vExecute_BIDI(void)
{
	if ((MpxTests.currentTest >= TEST_P4) && (MpxTests.currentTest <= TEST_P7))
		activeMPXports((MpxTests.currentTest - TEST_P0_H), PORT_HIGH);
}

/* Analysis ------------------------------------------------------------------*/
void mpxTest_vAnalyse_ID(void)
{
	if ( ( (MpxTests.currentTest == TEST_ID1) && (mpx.mpxId == 0x81) ) ||
		 ( (MpxTests.currentTest == TEST_ID2) && (mpx.mpxId == 0x82) ) ||
		 ( (MpxTests.currentTest == TEST_ID4) && (mpx.mpxId == 0x84) ) ||
		 ( (MpxTests.currentTest == TEST_ID0) && (mpx.mpxId == 0x80) ) )
	{
		printTestResult = print_IDTest_OK;
	}

	else
	{
		printTestResult = print_IDTest_erro;
		MpxTests.testError = true;
	}
}

void mpxTest_vAnalyse_PP10A(void)
{
	bool isMpxPortLow;
	bool isMpxPortHigh;
	bool isGiga3HighSideMosfetDrivingCurrent;

	if ( (MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_L) )
	{
		/* Is MPX device port low? If port is low, MPX device low side portx is working when low tests. */
		isMpxPortLow = !getPortStatus(MpxTests.currentTest-TEST_P0_L);
		/* If this bool is true, then giga3 mosfet is driving current. */
		isGiga3HighSideMosfetDrivingCurrent = getSRBitStatus(TEST_P0_L+27-MpxTests.currentTest);
	}

	else
	{
		/* Is MPX device port high? If port is high, MPX device low side portx is working when high tests. */
		isMpxPortHigh = getPortStatus(MpxTests.currentTest-TEST_P0_H);
	}


	/* Is a LOW TEST? */
	if ( (MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_L) )
	{
		/* is MPX port low? */
		if (isMpxPortLow)
		{
			/* GIGA3 high side mosfet is driving current? */
			if (isGiga3HighSideMosfetDrivingCurrent)
			{
				MpxTests.testError = false;
				printTestResult = print_PortTest_OK;
			}
			/* GIGA3 high side mosfet is not driving current? */
			else
			{
				MpxTests.testError = true;
				printTestResult = print_PortTest_erro;
			}
		}

		/* is MPX port high? */
		else
		{
			/* GIGA3 high side mosfet is driving current? */
			if (isGiga3HighSideMosfetDrivingCurrent)
			{
				MpxTests.testError = true;
				printTestResult = print_PortTest_erro;
			}
			/* GIGA3 high side mosfet is not driving current? */
			else
			{
				MpxTests.testError = true;
				printTestResult = print_PortTest_erro;
			}
		}
	}

	/* This condition ensures a HIGH TEST */
	else
	{
		/* If mpx.portInput[X] is not "0x01", then there is some problem with the port X. */
		if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] == 0x01)
		{
			MpxTests.testError = false;
			printTestResult = print_PortTest_OK;
		}

		else
		{
			MpxTests.testError = true;
			printTestResult = print_PortTest_erro;
		}
	}
}

void mpxTest_vAnalyse_BIDI(void)
{
	/* If mpx.portInput[X] is not "0x01", then there is some problem with the port X. */
	if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] == 0x01)
	{
		MpxTests.testError = false;
		printTestResult = print_PortTest_OK;
	}

	else
	{
		MpxTests.testError = true;
		printTestResult = print_PortTest_erro;
	}
}
/* ---------------------------------------------------------------------------*/
/* Print LCD functions -------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

void print_WaitMessage(void)
{
	if (MpxTests.boolIsLoopTest)
	{
		snprintf(TestMessages.lines[0],LINE_SIZE," Teste em Loop  ");
		if (MpxTests.numberTestDone < 10)
			sprintf(message, "  Contagem: %d    ", MpxTests.numberTestDone);
		else
			sprintf(message, " Contagem: %d     ", MpxTests.numberTestDone);
		printTestMessage(TestMessages.lines[1], message, 0);
	}

	else
	{
		sprintf(message, "Aguarde");
		printTestMessage(TestMessages.lines[0], message, 3);
		sniprintf(TestMessages.lines[1], LINE_SIZE, "                ");
	}
}

void print_AutoTest_OK(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste Auto: OK  ");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

/* ID test print functions ---------------------------------------------------*/
void print_IDTest_OK(void)
{
	char ID;

	if (MpxTests.currentTest == TEST_ID0)
		ID = '0';
	else if (MpxTests.currentTest == TEST_ID1)
		ID = '1';
	else if (MpxTests.currentTest == TEST_ID2)
		ID = '2';
	else if (MpxTests.currentTest == TEST_ID4)
		ID = '4';

	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste ID%c: OK     ", ID);
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_IDTest_erro(void)
{
	char ID;

	if (MpxTests.currentTest == TEST_ID0)
		ID = '0';
	else if (MpxTests.currentTest == TEST_ID1)
		ID = '1';
	else if (MpxTests.currentTest == TEST_ID2)
		ID = '2';
	else if (MpxTests.currentTest == TEST_ID4)
		ID = '4';

	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste ID%c: FALHA ", ID);
	snprintf(TestMessages.lines[1],LINE_SIZE,"Erro COM6 8 ou 9");
}

/* ID test print functions ---------------------------------------------------*/
void print_PortTest_OK(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste CN%s: OK  ", CN[MpxTests.currentTest - TEST_P0_L]);
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_PortTest_erro(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"  Erro CN%s E  ", CN[MpxTests.currentTest - TEST_P0_L]);
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
