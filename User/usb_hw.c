
#include "usb_hw.h"

// USB状态
// extern
__IO uint32_t DeviceState = UNCONNECTED; /* USB device status */

// 上帧发送完成标志
__IO uint8_t PrevXferComplete = 1;

// 中断服务变量
// extern
__IO uint16_t wIstr;  /* ISTR register last read value */ // usb_int.c用到,usb_reg.h extern声明
// local
__IO uint8_t bIntPackSOF = 0;  /* SOFs received between 2 consecutive packets */
__IO uint32_t esof_counter =0; /* expected SOF counter */
__IO bool fSuspendEnabled = TRUE;  /* true when suspend is possible */

// Resume变量
// local
__IO uint32_t remotewakeupon=0;
struct
{
  __IO RESUME_STATE eState;
  __IO uint8_t bESOFcnt;
}ResumeS;

// Callback
void (*pEpInt_IN[7])(void) =
{
	EP1_IN_Callback,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
};
void (*pEpInt_OUT[7])(void) =
{
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
};


// USB硬件初始化
void USB_HW_Init(void)
{
	// 使能PA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// 设置引脚（PA11:USBDM  PA12:USBDP）
  GPIO_InitTypeDef  GPIO_InitStructure;  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	// 设置pull-up引脚(PA8)
	GPIO_InitStructure.GPIO_Pin = USB_CONNECT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(USB_CONNECT_GPIO, &GPIO_InitStructure);
	USB_Connect(FALSE);
	// 设置USB时钟
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);	// 72/48=1.5分频
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
	// 配置USB中断
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


// USB中断服务
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	uint32_t i=0;
	__IO uint32_t EP[8];
  
  wIstr = _GetISTR();
#if (IMR_MSK & ISTR_CTR)
  if (wIstr & ISTR_CTR & wInterrupt_Mask)
  {
    /* servicing of the endpoint correct transfer interrupt */
    /* clear of the CTR flag into the sub */
    CTR_LP();
#ifdef CTR_CALLBACK
    CTR_Callback();
#endif
  }
#endif  
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_RESET)
  if (wIstr & ISTR_RESET & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_RESET);
    Device_Property.Reset();
#ifdef RESET_CALLBACK
    RESET_Callback();
#endif
  }
#endif
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_DOVR)
  if (wIstr & ISTR_DOVR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_DOVR);
#ifdef DOVR_CALLBACK
    DOVR_Callback();
#endif
  }
#endif
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_ERR)
  if (wIstr & ISTR_ERR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_ERR);
#ifdef ERR_CALLBACK
    ERR_Callback();
#endif
  }
#endif
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_WKUP)
  if (wIstr & ISTR_WKUP & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_WKUP);
    Resume(RESUME_EXTERNAL);
#ifdef WKUP_CALLBACK
    WKUP_Callback();
#endif
  }
#endif
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_SUSP)
  if (wIstr & ISTR_SUSP & wInterrupt_Mask)
  {

    /* check if SUSPEND is possible */
    if (fSuspendEnabled)
    {
      Suspend();
    }
    else
    {
      /* if not possible then resume after xx ms */
      Resume(RESUME_LATER);
    }
    /* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
    _SetISTR((uint16_t)CLR_SUSP);
#ifdef SUSP_CALLBACK
    SUSP_Callback();
#endif
  }
#endif
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_SOF)
  if (wIstr & ISTR_SOF & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_SOF);
    bIntPackSOF++;

#ifdef SOF_CALLBACK
    SOF_Callback();
#endif
  }
#endif
  /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_ESOF)
  if (wIstr & ISTR_ESOF & wInterrupt_Mask)
  {
    /* clear ESOF flag in ISTR */
    _SetISTR((uint16_t)CLR_ESOF);
    
    if ((_GetFNR()&FNR_RXDP)!=0)
    {
      /* increment ESOF counter */
      esof_counter ++;
      
      /* test if we enter in ESOF more than 3 times with FSUSP =0 and RXDP =1=>> possible missing SUSP flag*/
      if ((esof_counter >3)&&((_GetCNTR()&CNTR_FSUSP)==0))
      {           
        /* this a sequence to apply a force RESET*/
      
        /*Store CNTR value */
				uint32_t wCNTR = _GetCNTR(); 
      
        /*Store endpoints registers status */
        for (i=0;i<8;i++) EP[i] = _GetENDPOINT(i);
      
        /*apply FRES */
        wCNTR|=CNTR_FRES;
        _SetCNTR(wCNTR);
 
        /*clear FRES*/
        wCNTR&=~CNTR_FRES;
        _SetCNTR(wCNTR);
      
        /*poll for RESET flag in ISTR*/
        while((_GetISTR()&ISTR_RESET) == 0);
  
        /* clear RESET flag in ISTR */
        _SetISTR((uint16_t)CLR_RESET);
   
       /*restore Enpoints*/
        for (i=0;i<8;i++)
        _SetENDPOINT(i, EP[i]);
      
        esof_counter = 0;
      }
    }
    else
    {
        esof_counter = 0;
    }
    
    /* resume handling timing is made with ESOFs */
    Resume(RESUME_ESOF); /* request without change of the machine state */
#ifdef ESOF_CALLBACK
    ESOF_Callback();
#endif
    
  }
#endif
}

// EP1 OUT Callback Routine
void EP1_IN_Callback(void)
{
  /* Set the transfer complete token to inform upper layer that the current 
  transfer has been complete */
  PrevXferComplete = 1; 
}
 

// USB软连接
void USB_Connect(bool con)
{
  if (con != FALSE)
  {
    GPIO_SetBits(USB_CONNECT_GPIO, USB_CONNECT_PIN);
  }
  else
  {
    GPIO_ResetBits(USB_CONNECT_GPIO, USB_CONNECT_PIN);
  }
}

// USB发送
uint8_t USB_Send(uint8_t* data)
{
	if ((DeviceState == CONFIGURED) && (PrevXferComplete))
	{
		PrevXferComplete = 0;
		USB_SIL_Write(EP1_IN, data, HID_REPORT_DATA_SIZE);
		SetEPTxValid(ENDP1);
		return 1;
	}
	return 0;
}


/**
  * Function Name  : HexToChar.
  * Description    : Convert Hex 32Bits value into char.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;
  
  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10; 
    }
    
    value = value << 4;
    
    pbuf[ 2* idx + 1] = 0;
  }
}

/**
  * Function Name  : Get_SerialNum.
  * Description    : Create the serial number string descriptor.
  * Input          : None.
  * Output         : None.
  * Return         : None.
  */
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(uint32_t*)ID1;
  Device_Serial1 = *(uint32_t*)ID2;
  Device_Serial2 = *(uint32_t*)ID3;
  
  Device_Serial0 += Device_Serial2;

  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &Gamepad_StringSerial[2] , 8);
    IntToUnicode (Device_Serial1, &Gamepad_StringSerial[18], 4);
  }
}



/**
  * Function Name  : PowerOn
  * Description    :
  * Input          : None.
  * Output         : None.
  * Return         : USB_SUCCESS.
  */
RESULT PowerOn(void)
{
  uint16_t wRegVal;
  
  /*** cable plugged-in ? ***/
  USB_Connect(TRUE);

  /*** CNTR_PWDN = 0 ***/
  wRegVal = CNTR_FRES;
  _SetCNTR(wRegVal);

  /* The following sequence is recommended:
    1- FRES = 0
    2- Wait until RESET flag = 1 (polling)
    3- clear ISTR register */

  /*** CNTR_FRES = 0 ***/
  wInterrupt_Mask = 0;
  
  _SetCNTR(wInterrupt_Mask);
  
  /* Wait until RESET flag = 1 (polling) */
  while((_GetISTR()&ISTR_RESET) == 1);
  
  /*** Clear pending interrupts ***/
  SetISTR(0);
  
  /*** Set interrupt mask ***/
  wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
  _SetCNTR(wInterrupt_Mask);
  
  return USB_SUCCESS;
}

/**
  * Function Name  : PowerOff
  * Description    : handles switch-off conditions
  * Input          : None.
  * Output         : None.
  * Return         : USB_SUCCESS.
  **/
RESULT PowerOff()
{
  /* disable all interrupts and force USB reset */
  _SetCNTR(CNTR_FRES);
  
  /* clear interrupt status register */
  _SetISTR(0);
  
  /* Disable the Pull-Up*/
  USB_Connect(FALSE);

  /* switch-off device */
  _SetCNTR(CNTR_FRES + CNTR_PDWN);
  /* sw variables reset */
  /* ... */

  return USB_SUCCESS;
}

/**
  * Function Name  : Suspend
  * Description    : sets suspend mode operating conditions
  * Input          : None.
  * Output         : None.
  * Return         : USB_SUCCESS.
  */
void Suspend(void)
{
  uint32_t i =0;
  uint16_t wCNTR;
	uint32_t EP[8];
  
  /* suspend preparation */
  /* ... */
  
  /*Store CNTR value */
  wCNTR = _GetCNTR();  
  
  /* This a sequence to apply a force RESET to handle a robustness case */
  
  /*Store endpoints registers status */
  for (i=0;i<8;i++) EP[i] = _GetENDPOINT(i);
  
  /* unmask RESET flag */
  wCNTR|=CNTR_RESETM;
  _SetCNTR(wCNTR);
  
  /*apply FRES */
  wCNTR|=CNTR_FRES;
  _SetCNTR(wCNTR);
  
  /*clear FRES*/
  wCNTR&=~CNTR_FRES;
  _SetCNTR(wCNTR);
  
  /*poll for RESET flag in ISTR*/
  while((_GetISTR()&ISTR_RESET) == 0);
  
  /* clear RESET flag in ISTR */
  _SetISTR((uint16_t)CLR_RESET);
  
  /*restore Enpoints*/
  for (i=0;i<8;i++)
    _SetENDPOINT(i, EP[i]);
  
  /* Now it is safe to enter macrocell in suspend mode */
  wCNTR |= CNTR_FSUSP;
  _SetCNTR(wCNTR);
  
  /* force low-power mode in the macrocell */
  wCNTR = _GetCNTR();
  wCNTR |= CNTR_LPMODE;
  _SetCNTR(wCNTR);
  
}

/**
  * Function Name  : Resume_Init
  * Description    : Handles wake-up restoring normal operations
  * Input          : None.
  * Output         : None.
  * Return         : USB_SUCCESS.
  */
void Resume_Init(void)
{
  uint16_t wCNTR;
  
  /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
  /* restart the clocks */
  /* ...  */

  /* CNTR_LPMODE = 0 */
  wCNTR = _GetCNTR();
  wCNTR &= (~CNTR_LPMODE);
  _SetCNTR(wCNTR);    

  /* reset FSUSP bit */
  _SetCNTR(IMR_MSK);

  /* reverse suspend preparation */
  /* ... */ 

}

/*******************************************************************************
* Function Name  : Resume
* Description    : This is the state machine handling resume operations and
*                 timing sequence. The control is based on the Resume structure
*                 variables and on the ESOF interrupt calling this subroutine
*                 without changing machine state.
* Input          : a state machine value (RESUME_STATE)
*                  RESUME_ESOF doesn't change ResumeS.eState allowing
*                  decrementing of the ESOF counter in different states.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Resume(RESUME_STATE eResumeSetVal)
{
  uint16_t wCNTR;

  if (eResumeSetVal != RESUME_ESOF)
    ResumeS.eState = eResumeSetVal;
  switch (ResumeS.eState)
  {
    case RESUME_EXTERNAL:
      if (remotewakeupon ==0)
      {
        Resume_Init();
        ResumeS.eState = RESUME_OFF;
      }
      else /* RESUME detected during the RemoteWAkeup signalling => keep RemoteWakeup handling*/
      {
        ResumeS.eState = RESUME_ON;
      }
      break;
    case RESUME_INTERNAL:
      Resume_Init();
      ResumeS.eState = RESUME_START;
      remotewakeupon = 1;
      break;
    case RESUME_LATER:
      ResumeS.bESOFcnt = 2;
      ResumeS.eState = RESUME_WAIT;
      break;
    case RESUME_WAIT:
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
        ResumeS.eState = RESUME_START;
      break;
    case RESUME_START:
      wCNTR = _GetCNTR();
      wCNTR |= CNTR_RESUME;
      _SetCNTR(wCNTR);
      ResumeS.eState = RESUME_ON;
      ResumeS.bESOFcnt = 10;
      break;
    case RESUME_ON:    
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
      {
        wCNTR = _GetCNTR();
        wCNTR &= (~CNTR_RESUME);
        _SetCNTR(wCNTR);
        ResumeS.eState = RESUME_OFF;
        remotewakeupon = 0;
      }
      break;
    case RESUME_OFF:
    case RESUME_ESOF:
    default:
      ResumeS.eState = RESUME_OFF;
      break;
  }
}


