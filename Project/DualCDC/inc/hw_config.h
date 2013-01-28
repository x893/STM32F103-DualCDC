#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include <stdio.h>

#include "usb_type.h"
#include "usb_desc.h"
#include "usb_prop.h"

#define USB_DISCONNECT					GPIOC
#define USB_DISCONNECT_PIN				GPIO_Pin_12
#define RCC_APB2Periph_GPIO_DISCONNECT	RCC_APB2Periph_GPIOC

typedef struct __packed__
{
	__IO uint32_t count_out;
	__IO uint32_t count_in;

	uint8_t EP_OUT;
	uint8_t EP_OUT_ADDR;
	uint8_t EP_IN;
	uint8_t EP_IN_ADDR;

	USART_TypeDef * USART;
	GPIO_TypeDef* USART_GPIO;
	const GPIO_InitTypeDef * TX_PIN;
	const GPIO_InitTypeDef * RX_PIN;
	uint32_t RCC_GPIO;
	uint32_t RCC_APB1_USART;
	uint32_t RCC_APB2_USART;

	LINE_CODING lineCoding;

	uint8_t buffer_out [VIRTUAL_COM_PORT_DATA_SIZE];
	uint8_t buffer_in  [VIRTUAL_COM_PORT_DATA_SIZE];

} USB_USART_Data_t;


void Get_SerialNum(void);

void HardwareInit(void);

void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);

void USB_Prepare(void);
void USB_Cable_Config (FunctionalState NewState);

void USARTx_Config_Default(USB_USART_Data_t * data);
bool USARTx_Config(USB_USART_Data_t * data);

void USB_To_USARTx_Send_Data(USB_USART_Data_t * data);
void USARTx_To_USB_Send_Data(USB_USART_Data_t * data);

extern USB_USART_Data_t USB_USART1_Data;
extern USB_USART_Data_t USB_USART2_Data;

#endif  /*__HW_CONFIG_H*/
