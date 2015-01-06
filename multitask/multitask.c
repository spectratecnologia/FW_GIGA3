#include "multitask.h"

static uint64_t virtualTimer[MAX_VIRTUAL_TIMERS]=
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static uint8_t functionEnabled[MAX_VIRTUAL_TIMERS]=
				{true,true,true,true,true,true,true,true,true,true,
				 true,true,true,true,true,true,true,true,true,true};

static uint32_t maxExecutionTimes[MAX_VIRTUAL_TIMERS];

/* 2^64ms = 5.84555e8 years. Enough =)
 * 1ms/count
 *  */
volatile uint64_t sysTickTimer=0;

void initMultiTask(){
	RCC_ClocksTypeDef RCC_Clocks;
#if DEBUG != 0
	printf("[MultiTask]Starting Setup\n");
#endif
	/* Set SysTick to 1ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

	/* Use only preemption priority (do not use sub priority) */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Set SysTick to highest priority */
	NVIC_SetPriority(SysTick_IRQn, 3);

#if DEBUG != 0
	printf("[MultiTask]Starting Setup\n");
#endif
}


/* CAUTION: IT ASSUMES SYSTICK CONFIGURED TO 1 ms */
void executeEveryInterval(uint8_t timerIndex, uint32_t intervalInMs, void (*function)(void)){
	if ( timerIndex >= MAX_VIRTUAL_TIMERS){
#if DEBUG != 0
		printf("[APP]Virtual timers out of bound\n");
#endif
		return;
	}

	if(((sysTickTimer - virtualTimer[timerIndex]) >= intervalInMs) && (functionEnabled[timerIndex])){
		int s,e,total;
		s=sysTickTimer;
		function();
		e=sysTickTimer;
		total=e-s;
		if(total>maxExecutionTimes[timerIndex]){
			maxExecutionTimes[timerIndex]=total;
		}
		virtualTimer[timerIndex] = sysTickTimer;
	}
}

void enableFunction(uint8_t timerIndex, uint8_t enable){
	functionEnabled[timerIndex] = enable;
}

uint32_t getMaxExecutionTime(uint8_t timerIndex){
	return maxExecutionTimes[timerIndex];
}


void delayMsUsingSysTick(uint32_t delayMs){
		uint64_t startTimeInMs=sysTickTimer;
		while(sysTickTimer-startTimeInMs<delayMs);
}

inline bool achievedTimeout(uint32_t startTimeInMs, uint32_t timeoutInMs){
	return (sysTickTimer-startTimeInMs>timeoutInMs);
}

/**
 * @brief  This function is the SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void){
	sysTickTimer++;
}




