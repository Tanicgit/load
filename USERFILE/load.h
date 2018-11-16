#ifndef __LOAD_H
#define __LOAD_H


#pragma  pack(1) 
typedef struct{
	uint8_t  sta;//0 无 0x01 有新的下载   0x02 正在下载
	
	uint32_t FileSize;
	uint16_t FilePackNum;
	uint16_t PackSize;
	uint8_t  packListSum;
	uint8_t  sum;
}_LOAD;
#pragma  pack() 

uint8_t NeedReLoad(void);
uint8_t savePackInfo(void);
uint8_t readPackInfo(void);
uint8_t saveLoadInfo(void);
uint8_t readLoadInfo(void);
void loadTask(void);
void loadInit(void);
extern _LOAD load;
#endif 