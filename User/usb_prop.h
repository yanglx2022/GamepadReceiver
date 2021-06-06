/**
  ******************************************************************************
  * @file    usb_prop.h
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   All processing related to Gamepad Mouse demo
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PROP_H
#define __USB_PROP_H

/* Includes ------------------------------------------------------------------*/
#include "usb_hw.h"
/* Exported types ------------------------------------------------------------*/
typedef enum _HID_REQUESTS
{
  GET_REPORT = 1,
  GET_IDLE,
  GET_PROTOCOL,

  SET_REPORT = 9,
  SET_IDLE,
  SET_PROTOCOL
} HID_REQUESTS;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Gamepad_init(void);
void Gamepad_Reset(void);
void Gamepad_SetConfiguration(void);
void Gamepad_SetDeviceAddress (void);
void Gamepad_Status_In (void);
void Gamepad_Status_Out (void);
RESULT Gamepad_Data_Setup(uint8_t);
RESULT Gamepad_NoData_Setup(uint8_t);
RESULT Gamepad_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *Gamepad_GetDeviceDescriptor(uint16_t );
uint8_t *Gamepad_GetConfigDescriptor(uint16_t);
uint8_t *Gamepad_GetStringDescriptor(uint16_t);
RESULT Gamepad_SetProtocol(void);
uint8_t *Gamepad_GetProtocolValue(uint16_t Length);
RESULT Gamepad_SetProtocol(void);
uint8_t *Gamepad_GetReportDescriptor(uint16_t Length);
uint8_t *Gamepad_GetHIDDescriptor(uint16_t Length);

/* Exported define -----------------------------------------------------------*/
#define Gamepad_GetConfiguration          NOP_Process
//#define Gamepad_SetConfiguration          NOP_Process
#define Gamepad_GetInterface              NOP_Process
#define Gamepad_SetInterface              NOP_Process
#define Gamepad_GetStatus                 NOP_Process
#define Gamepad_ClearFeature              NOP_Process
#define Gamepad_SetEndPointFeature        NOP_Process
#define Gamepad_SetDeviceFeature          NOP_Process
//#define Gamepad_SetDeviceAddress          NOP_Process

#define REPORT_DESCRIPTOR                  0x22

#endif /* __USB_PROP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
