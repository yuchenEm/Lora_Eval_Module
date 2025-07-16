/* Includes ------------------------------------------------------------------*/
#include "stm8l10x.h"
#include "hal_tim4.h"
#include "hal_usart.h"
#include "hal_spi.h"
#include "mid_sx1278.h"
#include "mid_usart.h"
#include "hal_sleep.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


void main(void)
{
    
    CLK_MasterPrescalerConfig(CLK_MasterPrescaler_HSIDiv1);
    
    HAL_TIM4_Config();
    //HAL_USART_Config();
    hal_spi_Config();
    mid_sx1278_Rest(); 
    mid_SX1278_LoRaRxConfig();
    mid_usart_init();
    
    Systick = 0;
     
    while (1)
    {
        if(Systick == 1)
        {
            Systick = 0;
            HAL_USART_Pro();
            mid_usart_Pro();
            hal_sleep_task();
        }
    }

}

INTERRUPT_HANDLER(EXTI3_IRQHandler, 11)
{
    EXTI_ClearITPendingBit (EXTI_IT_Pin3);
    mid_Sx1278RxDateGet();
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/