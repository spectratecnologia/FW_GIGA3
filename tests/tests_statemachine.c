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
{ST_IDLE			,EV_REFRESH       	,ST_IDLE           		,&test_vIdle	},

{ST_TEST_MPX		,EV_REFRESH	   		,ST_TEST_MPX	 	  	,&test_vIdle	},
{ST_TEST_MPX		,EV_AUTOMATIC   	,ST_TEST_MPX_ID1 	  	,&test_vIdle	},
{ST_TEST_MPX		,EV_LOOP		   	,ST_TEST_MPX_ID1 	  	,&test_vIdle	},

{ST_TEST_MPX_ID1	,EV_REFRESH	    	,ST_TEST_MPX_ID1 	  	,&test_vIdle	},
{ST_TEST_MPX_ID1 	,EV_AUTOMATIC	    ,ST_TEST_MPX_ID2 	  	,&test_vIdle	},
{ST_TEST_MPX_ID1	,EV_LOOP		    ,ST_TEST_MPX_ID2 	  	,&test_vIdle	},

{ST_TEST_MPX_ID2	,EV_REFRESH	    	,ST_TEST_MPX_ID2 	  	,&test_vIdle	},
{ST_TEST_MPX_ID2	,EV_AUTOMATIC	    ,ST_TEST_MPX_ID4 	  	,&test_vIdle	},
{ST_TEST_MPX_ID2	,EV_LOOP		    ,ST_TEST_MPX_ID4 	  	,&test_vIdle	},

{ST_TEST_MPX_ID4	,EV_REFRESH	    	,ST_TEST_MPX_ID4 	  	,&test_vIdle	},
{ST_TEST_MPX_ID4	,EV_AUTOMATIC	    ,ST_TEST_MPX_ID0 	  	,&test_vIdle	},
{ST_TEST_MPX_ID4	,EV_LOOP		    ,ST_TEST_MPX_ID0 	  	,&test_vIdle	},

{ST_TEST_MPX_ID0	,EV_REFRESH	   		,ST_TEST_MPX_ID0 	  	,&test_vIdle	},
{ST_TEST_MPX_ID0	,EV_AUTOMATIC	    ,ST_TEST_MPX_P0_L	  	,&test_vIdle	},
{ST_TEST_MPX_ID0	,EV_LOOP		    ,ST_TEST_MPX_P0_L 	  	,&test_vIdle	},

{ST_TEST_MPX_P0_L	,EV_REFRESH	    	,ST_TEST_MPX_P0_L 	  	,&test_vIdle	},
{ST_TEST_MPX_P0_L	,EV_AUTOMATIC	    ,ST_TEST_MPX_P1_L 	  	,&test_vIdle	},
{ST_TEST_MPX_P0_L	,EV_LOOP		    ,ST_TEST_MPX_P1_L 	  	,&test_vIdle	},

{ST_TEST_MPX_P1_L	,EV_REFRESH	    	,ST_TEST_MPX_P1_L 	  	,&test_vIdle	},
{ST_TEST_MPX_P1_L	,EV_AUTOMATIC	    ,ST_TEST_MPX_P2_L	  	,&test_vIdle	},
{ST_TEST_MPX_P1_L	,EV_LOOP		    ,ST_TEST_MPX_P2_L 	  	,&test_vIdle	},

{ST_TEST_MPX_P2_L	,EV_REFRESH	    	,ST_TEST_MPX_P2_L 	  	,&test_vIdle	},
{ST_TEST_MPX_P2_L	,EV_AUTOMATIC	    ,ST_TEST_MPX_P3_L 	  	,&test_vIdle	},
{ST_TEST_MPX_P2_L	,EV_LOOP		    ,ST_TEST_MPX_P3_L 	  	,&test_vIdle	},

{ST_TEST_MPX_P3_L	,EV_REFRESH	    	,ST_TEST_MPX_P3_L 	  	,&test_vIdle	},
{ST_TEST_MPX_P3_L	,EV_AUTOMATIC	    ,ST_TEST_MPX_P0_H 	  	,&test_vIdle	},
{ST_TEST_MPX_P3_L	,EV_LOOP		    ,ST_TEST_MPX_P0_H 	  	,&test_vIdle	},

{ST_TEST_MPX_P0_H	,EV_REFRESH	   		,ST_TEST_MPX_P0_H  		,&test_vIdle	},
{ST_TEST_MPX_P0_H	,EV_AUTOMATIC	    ,ST_TEST_MPX_P1_H 	  	,&test_vIdle	},
{ST_TEST_MPX_P0_H	,EV_LOOP		    ,ST_TEST_MPX_P1_H 	  	,&test_vIdle	},

{ST_TEST_MPX_P1_H	,EV_REFRESH	    	,ST_TEST_MPX_P1_H  		,&test_vIdle	},
{ST_TEST_MPX_P1_H	,EV_AUTOMATIC	    ,ST_TEST_MPX_P2_H 	  	,&test_vIdle	},
{ST_TEST_MPX_P1_H	,EV_LOOP		    ,ST_TEST_MPX_P2_H 	  	,&test_vIdle	},

{ST_TEST_MPX_P2_H	,EV_REFRESH	   	 	,ST_TEST_MPX_P2_H  		,&test_vIdle	},
{ST_TEST_MPX_P2_H	,EV_AUTOMATIC	    ,ST_TEST_MPX_P3_H 	  	,&test_vIdle	},
{ST_TEST_MPX_P2_H	,EV_LOOP		    ,ST_TEST_MPX_P3_H 	  	,&test_vIdle	},

{ST_TEST_MPX_P3_H	,EV_REFRESH	    	,ST_TEST_MPX_P3_H  		,&test_vIdle	},
{ST_TEST_MPX_P3_H	,EV_AUTOMATIC	    ,ST_TEST_MPX_P4 	  	,&test_vIdle	},
{ST_TEST_MPX_P3_H	,EV_LOOP		    ,ST_TEST_MPX_P4 	  	,&test_vIdle	},

{ST_TEST_MPX_P4		,EV_REFRESH	    	,ST_TEST_MPX_P4  		,&test_vIdle	},
{ST_TEST_MPX_P4		,EV_AUTOMATIC	    ,ST_TEST_MPX_P5 	  	,&test_vIdle	},
{ST_TEST_MPX_P4		,EV_LOOP		    ,ST_TEST_MPX_P5 	  	,&test_vIdle	},

{ST_TEST_MPX_P5		,EV_REFRESH	    	,ST_TEST_MPX_P5  		,&test_vIdle	},
{ST_TEST_MPX_P5		,EV_AUTOMATIC	    ,ST_TEST_MPX_P6 	  	,&test_vIdle	},
{ST_TEST_MPX_P5		,EV_LOOP		    ,ST_TEST_MPX_P6 	  	,&test_vIdle	},

{ST_TEST_MPX_P6		,EV_REFRESH	    	,ST_TEST_MPX_P6  		,&test_vIdle	},
{ST_TEST_MPX_P6		,EV_AUTOMATIC	    ,ST_TEST_MPX_P7 	  	,&test_vIdle	},
{ST_TEST_MPX_P6		,EV_LOOP		    ,ST_TEST_MPX_P7 	  	,&test_vIdle	},

{ST_TEST_MPX_END	,EV_REFRESH	   		,ST_TEST_MPX_END 	  	,&test_vIdle	}
};

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

void test_vStateMachineInit(void)
{
	testSM.state = ST_IDLE;
	testSM.trans = &testSMTrans;
	testSM.transCount = TRANS_COUNT;
	test_vSetNextEvent(EV_REFRESH);
}

void  test_vSetNextEvent(StEvents event)
{
	nextEvent = event;
}

void test_vJumpToState(StStates state)
{
	testSM.state = state;
	test_vSetNextEvent(EV_REFRESH);
}

int test_vGetState(void)
{
	return testSM.state;
}

void test_vIdle(void)
{
	test_vSetNextEvent(EV_REFRESH);
}

