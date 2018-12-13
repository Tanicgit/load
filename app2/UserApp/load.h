#ifndef __LOAD_H
#define __LOAD_H
#include "stm32f4xx_hal.h"

#pragma  pack(1) 
typedef struct{
	uint8_t  sta;//0没有升级 2下载包中  3合成包中 4升级文件准备 5app通知boot网络升级 
							//6boot通知app网络升级成功   7app通知boot恢复出场固件  8 boot通知app恢复出场固件成功
	uint8_t  nowBin;//当前运行的bin文件  1.bin  2.bin  上点启动 如果sta==3 则切换固件并修改本位
	uint16_t	nowVersion;//当前运行的版本 
	uint16_t	VersionToLoad;//准备下载的版本
	uint8_t	 bootErrCnt;//新版本已经boot的失败次数 超过一定次数sta置为0 boot代码中+1
	uint32_t FileSize;
	uint16_t FilePackNum;
	uint16_t PackSize;
	uint16_t lastPackSize;
	uint8_t  packListSum;//pack 表文件的校验和
	uint16_t  crc16;
	uint8_t  sum;//本结构体文件校验和
}_LOAD;

#pragma  pack() 
struct listPack{
	uint8_t id;//顺序
	uint8_t sta;//状态0未下载 已下载
	uint8_t checkSum;//该包bin的校验和
	uint16_t size;//该包的大小
	struct listPack *next;
};
extern struct listPack *packHead;
uint8_t NeedReLoad(void);
uint8_t savePackInfo(void);
uint8_t readPackInfo(void);
uint8_t saveLoadInfo(void);
void readLoadInfo(void);
void returnDefaultBin(void);
void loadTask(void);
void loadInit(void);
extern _LOAD load;
#endif 
