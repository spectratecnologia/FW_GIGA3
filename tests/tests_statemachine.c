/* Includes ------------------------------------------------------------------*/
#include "tests/tests_statemachine.h"

/* Local functions declaration -----------------------------------------------*/
void test_vIdle(void);
/* Local MPX functions declaration -------------------------------------------*/

/* Local Variables -----------------------------------------------------------*/
StateMachine testSM;

static volatile int8_t nextEvent;

const Transition testSMTrans[] =  		//TABELA DE ESTADOS
{
/*Current state		Event				Next state				callback */
{TST_IDLE			,TEV_REFRESH       	,TST_IDLE           	,&test_vIdle	},

{TST_MPX_TEST		,TEV_REFRESH	   	,TST_MPX_TEST	 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_AUTOMATIC   	,TST_MPX_TEST_ID1 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_LOOP		   	,TST_MPX_TEST_ID1 	  	,&test_vIdle	},

{TST_MPX_TEST_ID1	,TEV_REFRESH	    ,TST_MPX_TEST_ID1 	  	,&test_vIdle	},
{TST_MPX_TEST_ID1 	,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID2 	  	,&test_vIdle	},
{TST_MPX_TEST_ID1	,TEV_LOOP		    ,TST_MPX_TEST_ID2 	  	,&test_vIdle	},

{TST_MPX_TEST_ID2	,TEV_REFRESH	    ,TST_MPX_TEST_ID2 	  	,&test_vIdle	},
{TST_MPX_TEST_ID2	,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID4 	  	,&test_vIdle	},
{TST_MPX_TEST_ID2	,TEV_LOOP		    ,TST_MPX_TEST_ID4 	  	,&test_vIdle	},

{TST_MPX_TEST_ID4	,TEV_REFRESH	    ,TST_MPX_TEST_ID4 	  	,&test_vIdle	},
{TST_MPX_TEST_ID4	,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST_ID4	,TEV_LOOP		    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},

{TST_MPX_TEST_ID0	,TEV_REFRESH	    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST_ID0	,TEV_AUTOMATIC	    ,TST_MPX_TEST_END	  	,&test_vIdle	},
{TST_MPX_TEST_ID0	,TEV_LOOP		    ,TST_MPX_TEST_ID1 	  	,&test_vIdle	},

{TST_MPX_TEST_P0_L	,TEV_REFRESH	    ,TST_MPX_TEST_P0_L 	  	,&test_vIdle	},
{TST_MPX_TEST_P0_L	,TEV_AUTOMATIC	    ,TST_MPX_TEST_P0_H 	  	,&test_vIdle	},
{TST_MPX_TEST_P0_L	,TEV_LOOP		    ,TST_MPX_TEST_P0_H 	  	,&test_vIdle	},

{TST_MPX_TEST_P0_H	,TEV_REFRESH	    ,TST_MPX_TEST_P0_H  	,&test_vIdle	},
{TST_MPX_TEST_P0_H	,TEV_AUTOMATIC	    ,TST_MPX_TEST_END 	  	,&test_vIdle	},
{TST_MPX_TEST_P0_H	,TEV_LOOP		    ,TST_MPX_TEST_END 	  	,&test_vIdle	},

{TST_MPX_TEST_END	,TEV_REFRESH	    ,TST_MPX_TEST_END 	  	,&test_vIdle	}
};
	/* OLD
{TST_IDLE			,TEV_REFRESH       	,TST_IDLE           	,&test_vIdle	},
{TST_IDLE			,TEV_MPX_TEST      	,TST_MPX_TEST    		,&test_vIdle	},
{TST_IDLE			,TEV_PTC24_TEST	  	,TST_PTC24_TEST         ,&test_vIdle	},

{TST_MPX_TEST		,TEV_REFRESH	   	,TST_MPX_TEST	 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_LOOP		    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_TEST_ID0	    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_TEST_ID1	    ,TST_MPX_TEST_ID1 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_TEST_ID2	    ,TST_MPX_TEST_ID2 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_TEST_ID4	    ,TST_MPX_TEST_ID4 	  	,&test_vIdle	},
{TST_MPX_TEST		,TEV_TEST_END	    ,TST_MPX_TEST_END 	  	,&test_vIdle	},

{TST_MPX_TEST_ID1	,TEV_REFRESH	    ,TST_MPX_TEST_ID1 	  	,&test_vIdle	},
{TST_MPX_TEST_ID1 	,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID2 	  	,&test_vIdle	},
{TST_MPX_TEST_ID1	,TEV_LOOP		    ,TST_MPX_TEST_ID2 	  	,&test_vIdle	},
{TST_MPX_TEST_ID1	,TEV_TEST_END	    ,TST_MPX_TEST	 	  	,&test_vIdle	},

{TST_MPX_TEST_ID2	,TEV_REFRESH	    ,TST_MPX_TEST_ID2 	  	,&test_vIdle	},
{TST_MPX_TEST_ID2	,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID4 	  	,&test_vIdle	},
{TST_MPX_TEST_ID2	,TEV_LOOP		    ,TST_MPX_TEST_ID4 	  	,&test_vIdle	},
{TST_MPX_TEST_ID2	,TEV_TEST_END	    ,TST_MPX_TEST	 	  	,&test_vIdle	},

{TST_MPX_TEST_ID4	,TEV_REFRESH	    ,TST_MPX_TEST_ID4 	  	,&test_vIdle	},
{TST_MPX_TEST_ID4	,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST_ID4	,TEV_LOOP		    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST_ID4	,TEV_TEST_END	    ,TST_MPX_TEST	 	  	,&test_vIdle	},

{TST_MPX_TEST_ID0	,TEV_REFRESH	    ,TST_MPX_TEST_ID0 	  	,&test_vIdle	},
{TST_MPX_TEST_ID0	,TEV_AUTOMATIC	    ,TST_MPX_TEST_ID1 	  	,&test_vIdle	},
{TST_MPX_TEST_ID0	,TEV_LOOP		    ,TST_MPX_TEST_ID1 	  	,&test_vIdle	},
{TST_MPX_TEST_ID0	,TEV_TEST_END	    ,TST_MPX_TEST	 	  	,&test_vIdle	}
};

*/

#define TRANS_COUNT (sizeof(testSMTrans)/sizeof(*testSMTrans))

/* Functions -----------------------------------------------------------------*/

/* General state machine functions -------------------------------------------*/

void  test_vStateMachineLoop(void)
{
	int i;
	testSM.event = nextEvent;

	/* Execute test state machine */
	for (i = 0; i < TRANS_COUNT; i++)
	{
		if ((testSM.state == testSM.trans[i].state) || (TST_ANY == testSM.trans[i].state))
		{
			if (((testSM.event == testSM.trans[i].event) || (TEV_ANY == testSM.trans[i].event)))
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

void test_vStateMachineInit(void)
{
	testSM.state = TST_IDLE;
	testSM.trans = &testSMTrans;
	testSM.transCount = TRANS_COUNT;

	/* Use event EV_KBD_CANCEL to update lcd.sbLineMax in LCD_vMenuDebug */
	test_vSetNextEvent(TEV_REFRESH);
}

void  test_vSetNextEvent(TestsStEvents event)
{
	nextEvent = event;
}

void test_vJumpToState(TestsStStates state)
{
	testSM.state = state;
	test_vSetNextEvent(TEV_REFRESH);
}

TestsStStates test_vGetState(void)
{
	return testSM.state;
}

void test_vIdle(void)
{
	test_vSetNextEvent(TEV_REFRESH);
}

