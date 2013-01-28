#include "usb_lib.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
	USB_USART1_Data.count_in = 0;
}

void EP4_IN_Callback(void)
{
	USB_USART2_Data.count_in = 0;
}

/*******************************************************************************
* Function Name  : EP3_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EPx_OUT_Callback(USB_USART_Data_t * data)
{
	/* Get the received data buffer and update the counter */
	data->count_out = USB_SIL_Read(data->EP_OUT_ADDR, data->buffer_out);
}

void EP3_OUT_Callback(void)
{
	EPx_OUT_Callback(&USB_USART1_Data);
}

void EP6_OUT_Callback(void)
{
	EPx_OUT_Callback(&USB_USART2_Data);
}
