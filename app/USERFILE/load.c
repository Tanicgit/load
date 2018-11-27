#include "stm32f4xx_hal.h"
#include "ff.h"
#include "fatfs.h"
#include "string.h"
#include "load.h"
#include "myheap4.h"
_LOAD load;
uint8_t saveLoadInfo(void);


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
		
	sprintf(filename,"%spack",SDPath);
	if((res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		_Log("err->%s",FRESULT_err[res]);
		_Error_Handler(__FILE__, __LINE__);
	}
	
	bytestowrite = load.FilePackNum*sizeof(struct listPack);
	load.packListSum = U8checkSum((uint8_t*)packHead,bytestowrite);
	_Log("f_write(%s)",filename);
	res = f_write(&SDFile, pack, bytestowrite, (void *)&byteswritten);
	
	f_close(&SDFile);
	if(res!=FR_OK)
	{
		_Log("err->%s",FRESULT_err[res]);
		_Error_Handler(__FILE__, __LINE__);
	}
	else if(byteswritten != bytestowrite)
	{
		_Log("err->byteswritten != bytestowrite");
		_Error_Handler(__FILE__, __LINE__);
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
	if(load.sta==0)return 0;//没有下载任务
	if(pack!=NULL || packHead!=NULL)//开机两个必须肯定是空
	{
		myvPortFree(pack);
	}	
	sprintf(filename,"%spack",SDPath);
	if((res=f_open(&SDFile, filename, FA_READ)) != FR_OK)
	{
		_Log("err->%s",FRESULT_err[res]);
		_Error_Handler(__FILE__, __LINE__);
		re = 0xff;
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
			_Log("err->%s",FRESULT_err[res]);	
			_Error_Handler(__FILE__, __LINE__);
			re = 1;
		}
		else if(bytesread != bytestoread)
		{
			_Log("err_bytesread != bytestoread");	
			_Error_Handler(__FILE__, __LINE__);
			re = 1;
		}
		else if(load.packListSum != U8checkSum((uint8_t*)pack,bytestoread))
		{
			re = 0xff;
		}
	}
	if(re==0xff)//PackInfo.bin 文件不存在  校验错误
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
保存下载头信息
可以尝试做掉电保存
*/
uint8_t saveLoadInfo()
{
	char filename[16];
	FRESULT res;
	uint32_t byteswritten, bytestowrite; 
	
	sprintf(filename,"%sload",SDPath);
	if((res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		_Log(" err->%s",FRESULT_err[res]);
		_Error_Handler(__FILE__, __LINE__);
		return 1;
	}
	
	bytestowrite = sizeof(_LOAD);
	load.sum = U8checkSum((uint8_t*)&load,bytestowrite-1);
	_Log("f_write(%s)",filename);
	res = f_write(&SDFile, &load, bytestowrite, (void *)&byteswritten);
	f_close(&SDFile);
	if( res != FR_OK)
	{
		_Log("err->%s",FRESULT_err[res]);
		_Error_Handler(__FILE__, __LINE__);
		return 1;
	}
	else if((byteswritten != bytestowrite))
	{
		_Log("err->byteswritten != bytestowrite");
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
		_Log(" err->%s",FRESULT_err[res]);
		re = 0XFF;
	}
	else
	{
		bytestoread = sizeof(_LOAD);
		_Log("f_read(%s)",filename);
		res = f_read(&SDFile, &load, bytestoread, (void *)&bytesread);
		f_close(&SDFile);
		if(res != FR_OK)
		{
			_Log("err->%s",FRESULT_err[res]);
			_Error_Handler(__FILE__, __LINE__);
			re = 1;
		}
		else if((bytesread != bytestoread))	
		{
			_Log("err->bytesread != bytestoread");
			re = 0xff;
		}			
		else if(load.sum!=U8checkSum((uint8_t*)&load,bytestoread-1))
		{
			_Log("err->U8checkSum");			
			re = 0xff;
		}
		else
		{
			if(load.FileSize!= load.lastPackSize + (load.FilePackNum-1)*load.PackSize)
			{
				_Log("err->load data Err");
				re = 0xff;
			}
		}
	}
	if(re==0XFF)//loadInfo.bin 不存在  数据校验错误
	{
		memset(&load,0,sizeof(_LOAD));
		saveLoadInfo();
	}
	return re;
}

void loadInit()
{

	readLoadInfo();
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


