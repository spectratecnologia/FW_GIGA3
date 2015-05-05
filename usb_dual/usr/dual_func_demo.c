/**
  ******************************************************************************
  * @file    dual_func_demo.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file contain the demo implementation
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "dual_func_demo.h"
#include "usbh_core.h"
#include "usbh_msc_usr.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbh_msc_core.h"
#include "usbd_cdc_vcp.h"
#include "usb_conf.h"
#include "multitask.h"
#include "functions.h"
//#include "LCD/lcd_screen.h"
//#include "functions_config.h"

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_Core __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST               USB_Host __ALIGN_END ;


/* Dual USB Finite State Machine */
UsbState usbFsmState;

/* USB Host current init state */
uint8_t hostInitState = HOST_INIT_STATE_IDLE;

/*
 * Define if the USB is either on host or device mode
 * This Option is chosen on the USB Config Menu on the LCD screen.
 */
extern bool isUSBHostMode;

/* Define if the log which will be saved
 * will contain only the recent never saved data
 * or if it will contain all data in the flash, including
 * data not overwritten yet in the flash.
 */
extern uint8_t usbSaveMode;

FATFS fatfs;

uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;


/**
* @brief  initDualUSB
*         Dual USB initialization
* @param  None
* @retval None
*/
void initDualUSB (void)
{
  printf("[dualUSB]Init. USB Dual ... \n");
  usbFsmState = DUAL_USB_DEVICE_INIT;
  hostInitState = HOST_INIT_STATE_IDLE;
}


/**
* @brief  dualUSBProcess
*         dual usb background task
* @param  None
* @retval None
*/
void dualUSBProcess (void)
{
	/* If USB in on Host Mode
	 * Execute the State Machine related to the Host protocol.
	 */
	if(usbFsmState == DUAL_USB_HOST_INIT || usbFsmState == DUAL_USB_HOST_WAIT ||
	   usbFsmState == DUAL_USB_HOST_RUNNING || usbFsmState == DUAL_USB_HOST_DONE_WAITING_DISCONNECT) {
	  if(HCD_IsDeviceConnected(&USB_OTG_Core))
	  {
		  USBH_Process(&USB_OTG_Core, &USB_Host);
	  }
	}

	dualUsbApplication();
}

void dualUsbApplication() {
	switch(usbFsmState) {
	case DUAL_USB_HOST_INIT:
		if(isUSBHostMode) {
			if(hostInitState == HOST_INIT_STATE_IDLE) {
				USBH_Init(&USB_OTG_Core,
							USB_OTG_FS_CORE_ID,
							&USB_Host,
							&USBH_MSC_cb,
							&USR_USBH_MSC_cb);
				hostInitState = HOST_INIT_STATE_RUNNING;
			}
			if(hostInitState == HOST_INIT_STATE_DONE) {
				/* at this moment the usb device was already connected */
				usbFsmState = DUAL_USB_HOST_WAIT;
				LCD_vLogSaveStart();
				//multiplex.currentPtc.memory[MEMORY_INDEX_TEST_STEP] = TEST_USB_HOST;
				return;
			}
		}
		else {
			dualUsbHandleDisconnect();
			usbFsmState = DUAL_USB_DEVICE_INIT;
		}
		break;
	case DUAL_USB_HOST_WAIT:
		if(isUSBHostMode) {
			if (HCD_IsDeviceConnected(&USB_OTG_Core)) {
				/*waiting user input */
				if(usbSaveMode != USB_SAVE_MODE_NOT_CHOSEN) {
					usbFsmState = DUAL_USB_HOST_RUNNING;
				}
				/* if the user cancels the log save routine, but the pendrive is still connected */
				else if(LCD_vLogSaveCanceled()) {
					usbFsmState = DUAL_USB_HOST_DONE_WAITING_DISCONNECT;
				}
			}
			else {
				dualUsbHandleDisconnect();
				LCD_vLogSaveInterrupted(NULL);
				usbFsmState = DUAL_USB_HOST_INIT;
			}
		}
		else {
			dualUsbHandleDisconnect();
			usbFsmState = DUAL_USB_DEVICE_INIT;
		}
		break;
	case DUAL_USB_HOST_RUNNING:
		/*
		 * Execute the USB Host Function related to the application.
		 * In this application, try to save the data log to the USB Flash drive.
		 */
		if(isUSBHostMode) {
			if (HCD_IsDeviceConnected(&USB_OTG_Core)) {
				/* If the download isn't done and isn't started */
				if(!logDownloadToUSBStarted() && !logDownloadToUSBDone()) {
					int mountRet;
					if ((mountRet = f_mount(0, &fatfs))) {
						printf("[DUAL USB] Fat mount error\n");
						LCD_vLogSaveInterrupted(LCD_USB_MSG_ERROR_CANT_MOUNT);
						usbFsmState = DUAL_USB_HOST_DONE_WAITING_DISCONNECT;
					}
					if(mountRet == FR_OK) {
						if(!startLogDownloadToUSB(usbSaveMode == USB_SAVE_NEW)) {
							LCD_vLogSaveInterrupted(LCD_USB_MSG_ERROR_CANT_CREATE_FILE);
							usbFsmState = DUAL_USB_HOST_DONE_WAITING_DISCONNECT;
						}
					}
				}
				else {
					/* if the download started and is not done */
					if(logDownloadToUSBDone()) {
						setLogDownloadToUSBNotDone();
						f_mount(0, NULL);
						usbFsmState = DUAL_USB_HOST_DONE_WAITING_DISCONNECT;
					}
				}
			}
			else {
				f_mount(0, NULL);
				dualUsbHandleDisconnect();
				usbFsmState = DUAL_USB_HOST_INIT;
				LCD_vLogSaveInterrupted(NULL);
			}
		}
		else {
			dualUsbHandleDisconnect();
			usbFsmState = DUAL_USB_DEVICE_INIT;
		}
		break;

	case DUAL_USB_HOST_DONE_WAITING_DISCONNECT: /* Data log save attempt happens once. It is triggered when the pendrive is connected */
		if(!HCD_IsDeviceConnected(&USB_OTG_Core)) {
			dualUsbHandleDisconnect();
			usbFsmState = DUAL_USB_HOST_INIT;
		}
		break;
	case DUAL_USB_DEVICE_INIT:
		if(!isUSBHostMode) {
			initUSBVCP();
			usbFsmState = DUAL_USB_DEVICE_WAIT;
		}
		else { //to avoid problem ... Testing
			USBD_DeInit(&USB_OTG_Core);
			usbFsmState = DUAL_USB_HOST_INIT;
		}
		break;
	case DUAL_USB_DEVICE_WAIT:
		if(!isUSBHostMode) {
		}
		else {
			DCD_DevDisconnect (&USB_OTG_Core);
			USB_OTG_StopDevice(&USB_OTG_Core);
			usbFsmState = DUAL_USB_HOST_INIT;
		}
		break;
	default:
		break;
	}
}

void dualUsbHandleDisconnect (void) {
	hostInitState = HOST_INIT_STATE_IDLE;

	USBH_DeInit(&USB_OTG_Core, &USB_Host);

	f_mount(0, NULL);
	USB_OTG_StopHost(&USB_OTG_Core);

	/* Manage User disconnect operations*/
	USB_Host.usr_cb->DeviceDisconnected();

	/* Re-Initilaize Host for new Enumeration */
	USBH_DeInit(&USB_OTG_Core, &USB_Host);
	USB_Host.usr_cb->DeInit();
	USB_Host.class_cb->DeInit(&USB_OTG_Core, &USB_Host.device_prop);
}

void initUSBVCP(){
#if DEBUG == 1
	printf("[VCP]Initing USB VCP Device\n");
#endif
	USBD_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USR_desc,
			&USBD_CDC_cb, &USR_cb);
}

void writeTestFile() {

}


/**
* @}
*/ 

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
