/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H
#define __SPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_spi.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define SPI1_NSS GPIO_Pin_4
#define SPI1_SCK GPIO_Pin_5
#define SPI1_MISO GPIO_Pin_6
#define SPI1_MOSI GPIO_Pin_7
#define SPI1_PORT GPIOA

#define SPI2_NSS GPIO_Pin_12
#define SPI2_SCK GPIO_Pin_13
#define SPI2_MISO GPIO_Pin_14
#define SPI2_MOSI GPIO_Pin_15
#define SPI2_PORT GPIOB

/* Exported functions ------------------------------------------------------- */
void initSPIs();
void initSPI2();

#endif /* __SPI_H */
