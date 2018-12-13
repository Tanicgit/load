#include "lcd9325.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#define LCD_REG	*(__IO uint16_t*)(0x6C000000)
#define LCD_RAM	*(__IO uint16_t*)(0x6C000080)

void write_reg(uint16_t val)
{
	val=val;
	LCD_REG = val;
}

void write_data(uint16_t data)
{
	data=data;
	LCD_RAM = data;
}

uint16_t read_data()
{
	__IO uint16_t re;
	re = LCD_RAM;
	return re;
}

void LCD9325_WriteReg(uint16_t reg,uint16_t val)
{
	LCD_REG = reg;
	LCD_RAM = val;
}

uint16_t LCD9325_ReadReg(uint16_t reg)
{
	write_reg(reg);
	delay_us(5);
	return read_data();
}
static void Delay(uint8_t i)
{
	while(i--);
}

uint16_t LCD9325_ReadPoint(uint16_t x,uint16_t y)
{
	uint16_t color;
	LCD9325_WriteReg(0x20,x);
	LCD9325_WriteReg(0x21,y);
	color = LCD9325_ReadReg(0x22);
	Delay(2);
	color = LCD9325_ReadReg(0x22);
	return color;
}

void LCD9325_DrawPoint(uint16_t x,uint16_t y,uint16_t c)
{
	LCD9325_WriteReg(0x20,x);
	LCD9325_WriteReg(0x21,y);

	LCD_REG=0x22;
	LCD_RAM=c;

}

void LCD9325_SetWindows(uint16_t x,uint16_t y,uint16_t w,uint16_t h)
{
	LCD9325_WriteReg(0x50,x);
	LCD9325_WriteReg(0x51,y);
	LCD9325_WriteReg(0x52,x+w-1);
	LCD9325_WriteReg(0x53,y+h-1);
}

void LCD9325_DisplayOn()
{
	LCD9325_WriteReg(0x07,0x0173);
}

void LCD9325_DisplayOff()
{
	LCD9325_WriteReg(0x07,0x0);
}

void LCD9325_FillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t c)
{
	for(uint16_t h_i=0;h_i<h;h_i++)
	{
		LCD9325_WriteReg(0x20,x);
		LCD9325_WriteReg(0x21,y+h_i);
		LCD_REG=0x22;
		for(uint16_t w_i=0;w_i<w;w_i++)
		{
			LCD_RAM = c;
		}	
	}
}

void lcd9325_Init()
{
	uint16_t id=0;
	HAL_Delay(50);
	LCD9325_WriteReg(0x00,0x01);
	id = LCD9325_ReadReg(0);
	if(id!=0x9325)
	{
		while(1)
		{
			osDelay(200);
		}
	}
	LCD9325_WriteReg(0x00E5,0x78F0); 
	LCD9325_WriteReg(0x0001,0x0100); 
	LCD9325_WriteReg(0x0002,0x0700); 
	LCD9325_WriteReg(0x0003,0x1030); 
	LCD9325_WriteReg(0x0004,0x0000); 
	LCD9325_WriteReg(0x0008,0x0202);  
	LCD9325_WriteReg(0x0009,0x0000);
	LCD9325_WriteReg(0x000A,0x0000); 
	LCD9325_WriteReg(0x000C,0x0000); 
	LCD9325_WriteReg(0x000D,0x0000);
	LCD9325_WriteReg(0x000F,0x0000);
	//power on sequence VGHVGL
	LCD9325_WriteReg(0x0010,0x0000);   
	LCD9325_WriteReg(0x0011,0x0007);  
	LCD9325_WriteReg(0x0012,0x0000);  
	LCD9325_WriteReg(0x0013,0x0000); 
	LCD9325_WriteReg(0x0007,0x0000); 
	//vgh 
	LCD9325_WriteReg(0x0010,0x1690);   
	LCD9325_WriteReg(0x0011,0x0227);
	//delayms(100);
	//vregiout 
	LCD9325_WriteReg(0x0012,0x009D); //0x001b
	//delayms(100); 
	//vom amplitude
	LCD9325_WriteReg(0x0013,0x1900);
	//delayms(100); 
	//vom H
	LCD9325_WriteReg(0x0029,0x0025); 
	LCD9325_WriteReg(0x002B,0x000D); 
	//gamma
	LCD9325_WriteReg(0x0030,0x0007);
	LCD9325_WriteReg(0x0031,0x0303);
	LCD9325_WriteReg(0x0032,0x0003);// 0006
	LCD9325_WriteReg(0x0035,0x0206);
	LCD9325_WriteReg(0x0036,0x0008);
	LCD9325_WriteReg(0x0037,0x0406); 
	LCD9325_WriteReg(0x0038,0x0304);//0200
	LCD9325_WriteReg(0x0039,0x0007); 
	LCD9325_WriteReg(0x003C,0x0602);// 0504
	LCD9325_WriteReg(0x003D,0x0008); 
	//ram
	LCD9325_WriteReg(0x0050,0x0000); 
	LCD9325_WriteReg(0x0051,0x00EF);
	LCD9325_WriteReg(0x0052,0x0000); 
	LCD9325_WriteReg(0x0053,0x013F);  
	LCD9325_WriteReg(0x0060,0xA700); 
	LCD9325_WriteReg(0x0061,0x0001); 
	LCD9325_WriteReg(0x006A,0x0000); 
	//
	LCD9325_WriteReg(0x0080,0x0000); 
	LCD9325_WriteReg(0x0081,0x0000); 
	LCD9325_WriteReg(0x0082,0x0000); 
	LCD9325_WriteReg(0x0083,0x0000); 
	LCD9325_WriteReg(0x0084,0x0000); 
	LCD9325_WriteReg(0x0085,0x0000); 
	//
	LCD9325_WriteReg(0x0090,0x0010); 
	LCD9325_WriteReg(0x0092,0x0600); 
	
	LCD9325_WriteReg(0x0007,0x0133);
	LCD9325_WriteReg(0x00,0x0022);//

	LCD_BK_ON();
	
}






