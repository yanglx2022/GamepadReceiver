/**
  ******************************************************************************
  * @file    usb_prop.c
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   All processing related to Gamepad Mouse Demo
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
  *      may be used to endorse or promote
  products derived from this software
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


/* Includes ------------------------------------------------------------------*/
#include "usb_prop.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t ProtocolValue;

/* -------------------------------------------------------------------------- */
/*  Structures initializations */
/* -------------------------------------------------------------------------- */

DEVICE Device_Table =
  {
    2,	// Total_Endpoint;     /* Number of endpoints that are used */
    1		// Total_Configuration;/* Number of configuration available */
  };

DEVICE_PROP Device_Property =
  {
    Gamepad_init,
    Gamepad_Reset,
    Gamepad_Status_In,
    Gamepad_Status_Out,
    Gamepad_Data_Setup,
    Gamepad_NoData_Setup,
    Gamepad_Get_Interface_Setting,
    Gamepad_GetDeviceDescriptor,
    Gamepad_GetConfigDescriptor,
    Gamepad_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
  };
USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    Gamepad_GetConfiguration,
    Gamepad_SetConfiguration,
    Gamepad_GetInterface,
    Gamepad_SetInterface,
    Gamepad_GetStatus,
    Gamepad_ClearFeature,
    Gamepad_SetEndPointFeature,
    Gamepad_SetDeviceFeature,
    Gamepad_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (uint8_t*)Gamepad_DeviceDescriptor,
    GAMEPAD_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (uint8_t*)Gamepad_ConfigDescriptor,
    GAMEPAD_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR Gamepad_Report_Descriptor =
  {
    (uint8_t *)Gamepad_ReportDescriptor,
    GAMEPAD_SIZ_REPORT_DESC
  };

ONE_DESCRIPTOR Mouse_Hid_Descriptor =
  {
    (uint8_t*)Gamepad_ConfigDescriptor + GAMEPAD_OFF_HID_DESC,
    GAMEPAD_SIZ_HID_DESC
  };

ONE_DESCRIPTOR String_Descriptor[4] =
  {
    {(uint8_t*)Gamepad_StringLangID, GAMEPAD_SIZ_STRING_LANGID},
    {(uint8_t*)Gamepad_StringVendor, GAMEPAD_SIZ_STRING_VENDOR},
    {(uint8_t*)Gamepad_StringProduct, GAMEPAD_SIZ_STRING_PRODUCT},
    {(uint8_t*)Gamepad_StringSerial, GAMEPAD_SIZ_STRING_SERIAL}
  };

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * Function Name  : Gamepad_init.
  * Description    : Gamepad Mouse init routine.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
void Gamepad_init(void)
{
  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum();

  pInformation->Current_Configuration = 0;
  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  DeviceState = UNCONNECTED;
}

/**
  * Function Name  : Gamepad_Reset.
  * Description    : Gamepad Mouse reset routine.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
void Gamepad_Reset(void)
{
  /* Set Gamepad_DEVICE as not configured */
  pInformation->Current_Configuration = 0;
  pInformation->Current_Interface = 0;/*the default Interface*/

  /* Current Feature initialization */
  pInformation->Current_Feature = Gamepad_ConfigDescriptor[7];
  SetBTABLE(BTABLE_ADDRESS);
  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_INTERRUPT);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxCount(ENDP1, HID_REPORT_DATA_SIZE);
  SetEPRxStatus(ENDP1, EP_RX_DIS);
  SetEPTxStatus(ENDP1, EP_TX_NAK);

  /* Set this device to response on default address */
  SetDeviceAddress(0);
  DeviceState = ATTACHED;
}

/**
  * Function Name  : Gamepad_SetConfiguration.
  * Description    : Update the device state to configured.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
void Gamepad_SetConfiguration(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    DeviceState = CONFIGURED;
  }
}

/**
  * Function Name  : Gamepad_SetConfiguration.
  * Description    : Update the device state to addressed.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
void Gamepad_SetDeviceAddress (void)
{
  DeviceState = ADDRESSED;
}
/**
  * Function Name  : Gamepad_Status_In.
  * Description    : Gamepad status IN routine.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
void Gamepad_Status_In(void)
{}

/**
  * Function Name  : Gamepad_Status_Out
  * Description    : Gamepad status OUT routine.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
void Gamepad_Status_Out (void)
{}

/**
  * Function Name  : Gamepad_Data_Setup
  * Description    : Handle the data class specific requests.
  * Input          : Request Nb.
  * Output         : None.
  * Return         : USB_UNSUPPORT or USB_SUCCESS.
  */
RESULT Gamepad_Data_Setup(uint8_t RequestNo)
{
  uint8_t *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;
  if ((RequestNo == GET_DESCRIPTOR)
      && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
      && (pInformation->USBwIndex0 == 0))
  {
    if (pInformation->USBwValue1 == REPORT_DESCRIPTOR)
    {
      CopyRoutine = Gamepad_GetReportDescriptor;
    }
    else if (pInformation->USBwValue1 == HID_DESCRIPTOR_TYPE)
    {
      CopyRoutine = Gamepad_GetHIDDescriptor;
    }

  } /* End of GET_DESCRIPTOR */

  /*** GET_PROTOCOL ***/
  else if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
           && RequestNo == GET_PROTOCOL)
  {
    CopyRoutine = Gamepad_GetProtocolValue;
  }
  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }
  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

/**
  * Function Name  : Gamepad_NoData_Setup
  * Description    : handle the no data class specific requests
  * Input          : Request Nb.
  * Output         : None.
  * Return         : USB_UNSUPPORT or USB_SUCCESS.
  */
RESULT Gamepad_NoData_Setup(uint8_t RequestNo)
{
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == SET_PROTOCOL))
  {
    return Gamepad_SetProtocol();
  }

  else
  {
    return USB_UNSUPPORT;
  }
}

/**
  * Function Name  : Gamepad_GetDeviceDescriptor.
  * Description    : Gets the device descriptor.
  * Input          : Length
  * Output         : None.
  * Return         : The address of the device descriptor.
  */
uint8_t *Gamepad_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/**
  * Function Name  : Gamepad_GetConfigDescriptor.
  * Description    : Gets the configuration descriptor.
  * Input          : Length
  * Output         : None.
  * Return         : The address of the configuration descriptor.
  */
uint8_t *Gamepad_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/**
  * Function Name  : Gamepad_GetStringDescriptor
  * Description    : Gets the string descriptors according to the needed index
  * Input          : Length
  * Output         : None.
  * Return         : The address of the string descriptors.
  */
uint8_t *Gamepad_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  if (wValue0 >= 4)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

/**
  * Function Name  : Gamepad_GetReportDescriptor.
  * Description    : Gets the HID report descriptor.
  * Input          : Length
  * Output         : None.
  * Return         : The address of the configuration descriptor.
  */
uint8_t *Gamepad_GetReportDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Gamepad_Report_Descriptor);
}

/**
  * Function Name  : Gamepad_GetHIDDescriptor.
  * Description    : Gets the HID descriptor.
  * Input          : Length
  * Output         : None.
  * Return         : The address of the configuration descriptor.
  */
uint8_t *Gamepad_GetHIDDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Mouse_Hid_Descriptor);
}

/**
  * Function Name  : Gamepad_Get_Interface_Setting.
  * Description    : tests the interface and the alternate setting according to the
  *                  supported one.
  * Input          : - Interface : interface number.
  *                  - AlternateSetting : Alternate Setting number.
  * Output         : None.
  * Return         : USB_SUCCESS or USB_UNSUPPORT.
  */
RESULT Gamepad_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  else if (Interface > 0)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}

/**
  * Function Name  : Gamepad_SetProtocol
  * Description    : Gamepad Set Protocol request routine.
  * Input          : None.
  * Output         : None.
  * Return         : USB SUCCESS.
  */
RESULT Gamepad_SetProtocol(void)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  ProtocolValue = wValue0;
  return USB_SUCCESS;
}

/**
  * Function Name  : Gamepad_GetProtocolValue
  * Description    : get the protocol value
  * Input          : Length.
  * Output         : None.
  * Return         : address of the protocol value.
  */
uint8_t *Gamepad_GetProtocolValue(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 1;
    return NULL;
  }
  else
  {
    return (uint8_t *)(&ProtocolValue);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
