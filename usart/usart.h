/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H
#define __USART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void initUSART();

inline void onUSART2Interrupt();

inline void processReceivedChar(char c);

bool isAllDateTimeDownloaded();

bool isUSARTAvailableToWrite();

void startDownloadDateTime();
bool isDownloadDateTimeDetected();
void finishDateTimeDownload();

uint32_t writeBytesToUSART(char *data, uint32_t len);

/* Place both these functions in a better place */
void rebootToBootloader(void);
void rebootPTC();

#endif /* __USART_H */
