#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "usart.h"
#include "comm.h"
#include "myAc.h"
#include "load.h"
#include "ff.h"
#include "fatfs.h"
#include "myheap4.h"
#include "net.h"


char SSID[20];//="TP-LINK_2052";//20
char KEY[20];//="";//20
char IP[20];//="192.168.0.100";//20
char port[10];//="3200";//10
uint8_t wifi_sta=0;
uint8_t server_sta=0;
#include "touch.h"
#include "24CXX.H"
#define  NET_SAVE_ADDR_BASE  TOUCH_SAVE_ADDR_BASE+TOUCH_SAVE_SIZE
void ReadNetSet()
{
	AT24CXX_Read(NET_SAVE_ADDR_BASE,(uint8_t*)SSID,20);
	AT24CXX_Read(NET_SAVE_ADDR_BASE+20,(uint8_t*)KEY,20);
	AT24CXX_Read(NET_SAVE_ADDR_BASE+40,(uint8_t*)IP,20);
	AT24CXX_Read(NET_SAVE_ADDR_BASE+60,(uint8_t*)port,10);	
	
}
//默认配置
void SaveNetReset()
{
	strcpy(SSID,"TP-LINK_2052");
//	strcpy(KEY,"");//
	strcpy(IP,"192.168.0.103");
	strcpy(port,"3200");
	SaveNetSet();
}

void SaveNetSet()
{
	AT24CXX_Write(NET_SAVE_ADDR_BASE,(uint8_t*)SSID,20);
	AT24CXX_Write(NET_SAVE_ADDR_BASE+20,(uint8_t*)KEY,20);
	AT24CXX_Write(NET_SAVE_ADDR_BASE+40,(uint8_t*)IP,20);
	AT24CXX_Write(NET_SAVE_ADDR_BASE+60,(uint8_t*)port,10);
	wifi_sta &=~0x7f;
	server_sta &=~0x7f;
}



_NET_STA net_sta;

static char buffer[256];
int Ac_pf3(char *format,...)
{
	va_list aptr;
	int ret;
	va_start(aptr,format);
	ret = vsprintf(buffer, format, aptr);
  va_end(aptr);
	HAL_UART_Transmit(&huart3,(uint8_t*)buffer,strlen(buffer),0xffff);
	_Log("%s",buffer);
  return(ret);
}

static  char RX_buffer[COMM_RX_BUF_SIZE]; 
static uint16_t Rx_p=0;
static uint8_t _r_n=0;
static uint8_t mode=0;//0 set  1 trans

void getData(uint8_t a)
{
	if(Rx_p<COMM_RX_BUF_SIZE)
	{
		RX_buffer[Rx_p++] = a;
		if(a=='\n')_r_n=1;
	}
}

void getTransMode(uint8_t a)
{
	if(Rx_p<COMM_RX_BUF_SIZE && _r_n==0)
	{
		RX_buffer[Rx_p++] = a;
		if(a=='\n')_r_n=1;
	}
}

void clearRxbuf()
{
	memset(RX_buffer,0,COMM_RX_BUF_SIZE);
	Rx_p=0;
	_r_n=0;
}

typedef struct{
	char res[16];
	uint16_t offset;
}_EDN_STR;
#define MAX_END_STR	4
typedef struct{
	uint8_t endStrNum;//表示消息结束的特征字符串数量  出现任意一个即该次通讯完成
	uint8_t result;
	_EDN_STR endStr[MAX_END_STR];//
	uint16_t ms;
	uint8_t cnt;
}_ESP_RE;

/*
0 ok
-1 input err
*/
_CERR waitEspAck(_ESP_RE *esp,uint16_t ms)
{
	uint16_t offset,i=0,j=0;
	uint8_t num=0;
	if(esp->endStrNum==0)return CM_ERR;
	if(esp->endStrNum>=MAX_END_STR)return CM_ERR;
	do
	{
		osDelay(100);
		if(_r_n==0)continue;
		_r_n=0;
		for(j=0;j<esp->endStrNum;j++)
		{
			num=1;
			if(esp->endStr[j].res[0]!=0)
			{
				if(AC_OK==str_find(RX_buffer,esp->endStr[j].res,&offset,&num))
				{
					esp->endStr[j].offset = offset;
					esp->result = j;
					goto retur;
				}
			}
			else
			{
				goto retur;
			}
		}
	}while(i++<ms/100);
	esp->result = 0;
	retur:
	if(i>=ms/100)return CM_NOACK_TIME_OUT;
	else return CM_OK;
}
/*
0 ok
1 timeout
*/
_CERR ESP_sendMsg(char *msg,_ESP_RE *esp)
{
	uint8_t i=0;
	do{
	clearRxbuf();
	Ac_pf3(msg);
	if(CM_OK==waitEspAck(esp,esp->ms))return CM_OK;
	}while(i++<esp->cnt);
	return CM_NOACK_TIME_OUT;
}
_CERR ESP_mode_set()
{
	uint16_t offset;
	uint8_t num,mode=0;
	_ESP_RE re;
	_CERR err=CM_OK;
	re.endStrNum=1;
	re.cnt=3;
	re.ms=500;
	strcpy(re.endStr[0].res,"OK");
	err=ESP_sendMsg("AT+CWMODE?\r\n",&re);
	if(err==CM_OK)
	{
		num=1;
		if(AC_OK==str_find(RX_buffer,"+CWMODE:",&offset,&num))
		{
			mode = RX_buffer[offset+8]-0x30;
			if(mode==1)err = CM_OK;
			else
			{
				re.endStrNum=1;
				re.cnt=3;
				re.ms=200;
				strcpy(re.endStr[0].res,"+CWMODE:1");
				ESP_sendMsg("AT+CWMODE=1\r\n",&re);
				err = CM_NONE;
			}
		}
		else err = CM_NONE;
	}
	return err;
}
/*

*/
_CERR ESP_CIPMUX()
{
	
	_ESP_RE re;
	_CERR err=CM_OK;
	re.endStrNum=1;
	re.cnt=3;
	re.ms=100;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	err = ESP_sendMsg("AT+CIPMUX=0\r\n",&re);
	return err;	
	
}


/*
0 联网成功
*/
_CERR ESP_ssid_key_set()
{
	char *tbuf=NULL;
	_ESP_RE re;
	_CERR err=CM_OK;
	ReadNetSet();
	tbuf = mypvPortMalloc(128);
	sprintf(tbuf,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",SSID,KEY);
	re.endStrNum=2;
	re.cnt=3;
	re.ms=20000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	strcpy(re.endStr[1].res,"FAIL");
	err=ESP_sendMsg(tbuf,&re);
	if(err==CM_OK)
	{
		if(re.result==0)err = CM_OK;	
		else err = CM_NONE;		
	}
	myvPortFree(tbuf);
	
	return err;
}

/**/
_CERR ESP_AT_reset()
{
	_ESP_RE re;
	_CERR err=CM_OK;
	re.endStrNum=1;
	re.cnt=1;
	re.ms=8000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"ready");
	err = ESP_sendMsg("AT+RST\r\n",&re);
	return err;
}

/**/
_CERR ESP_creat_TCP_socket()
{
	char *tbuf=NULL;
	_ESP_RE re;
	_CERR err=CM_OK;
	tbuf = mypvPortMalloc(128);
	sprintf(tbuf,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",IP,port);
	re.endStrNum=1;
	re.cnt=10;
	re.ms=2000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"CONNECT");
	
	err=ESP_sendMsg(tbuf,&re);
	myvPortFree(tbuf);
	
	return err;	
}

/**/
_CERR ESP_into_transMode()
{
	_ESP_RE re;
	re.endStrNum=1;
	re.cnt=2;
	re.ms=1000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	return ESP_sendMsg("AT+CIPMODE=1\r\n",&re);
	
}

/**/
_CERR ESP_start_trans()
{
	_ESP_RE re;
	re.endStrNum=1;
	re.cnt=2;
	re.ms=2000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	return ESP_sendMsg("AT+CIPSEND\r\n",&re);

}

void ESP_exit_TransMode()
{

	HAL_GPIO_WritePin(ES_CTRL_GPIO_Port, ES_CTRL_Pin, GPIO_PIN_RESET);
	osDelay(120);		//等待120ms
	HAL_GPIO_WritePin(ES_CTRL_GPIO_Port, ES_CTRL_Pin, GPIO_PIN_SET);
}


/**/
void ESP_init()
{

	ESP_exit_TransMode();
	clearRxbuf();
	UART3_Callback = getData;
	StartUART_Rx(&huart3);
	
	net_sta.sta = 1;
	
	/**/
	mode = 0;
	re:
	if(0!=ESP_AT_reset())goto re;
	
	if(0!=ESP_mode_set())goto re;
	
	while(0!=ESP_ssid_key_set())
	{
		osDelay(2000);
		if((wifi_sta&0x7f)<3)wifi_sta++;//
		else{
			while(wifi_sta)osDelay(2000);		
		}
	}
	wifi_sta |= 0x80;
	
	if(0!=ESP_CIPMUX())goto re;
	
//	if(0!=ESP_creat_TCP_socket())goto re;
	while(0!=ESP_creat_TCP_socket())
	{
		osDelay(2000);
		if((server_sta&0x7f)<3)server_sta++;//
		else{
			while(wifi_sta)osDelay(2000);		
		}
	}
	server_sta |=0x80;
	
	if(0!=ESP_into_transMode())goto re;
	
	if(0!=ESP_start_trans())goto re;
	
	_Log("net ok");
	net_sta.sta=2;
	clearRxbuf();
	UART3_Callback = getTransMode;
	mode = 1;
}

_CERR Trans_WaitPackRx(uint16_t ms)
{
	uint8_t i=0;
	if(mode == 0) return CM_ERR;

	do{
		osDelay(500);
		if(_r_n && Rx_p!=0)return CM_OK;
	}while(i++<ms/500);
	return CM_NOACK_TIME_OUT;
}

_CERR Trans_PackSend(char *a,uint16_t len)
{
	uint8_t *data=NULL;
	data = mypvPortMalloc(2+len*2);
	if(data==NULL)return CM_RAM;
	data[0] = '@';
	data[1+len*2] = '\n';
	for(int i=0;i<len;i++)
	{
		data[1+i*2] = BinTo_09_AF((a[i]&0xf0)>>4);
		data[1+i*2+1] =BinTo_09_AF(a[i]&0xf);
	}	
	clearRxbuf();
	HAL_UART_Transmit(&huart3,data,len*2+2,0xffff);
	_Log("TX->");
	HAL_UART_Transmit(&huart1,data,len*2+2,0xffff);
	myvPortFree(data);
	return CM_OK;
}
/**/
_CERR Trans_PackRevMsg01(_MSG01_r *p)
{
	uint16_t i,head,tail,size=sizeof(_MSG01_r);
	for(i=0;i<Rx_p;i++)
	{
		if(RX_buffer[i]=='@')
		{
			head = i;
		}
		if(RX_buffer[i]=='\n')
		{
			tail = i;
		}
	}
	
	_Log("RX->%s",RX_buffer);
	
	if((tail-head-1)!=size*2)return CM_PACK;
	
	if(AC_OK!=HexStr2Bins(&RX_buffer[head+1],size*2,(uint8_t*)p))return CM_PACK;
	
	if(p->sum != U8checkSum((uint8_t *)p,sizeof(_MSG01_r)-1))return CM_PACK;
	
	if(p->head.dir!=0x81)return CM_PACK;//下行方向
	
	if(p->head.msg!=0x01)return CM_PACK;//
	
	return CM_OK;
}
//static uint8_t aabuf[2048];
_CERR Trans_PackRevMsg02(_MSG02_r *p,uint16_t packid,uint16_t packsize,uint8_t *binPack)
{
	uint16_t i,tp=0,j=0;
	uint8_t *data=NULL,sum=0;
	char *da;
	_CERR err=CM_OK;
	for(i=0;i<Rx_p;i++)
	{
		
		if(RX_buffer[i]=='\n')
		{
			tp = 0;
		}
		if(tp)
		{
			j++;
		}
		if(RX_buffer[i]=='@')
		{
			tp = 1;
			da = RX_buffer+i+1;
		}
	}
	_Log("RX-> pack%04d num = %d",packid,Rx_p);
	if(j==0)return CM_PACK;
	data = mypvPortMalloc(j/2);
	if(data==NULL)
	{
		err = CM_RAM;
		goto exitfun;
	}
	if(AC_OK!=HexStr2Bins(da,j,data))
	{
		err = CM_PACK;
		goto exitfun;	
	}
	
	memcpy(p,data,sizeof(_MSG)+MSG02_R_OFF);//把data中的固定长度部分结构化
	
	sum = data[sizeof(_MSG)+MSG02_R_OFF+p->packSize];//包中的校验和位
	
	if(sum != U8checkSum(data,j/2-1))//校验通不过
	{
		err = CM_PACK;
		goto exitfun;	
	}
	
	if(p->head.dir!=0x81)	//方向位不对
	{
		err = CM_PACK;
		goto exitfun;	
	}
	if(p->head.msg!=0x02)//消息类型不对
	{
		err = CM_PACK;
		goto exitfun;	
	}
	
	if(p->ver!=load.VersionToLoad)//下载过程出现了又一个新版本
	{
		err = CM_PACK;
		load.sta = 0;//重新检查
		goto exitfun;	
	}
	
	if(p->packSize!=packsize)//下载的包大小和请求的不同
	{
		err = CM_PACK;
		load.sta = 0;//重新检查
		goto exitfun;	
	}
	
	if(p->packId!=packid)//下行的包编号和请求的不同
	{
		err = CM_PACK;
		load.sta = 0;//重新检查
		goto exitfun;	
	}
	
	memcpy(binPack,data+sizeof(_MSG)+MSG02_R_OFF,p->packSize);//将bin数据提取出来
	

	exitfun:
	myvPortFree(data);
	return err;
}



static uint16_t mid=0;
uint32_t sysaddr = 1;
_MSG01_t msg01t;
_MSG01_r msg01r;
_CERR updataCheck()
{
	_MSG01_t *mt=&msg01t;
	_MSG01_r *mr=&msg01r;
	_CERR err=CM_OK;
	
	mt->head.len = sizeof(_MSG01_t);
	mt->head.mid = mid++;
	mt->head.dir = 0x01;
	mt->head.ver = load.nowVersion;
	mt->head.addr = sysaddr;
	mt->head.msg = 1;
	mt->loadsta = load.sta; 
	mt->sum = U8checkSum((uint8_t*)mt,mt->head.len-1);
	Trans_PackSend((char*)mt,mt->head.len);
	err = Trans_WaitPackRx(10000);
	if(err==CM_OK)
	{
		err = Trans_PackRevMsg01(mr);
		if(err==CM_OK)
		{			
			if(mr->upTp)
			{
				load.FileSize = mr->fileSize;
				load.PackSize = mr->packSize;
				load.lastPackSize = load.FileSize%load.PackSize;
				load.FilePackNum = load.lastPackSize ? load.FileSize/load.PackSize+1 : load.FileSize/load.PackSize;
				load.sta = 2;
				load.crc16=mr->crc16;
				load.VersionToLoad = mr->ver;
				
				if(load.PackSize>BIN_PACK_MAX_SIZE)
				{
					err = CM_NONE;
				}
				else
				{
					NeedReLoad();
				}
			}
		}
	}
	return err;
}

_MSG02_t msg02t;
_MSG02_r msg02r;
_CERR loading()
{
	_MSG02_t *mt=&msg02t;
	_MSG02_r *mr=&msg02r;
	uint8_t  *binPack=NULL;
	_CERR err=CM_OK;
	struct listPack *temp=NULL;
	char filename[25];
	FRESULT res;
	uint32_t byteswritten=0;
	

	
	for(temp=packHead;temp!=NULL;temp=temp->next)
	{
		if(temp->sta==0)break;
	}
	
	binPack = mypvPortMalloc(temp->size);
	if(binPack==NULL)
	{
		return CM_RAM;
	}	
	
	if(temp==NULL)
	{
		load.sta = 3;//下载完成  接下来的工作交给其他处理
		return CM_OK;
	}
		

	
	mt->head.len = sizeof(_MSG02_t);
	mt->head.mid = mid++;
	mt->head.dir = 0x01;
	mt->head.ver = load.nowVersion;
	mt->head.addr = sysaddr;
	mt->head.msg = 2;
	mt->packId = temp->id; 
	mt->packSize = temp->size;
	mt->sum = U8checkSum((uint8_t*)mt,mt->head.len-1);
	Trans_PackSend((char*)mt,mt->head.len);	
	err = Trans_WaitPackRx(10000);
	if(err==CM_OK)
	{
		err = Trans_PackRevMsg02(mr,temp->id,temp->size,binPack);
		if(err==CM_OK)
		{
				sprintf(filename,"%spk%04d",SDPath,temp->id);
				res=f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
				if(res!= FR_OK)
				{
					err = CM_FILE;
					goto exitfun ;
				}
				_Log("f_write");
				res = f_write(&SDFile, binPack, mr->packSize, (void *)&byteswritten);
				f_close(&SDFile);
				if((byteswritten != mr->packSize) || (res != FR_OK))
				{
					err = CM_FILE;
					goto exitfun;
				}
				else
				{
					temp->sta = 1;
					temp->checkSum = U8checkSum(binPack,mr->packSize);				
				}
		}			
	}
	
	exitfun:
	myvPortFree(binPack);
	if(err!=CM_OK && err!=CM_PACK)
		_Error_Handler(__FILE__, err);
	return err;
}

_CERR binSave()
{
	FRESULT res;
	char SD_FileName[16],tFileName[16];
	uint32_t rw;
//	uint16_t crc16=0xffff;
	_CERR err = CM_OK;
	struct listPack *temp=NULL;
	uint8_t *readbuf=NULL;
	FIL *fs=NULL;
	fs = mypvPortMalloc(sizeof(FIL));
	readbuf = mypvPortMalloc(load.PackSize);
	if(readbuf==NULL|| fs==NULL)return CM_RAM;
	
	
	/*创建一个临时文件把下载的各个pack合成一个文件t.bin*/
	sprintf(SD_FileName,"%s%s",SDPath,"t.bin");
	res=f_open(&SDFile, SD_FileName, FA_CREATE_ALWAYS | FA_WRITE);
	if(res!=FR_OK)	
	{
		err= CM_FILE;
		goto exitfun;
	}
	/*----合成----*/
	for(temp=packHead;temp!=NULL;temp=temp->next)
	{
		if(temp->sta==0)/*PACK信息有误,有包未下载*/
		{
			err=CM_DATA;
			load.sta=2;//转到下载ing
			continue;
		}	
		sprintf(tFileName,"%spk%04d",SDPath,temp->id);
		/*打开一个包文件  x.bin*/
		if((res=f_open(fs, tFileName, FA_READ)) != FR_OK)
		{
			err = CM_FILE;
			break;
		}	
		/*读出数据校验,与包信息校验位比较*/
		res = f_read(fs, readbuf,temp->size, (void *)&rw);
		if(res!=FR_OK)
		if(temp->checkSum!=U8checkSum(readbuf,rw))
		{
			temp->sta = 0;//清下载成功标志
			load.sta = 2;//有包校验错误,重新下载该包
		}
		res = f_write(&SDFile,readbuf,temp->size,(void *)&rw);//文件关闭之前 数据指针自动移动
		if(res!=FR_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		else if(temp->size!=rw)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		f_close(fs);
	}
	f_close(fs);
	f_close(&SDFile);
	if(err!=CM_OK)goto exitfun;
	
//	/*CRC校验 窗口式的算法 对整个t.bin进行CRC16校验*/
//	res=f_open(&SDFile, SD_FileName, FA_READ);
//	if(res != FR_OK)
//	{
//		err= CM_FILE;
//		goto exitfun;
//	}
//	if(load.FileSize==f_size(&SDFile)) 
//	{
//		for(temp=packHead;temp!=NULL;temp=temp->next)
//		{
//			res = f_read(&SDFile, readbuf,temp->size, (void *)&rw);
//			crc16=Crc16_Windows(crc16,readbuf,temp->size);
//		}
//		f_close(&SDFile);
//		if(crc16!=load.crc16)
//		{
//			load.sta = 0;
//			err = CM_RELOAD;
//		}
//	}
//	else
//	{
//		load.sta = 0;
//		err = CM_RELOAD;
//	}
//	f_close(&SDFile);
	if(err==CM_OK)
	{
			load.sta = 4;
			savePackInfo();
			saveLoadInfo();
	}
	

	
	exitfun:
	myvPortFree(readbuf);
	myvPortFree(fs);
	return err;	
}

_CERR fileCopy()
{
	uint8_t e=0;
	_CERR err = CM_OK;
	char SD_FileName[16],tFileName[16];
	
	sprintf(SD_FileName,"%s%s",SDPath,"t.bin");
	if(load.nowBin==0)
	{
		load.nowBin=1;
		sprintf(tFileName,"%s%s",SDPath,"1.bin");
		e = exf_copy(tFileName,SD_FileName,1);/*文件copy  t.bin->1.bin*/
		if(e!=0)
		{
			err = CM_COPY;
		}

	}
	else
	{
		load.nowBin=0;
		sprintf(tFileName,"%s%s",SDPath,"0.bin");
		e = exf_copy(tFileName,SD_FileName,1);
		if(e!=0)	/*文件copy  t.bin->0.bin*/
		{
			err = CM_COPY;
		}		
	}

	if(err==CM_OK)
	{
		load.sta =  5;
		saveLoadInfo();	
	}
	return err;
}
extern uint8_t SD_sta;
uint8_t copyerr=0;
void CommTask()
{
	uint16_t checkupdataCnt=0;
	loadInit();//初始化下载状态
	ESP_init();//联网 TCP 透传

	while(1)
	{
		osDelay(1000);
		
		/*其他通讯任务*/
		if(SD_sta==0)
		{
			if(load.sta==0)
			{
				//检查更新
				if(checkupdataCnt>60)
				{
					checkupdataCnt=0;
					_Log("-----updataCheck()-----");
					if(CM_NOACK_TIME_OUT==updataCheck())
					{
						if(net_sta.err_cnt<10)net_sta.err_cnt++;
						else
						{
							net_sta.sta=3;
						}
					}
				}
				checkupdataCnt++;
			}
			else if(load.sta==2)
			{
				_Log("-----loading()-----");
				loading();
			}
			else if(load.sta==3)
			{
				_Log("-----binSave()-----");
				binSave();	
			}
			else if(load.sta==4)//升级准备
			{
				_Log("-----fileCopy()-----");
				if(CM_OK!=fileCopy())
				{
					_Error_Handler(__FILE__, __LINE__);
				}
			}
			else if(load.sta==5)
			{
				_Log("-----load.sta==5-----");
				SCB->VTOR = FLASH_BASE;
				HAL_NVIC_SystemReset();//重启升级
			}
		}
	}
}






