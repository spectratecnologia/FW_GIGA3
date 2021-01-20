#include <time.h>

#include "rtc.h"
#include "multitask.h"

bool isRTCConfigured();
void configureRTC();
void printResetCause();

struct tm realtime;

void initRTC(){
#if DEBUG != 0
	printf("[RTC]Starting Setup\n");
#endif
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	if ( !isRTCConfigured() ){
#if DEBUG != 0
		printf("[RTC]Configuring RTC\n");
#endif
		configureRTC();
	}
	else{
#if DEBUG != 0
		printf("[RTC]No need to configure RTC\n");
#endif
		printResetCause();
	}

	/* Clear reset flags */
	RCC_ClearFlag();

#if DEBUG != 0
	printf("[RTC]Setup complete\n");
#endif


}

bool isRTCConfigured(){
	return (BKP_ReadBackupRegister(RTC_CONFIGURED_BKP_REGISTER) == RTC_CONFIGURED_BKP_CONSTANT);
}

void configureRTC(){

	  /* Allow access to BKP Domain */
	  PWR_BackupAccessCmd(ENABLE);

	  /* Reset Backup Domain */
	  BKP_DeInit();

	  /* Enable LSE */
	  RCC_LSEConfig(RCC_LSE_ON);
#if DEBUG != 0
	  printf("[RTC]Waiting for RTC crystal\n");
#endif
	  /* Wait till LSE is ready */
	  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
#if DEBUG != 0
	  printf("[RTC]RTC crystal Ok\n");
#endif
	  /* Select LSE as RTC Clock Source */
	  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	  /* Enable RTC Clock */
	  RCC_RTCCLKCmd(ENABLE);

	  /* Wait for RTC registers synchronization */
	  RTC_WaitForSynchro();

	  /* Wait until last write operation on RTC registers has finished */
	  RTC_WaitForLastTask();

	  /* Set RTC prescaler: set RTC period to 1sec */
	  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	  /* Wait until last write operation on RTC registers has finished */
	  RTC_WaitForLastTask();

	  /* Wait until last write operation on RTC registers has finished */
	  RTC_WaitForLastTask();

	  /* Change the current time */
	  RTC_SetCounter(0);

	  /* Wait until last write operation on RTC registers has finished */
	  RTC_WaitForLastTask();

	  /* Set Configured Constant to backup register */
	  BKP_WriteBackupRegister(RTC_CONFIGURED_BKP_REGISTER, RTC_CONFIGURED_BKP_CONSTANT);

	  /* Allow access to BKP Domain */
	  PWR_BackupAccessCmd(DISABLE);
}

void printResetCause(){
    /* Check if the Power On Reset flag is set */
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
#if DEBUG != 0
      printf("[RTC]Power On Reset occurred\n");
#endif
    }
    /* Check if the Pin Reset flag is set */
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {
#if DEBUG != 0
      printf("[RTC]External Reset occurred\n");
#endif
    }
}

static const uint8_t MONTH_DAYS[12] = {31, 27, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

bool isLeapYear(time_t time) {
	return(getYear(time)%4 == 0);
}

uint8_t maxDaysOnMoth(time_t time) {

	uint8_t mon = getMonth(time);

	if(mon == 2 && isLeapYear(time)) {
		return 28;
	}

	return MONTH_DAYS[mon-1];;
}

time_t time(time_t* t) {
	time_t time = readRTC();
	*t = time;
	return time;
}

void setTime(time_t t) {

	PWR_BackupAccessCmd(ENABLE);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Change the current time */
	RTC_SetCounter(t);
	/* Wait until last write operation on RTC registers has finished */

	RTC_WaitForLastTask();
	PWR_BackupAccessCmd(DISABLE);

}

uint32_t readRTC(void){
//	uint32_t seconds_aux = 0;

	RTC_WaitForLastTask();
	uint32_t rtc_seconds = RTC_GetCounter();
	return rtc_seconds;
}

void writeTime(uint32_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds){

//	uint32_t rtc_seconds = 0;

	realtime.tm_year = year - 1900;
	realtime.tm_mon = month-1;
	realtime.tm_mday = day;
	realtime.tm_hour = hours;
	realtime.tm_min = minutes;
	realtime.tm_sec = seconds;

	time_t now;
	now = mktime(&realtime);

	setTime(now);
}

uint8_t getSeconds(time_t now){
	struct tm* ltime = localtime(&now);
	return ltime->tm_sec;
}

uint8_t getMinutes(time_t now){
	struct tm* ltime = localtime(&now);
	return ltime->tm_min;
}

uint8_t getHours(time_t now){
	struct tm* ltime = localtime(&now);
	return ltime->tm_hour;
}

uint8_t getDay(time_t now){
	struct tm* ltime = localtime(&now);
	return ltime->tm_mday;
}

uint8_t getMonth(time_t now){
	struct tm* ltime = localtime(&now);
	return ltime->tm_mon+1;
}

uint32_t getYear(time_t now){
	struct tm* ltime = localtime(&now);
	return ltime->tm_year + 1900;
}
