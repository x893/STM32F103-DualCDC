#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"

USART_InitTypeDef USART_InitStructure;

const GPIO_InitTypeDef TX1_pin = { GPIO_Pin_9,  GPIO_Speed_50MHz, GPIO_Mode_AF_PP };
const GPIO_InitTypeDef RX1_pin = { GPIO_Pin_10, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING };
const GPIO_InitTypeDef TX2_pin = { GPIO_Pin_2,  GPIO_Speed_50MHz, GPIO_Mode_AF_PP };
const GPIO_InitTypeDef RX2_pin = { GPIO_Pin_3,  GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING };

#ifdef USB_DISCONNECT_PIN
const GPIO_InitTypeDef USB_DISC_pin = {
	USB_DISCONNECT_PIN,
	GPIO_Speed_50MHz,
	GPIO_Mode_Out_OD
	};
#endif

USB_USART_Data_t USB_USART1_Data = { 0, 0, ENDP3, EP3_OUT, ENDP1, EP1_IN,
	USART1,
	GPIOA, &TX1_pin, &RX1_pin,
	RCC_APB2Periph_GPIOA,
	0,
	RCC_APB2Periph_USART1,
	{
		115200, /* baud rate*/
		0x00,   /* stop bits-1*/
		0x00,   /* parity - none*/
		0x08    /* no. of bits 8*/
	}
};

USB_USART_Data_t USB_USART2_Data = { 0, 0, ENDP6, EP6_OUT, ENDP4, EP4_IN,
	USART2,
	GPIOA, &TX2_pin, &RX2_pin,
	RCC_APB2Periph_GPIOA,
	RCC_APB1Periph_USART2,
	0,
	{
		115200, /* baud rate*/
		0x00,   /* stop bits-1*/
		0x00,   /* parity - none*/
		0x08    /* no. of bits 8*/
	}
};

/*******************************************************************************
* Function Name  : HardwareInit
* Description    : Configures hardware
* Input          : None.
* Return         : None.
*******************************************************************************/
void HardwareInit(void)
{
#ifdef USB_DISCONNECT_PIN

	/* Enable USB_DISCONNECT GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

	USB_Cable_Config(DISABLE);

	/* Configure USB pull-up pin */
	GPIO_Init(USB_DISCONNECT, (GPIO_InitTypeDef *)&USB_DISC_pin);
	
#endif
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
		bDeviceState = CONFIGURED;
	else
		bDeviceState = ATTACHED;
}

/*******************************************************************************
* Function Name  : USB_Prepare
* Description    : Configures USB Clock input (48MHz) and USB interrupts
* Input          : None.
* Return         : None.
*******************************************************************************/
void USB_Prepare(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

#ifdef STM32F10X_CL
	/* Select USBCLK source */
	RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3);

	/* Enable the USB clock */ 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE) ;
#else 
	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  
	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
#endif /* STM32F10X_CL */

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

#ifdef STM32F10X_CL 
	/* Enable the USB Interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#else
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif /* STM32F10X_CL */

	/* Enable USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : USB_Cable_Config
* Description    : Software Connection/Disconnection of USB Cable
* Input          : None.
* Return         : Status
*******************************************************************************/
void USB_Cable_Config (FunctionalState NewState)
{
#ifdef USB_DISCONNECT_PIN
	if (NewState != DISABLE)
		GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	else
		GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
#else
	if (NewState != DISABLE)
		USB_DevConnect();
	else
		USB_DevDisconnect();
#endif
}

/*******************************************************************************
* Function Name  :  USARTx_Config_Default.
* Description    :  configure the USARTx with default values.
* Input          :  None.
* Return         :  None.
*******************************************************************************/
void USARTx_Init(USB_USART_Data_t * data)
{
	RCC_APB2PeriphClockCmd(data->RCC_GPIO,  ENABLE);

	if (data->RCC_APB1_USART != 0)
		RCC_APB1PeriphClockCmd(data->RCC_APB1_USART, ENABLE);
	if (data->RCC_APB2_USART != 0)
		RCC_APB2PeriphClockCmd(data->RCC_APB2_USART, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,   ENABLE);

	GPIO_Init(data->USART_GPIO, (GPIO_InitTypeDef *)data->TX_PIN);
	GPIO_Init(data->USART_GPIO, (GPIO_InitTypeDef *)data->RX_PIN);

    USART_Init(data->USART, &USART_InitStructure);
    USART_Cmd(data->USART, ENABLE);

  	/* Enable the USART Receive interrupt */
  	USART_ITConfig(data->USART, USART_IT_RXNE, ENABLE);
}

/* USART1_Config_Default */
void USARTx_Config_Default(USB_USART_Data_t * data)
{
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USARTx_Init(data);
}

/*******************************************************************************
* Function Name  :  USARTx_Config.
* Description    :  Configure the USARTx according to the linecoding structure.
* Input          :  None.
* Return         :  Configuration status
                    TRUE : configuration done with success
                    FALSE : configuration aborted.
*******************************************************************************/
bool USARTx_Config(USB_USART_Data_t * data)
{
	/* set the Stop bit*/
	switch (data->lineCoding.format)
	{
	case 0:
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		break;
	case 1:
		USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
		break;
	case 2:
		USART_InitStructure.USART_StopBits = USART_StopBits_2;
		break;
	default :
		USARTx_Config_Default(data);
		return (FALSE);
	}

	/* set the parity bit*/
	switch (data->lineCoding.paritytype)
	{
	case 0:
		USART_InitStructure.USART_Parity = USART_Parity_No;
		break;
	case 1:
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		break;
	case 2:
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		break;
	default :
		USARTx_Config_Default(data);
		return (FALSE);
	}

	/*set the data type : only 8bits and 9bits is supported */
	switch (data->lineCoding.datatype)
	{
	case 7:
		/* With this configuration a parity (Even or Odd) should be set */
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	case 8:
		if (USART_InitStructure.USART_Parity == USART_Parity_No)
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		else 
			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	default :
		USARTx_Config_Default(data);
		return (FALSE);
	}

	USART_InitStructure.USART_BaudRate = data->lineCoding.bitrate;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USARTx_Init(data);

	return (TRUE);
}

/*******************************************************************************
* Function Name  : USB_To_USARTx_Send_Data.
* Description    : send the received data from USB to the UART.
* Input          : data_buffer: data address.
                   Nb_bytes: number of bytes to send.
* Return         : none.
*******************************************************************************/
void USB_To_USARTx_Send_Data(USB_USART_Data_t * data)
{
	uint8_t * buffer = data->buffer_out;

	while (data->count_out != 0)
	{
		--data->count_out;
		USART_SendData(data->USART, *buffer++);
		while(USART_GetFlagStatus(data->USART, USART_FLAG_TXE) == RESET);
	}

#ifndef STM32F10X_CL
	SetEPRxValid(data->EP_OUT);
#endif
}

/*******************************************************************************
* Function Name  : UART_To_USB_Send_Data.
* Description    : send the received data from UART 0 to USB.
* Input          : None.
* Return         : none.
*******************************************************************************/
void USARTx_To_USB_Send_Data(USB_USART_Data_t * data)
{
	uint8_t ch;
	if (data->lineCoding.datatype == 7)
		ch = USART_ReceiveData(data->USART) & 0x7F;
	else
		ch = USART_ReceiveData(data->USART);
	
	data->buffer_in[data->count_in] = ch;
	data->count_in++;

	/* Write the data to the USB endpoint */
	USB_SIL_Write(data->EP_IN_ADDR, data->buffer_in, data->count_in);

#ifndef STM32F10X_CL
	SetEPTxValid(data->EP_IN);
#endif /* STM32F10X_CL */
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
	Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
	Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);

	if (Device_Serial0 != 0)
	{
		Virtual_Com_Port_StringSerial[2] = (uint8_t)(Device_Serial0  & 0x000000FF);
		Virtual_Com_Port_StringSerial[4] = (uint8_t)((Device_Serial0 & 0x0000FF00) >> 8);
		Virtual_Com_Port_StringSerial[6] = (uint8_t)((Device_Serial0 & 0x00FF0000) >> 16);
		Virtual_Com_Port_StringSerial[8] = (uint8_t)((Device_Serial0 & 0xFF000000) >> 24);

		Virtual_Com_Port_StringSerial[10] = (uint8_t)(Device_Serial1  & 0x000000FF);
		Virtual_Com_Port_StringSerial[12] = (uint8_t)((Device_Serial1 & 0x0000FF00) >> 8);
		Virtual_Com_Port_StringSerial[14] = (uint8_t)((Device_Serial1 & 0x00FF0000) >> 16);
		Virtual_Com_Port_StringSerial[16] = (uint8_t)((Device_Serial1 & 0xFF000000) >> 24);

		Virtual_Com_Port_StringSerial[18] = (uint8_t)(Device_Serial2  & 0x000000FF);
		Virtual_Com_Port_StringSerial[20] = (uint8_t)((Device_Serial2 & 0x0000FF00) >> 8);
		Virtual_Com_Port_StringSerial[22] = (uint8_t)((Device_Serial2 & 0x00FF0000) >> 16);
		Virtual_Com_Port_StringSerial[24] = (uint8_t)((Device_Serial2 & 0xFF000000) >> 24);
	}
}
