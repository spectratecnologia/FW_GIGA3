/* Includes ------------------------------------------------------------------*/
#include "tests/smtest.h"

/* Local functions declaration -----------------------------------------------*/
void test_vIdle(void);
/* Local MPX functions declaration -------------------------------------------*/
void test_vMPXtest(void);
void test_vMPXIDtest(void);
void test_vMPXEndtest(void);

/* Local MPX functions declaration -------------------------------------------*/
void test_vPTC24test(void);

/* Local Variables -----------------------------------------------------------*/
StateMachine testSM;

static volatile int8_t nextEvent;

const Transition testSMTrans[] =  		//TABELA DE ESTADOS
{
/*Current state		Event				Next state				callback */
{ST_IDLE			,EV_REFRESH       	,ST_IDLE           		,&test_vIdle		},
{ST_IDLE			,EV_MPX_TEST       	,ST_MPX_TEST    		,&test_vMPXtest		},
{ST_IDLE			,EV_PTC24_TEST	  	,ST_PTC24_TEST         	,&test_vPTC24test	},

/* MPX's State Machine */
{ST_MPX_TEST		,EV_REFRESH	      	,ST_MPX_TEST	 	  	,&test_vMPXtest		},
{ST_MPX_TEST		,EV_AUTOMATIC	    ,ST_MPX_TEST_ID0 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST		,EV_TEST_ID0	    ,ST_MPX_TEST_ID0 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST		,EV_TEST_ID1	    ,ST_MPX_TEST_ID1 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST		,EV_TEST_ID2	    ,ST_MPX_TEST_ID2 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST		,EV_TEST_ID4	    ,ST_MPX_TEST_ID4 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST		,EV_TEST_END	    ,ST_MPX_TEST_END 	  	,&test_vMPXEndtest	},

{ST_MPX_TEST_ID0	,EV_AUTOMATIC	    ,ST_MPX_TEST_ID1 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST_ID0	,EV_MANUAL		    ,ST_MPX_TEST	 	  	,&test_vMPXtest		},

{ST_MPX_TEST_ID1 	,EV_AUTOMATIC	    ,ST_MPX_TEST_ID2 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST_ID1	,EV_MANUAL		    ,ST_MPX_TEST	 	  	,&test_vMPXtest		},

{ST_MPX_TEST_ID2	,EV_AUTOMATIC	    ,ST_MPX_TEST_ID4 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST_ID2	,EV_MANUAL		    ,ST_MPX_TEST	 	  	,&test_vMPXtest		},

{ST_MPX_TEST_ID4	,EV_AUTOMATIC	    ,ST_MPX_TEST_END 	  	,&test_vMPXEndtest	},
{ST_MPX_TEST_ID4	,EV_LOOP		    ,ST_MPX_TEST_ID0 	  	,&test_vMPXIDtest	},
{ST_MPX_TEST_ID4	,EV_MANUAL		    ,ST_MPX_TEST	 	  	,&test_vMPXtest		}};

#define TRANS_COUNT (sizeof(testSMTrans)/sizeof(*testSMTrans))

/* Functions -----------------------------------------------------------------*/

/* General state machine functions -------------------------------------------*/
void  test_vStateMachineLoop(void)
{
	int i;
	testSM.event = test_vGetNextEvent();

	/* Execute test state machine */
	for (i = 0; i < TRANS_COUNT; i++)
	{
		if ((testSM.state == testSM.trans[i].state) || (ST_ANY == testSM.trans[i].state))
		{
			if (((testSM.event == testSM.trans[i].event) || (EV_ANY == testSM.trans[i].event)))
			{
				testSM.state = testSM.trans[i].next;

                if (testSM.trans[i].fn)
                {
                    (testSM.trans[i].fn)();
                }
                break;
			}
		}
	}
}

void  test_vSetNextEvent(TestsStEvents event)
{
	nextEvent = event;
}

void test_vIdle(void)
{
	return;
}

/* MPX state machine functions -----------------------------------------------*/
void test_vMPXtest(void)
{
	return;
}

void test_vMPXIDtest(void)
{
	return;
}

void test_vMPXEndtest(void)
{
	return;
}
/* PTC24 state machine functions ---------------------------------------------*/
void test_vPTC24test(void)
{
	return;
}
/* PTC16 state machine functions ---------------------------------------------*/
