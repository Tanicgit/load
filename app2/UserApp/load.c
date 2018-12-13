#include "stm32f4xx_hal.h"
#include "ff.h"
#include "fatfs.h"
#include "string.h"
#include "load.h"
#include "myheap4.h"
_LOAD load;
uint8_t saveLoadInfo(void);


/*
�����ڿ���ʱ����ʵ�ʷ���ram
���߼�⵽���º���ݷ�������Ϣ���·���ram
*/
struct listPack *pack=NULL;
struct listPack *packHead=NULL;

void listPack_add(struct listPack **head,struct listPack *e)
{
	struct listPack *temp;
	if(*head==NULL)
	{
		*head = e;
		(*head)->next = NULL;
	}
	else
	{
		temp = *head;
		while(temp)
		{
			if(temp->next==NULL)
			{
				temp->next = e;
				e->next = NULL;
			}
			temp = temp->next;
		}
	}
}


/*��Ҫ�������� ��ʼ�������Ϣ*/
uint8_t NeedReLoad()
{
	uint32_t filesize=0;
	uint16_t i=0;
	if(pack!=NULL)myvPortFree(pack);
	if(packHead!=NULL)packHead=NULL;
	pack = mypvPortMalloc(load.FilePackNum*sizeof(struct listPack));
	{
		if(pack==NULL)_Error_Handler(__FILE__, __LINE__);
	}
	
	filesize = load.FileSize;
	for(i=0;i<load.FilePackNum;i++)
	{
		pack[i].id=i;
		pack[i].sta=0;
		pack[i].checkSum=0;
		pack[i].size = filesize>load.PackSize ? load.PackSize:filesize;
		filesize-=load.PackSize;
		listPack_add(&packHead,&pack[i]);
	}
	
	savePackInfo();
	saveLoadInfo();
	load.sta = 2;
	return 0;
}
/*
�����ؽ�����Ϣ����flash,�Ա��´ο�����������
ÿ���һ�����ظ���һ��
���߿��Գ��������籣���Լ��ٴ��̲�д����
*/
uint8_t savePackInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t byteswritten, bytestowrite; 
	if(packHead==NULL||pack==NULL)return 1;//��������ǿ�
	if(load.FilePackNum==0)return 1;	
	sprintf(filename,"%spack",SDPath);
	if((res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	
	bytestowrite = load.FilePackNum*sizeof(struct listPack);
	load.packListSum = U8checkSum((uint8_t*)packHead,bytestowrite);
	_Log("f_write(%s)",filename);
	res = f_write(&SDFile, pack, bytestowrite, (void *)&byteswritten);
	
	f_close(&SDFile);
	if(res!=FR_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	else if(byteswritten != bytestowrite)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	return 0;
}

uint8_t readPackInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t bytesread, bytestoread; 
	uint8_t re=0;
	if(load.sta==0||load.FilePackNum==0)return 0;
	if(pack!=NULL || packHead!=NULL)
	{
		myvPortFree(pack);
	}	
	sprintf(filename,"%spack",SDPath);
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	else
	{
		pack = mypvPortMalloc(load.FilePackNum*sizeof(struct listPack));	
		if(pack==NULL)_Error_Handler(__FILE__, __LINE__);
		
		bytestoread = load.FilePackNum*sizeof(struct listPack);
		_Log("f_read(%s)",filename);
		res = f_read(&SDFile, pack, bytestoread, (void *)&bytesread);
		f_close(&SDFile);
		if(res != FR_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		else if(bytesread != bytestoread)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		else if(load.packListSum != U8checkSum((uint8_t*)pack,bytestoread))
		{
			re = 0xff;
		}
	}
	if(re==0xff)//PackInfo.bin �ļ�������  У�����
	{
		NeedReLoad();
	}
	else
	{
		for(uint16_t i=0;i<load.FilePackNum;i++)
		{
			listPack_add(&packHead,&pack[i]);
		}	
	}	
	return 0;
}




/*
��������ͷ��Ϣ
���Գ��������籣��
*/
uint8_t saveLoadInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t byteswritten, bytestowrite; 
	
	sprintf(filename,"%sload",SDPath);
	if((res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	
	bytestowrite = sizeof(_LOAD);
	load.sum = U8checkSum((uint8_t*)&load,bytestowrite-1);
	_Log("f_write(%s)",filename);
	res = f_write(&SDFile, &load, bytestowrite, (void *)&byteswritten);
	f_close(&SDFile);
	if( res != FR_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	else if((byteswritten != bytestowrite))
	{
		_Error_Handler(__FILE__, __LINE__);
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
}



void returnDefaultBin()
{
	load.sta=7;
	saveLoadInfo();
	_Log("-return-default-bin---");
	SCB->VTOR = FLASH_BASE;
	HAL_NVIC_SystemReset();//��������
}
extern uint8_t SD_sta;
void loadInit()
{
	if(SD_sta==0)
	{
		readLoadInfo();
		
		if(load.sta==6)//�Զ������ɹ�
		{
			load.sta=0;
		}
		else if(load.sta==8)//ǿ�������ɹ�
		{
			load.sta=0;
		}
		
		readPackInfo();	
		
		_Log("load.sta=%d",load.sta);
		_Log("load.nowBin=%d",load.nowBin);
		_Log("load.nowVersion=%d",load.nowVersion);
		_Log("load.VersionToLoad=%d",load.VersionToLoad);
		_Log("load.FileSize=%d",load.FileSize);
		_Log("load.FilePackNum=%d",load.FilePackNum);
		_Log("load.PackSize=%d",load.PackSize);
		_Log("\r\n");
	}
}

