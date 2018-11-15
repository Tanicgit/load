#include "myheap4.h"
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "fatfs.h"
#include "string.h"

uint8_t saveLoadInfo(void);


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
_LOAD load;




struct listPack{
	uint8_t id;//顺序
	uint8_t sta;//状态0未下载 已下载
	uint8_t checkSum;//该包的校验和
	uint16_t size;//该包的大小
	struct listPack *next;
};
/*
链表在开机时根据实际分配ram
或者检测到更新后根据服务器消息重新分配ram
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


/*需要重新下载 初始化相关信息*/
uint8_t NeedReLoad()
{
	uint32_t filesize=0;
	uint16_t i=0;
	
	/**/
	//load.sta = 1;//服务器通讯文件内置为
	//获取到了服务器新下载消息
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
将下载进程信息保存flash,以便下次开机继续下载
每完成一包下载更新一次
或者可以尝试做掉电保存以减少磁盘擦写次数
*/
uint8_t savePackInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t byteswritten, bytestowrite; 
	if(packHead==NULL||pack==NULL)return 1;//两个必须非空
	
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
	saveLoadInfo();//可以尝试做掉电保存
	res = f_write(&SDFile, packHead, bytestowrite, (void *)&byteswritten);
	f_close(&SDFile);
	if((byteswritten != bytestowrite) || (res != FR_OK))
	{
		_Log("f_write byteswritten=%d err=%s",byteswritten,FRESULT_err[res]);
		return 1;
	}
	return 0;
}

/*仅仅开机读取下载进程信息 仅仅在上次下载未完成的条件下需要调用*/
uint8_t readPackInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t bytesread, bytestoread; 
	uint8_t re=0;
	if(load.sta==0)return 1;//没有下载任务
	if(pack!=NULL || packHead!=NULL)return 1;//开机两个必须肯定是空
	
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
	if(re)//PackInfo.bin 文件错误  重建文件 从头下载
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
保存下载头信息
可以尝试做掉电保存
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
/*读取下载头信息*/
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
	if(re)//loadInfo.bin 文件错误  重建并初始化为非下载状态
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
			/*检测到下载信号*/
			if(0==NeedReLoad())
			{			
				if(0==saveLoadInfo())
				{
					load.sta=0;
				}
			}
			else
			{
				//多次创建启动下载信息不成功处理
			}
		}
		else if(load.sta==2)
		{
			//下载中
		}
	}
}

