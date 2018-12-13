#include "boot.h"
#include "fatfs.h"
#include "ff.h"
#include "string.h"
#include "flash.h"
#pragma  pack(1) 
typedef struct{
	uint8_t  sta;//0û������ 2���ذ���  3�ϳɰ��� 4�����ļ�׼�� 5app֪ͨboot�������� 
					//		6boot֪ͨapp���������ɹ�   7app֪ͨboot�ָ������̼�  8 boot֪ͨapp�ָ������̼��ɹ�
	uint8_t  nowBin;//��ǰ���е�bin�ļ�  1.bin  2.bin  �ϵ����� ���sta==3 ���л��̼����޸ı�λ
	uint16_t	nowVersion;//��ǰ���еİ汾
	uint16_t	VersionToLoad;//׼�����صİ汾
	uint8_t	 bootErrCnt;//�°汾�Ѿ�boot��ʧ�ܴ��� ����һ������sta��Ϊ0 boot������+1
	uint32_t FileSize;
	uint16_t FilePackNum;
	uint16_t PackSize;
	uint16_t lastPackSize;
	uint8_t  packListSum;//pack ���ļ���У���
	uint16_t  crc16;
	uint8_t  sum;//���ṹ���ļ�У���
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
/*��ȡ����ͷ��Ϣ*/
void readLoadInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t bytesread, bytestoread; 
	uint8_t re=0;
	
	sprintf(filename,"%spack",SDPath);
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		if(res==4)f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
	}
	f_close(&SDFile);
	sprintf(filename,"%sload",SDPath);
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		if(res==4)f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
	}
	f_close(&SDFile);
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		_Log("err=%d",res);
		_Error_Handler(__FILE__, __LINE__);
	}
	else
	{
		bytestoread = sizeof(_LOAD);
		res = f_read(&SDFile, &load, bytestoread, (void *)&bytesread);
		f_close(&SDFile);
		if(res != FR_OK)
		{
			_Log("err=%d",res);
			_Error_Handler(__FILE__, __LINE__);
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
	if(re==0xff)//load У����� ��0
	{
		_Log("load err,clear 0");
		memset(&load,0,sizeof(_LOAD));
		saveLoadInfo();
	}
	_Log("load.sta=%d",load.sta);
	_Log("load.nowBin=%d",load.nowBin);
	_Log("load.nowVersion=%d",load.nowVersion);
	_Log("load.VersionToLoad=%d",load.VersionToLoad);
	_Log("load.FileSize=%d",load.FileSize);
	_Log("load.FilePackNum=%d",load.FilePackNum);
	_Log("load.PackSize=%d",load.PackSize);
}

uint8_t rxfileBuf[512];
/*
*/
void boot()
{
	uint8_t err=0;
	int32_t size=0;
	uint32_t addr = ADDR_FLASH_SECTOR_2;
	uint32_t rw=0;
	uint8_t  updata_type=0;
	char filename[16];
	FRESULT res;
	readLoadInfo();
	
	sprintf(filename,"%sdefault.bin",SDPath);
	if(FR_OK!=f_open(&SDFile, filename, FA_READ))
	{
		_Log("have no default.bin");
		_Error_Handler(__FILE__, __LINE__);	
	}
	f_close(&SDFile);
	
	sprintf(filename,"%shandup.txt",SDPath);
	res = f_open(&SDFile, filename, FA_READ);
	if(res==FR_OK) 
	{
		updata_type=1;
		f_close(&SDFile);
		sprintf(filename,"%sdefault.bin",SDPath);
	}
	else if(load.sta==5)
	{
		updata_type=2;
		if(load.nowBin==0)
		{
			sprintf(filename,"%s0.bin",SDPath);
		}
		else
		{
			sprintf(filename,"%s1.bin",SDPath);
		}	
		
		if(FR_OK!=f_open(&SDFile, filename, FA_READ))
		{
			sprintf(filename,"%sdefault.bin",SDPath);
			updata_type=4;
		}
		f_close(&SDFile);
	}
	else if(load.sta==7)//ǿ�ƻָ�����Ĭ�Ϲ̼�
	{
		updata_type=3;
		sprintf(filename,"%sdefault.bin",SDPath);
	}
	if(updata_type)
	{ 
		/*copy bin*/
		res=f_open(&SDFile, filename, FA_READ);
		if(res!=FR_OK)
		{				
			_Log("err=%d",res);
			_Error_Handler(__FILE__, __LINE__);		
		}
		HAL_FLASH_Unlock();			
		size = f_size(&SDFile);
		EraseSpace(size);
		while(size>0){
			res = f_read(&SDFile, rxfileBuf, 512, (void *)&rw);
			if(res==FR_OK&&rw!=0){
				if(0!=Flash_If_Write(rxfileBuf,addr,rw)){
					err = 1;
					break;
				}
				addr += rw;
				size -= rw;
			}
			else{
				err = 1;
				break;
			}
		}
		HAL_FLASH_Lock();
		f_close(&SDFile);/////////
	}
	/*copy bin end*/
	if(err==0)
	{
		if(updata_type==1||updata_type==4)
		{
			sprintf(filename,"%shandup.txt",SDPath);
			f_unlink(filename);
			load.sta=0;
			load.nowVersion=0;
			load.nowBin=0xff;
			saveLoadInfo();
		}
		else if(updata_type==2)
		{
			load.sta = 6;
			load.nowVersion=load.VersionToLoad;
			saveLoadInfo();
		}
		else if(updata_type==3)
		{
			load.sta = 8;	
			load.nowVersion=0;
			load.nowBin=0xff;
			saveLoadInfo();
		}
		else
		{
			//δ��������
		}	
	}
	else//�����ڲ�flash��������
	{
		if(load.bootErrCnt<3)
		{
			load.bootErrCnt++;
			saveLoadInfo();
			HAL_NVIC_SystemReset();
		}
		else{
			_Log("write flash err = %d",load.bootErrCnt);
			_Error_Handler(__FILE__, __LINE__);	
		}	
	}
}








