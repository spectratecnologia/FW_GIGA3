/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GIGA3_SR_H
#define __GIGA3_SR_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* SPI2 input/output (shift register control) */
#define SR_READ GPIO_Pin_11
#define SR_WRITE GPIO_Pin_12
#define SR_WR_PORT GPIOB

#define SPI2_SCK GPIO_Pin_13
#define SPI2_MISO GPIO_Pin_14
#define SPI2_MOSI GPIO_Pin_15
#define SPI2_PORT GPIOB

/* Exported functions ------------------------------------------------------- */
void initShiftRegisters(void);
void sendDataToSR(uint32_t);
uint32_t readDataFromSR(void);
uint8_t getSRBitStatus(uint8_t);

#endif /* GIGA3_SR_H */
