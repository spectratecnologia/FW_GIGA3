/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H
#define __RTC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define RTC_CONFIGURED_BKP_REGISTER BKP_DR1
#define RTC_CONFIGURED_BKP_CONSTANT 0xA5A5



/* Exported functions ------------------------------------------------------- */
void initRTC();

uint32_t readRTC();
time_t time(time_t* t);

inline void setTime(time_t t);

bool isLeapYear(time_t time);
uint8_t maxDaysOnMoth(time_t time);

/* These functions implement conversion
 * from Unix time format to user understandable format
 *
 */
uint8_t getSeconds(time_t now);
uint8_t getMinutes(time_t now);
uint8_t getHours(time_t now);
uint8_t getDay(time_t now);
uint8_t getMonth(time_t now);
uint32_t getYear(time_t now);


/* Seconds. [0-60] (1 leap second)
/* Minutes. [0-59] */
/* Hours.   [0-23] */
/* Day.     [1-31] */
/* Month.   [1-12] */
/* Year [1900 .. 1900 + (2^32 -1)] - Infinity ...  */
/* This function should be use to set RTC using data in user understandable format to UNIX time format*/
void writeTime(uint32_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds);

#endif /* __RTC_H */
