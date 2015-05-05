/**
  ******************************************************************************
  * @file    usbh_usr.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file is the header file for usb usr file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USR_DEMO_H__
#define __USR_DEMO_H__


/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include <stdio.h>

#include "lcd_screen.h"

/** @addtogroup USBH_USER
* @{
*/

/** @addtogroup USBH_DUAL_FUNCT_DEMO
* @{
*/
  
/** @defgroup USBH_DUAL_FUNCT_DEMO
  * @brief This file is the header file for user action
  * @{
  */ 


/** @defgroup USBH_DUAL_FUNCT_DEMO_Exported_Variables
  * @{
  */ 

#define _MAX_LFN 15;

typedef enum {
	DUAL_USB_HOST_INIT = 0,
	DUAL_USB_HOST_WAIT,
	DUAL_USB_HOST_RUNNING,
	DUAL_USB_HOST_DONE_WAITING_DISCONNECT,
	DUAL_USB_DEVICE_INIT,
	DUAL_USB_DEVICE_WAIT
}UsbState;

typedef enum {
	HOST_INIT_STATE_IDLE = 0,
	HOST_INIT_STATE_RUNNING = 1,
	HOST_INIT_STATE_DONE = 2
}HostInitState;


/**
  * @}
  */ 

/** @defgroup USBH_DUAL_FUNCT_DEMO_Exported_FunctionsPrototype
  * @{
  */ 
void initUSBVCP();

void initDualUSB (void);
void dualUSBProcess (void);
void dualUsbApplication();
void dualUsbHandleDisconnect();

void writeTestFile();

#endif /* __USR_DEMO_H__ */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
