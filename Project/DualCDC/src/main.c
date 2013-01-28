#include "stm32f10x.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"

int fputc(int ch, FILE *f)
{
    USART1->DR = (u8) ch;
    // Loop until the end of transmission
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

/*******************************************************************************
* Function Name  : main.
* Description    : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int main(void)
{
	HardwareInit();

	USB_Prepare();
	USB_Init();

	while (1)
	{
		if (bDeviceState == CONFIGURED)
		{
			if (USB_USART1_Data.count_out != 0)
				USB_To_USARTx_Send_Data(&USB_USART1_Data);

			if (USB_USART2_Data.count_out != 0)
				USB_To_USARTx_Send_Data(&USB_USART2_Data);
		}
	}
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif
