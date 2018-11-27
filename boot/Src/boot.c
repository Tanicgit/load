#include "boot.h"
#include "fatfs.h"
#include "ff.h"
#include "string.h"
#include "flash.h"
#pragma  pack(1) 
typedef struct{
	uint8_t  sta;//0 无 0x01 有新的下载   0x02 正在下载 0x03下载完成
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

_LOAD load;

uint8_t U8checkSum(uint8_t *a,uint16_t len)
{
	uint8_t sum=0;
	for(uint32_t i=0;i<len;i++)
	{
		sum += a[i];
	}
	return sum;
}

uint8_t saveLoadInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t byteswritten, bytestowrite; 
	
	sprintf(filename,"%sload",SDPath);
	if((res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
		return 1;
	}
	
	bytestowrite = sizeof(_LOAD);
	load.sum = U8checkSum((uint8_t*)&load,bytestowrite-1);
	res = f_write(&SDFile, &load, bytestowrite, (void *)&byteswritten);
	f_close(&SDFile);
	if( res != FR_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
		return 1;
	}
	else if((byteswritten != bytestowrite))
	{
		_Error_Handler(__FILE__, __LINE__);
		return 1;
	}
	return 0;
}
/*读取下载头信息*/
uint8_t readLoadInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t bytesread, bytestoread; 
	uint8_t re=0;
	sprintf(filename,"%sload",SDPath);
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		re = 0XFF;
	}
	else
	{
		bytestoread = sizeof(_LOAD);
		res = f_read(&SDFile, &load, bytestoread, (void *)&bytesread);
		f_close(&SDFile);
		if(res != FR_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
			re = 1;
		}
		else if((bytesread != bytestoread))	
		{
			re = 0xff;
		}			
		else if(load.sum!=U8checkSum((uint8_t*)&load,bytestoread-1))
		{			
			re = 0xff;
		}
		else
		{
			if(load.FileSize!= load.lastPackSize + (load.FilePackNum-1)*load.PackSize)
			{
				re = 0xff;
			}
			
		}
	}
	_Log("load.sta=%d",load.sta);
	_Log("load.nowBin=%d",load.nowBin);
	_Log("load.nowVersion=%d",load.nowVersion);
	_Log("load.VersionToLoad=%d",load.VersionToLoad);
	_Log("load.FileSize=%d",load.FileSize);
	_Log("load.FilePackNum=%d",load.FilePackNum);
	_Log("load.PackSize=%d",load.PackSize);
	return re;
}

uint8_t rxfileBuf[512];
/*
-2 flash写完成  但是成功标志写失败
-1 boot 错误 已经更新部分内部flash后出错
0 OK    升级成功
1 不需要升级 没有升级 
2 最新的bin文件打开失败没有进行内部flash更新
3 load文件读取错误 升级未进行
*/
int8_t boot()
{
	int8_t err=0;
	int32_t size=0;
	uint32_t addr = ADDR_FLASH_SECTOR_2;
	uint32_t rw=0;
	char filename[16];
	FRESULT res;
	err = readLoadInfo();
	if(err==0)
	{
		sprintf(filename,"%shandup.txt",SDPath);
		res = f_open(&SDFile, filename, FA_READ);
		if(res==FR_OK) f_close(&SDFile);
		if(load.sta==5||res==FR_OK)
		{
			
			if(load.nowBin==0)
			{
				sprintf(filename,"%s0.bin",SDPath);
			}
			else
			{
				sprintf(filename,"%s1.bin",SDPath);
			}
			res=f_open(&SDFile, filename, FA_READ);
			if(res==FR_OK)
			{
				HAL_FLASH_Unlock();
				EraseSpace(load.FileSize);
				size = f_size(&SDFile);
				while(size>0)
				{
					res = f_read(&SDFile, rxfileBuf, 512, (void *)&rw);
					if(res==FR_OK&&rw!=0)
					{
						if(0!=Flash_If_Write(rxfileBuf,addr,rw))
						{
							err = -1;
							break;
						}
						addr += rw;
						size -= rw;
					}
					else 
					{
						err = -1;
						break;
					}
				}
				HAL_FLASH_Lock();
				if(err==0&&load.sta==5)
				{
					if(load.sta==5)
					{
						load.sta = 6;
						load.bootErrCnt = 0;
						if(0!=saveLoadInfo()) err = -2;
					}
					else
					{
						sprintf(filename,"%shandup.txt",SDPath);//手动修改SD卡文件升级
						f_unlink(filename);
						load.nowBin=0;
						load.sta=0;			
						if(0!=saveLoadInfo()) err = -2;
					}
				}
			}
			else
			{
				err = 2;
			}
		}
		else if(load.sta==0)
		{
			err = 0;
		}
		else
		{
			err = 1;
		}
	}
	else
	{
		err = 3;
	}
	
	if(err>0)
	{
		load.sta = 5;
		if(load.bootErrCnt<5)load.bootErrCnt++;
		else load.sta=0;
		if(0!=saveLoadInfo())err = -2;	
	}
	else if(err<0)
	{
		HAL_Delay(10000);
	}
	return err;
}








