#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"



//------------------------------------------------------------------------------------------------
/* Allocate two blocks of RAM for use by the heap.  The first is a block of 0x10000
bytes starting from address 0x80000000, and the second a block of 0xa0000 bytes
starting from address 0x90000000.  The block starting at 0x80000000 has the lower
start address so appears in the array fist. */
/*
Адрес начала блока памяти должен быть больше суммы
Data size и BSS size
которые выводятся линкером после сборки.
Нужно следить чтобы размер не перекрывал стек.
*/
const HeapRegion_t xHeapRegions[] =
{
    { ( uint8_t * ) 0x20000800UL, configTOTAL_HEAP_SIZE },
    { NULL, 0 } /* Terminates the array. */
};


//------------------------------------------------------------------------------------------------
#define Blink1_PRIORITY					( tskIDLE_PRIORITY + 1 )
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
static void Blink1( void *pvParameters )
{
    GPIO_InitTypeDef GPIO_InitStruct;

  	//volatile int State=0;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GREEN_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GREEN_LED_GPIO_Port, &GPIO_InitStruct);

    //RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    //GPIOB->CRH      |= GPIO_CRH_MODE12_1;//Output mode, max speed 2 MHz.
    //GPIOB->CRH      &= ~GPIO_CRH_CNF12;

	while( 1 )
    {
        HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port,GREEN_LED_Pin);
        HAL_Delay (100);
		//vTaskDelay( 100/ portTICK_RATE_MS );
	}
}
//------------------------------------------------------------------------------------------------
uint32_t HAL_GetTick(void)
{
    return xTaskGetTickCount();
}
void HAL_Delay(__IO uint32_t Delay)
{
    TickType_t ticks = Delay / portTICK_PERIOD_MS;
    vTaskDelay(ticks ? ticks : 1);          /* Minimum delay = 1 tick */
}

//------------------------------------------------------------------------------------------------
/** System Clock Configuration
*/
void SystemClock_Config(void)
{
    //12 Mhz oscillator !!!!


  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

}
//------------------------------------------------------------------------------------------------

int main(void)
{



    /* HEAP initialization.
    Pass the array into vPortDefineHeapRegions(). */
    vPortDefineHeapRegions( xHeapRegions );

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();


	xTaskCreate( Blink1,"Blink1", configMINIMAL_STACK_SIZE, NULL, Blink1_PRIORITY, NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();
}
