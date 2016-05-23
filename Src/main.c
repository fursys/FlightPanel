#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "ILI9341_lib.h"



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
    { ( uint8_t * ) 0x20000C00UL, configTOTAL_HEAP_SIZE },
    { NULL, 0 } /* Terminates the array. */
};


//------------------------------------------------------------------------------------------------
#define Blink1_PRIORITY					( tskIDLE_PRIORITY + 1 )
//------------------------------------------------------------------------------------------------
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;


static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
//static void MX_SPI2_Init(void);
//static void MX_TIM1_Init(void);
//static void MX_USART1_UART_Init(void);

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
    LCD_FillRect(0, 0, 320, 50, Red);
    LCD_FillRect(0, 50, 320, 50, Green);
    LCD_FillRect(0, 100, 320, 50, Blue);
    LCD_FillRect(0, 150, 320, 50, Cyan);
    LCD_FillRect(0, 200, 320, 40, Magenta);

    //LCD_DrawFilledRectangle (50, 0, 80, 310, 0xFFFF);
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
    MX_DMA_Init();
    MX_SPI1_Init();

    LCD_Init(&hspi1);

	xTaskCreate( Blink1,"Blink1", configMINIMAL_STACK_SIZE, NULL, Blink1_PRIORITY, NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();
}
//------------------------------------------------------------------------------------------------
/* SPI1 init function */
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi1.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&hspi1);

}
//------------------------------------------------------------------------------------------------
/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}
