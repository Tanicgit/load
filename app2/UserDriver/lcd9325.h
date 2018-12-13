#ifndef __LCD9325_H
#define __LCD9325_H

#include "stm32f4xx_hal.h"


#define 	USER_LCD_X_SIZE	240
#define   USER_LCD_Y_SIZE	320

#define LCD_BK_ON()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET)
#define LCD_BK_OFF()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET)


void lcd9325_Init(void);
void LCD9325_FillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t c);
void LCD9325_DrawPoint(uint16_t x,uint16_t y,uint16_t c);
uint16_t LCD9325_ReadPoint(uint16_t x,uint16_t y);





#endif
