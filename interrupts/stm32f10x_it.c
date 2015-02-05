/**
  ******************************************************************************
  * @file    EXTI/EXTI_Config/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "multitask/multitask.h"

/* USB includes */
#include "usb_core.h"
#include "usbd_core.h"
#include "usbd_cdc_core.h"

extern USB_OTG_CORE_HANDLE           USB_OTG_dev;
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

//extern inline void LCD_ClockTick();
//extern inline void updateLedScan();
//extern inline void onCAN1ReceiveInterrupt();
//extern inline void onUSART2Interrupt();
//extern inline void onCAN1StatusChangeError();

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/


/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	printf("NMI_Handler\n");
}

// From Joseph Yiu, minor edits by FVH
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler
/*void hard_fault_handler_c (unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;

  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);

  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);

  printf ("\n\n[Hard fault handler - all numbers in hex]\n");
  printf ("R0 = %x\n", stacked_r0);
  printf ("R1 = %x\n", stacked_r1);
  printf ("R2 = %x\n", stacked_r2);
  printf ("R3 = %x\n", stacked_r3);
  printf ("R12 = %x\n", stacked_r12);
  printf ("LR [R14] = %x  subroutine call return address\n", stacked_lr);
  printf ("PC [R15] = %x  program counter\n", stacked_pc);
  printf ("PSR = %x\n", stacked_psr);
  printf ("BFAR = %lx\n", (*((volatile unsigned long *)(0xE000ED38))));
  printf ("CFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED28))));
  printf ("HFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED2C))));
  printf ("DFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED30))));
  printf ("AFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED3C))));
  printf ("SCB_SHCSR = %lx\n", SCB->SHCSR);

  while (1);
}*/

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
/*
void HardFault_Handler(void)
{
	/*__asm("TST LR, #4\n"
		  "ITE EQ\n"
	      "MRSEQ R0, MSP\n"
	      "MRSNE R0, PSP\n"
	      "B hard_fault_handler_c\n");*/

	//printf("HardFault_Handler\n");

  /* Go to infinite loop when Hard Fault exception occurs */
  //while (1)
  //{
  //}
//}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	printf("MemManage_Handler\n");
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	printf("BusFault_Handler\n");
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  printf("UsageFault_Handler\n");
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	printf("SVC_Handler\n");
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
	printf("DebugMon_Handler\n");
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	printf("PendSV_Handler\n");
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

void OTG_FS_WKUP_IRQHandler(void)
{
	/*
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }*/
  //EXTI_ClearITPendingBit(EXTI_Line18);
}

/**
  * @brief  This function handles OTG_FS Handler.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

/**
  * @brief  This function handles USART2 global interrupt request.
  * @param  None
  * @retval None
  */


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
