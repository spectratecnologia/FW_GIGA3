/* Includes ------------------------------------------------------------------*/
#include "mpx_statemachine.h"

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
void mpxTest_vExecute_Flash(void);
void mpxTest_vExecute_IGN(void);
void mpxTest_vExecute_ID(void);
void mpxTest_vExecute_PP10A(void);
void mpxTest_vExecute_BIDI(void);
void mpxTest_vExecute_LODIN(void);
void mpxTest_vAnalyse_CAN(void);
void mpxTest_vAnalyse_Flash(void);
void EraseFlash(void);
void mpxTest_vAnalyse_IGN(void);
void mpxTest_vAnalyse_SwitchedPort(void);
void mpxTest_vAnalyse_ID(void);
void mpxTest_vAnalyse_PP10A(void);
void mpxTest_vAnalyse_BIDI(void);
void mpxTest_vAnalyse_LODIN(void);
void mpxTest_vAnalyse_NTC(void);

/* Local functions declaration #3 --------------------------------------------*/
void (*printTestResult)(void);
void printTestMessage(char*, char*, uint8_t);
void print_WaitMessage(void);
void print_ClearMessages(void);
void print_CANTest_error(void);
void print_FlashTest_OK(void);
void print_FlashTest_error(void);
void print_SwitchedPortTest_error(void);
void print_AutoTest_OK(void);
void print_IgnTest_OK(void);
void print_IgnTest_error(void);
void print_IDTest_OK(void);
void print_IDTest_error(void);
void print_PortTest_OK(void);
void print_PortTest_error(void);
void print_PortTest_PortOpenError(void);
void print_PortTest_PortShortError(void);
void print_PortTest_FetError(void);
void print_NTCTest_OK(void);
void print_NTCTest_error(void);

void print_OnGoing(void);

char CN[NUM_PORTS+4][5] = {"1.1L", "1.3L", "2.1L", "2.3L"
						  ,"1.1H", "1.3H", "2.1H", "2.3H"
						  ,"3.1",  "3.3",  "4.1",  "4.3"
						  ,"1.2",  "1.4",  "1.6",  "1.7"
						  ,"1.9",  "2.2",  "2.4",  "2.6"
						  ,"2.7",  "2.9",  "3.2",  "3.4"
						  ,"3.6",  "3.7",  "3.9",  "4.2"
						  ,"4.4",  "4.6",  "4.7",  "4.9"
						  ,"1.5",  "1.8",  "2.5",  "2.8"
						  ,"3.5",  "3.8",  "4.5",  "4.8"};

char CN2[NUM_PORTS][4] =  {"1.1",  "1.3",  "2.1",  "2.3"
						  ,"3.1",  "3.3",  "4.1",  "4.3"
						  ,"1.2",  "1.4",  "1.6",  "1.7"
						  ,"1.9",  "2.2",  "2.4",  "2.6"
						  ,"2.7",  "2.9",  "3.2",  "3.4"
						  ,"3.6",  "3.7",  "3.9",  "4.2"
						  ,"4.4",  "4.6",  "4.7",  "4.9"
						  ,"1.5",  "1.8",  "2.5",  "2.8"
						  ,"3.5",  "3.8",  "4.5",  "4.8"};

char FET[NUM_PORTS][6] =  {"FTL1",  "FTL2",   "FTL3",  "FTL4"
						  ,"FTH1",  "FTH2",   "FTH3",  "FTH4"
						  ,"FTH5",  "FTH6",   "FTH7",  "FTH8"
						  ,"FTQH1", "FTQH1",  "FTQH1", "FTQH1"
						  ,"FTQH2", "FTQH2",  "FTQH2", "FTQH2"
						  ,"FTQH3", "FTQH3",  "FTQH3", "FTQH3"
						  ,"FTQH4", "FTQH4",  "FTQH4", "FTQH4"
						  ,"FTQH5", "FTQH5",  "FTQH5", "FTQH5"};

/* Local Variables -----------------------------------------------------------*/
StateMachine MpxStateMachine;
Test MpxTests;
char message[LINE_SIZE];
static volatile int8_t nextEvent;
static volatile int NUM_NTC_TRIES;

const Transition MpxSMTrans[] =  		//TABELA DE ESTADOS
{
/*Current state		Event				Next state			callback */
{MPX_ST_IDLE		,MPX_EV_REFRESH   	,MPX_ST_IDLE       	,&mpxTest_vIdle		},
{MPX_ST_IDLE		,MPX_EV_EXECUTE    	,MPX_ST_EXECUTE    	,&mpxTest_vExecute	},
{MPX_ST_IDLE		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_EXECUTE		,MPX_EV_REFRESH   	,MPX_ST_EXECUTE    	,&mpxTest_vExecute	},
{MPX_ST_EXECUTE		,MPX_EV_PRINT    	,MPX_ST_PRINT    	,&mpxTest_vPrint 	},
{MPX_ST_EXECUTE		,MPX_EV_WAIT    	,MPX_ST_WAIT       	,&mpxTest_vWait 	},
{MPX_ST_EXECUTE		,MPX_EV_FINALIZE   	,MPX_ST_FINALIZE  	,&mpxTest_vFinalize	},
{MPX_ST_EXECUTE		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_WAIT		,MPX_EV_REFRESH   	,MPX_ST_WAIT       	,&mpxTest_vWait		},
{MPX_ST_WAIT		,MPX_EV_PRINT	 	,MPX_ST_PRINT_WAIT  ,&mpxTest_vPrint	},
{MPX_ST_WAIT		,MPX_EV_ANALYSE    	,MPX_ST_ANALYSE    	,&mpxTest_vAnalyse 	},
{MPX_ST_WAIT		,MPX_EV_FINISH    	,MPX_ST_FINISH    	,&mpxTest_vFinish	},

{MPX_ST_ANALYSE		,MPX_EV_REFRESH   	,MPX_ST_ANALYSE    	,&mpxTest_vAnalyse	},
{MPX_ST_ANALYSE		,MPX_EV_EXECUTE   	,MPX_ST_EXECUTE    	,&mpxTest_vExecute	},
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

void mpxTest_vContinueTest(void)
{
	MpxTests.testFinished = false;
	MpxTests.testError = false;
}

void mpxTest_vFinishTest()
{
	mpxTest_vSetNextEvent(MPX_EV_FINISH);
}

int mpxTest_vGetTest()
{
	return MpxTests.currentTest;
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
	MpxTests.testFinished = false;
	MpxTests.finishedTestBeep = false;
	MpxTests.testError = false;
	MpxTests.seriousError = false;
	MpxTests.numberTestDone = 0;
	MpxTests.boolEraseFlash = false;
	setBeep(1,0);
	turnOffMpxPorts();
	activeMPXIgnition(PORT_LOW);
	initMPXstruct();
	print_ClearMessages();
	NUM_NTC_TRIES = 0;
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
			MpxTests.currentTest = TEST_ID1;
		else
			MpxTests.currentTest++;

		/* If TEST_END is reached, tests loop must be restarted. */
		if (MpxTests.currentTest == TEST_END)
		{
			/* Flash test is only done one time in loop mode. */
			MpxTests.currentTest = TEST_ID1;
			/* Everytime that TESD_END is reached, loop test show counting and
			 * a set a beep. */
			MpxTests.numberTestDone++;
			MpxTests.finishedTestBeep = false;
		}
	}
}

/* Idle ----------------------------------------------------------------------*/
void mpxTest_vIdle(void)
{
	/* Update MpxTests.currentTest */
	mpxTest_vUpdateTests();

	if ((MpxTests.currentTest > TEST_NOTHING) && (MpxTests.currentTest <= TEST_END))
		mpxTest_vSetNextEvent(MPX_EV_EXECUTE);

	else
		mpxTest_vSetNextEvent(MPX_EV_REFRESH);
}

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute(void)
{
	MpxTests.statedTestTime = sysTickTimer;
	mpxTest_vSetNextEvent(MPX_EV_WAIT);

	if (MpxTests.currentTest == TEST_FLASH)
		mpxTest_vExecute_Flash();

	/* Ignition test */
	if ((MpxTests.currentTest == TEST_IGN_L) || (MpxTests.currentTest == TEST_IGN_H))
		mpxTest_vExecute_IGN();

	/* ID test */
	if ((MpxTests.currentTest >= TEST_ID1) && (MpxTests.currentTest <= TEST_ID0))
		mpxTest_vExecute_ID();

	/* Push Pull 10A test */
	else if ((MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_H ))
		mpxTest_vExecute_PP10A();

	else if ((MpxTests.currentTest >= TEST_P4) && (MpxTests.currentTest <= TEST_P27 ))
		mpxTest_vExecute_BIDI();

	else if ((MpxTests.currentTest >= TEST_P28) && (MpxTests.currentTest <= TEST_P35 ))
		mpxTest_vExecute_LODIN();

	/* This state is only reached by automatic test when all tests is working. */
	else if (MpxTests.currentTest == TEST_END)
	{
		MpxTests.testFinished = true;
		printTestResult = print_AutoTest_OK;
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

	/* Flash tests are too slow. */
	if ( (MpxTests.currentTest == TEST_FLASH) &&
	   ( (sysTickTimer - MpxTests.statedTestTime) > DELAY_TO_ANALISE_FLASH_TEST) )
	{
		mpxTest_vSetNextEvent(MPX_EV_ANALYSE);
	}

	/* ID tests is slow, therefore it will be used a greater delay than other tests. */
	else if ( (MpxTests.currentTest >= TEST_ID1) && (MpxTests.currentTest <= TEST_NTC) &&
	   ( (sysTickTimer - MpxTests.statedTestTime) > DELAY_TO_ANALISE_SLOW_TEST) )
	{
		mpxTest_vSetNextEvent(MPX_EV_ANALYSE);
	}

	/* Other tests (ports), that is slow, is used a less delay. */
	else if ( (MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_END) &&
			( (sysTickTimer - MpxTests.statedTestTime) > DELAY_TO_ANALISE_FAST_TEST) )
	{
		mpxTest_vSetNextEvent(MPX_EV_ANALYSE);
	}

	else
	{
		printTestResult = print_WaitMessage;
		mpxTest_vSetNextEvent(MPX_EV_PRINT);
	}
}

/* Analysis ------------------------------------------------------------------*/
void mpxTest_vAnalyse(void)
{
	mpxTest_vSetNextEvent(MPX_EV_FINALIZE);

	/* CAN test */
	mpxTest_vAnalyse_CAN();

	if ((MpxTests.currentTest == TEST_FLASH) && !MpxTests.testError)
		mpxTest_vAnalyse_Flash();

	if ( (MpxTests.currentTest == TEST_IGN_L || MpxTests.currentTest == TEST_IGN_H) && !MpxTests.testError)
		mpxTest_vAnalyse_IGN();

	/* Switched ports test */
	if (!MpxTests.testError)
		mpxTest_vAnalyse_SwitchedPort();

	/* ID test */
	if ((MpxTests.currentTest >= TEST_ID1) && (MpxTests.currentTest <= TEST_ID0 ) && !MpxTests.testError)
		mpxTest_vAnalyse_ID();

	/* Push Pull 10A test */
	else if ((MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_H ) && !MpxTests.testError)
		mpxTest_vAnalyse_PP10A();

	/* BIDI test */
	else if ((MpxTests.currentTest >= TEST_P4) && (MpxTests.currentTest <= TEST_P27 ) && !MpxTests.testError)
		mpxTest_vAnalyse_BIDI();

	/* LODIN test */
	else if ((MpxTests.currentTest >= TEST_P28) && (MpxTests.currentTest <= TEST_P35 ) && !MpxTests.testError)
		mpxTest_vAnalyse_LODIN();

	else if ((MpxTests.currentTest == TEST_NTC) && !MpxTests.testError)
		mpxTest_vAnalyse_NTC();
}

/* Finalize ------------------------------------------------------------------*/
void mpxTest_vFinalize(void)
{
	turnOffMpxPorts();

	MpxTests.testFinished = true;

	if(MpxTests.testError)
		MpxTests.finishedTestBeep = false;

	mpxTest_vSetNextEvent(MPX_EV_PRINT);

	if(!MpxTests.testError && (MpxTests.boolIsAutoTest || MpxTests.boolIsLoopTest) )
	{
		mpxTest_vSetNextEvent(MPX_EV_IDLE);
		MpxTests.testFinished = false;
	}
}

/* Print ---------------------------------------------------------------------*/
void mpxTest_vPrint(void)
{
	if (printTestResult)
		printTestResult();

	if (MpxTests.seriousError)
		//setBeep(1,100);

	if (MpxTests.testFinished  && !MpxTests.finishedTestBeep)
	{
		if (MpxTests.testError)
			setBeep(1, 2000);

		else
			setBeep(3, 100);

		MpxTests.finishedTestBeep = true;
	}

	if (MpxTests.boolIsLoopTest  && !MpxTests.finishedTestBeep)
	{
		setBeep(1, 250);
		MpxTests.finishedTestBeep = true;
	}

	mpxTest_vSetNextEvent(MPX_EV_REFRESH);

	if (!MpxTests.testFinished && !MpxTests.testError && MpxTests.currentTest != TEST_END)
		mpxTest_vSetNextEvent(MPX_EV_IDLE);

	if (MpxStateMachine.state == MPX_ST_PRINT_WAIT)
		mpxTest_vSetNextEvent(MPX_EV_WAIT);
}

/* Finish test----------------------------------------------------------------*/
void mpxTest_vFinish(void)
{
	mpxTest_vResetTests();
	mpxTest_vSetNextEvent(MPX_EV_IDLE);
}

/* ---------------------------------------------------------------------------*/
/* State machine secondary functions -----------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* Execute -------------------------------------------------------------------*/
void mpxTest_vExecute_Flash(void)
{
	PortParameter portOnHigh;
	portOnHigh.mode = 3;
	portOnHigh.duty = 10;
	portOnHigh.fastSoftUp = 0;
	portOnHigh.fastSoftDown = 0;
	portOnHigh.ton = 0;
	portOnHigh.toff = 0;
	portOnHigh.slowSoftUp = 0;
	portOnHigh.slowSoftDown = 0;

	PortParameter portOff;
	portOff.mode = 0;
	portOff.duty = 0;
	portOff.fastSoftUp = 0;
	portOff.fastSoftDown = 0;
	portOff.ton = 0;
	portOff.toff = 0;
	portOff.slowSoftUp = 0;
	portOff.slowSoftDown = 0;

	print_WaitMessage();

	/* Keep execute function until send all emergency port configuration. */
	mpxTest_vSetNextEvent(MPX_EV_REFRESH);

	/* Stop this function if GIGA3 did not receive a new ack. If elapsed a large time since last CAN message,
	 * it will be assumed a memory flash error. */
	if ((mpx.ackIndex == mpx.lastAckIndex) && mpx.ackIndex)
	{
		if (sysTickTimer - MpxTests.startedMemoryAnalyseTime > DELAY_TO_RECEIVE_NEW_MEMORY_ACK)
		{
			MpxTests.testError = true;
			printTestResult = print_FlashTest_error;
			mpxTest_vSetNextEvent(MPX_EV_FINALIZE);
		}
		return;
	}

	/* Save the last ack value and time. */
	mpx.lastAckIndex = mpx.ackIndex;
	MpxTests.startedMemoryAnalyseTime = sysTickTimer;

	if (mpx.ackIndex == 0)
	{
		mpx.portEmeIgn[0] = portOnHigh;
		sendCanPacket(CAN1, CAN_COMMAND_WRITE, CAN_INDEX_EME_IGN_START, MY_ID, mpx.mpxId, &mpx.portEmeIgn[0], 8);
	}

	else if ((mpx.ackIndex >= CAN_INDEX_EME_IGN_START) && (mpx.ackIndex < (CAN_INDEX_EME_IGN_START + NUM_EFFECTIVE_PORTS)))
	{
		if(mpx.ackIndex%2)
		{
			mpx.portEmeIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_IGN_START] = portOnHigh;
			sendCanPacket(CAN1, CAN_COMMAND_WRITE, mpx.ackIndex + 1, MY_ID, mpx.mpxId, &mpx.portEmeIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_IGN_START], 8);
		}

		else
		{
			mpx.portEmeIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_IGN_START] = portOff;
			sendCanPacket(CAN1, CAN_COMMAND_WRITE, mpx.ackIndex + 1, MY_ID, mpx.mpxId, &mpx.portEmeIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_IGN_START], 8);
		}
	}

	else if (mpx.ackIndex == (CAN_INDEX_EME_IGN_START + NUM_EFFECTIVE_PORTS))
	{
		mpx.portEmeNIgn[0] = portOnHigh;
		sendCanPacket(CAN1, CAN_COMMAND_WRITE, CAN_INDEX_EME_N_IGN_START, MY_ID, mpx.mpxId, &mpx.portEmeNIgn[0], 8);
	}

	else if ((mpx.ackIndex >= CAN_INDEX_EME_N_IGN_START) && (mpx.ackIndex < (CAN_INDEX_EME_N_IGN_START + NUM_EFFECTIVE_PORTS)))
	{
		if(mpx.ackIndex%2)
		{
			mpx.portEmeNIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_N_IGN_START] = portOnHigh;
			sendCanPacket(CAN1, CAN_COMMAND_WRITE, mpx.ackIndex + 1, MY_ID, mpx.mpxId, &mpx.portEmeNIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_N_IGN_START], 8);
		}

		else
		{
			mpx.portEmeIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_N_IGN_START] = portOff;
			sendCanPacket(CAN1, CAN_COMMAND_WRITE, mpx.ackIndex + 1, MY_ID, mpx.mpxId, &mpx.portEmeNIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_N_IGN_START], 8);
		}
	}

	/* Finalize execute function when all messages were sent. */
	else if (mpx.ackIndex == (CAN_INDEX_EME_N_IGN_START + NUM_EFFECTIVE_PORTS) )
	{
		mpxTest_vSetNextEvent(MPX_EV_WAIT);
		mpx.ackIndex = 0;
		mpx.lastAckIndex = 0;
	}
}

void mpxTest_vExecute_IGN(void)
{
	if (MpxTests.currentTest == TEST_IGN_L)
		activeMPXIgnition(PORT_LOW);

	else if (MpxTests.currentTest == TEST_IGN_H)
		activeMPXIgnition(PORT_HIGH);
}
void mpxTest_vExecute_ID(void)
{
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
	if ((MpxTests.currentTest >= TEST_P4) && (MpxTests.currentTest <= TEST_P27))
		activeMPXports((MpxTests.currentTest - TEST_P0_H), PORT_HIGH);
}

void mpxTest_vExecute_LODIN(void)
{
	if ((MpxTests.currentTest >= TEST_P28) && (MpxTests.currentTest <= TEST_P35))
		activeMPXports((MpxTests.currentTest - TEST_P0_H), PORT_LOW);
}

/* Analysis ------------------------------------------------------------------*/
void mpxTest_vAnalyse_CAN(void)
{
	if( (sysTickTimer - mpx.lastTimeSeen) > DELAY_TO_CAN_SEND_DATA)
	{
		MpxTests.testError = true;
		MpxTests.seriousError = true;
		printTestResult = print_CANTest_error;
	}
}

void mpxTest_vAnalyse_Flash(void)
{
	int i=0, j=0, sizeStruct = sizeof(PortParameter);
	uint32_t checkSum=0;
	uint8_t *emeIgn, *emeNIgn;

	if(!MpxTests.boolEraseFlash)
	{
		emeIgn = (uint8_t*)&mpx.portEmeIgn[0];
		emeNIgn = (uint8_t*)&mpx.portEmeNIgn[0];

		for (i=0; i<NUM_EFFECTIVE_PORTS; i++)
		{
			for(j=0; j<sizeStruct; j++)
			{
				checkSum += (uint8_t)(*emeIgn);
				checkSum += (uint8_t)(*emeNIgn);
				/* +1 Because the struct is byte aligned */
				emeIgn+=1;
				emeNIgn+=1;
			}
		}

		if ((mpx.readChecksum == checkSum) && (mpx.calculatedChecksum == checkSum))
		{
			printTestResult = print_FlashTest_OK;
			MpxTests.boolEraseFlash = true;
			/* To keep this function until flash be erased. */
			mpxTest_vSetNextEvent(MPX_EV_REFRESH);
		}

		else
		{
			printTestResult = print_FlashTest_error;
			MpxTests.testError = true;
		}

		/* Set parameters to the program erase memory flash. */
		mpx.ackIndex = 0;
		mpx.lastAckIndex = 0;
	}

	else
	{
		/* Flash memory will be erase if there is no error in this test. */
		if (!MpxTests.testError)
			EraseFlash();
	}
}

void EraseFlash(void)
{
	PortParameter portOff;
	portOff.mode = 0;
	portOff.duty = 0;
	portOff.fastSoftUp = 0;
	portOff.fastSoftDown = 0;
	portOff.ton = 0;
	portOff.toff = 0;
	portOff.slowSoftUp = 0;
	portOff.slowSoftDown = 0;

	print_WaitMessage();

	/* Stop this function if GIGA3 did not receive a new ack. */
	if ((mpx.ackIndex == mpx.lastAckIndex) && mpx.ackIndex)
		return;

	/* Save the last ack. */
	mpx.lastAckIndex = mpx.ackIndex;

	if (mpx.ackIndex == 0)
	{
		mpx.portEmeIgn[0] = portOff;
		sendCanPacket(CAN1, CAN_COMMAND_WRITE, CAN_INDEX_EME_IGN_START, MY_ID, mpx.mpxId, &mpx.portEmeIgn[0], 8);
	}

	else if ((mpx.ackIndex >= CAN_INDEX_EME_IGN_START) && (mpx.ackIndex < (CAN_INDEX_EME_IGN_START + NUM_EFFECTIVE_PORTS)))
	{
		mpx.portEmeIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_IGN_START] = portOff;
		sendCanPacket(CAN1, CAN_COMMAND_WRITE, mpx.ackIndex + 1, MY_ID, mpx.mpxId, &mpx.portEmeIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_IGN_START], 8);
	}

	else if (mpx.ackIndex == (CAN_INDEX_EME_IGN_START + NUM_EFFECTIVE_PORTS))
	{
		mpx.portEmeNIgn[0] = portOff;
		sendCanPacket(CAN1, CAN_COMMAND_WRITE, CAN_INDEX_EME_N_IGN_START, MY_ID, mpx.mpxId, &mpx.portEmeNIgn[0], 8);
	}

	else if ((mpx.ackIndex >= CAN_INDEX_EME_N_IGN_START) && (mpx.ackIndex < (CAN_INDEX_EME_N_IGN_START + NUM_EFFECTIVE_PORTS)))
	{
		mpx.portEmeNIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_N_IGN_START] = portOff;
		sendCanPacket(CAN1, CAN_COMMAND_WRITE, mpx.ackIndex + 1, MY_ID, mpx.mpxId, &mpx.portEmeNIgn[mpx.ackIndex + 1 - CAN_INDEX_EME_N_IGN_START], 8);
	}

	/* Finalize ANALYSE event when all messages were sent. */
	else if (mpx.ackIndex == (CAN_INDEX_EME_N_IGN_START + NUM_EFFECTIVE_PORTS) )
		mpxTest_vSetNextEvent(MPX_EV_FINALIZE);
}

void mpxTest_vAnalyse_IGN(void)
{
	if (MpxTests.currentTest == TEST_IGN_H && (mpx.MpxFlags[0] && 0x40))
	{
		printTestResult = print_IgnTest_OK;
	}

	else if (MpxTests.currentTest == TEST_IGN_L && !(mpx.MpxFlags[0] && 0x40))
	{
		/* Both next code line is used to analyse high ignition. */
		MpxTests.currentTest = TEST_IGN_H;
		mpxTest_vSetNextEvent(MPX_EV_EXECUTE);
	}

	else
	{
		MpxTests.testError = true;
		printTestResult = print_IgnTest_error;
	}
}

void mpxTest_vAnalyse_SwitchedPort(void)
{
	int i;
	uint8_t status;

	/* Verify if some port is switched with LODIN ports. */
	if ((MpxTests.currentTest >= TEST_P0_H) && (MpxTests.currentTest <= TEST_P35))
		for (i=28; i<36; i++)
		{
			if (i == (MpxTests.currentTest - TEST_P0_H))
				continue;

			if (mpx.portInput[i] == 0x02)
			{
				MpxTests.switchPort = i;
				MpxTests.testError = true;
				printTestResult = print_SwitchedPortTest_error;
				return;
			}
		}

	if ((MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_L))
		for (i=0; i<28; i++)
		{
			if (i == (MpxTests.currentTest - TEST_P0_L))
				continue;

			if (mpx.portInput[i] == 0x01)
			{
				MpxTests.switchPort = i;
				MpxTests.testError = true;
				printTestResult = print_SwitchedPortTest_error;
				return;
			}
		}

	else if ((MpxTests.currentTest >= TEST_P0_H) && (MpxTests.currentTest <= TEST_P27))
		for (i=0; i<28; i++)
		{
			if (i == (MpxTests.currentTest - TEST_P0_H))
				continue;

			if (getPortStatus(i))
			{
				MpxTests.switchPort = i;
				MpxTests.testError = true;
				printTestResult = print_SwitchedPortTest_error;
				return;
			}
		}
}

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
		printTestResult = print_IDTest_error;
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
		}

		/* is MPX port high? */
		else
		{
			/* Is MPX device port high? */
			if (mpx.portInput[MpxTests.currentTest-TEST_P0_L] && 0x01)
			{
				MpxTests.testError = true;
				printTestResult = print_PortTest_FetError;
			}
			/* Is MPX device port low? */
			else
			{
				MpxTests.testError = true;
				printTestResult = print_PortTest_PortOpenError;
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
			if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] == 0x0D)
				printTestResult = print_PortTest_PortOpenError;

			else if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] == 0x00)
				printTestResult = print_PortTest_FetError;

			else if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] && 0x10)
			{
				printTestResult = print_PortTest_PortShortError;
				MpxTests.seriousError = true;
			}
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
		if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] == 0x0D)
			printTestResult = print_PortTest_PortOpenError;

		else if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] == 0x00)
			printTestResult = print_PortTest_FetError;

		else if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] && 0x10)
		{
			printTestResult = print_PortTest_PortShortError;
			MpxTests.seriousError = true;
		}
	}
}

void mpxTest_vAnalyse_LODIN(void)
{
	if (mpx.portInput[MpxTests.currentTest-TEST_P0_H] == 0x02)
	{
		MpxTests.testError = false;
		printTestResult = print_PortTest_OK;
	}

	else
	{
		MpxTests.testError = true;
		printTestResult = print_PortTest_error;
	}
}

void mpxTest_vAnalyse_NTC(void)
{
	int max_tries=10;

	if ((mpx.ntcTemperature >= 0x17) && (mpx.ntcTemperature <= 0x23))
	{
		MpxTests.testError = false;
		printTestResult = print_NTCTest_OK;
		NUM_NTC_TRIES = 0;
		return;
	}

	else
	{
		printTestResult = print_NTCTest_error;
	}

	NUM_NTC_TRIES++;

	/* Stop Analyse loop if number of tries reaches a max value. */
	if (NUM_NTC_TRIES == max_tries*4)
	{
		MpxTests.testError = true;
		mpxTest_vSetNextEvent(MPX_EV_FINALIZE);
		NUM_NTC_TRIES = 0;
	}

	else
		/* Keep the program in Analyse state until it reaches max_tries. */
		mpxTest_vSetNextEvent(MPX_EV_REFRESH);
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
		snprintf(TestMessages.lines[0], LINE_SIZE, "    Aguarde!    ");
		sprintf(message, "Fazendo Testes");
		sprintf(message, "  Executando");
		printTestMessage(TestMessages.lines[1], message, 3);
	}
}

void print_ClearMessages(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"                  ");
	snprintf(TestMessages.lines[1],LINE_SIZE,"                  ");
}

void print_CANTest_error(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste CAN: erro ");
	snprintf(TestMessages.lines[1],LINE_SIZE,"Erro COM1 2 ou 4");
}

void print_FlashTest_OK(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste Flash: OK  ");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_FlashTest_error(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste Flash:erro");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_SwitchedPortTest_error(void)
{
	if ( (MpxTests.currentTest >= TEST_P0_L) && (MpxTests.currentTest <= TEST_P3_L) )
		snprintf(TestMessages.lines[0],LINE_SIZE,"Erro: Conect %s ", CN2[MpxTests.currentTest - TEST_P0_L]);
	else if ( (MpxTests.currentTest >= TEST_P0_H) && (MpxTests.currentTest <= TEST_P35) )
		snprintf(TestMessages.lines[0],LINE_SIZE,"Erro: Conect %s ", CN2[MpxTests.currentTest - TEST_P0_H]);

	sprintf(message,"e %s trocados", CN2[MpxTests.switchPort]);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_AutoTest_OK(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste Auto: Fim ");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

/* Ignition test print functions ---------------------------------------------*/
void print_IgnTest_OK(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste Ignicao:OK");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_IgnTest_error(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Falha Ignicao   ");
	sprintf(message, "Problema COM7");
	printTestMessage(TestMessages.lines[1], message, 3);
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

void print_IDTest_error(void)
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

void print_PortTest_error(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Erro CN%s        ", CN[MpxTests.currentTest - TEST_P0_L]);
	sprintf(message, "Pressione Enter", 1);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_PortTest_PortOpenError(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Erro CN%s        ", CN[MpxTests.currentTest - TEST_P0_L]);
	sprintf(message, "Canal em aberto");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_PortTest_PortShortError(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Erro CN%s        ", CN[MpxTests.currentTest - TEST_P0_L]);
	sprintf(message, "Canal em curto");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_PortTest_FetError(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Erro CN%s        ", CN[MpxTests.currentTest - TEST_P0_L]);
	sprintf(message, "Problema %s", FET[MpxTests.currentTest - TEST_P0_L]);
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_NTCTest_OK(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste NTC: OK    ");
	sprintf(message, "Pressione Enter");
	printTestMessage(TestMessages.lines[1], message, 1);
}

void print_NTCTest_error(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE,"Teste NTC: erro    ");
	sprintf(message, "Verificar CN5");
	printTestMessage(TestMessages.lines[1], message, 3);
}

void print_OnGoing(void)
{
	snprintf(TestMessages.lines[0],LINE_SIZE," Em Construcao. ");
	sprintf(message, "Pressione Enter", 1);
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
