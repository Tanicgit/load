#ifndef __IOIIC_H
#define __IOIIC_H

#include "stm32f4xx_hal.h"
#include "main.h"

#define IIC_SDA_0  LL_GPIO_ResetOutputPin(SDA_GPIO_Port, SDA_Pin)
#define IIC_SDA_1  LL_GPIO_SetOutputPin(SDA_GPIO_Port, SDA_Pin)

#define IIC_SCL_0  LL_GPIO_ResetOutputPin(SCL_GPIO_Port, SCL_Pin)
#define IIC_SCL_1  LL_GPIO_SetOutputPin(SCL_GPIO_Port, SCL_Pin)

#define SDA_IN() LL_GPIO_SetPinMode(SDA_GPIO_Port,SDA_Pin,GPIO_MODE_INPUT)
#define SDA_OUT() LL_GPIO_SetPinMode(SDA_GPIO_Port,SDA_Pin,GPIO_MODE_OUTPUT_PP)

#define SDA_DATA_IN_1 LL_GPIO_IsInputPinSet(SDA_GPIO_Port,SDA_Pin)
void IIC_Start(void);
void IIC_Stop(void);
uint8_t IIC_Wait_Ack(void);
void IIC_Ack(void);
void IIC_NAck(void);
void IIC_Send_Byte(uint8_t txd);
uint8_t IIC_Read_Byte(unsigned char ack);
#endif
