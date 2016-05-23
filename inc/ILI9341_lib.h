#ifndef ILI9341_LIB_H_INCLUDED
#define ILI9341_LIB_H_INCLUDED

#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

//#include "LcdFont.h"

#define LCD_task_PRIORITY					( tskIDLE_PRIORITY + 1 )
#define LCD_DataSenderTask_PRIORITY			( tskIDLE_PRIORITY + 2 )

#define LCD_QUEUE_LEN   3




#define LCD_DATA_COMMAND_Pin GPIO_PIN_2
#define LCD_DATA_COMMAND_GPIO_Port GPIOA

#define LCD_RESET_Pin GPIO_PIN_3
#define LCD_RESET_GPIO_Port GPIOA

#define LCD_LIGHT_Pin GPIO_PIN_8
#define LCD_LIGHT_GPIO_Port GPIOA

#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA

#define LCD_DC_SET      HAL_GPIO_WritePin(LCD_DATA_COMMAND_GPIO_Port,LCD_DATA_COMMAND_Pin,GPIO_PIN_SET)
#define LCD_DC_RESET    HAL_GPIO_WritePin(LCD_DATA_COMMAND_GPIO_Port,LCD_DATA_COMMAND_Pin,GPIO_PIN_RESET)

#define LCD_RST_SET      HAL_GPIO_WritePin(LCD_RESET_GPIO_Port,LCD_RESET_Pin,GPIO_PIN_SET)
#define LCD_RST_RESET    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port,LCD_RESET_Pin,GPIO_PIN_RESET)

#define LCD_CS_SET      HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_SET)
#define LCD_CS_RESET    HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_RESET)

#define WIDTH   320
#define HEIGHT  240

#define ILI9341_FLIP_X      0x01
#define ILI9341_FLIP_Y      0x02
#define ILI9341_SWITCH_XY   0x04
#define ILI9341_BGR         0x08

//Commands
#define ILI9341_RESET				0x01
#define ILI9341_SLEEP_OUT			0x11
#define ILI9341_INVERSION		    0x21
#define ILI9341_GAMMA				0x26
#define ILI9341_DISPLAY_OFF			0x28
#define ILI9341_DISPLAY_ON			0x29
#define ILI9341_COLUMN_ADDR			0x2A
#define ILI9341_PAGE_ADDR			0x2B
#define ILI9341_GRAM				0x2C
#define ILI9341_MAC			        0x36
#define ILI9341_PIXEL_FORMAT        0x3A
#define ILI9341_WDB			    	0x51
#define ILI9341_WCD				    0x53
#define ILI9341_RGB_INTERFACE       0xB0
#define ILI9341_FRC				    0xB1
#define ILI9341_BPC				    0xB5
#define ILI9341_DFC				    0xB6
#define ILI9341_POWER1				0xC0
#define ILI9341_POWER2				0xC1
#define ILI9341_VCOM1				0xC5
#define ILI9341_VCOM2				0xC7
#define ILI9341_POWERA				0xCB
#define ILI9341_POWERB				0xCF
#define ILI9341_PGAMMA				0xE0
#define ILI9341_NGAMMA				0xE1
#define ILI9341_DTCA				0xE8
#define ILI9341_DTCB				0xEA
#define ILI9341_POWER_SEQ			0xED
#define ILI9341_3GAMMA_EN			0xF2
#define ILI9341_INTERFACE_CTRL		0xF6
#define ILI9341_PRC				    0xF7

//Colors
/* some RGB color definitions                                                 */
#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */



typedef struct
{
    uint8_t command;
	uint16_t lenght;
	uint16_t * frame_array;
} lcd_frame;


void LCD_Init(SPI_HandleTypeDef *hspi);
void LCD_FillRect (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void LCD_DrawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
#endif /* ILI9341_LIB_H_INCLUDED */
