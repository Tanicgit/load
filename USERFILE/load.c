#include "myheap4.h"
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "fatfs.h"
#include "string.h"

uint8_t saveLoadInfo(void);


#pragma  pack(1) 
typedef struct{
	uint8_t  sta;//0 �� 0x01 ���µ�����   0x02 ��������
	
	uint32_t FileSize;
	uint16_t FilePackNum;
	uint16_t PackSize;
	uint8_t  packListSum;
	uint8_t  sum;
}_LOAD;
#pragma  pack() 
_LOAD load;




struct listPack{
	uint8_t id;//˳��
	uint8_t sta;//״̬0δ���� ������
	uint8_t checkSum;//�ð���У���
	uint16_t size;//�ð��Ĵ�С
	struct listPack *next;
};
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
	
	/**/
	//load.sta = 1;//������ͨѶ�ļ�����Ϊ
	//��ȡ���˷�������������Ϣ
//		load.FileSize = 10*1024+126;
//		load.PackSize = 512;
//		load.FilePackNum = load.FileSize/load.PackSize + (load.FileSize%load.PackSize ? 1:0);
		
		myvPortFree(pack);
		pack = mypvPortMalloc(load.FilePackNum*sizeof(struct listPack));
		{
			if(pack==NULL)return 1;
		}
		
		filesize = load.FileSize;
		for(i=0;i<load.FilePackNum;i++)
		{
			pack[i].id=0;
			pack[i].sta=0;
			pack[i].checkSum=0;
			pack[i].size = filesize>load.PackSize ? load.PackSize:filesize;
			filesize-=load.PackSize;
			listPack_add(&packHead,&pack[i]);
		}
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
	
	filename[0]=0;
	strcat(filename,SDPath);
	strcat(filename,"PackInfo.bin");	
	if((res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		_Log("f_open err=%s",FRESULT_err[res]);
		f_close(&SDFile);
		return 1;
	}
	
	bytestowrite = load.FilePackNum*sizeof(struct listPack);
	load.packListSum = U8checkSum((uint8_t*)packHead,bytestowrite);
	saveLoadInfo();//���Գ��������籣��
	res = f_write(&SDFile, packHead, bytestowrite, (void *)&byteswritten);
	f_close(&SDFile);
	if((byteswritten != bytestowrite) || (res != FR_OK))
	{
		_Log("f_write byteswritten=%d err=%s",byteswritten,FRESULT_err[res]);
		return 1;
	}
	return 0;
}

/*����������ȡ���ؽ�����Ϣ �������ϴ�����δ��ɵ���������Ҫ����*/
uint8_t readPackInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t bytesread, bytestoread; 
	uint8_t re=0;
	if(load.sta==0)return 1;//û����������
	if(pack!=NULL || packHead!=NULL)return 1;//������������϶��ǿ�
	
	filename[0]=0;
	strcat(filename,SDPath);
	strcat(filename,"PackInfo.bin");	
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		_Log("f_open err=%s",FRESULT_err[res]);
		f_close(&SDFile);
		re = 1;
	}
	
	pack = mypvPortMalloc(load.FilePackNum*sizeof(struct listPack));	
	if(pack==NULL)return 1;
	
	bytestoread = load.FilePackNum*sizeof(struct listPack);
	res = f_read(&SDFile, packHead, bytestoread, (void *)&bytesread);
	f_close(&SDFile);
	if((bytesread != bytestoread) || (res != FR_OK))
	{
		_Log("f_write byteswritten=%d err=%s",bytesread,FRESULT_err[res]);	
		re = 1;
	}
	if(load.packListSum != U8checkSum((uint8_t*)packHead,bytestoread))
	{
		re = 1;
	}
	if(re)//PackInfo.bin �ļ�����  �ؽ��ļ� ��ͷ����
	{
		NeedReLoad();
		savePackInfo();
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
	
	filename[0]=0;
	strcat(filename,SDPath);
	strcat(filename,"loadInfo.bin");
	if((res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		_Log("f_open err=%s",FRESULT_err[res]);
		f_close(&SDFile);
		return 1;
	}
	
	bytestowrite = sizeof(_LOAD);
	load.sum = U8checkSum((uint8_t*)&load,bytestowrite-1);
	res = f_write(&SDFile, &load, bytestowrite, (void *)&byteswritten);
	f_close(&SDFile);
	if((byteswritten != bytestowrite) || (res != FR_OK))
	{
		_Log("f_write byteswritten=%d err=%s",byteswritten,FRESULT_err[res]);
		return 1;
	}
	return 0;
}
/*��ȡ����ͷ��Ϣ*/
uint8_t readLoadInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t bytesread, bytestoread; 
	uint8_t re=0;
	filename[0]=0;
	strcat(filename,SDPath);
	strcat(filename,"loadInfo.bin");	
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		_Log("f_open err=%s",FRESULT_err[res]);
		f_close(&SDFile);
		re = 1;
	}
	bytestoread = sizeof(_LOAD);
	res = f_read(&SDFile, &load, bytestoread, (void *)&bytesread);
	f_close(&SDFile);
	if((bytesread != bytestoread) || (res != FR_OK))
	{
		_Log("f_read byteswritten=%d err=%s",bytesread,FRESULT_err[res]);
		re = 1;
	}	
	if(load.sum!=U8checkSum((uint8_t*)&load,bytestoread-1))
	{
		_Log("f_read check err");
		re = 1;
	}
	if(re)//loadInfo.bin �ļ�����  �ؽ�����ʼ��Ϊ������״̬
	{
		memset(&load,0,sizeof(_LOAD));
		saveLoadInfo();
	}
	return re;
}

void loadTask()
{

	osDelay(1000);
	readLoadInfo();
	readPackInfo();
	while(1)
	{
		osDelay(100);
		if(load.sta==0)
		{
		
		}
		else if(load.sta==1)
		{
			/*��⵽�����ź�*/
			if(0==NeedReLoad())
			{			
				if(0==saveLoadInfo())
				{
					load.sta=0;
				}
			}
			else
			{
				//��δ�������������Ϣ���ɹ�����
			}
		}
		else if(load.sta==2)
		{
			//������
		}
	}
}

