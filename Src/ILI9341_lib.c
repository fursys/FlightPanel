#include "ILI9341_lib.h"


SPI_HandleTypeDef * LCD_hspi;
xQueueHandle LCD_queue;
xSemaphoreHandle LCD_send_Semaphore;

void LCD_Send (uint8_t cmd, uint8_t * data, uint32_t len);
static void LCD_task( void *pvParameters );
void LCD_HW_Init (void);
//------------------------------------------------------------------------------------------------

void LCD_SetCursorPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data [30];


    data [0] = (x1 >> 8);
    data [1] = (x1 & 0xFF);
    data [2] = (x2 >> 8);
    data [3] = (x2 & 0xFF);
    LCD_Send (ILI9341_COLUMN_ADDR,data,4);

    data [0] = (y1 >> 8);
    data [1] = (y1 & 0xFF);
    data [2] = (y2 >> 8);
    data [3] = (y2 & 0xFF);
    LCD_Send (ILI9341_PAGE_ADDR,data,4);
}
//------------------------------------------------------------------------------------------------
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t data [30];
    LCD_SetCursorPosition(x, y, x, y);

    data [0] = (color >> 8);
    data [1] = (color & 0xFF);
    LCD_Send (ILI9341_GRAM,data,2);
}
//------------------------------------------------------------------------------------------------
void LCD_DrawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint8_t data [2];
    uint16_t i = 0;
    uint16_t j = 0;
    xSemaphoreTake( LCD_send_Semaphore, portMAX_DELAY);
    for(i=0;i<(y1-y0);i++)
    {
        LCD_SetCursorPosition(x0, y0 + i, x1, y0 + i+1);
        LCD_Send (ILI9341_GRAM,0,0);
        for(j=0;j<(x1-x0);j++)
        {
            data[0] = color >> 8;
            data[1] = color & 0xFF;
            LCD_Send (0, data,2);
        }
    }
    xSemaphoreGive (LCD_send_Semaphore);
}
void LCD_FillRect (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    lcd_frame line;
    // rudimentary clipping (drawChar w/big text requires this)
	if((x >= WIDTH) || (y >= HEIGHT)) return;
	if((x + w - 1) >= WIDTH)  w = WIDTH  - x;
	if((y + h - 1) >= HEIGHT) h = HEIGHT - y;
    xSemaphoreTake( LCD_send_Semaphore, portMAX_DELAY);
	LCD_SetCursorPosition(x, y, x+w-1, y+h-1);
    xSemaphoreGive (LCD_send_Semaphore);

    line.command = ILI9341_GRAM;
    //LCD_Send(ILI9341_GRAM, 0, 0);
    LCD_hspi->Instance->CR1 &= ~SPI_CR1_SPE;
	for(y=0; y<h; y++)
	{

        line.lenght = w;
        line.frame_array = pvPortMalloc(w*2);

		for(x=0; x<w; x++)
        {
			line.frame_array[x] = color;
		}
		xQueueSendToBack (LCD_queue, &line, portMAX_DELAY);
		line.command = 0;
	}
}
//------------------------------------------------------------------------------------------------
static void LCD_DataSenderTask( void *pvParameters )
{
    lcd_frame dataframe;

    xSemaphoreTake( LCD_send_Semaphore, portMAX_DELAY);
    LCD_HW_Init();
    xSemaphoreGive (LCD_send_Semaphore);

    while (1)
    {

        xQueueReceive (LCD_queue, &dataframe, portMAX_DELAY);
        LCD_CS_RESET;
        if (dataframe.command)
        {
            xSemaphoreTake( LCD_send_Semaphore, portMAX_DELAY);
            LCD_DC_RESET;
            HAL_SPI_Transmit (LCD_hspi, &dataframe.command, 1, 1000);
            xSemaphoreGive (LCD_send_Semaphore);
        }
        if (dataframe.lenght > 5)
        {
            LCD_DC_SET;
            //send using DMA
            xSemaphoreTake( LCD_send_Semaphore, portMAX_DELAY);
            LCD_hspi->Instance->CR1 |= SPI_CR1_DFF;
            HAL_SPI_Transmit_DMA(LCD_hspi, (uint8_t*) dataframe.frame_array, dataframe.lenght);
            xSemaphoreTake( LCD_send_Semaphore, portMAX_DELAY);
            vPortFree(dataframe.frame_array);
            xSemaphoreGive (LCD_send_Semaphore);
        }
        else if (dataframe.lenght > 0)
        {
            xSemaphoreTake( LCD_send_Semaphore, portMAX_DELAY);
            LCD_DC_SET;
            LCD_hspi->Instance->CR1 |= SPI_CR1_DFF;
            HAL_SPI_Transmit (LCD_hspi, (uint8_t*) dataframe.frame_array, dataframe.lenght, 1000);
            vPortFree(dataframe.frame_array);
            xSemaphoreGive (LCD_send_Semaphore);
        }
        LCD_CS_SET;


    }
}
//------------------------------------------------------------------------------------------------

void LCD_Send (uint8_t cmd, uint8_t * data, uint32_t len)
{

    LCD_CS_RESET;
    if (cmd)
    {
        LCD_DC_RESET;
        HAL_SPI_Transmit (LCD_hspi, &cmd, 1, 1000);
    }
    if ((data != 0) && (len > 0))
    {
        LCD_DC_SET;
        HAL_SPI_Transmit (LCD_hspi, data, len, 1000);
    }
    LCD_CS_SET;
}
//------------------------------------------------------------------------------------------------
void LCD_Init(SPI_HandleTypeDef *hspi)
{

    LCD_hspi = hspi;
    LCD_queue = xQueueCreate (LCD_QUEUE_LEN,sizeof (lcd_frame));
    vSemaphoreCreateBinary(LCD_send_Semaphore);

    xTaskCreate( LCD_DataSenderTask,"LCD_DataSenderTask", configMINIMAL_STACK_SIZE, NULL, LCD_DataSenderTask_PRIORITY, NULL );
    //xTaskCreate( LCD_task,"LCD_task", configMINIMAL_STACK_SIZE, NULL, LCD_task_PRIORITY, NULL );
}

//------------------------------------------------------------------------------------------------
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    LCD_hspi->Instance->CR1 &= ~SPI_CR1_SPE;
    LCD_hspi->Instance->CR1 &= ~(SPI_CR1_DFF);
    xSemaphoreGive (LCD_send_Semaphore);
}
//------------------------------------------------------------------------------------------------
static void LCD_task( void *pvParameters )
{

    while (1)
    {

        HAL_Delay(100);
    }

}
//------------------------------------------------------------------------------------------------
void LCD_SetOrientation (uint8_t flags)
{
    uint8_t madctl = 0x48;

    uint8_t data [5];

    if (flags & ILI9341_FLIP_X)
    {
        madctl &= ~(1 << 6);
    }

    if (flags & ILI9341_FLIP_Y)
    {
        madctl |= 1 << 7;
    }

    if (flags & ILI9341_SWITCH_XY)
    {
        madctl |= 1 << 5;
    }

    data [0] = (madctl);
    LCD_Send (ILI9341_MAC,data,1);
}
//------------------------------------------------------------------------------------------------

void LCD_HW_Init (void)
{
    uint8_t data [30];

    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __GPIOA_CLK_ENABLE();

    /*Configure GPIO pins : LCD_DATA_COMMAND_Pin LCD_RESET_Pin */
    GPIO_InitStruct.Pin = LCD_DATA_COMMAND_Pin|LCD_RESET_Pin|LCD_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    LCD_CS_SET;

    //LCD_TestSPI();


    // LCD reset

    LCD_RST_SET;
    HAL_Delay(10);
    LCD_RST_RESET;
    HAL_Delay(10);
    LCD_RST_SET;
    HAL_Delay(150);

    LCD_Send (ILI9341_RESET,0,0);
    HAL_Delay(100);


    LCD_Send (ILI9341_SLEEP_OUT,0,0);

    HAL_Delay(100);

    LCD_Send (ILI9341_DISPLAY_ON,0,0);
    HAL_Delay(100);

 // LCD setup
    data [0] = (0x39);
    data [1] = (0x2C);
    data [2] = (0x00);
    data [3] = (0x34);
    data [4] = (0x02);
    LCD_Send (ILI9341_POWERA,data,5);

    data [0] = (0x00);
    data [1] = (0xAA);
    data [2] = (0xB0);
    LCD_Send (ILI9341_POWERB,data,3);

    data [0] = (0x30);
    LCD_Send (ILI9341_PRC,data,1); //ILI9341_CMD_PUMP_RATIO_CONTROL

    data [0] = (0x25);
    LCD_Send (ILI9341_POWER1,data,1); //ILI9341_CMD_POWER_CONTROL_1

    data [0] = (0x11);
    LCD_Send (ILI9341_POWER2,data,1); //ILI9341_CMD_POWER_CONTROL_2

    data [0] = (0x5C);
    data [1] = (0x4C);
    LCD_Send (ILI9341_VCOM1,data,2); //ILI9341_CMD_VCOM_CONTROL_1

    data [0] = (0x94);
    LCD_Send (ILI9341_VCOM2,data,1); //ILI9341_CMD_VCOM_CONTROL_2

    data [0] = (0x85);
    data [1] = (0x01);
    data [2] = (0x78);
    LCD_Send (ILI9341_DTCA,data,3); //ILI9341_CMD_DRIVER_TIMING_CONTROL_A


    data [0] = (0x00);
    data [1] = (0x00);
    LCD_Send (ILI9341_DTCB,data,2); //ILI9341_CMD_DRIVER_TIMING_CONTROL_B

    data [0] = (0x55);
    LCD_Send (ILI9341_PIXEL_FORMAT,data,1); //ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET


    LCD_SetOrientation(ILI9341_SWITCH_XY);



    data [0] = (0x00);
    data [1] = (0x00);
    data [2] = (0x00);
    data [3] = (0xEF);
    LCD_Send (ILI9341_COLUMN_ADDR,data,4);

    data [0] = (0x00);
    data [1] = (0x00);
    data [2] = (0x01);
    data [3] = (0x3F);
    LCD_Send (ILI9341_PAGE_ADDR,data,4);

    data [0] = (0x01);
    LCD_Send (ILI9341_GAMMA,data,1);

    data [0] = (0x0F);
    data [1] = (0x31);
    data [2] = (0x2B);
    data [3] = (0x0C);
    data [4] = (0x0E);
    data [5] = (0x08);
    data [6] = (0x4E);
    data [7] = (0xF1);
    data [8] = (0x37);
    data [9] = (0x07);
    data [10] = (0x10);
    data [11] = (0x03);
    data [12] = (0x0E);
    data [13] = (0x09);
    data [14] = (0x00);
    LCD_Send (ILI9341_PGAMMA,data,15);

    data [0] = (0x00);
    data [1] = (0x0E);
    data [2] = (0x14);
    data [3] = (0x03);
    data [4] = (0x11);
    data [5] = (0x07);
    data [6] = (0x31);
    data [7] = (0xC1);
    data [8] = (0x48);
    data [9] = (0x08);
    data [10] = (0x0F);
    data [11] = (0x0C);
    data [12] = (0x31);
    data [13] = (0x36);
    data [14] = (0x0F);
    LCD_Send (ILI9341_NGAMMA,data,15);

    //LCD_Send (ILI9341_INVERSION,0,0);


}
