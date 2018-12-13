#ifndef __TOUCH_H
#define __TOUCH_H

#include "stm32f4xx_hal.h"
#include "main.h"
#include "gpio.h"
#define TOUCH_SAVE_ADDR_BASE 40
#define TOUCH_SAVE_SIZE 20
#define TDIN_0  LL_GPIO_ResetOutputPin(TDIN_GPIO_Port, TDIN_Pin)
#define TDIN_1  LL_GPIO_SetOutputPin(TDIN_GPIO_Port, TDIN_Pin)
#define TCLK_0  LL_GPIO_ResetOutputPin(TCLK_GPIO_Port, TCLK_Pin)
#define TCLK_1  LL_GPIO_SetOutputPin(TCLK_GPIO_Port, TCLK_Pin)
#define TCS_0  LL_GPIO_ResetOutputPin(TCS_GPIO_Port, TCS_Pin)
#define TCS_1  LL_GPIO_SetOutputPin(TCS_GPIO_Port, TCS_Pin)

//#define PE_DATA_IN_1 LL_GPIO_IsInputPinSet(PEN_GPIO_Port,PEN_Pin)

#define PE_DATA_IN_1 HAL_GPIO_ReadPin(PEN_GPIO_Port,PEN_Pin)

#define DOUT_DATA_IN_1 LL_GPIO_IsInputPinSet(DOUT_GPIO_Port,DOUT_Pin)


typedef struct{
	uint16_t Log;
	uint16_t Phy;
}_LogPhy;

extern _LogPhy px0;
extern _LogPhy px1;
extern _LogPhy py0;
extern _LogPhy py1;

uint16_t TP_Read_XOY(uint8_t xy);

uint16_t TP_Read_AD(uint8_t CMD)	;
void TP_Save_Adjdata(void);
void ClearAdjdata(void);
uint8_t TP_Get_Adjdata(void);
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y) ;
#endif
