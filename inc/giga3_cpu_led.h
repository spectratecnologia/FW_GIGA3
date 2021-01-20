/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GIGA3_CPU_LED_H
#define __GIGA3_CPU_LED_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define IO_STATUS_LED_CPU_PORT GPIOD
#define IO_STATUS_LED_CPU_PIN GPIO_Pin_15

/* Exported functions --------------------------------------------------------*/
void initCPULed(void);
void toggleCPULED(void);
void setStatusLed(void);
void resetStatusLed(void);
bool getCPULEDstatus(void);

#endif /* __GIGA3_CPU_LED_H */
