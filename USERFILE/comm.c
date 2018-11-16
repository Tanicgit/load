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
#include "myheap4.h"
char SSID[]="TP-LINK_2052";
char KEY[]="ysjoowxf0713";
char IP[]="47.107.228.113";
char port[]="3200";

static char buffer[256];
int Ac_pf3(char *format,...)
{
	va_list aptr;
	int ret;
	va_start(aptr,format);
	ret = vsprintf(buffer, format, aptr);
  va_end(aptr);
	HAL_UART_Transmit(&huart3,(uint8_t*)buffer,strlen(buffer),0xffff);
  return(ret);
}
#define RX_BUF_SIZE	2048
#define TX_BUF_SIZE	512
static  char RX_buffer[RX_BUF_SIZE]; 
static  char TX_buffer[TX_BUF_SIZE]; 
static uint16_t Rx_p=0;
static uint8_t _r_n=0;
static uint8_t mode=0;//0 set  1 trans

void getData(uint8_t a)
{
	if(Rx_p<RX_BUF_SIZE)
	{
		RX_buffer[Rx_p++] = a;
		if(a=='\n')_r_n=1;
	}
}

void getTransMode(uint8_t a)
{
	if(Rx_p<RX_BUF_SIZE && _r_n==0)
	{
		RX_buffer[Rx_p++] = a;
		if(a=='\n')_r_n=1;
	}
}

void clearRxbuf()
{
	memset(RX_buffer,0,RX_BUF_SIZE);
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
int waitEspAck(_ESP_RE *esp,uint16_t ms)
{
	uint16_t offset,i=0,j=0;
	uint8_t num=0;
	if(esp->endStrNum==0)return -1;
	if(esp->endStrNum>=MAX_END_STR)return -1;
	do
	{
		osDelay(100);
		if(_r_n==0)continue;
		_r_n=0;
		for(j=0;j<esp->endStrNum;j++)
		{
			num=1;
			if(AC_OK==str_find(RX_buffer,esp->endStr[j].res,&offset,&num))
			{
				esp->endStr[j].offset = offset;
				esp->result = j;
				goto retur;
			}
		}
	}while(i++<ms/100);
	esp->result = 0;
	retur:
	return 0;
}
/*
0 ok
1 timeout
*/
int ESP_sendMsg(char *msg,_ESP_RE *esp)
{
	uint8_t i=0;
	do{
	clearRxbuf();
	Ac_pf3(msg);
	if(0==waitEspAck(esp,esp->ms))return 0;
	}while(i++<esp->cnt);
	return 1;
}
int ESP_mode_set()
{
	uint16_t offset;
	uint8_t num,mode=0;
	_ESP_RE re;
	
	re.endStrNum=1;
	re.cnt=3;
	re.ms=200;
	strcpy(re.endStr[0].res,"OK");
	if(0==ESP_sendMsg("AT+CWMODE?",&re))
	{
		if(AC_OK==str_find(RX_buffer,"+CWMODE:",&offset,&num))
		{
			mode = RX_buffer[offset+8]-0x30;
			if(mode==1)return 0;
			else
			{
				re.endStrNum=1;
				re.cnt=3;
				re.ms=200;
				strcpy(re.endStr[0].res,"+CWMODE:1");
				ESP_sendMsg("AT+CWMODE=1",&re);
			}
		}
	}
	return 1;
}

/*
0 联网成功
*/
int ESP_ssid_key_set()
{
	char *tbuf;
	_ESP_RE re;
	tbuf = mypvPortMalloc(128);
	sprintf(tbuf,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",SSID,KEY);
	re.endStrNum=2;
	re.cnt=3;
	re.ms=20000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	strcpy(re.endStr[1].res,"FAIL");
	if(0==ESP_sendMsg(tbuf,&re))
	{
		myvPortFree(tbuf);
		if(re.result==0)
		{
			return 0;
		}
		else 
		{
			return 1;
		}
	}
	myvPortFree(tbuf);
	return 1;
}

/**/
int ESP_AT_reset()
{
	_ESP_RE re;
	re.endStrNum=1;
	re.cnt=1;
	re.ms=5000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"ready");
	if(0==ESP_sendMsg("AT+RST\r\n",&re))
	{
			return 0;

	}
	return 1;
}

/**/
int ESP_creat_TCP_socket()
{
	char *tbuf;
	_ESP_RE re;
	tbuf = mypvPortMalloc(128);
	sprintf(tbuf,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",IP,port);
	re.endStrNum=1;
	re.cnt=10;
	re.ms=2000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"CONNECT");
	if(0==ESP_sendMsg(tbuf,&re))
	{
		myvPortFree(tbuf);
		return 0;

	}
	myvPortFree(tbuf);
	return 1;	
}

/**/
int ESP_into_transMode()
{
	_ESP_RE re;
	re.endStrNum=1;
	re.cnt=2;
	re.ms=1000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	if(0==ESP_sendMsg("AT+CIPMODE=1\r\n",&re))
	{
			return 0;

	}
	return 1;	
}

/**/
int ESP_start_trans()
{
	_ESP_RE re;
	re.endStrNum=1;
	re.cnt=2;
	re.ms=1000;
	re.result=0xff;
	strcpy(re.endStr[0].res,">");
	if(0==ESP_sendMsg("AT+CIPSEND\r\n",&re))
	{
			return 0;

	}
	return 1;	
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
	clearRxbuf();
	UART3_Callback = getData;
	/**/
	mode = 0;
	re:
	ESP_AT_reset();
	
	if(0!=ESP_mode_set())goto re;
	
	if(0!=ESP_ssid_key_set())goto re;
	
	if(0!=ESP_creat_TCP_socket())goto re;
	
	if(0!=ESP_into_transMode())goto re;
	
	if(0!=ESP_start_trans())goto re;
	
	clearRxbuf();
	UART3_Callback = getTransMode;
	mode = 1;
}

char* Trans_PackRx()
{
	if(mode == 0) return NULL;
	if(_r_n && Rx_p!=0)
	{
			return RX_buffer;
	}
	return NULL;
}

void Trans_PackSend(char *a,uint16_t len)
{

}

#include "load.h"
void CommTask()
{
	ESP_init();
	loadInit();
	
	while(1)
	{
		osDelay(200);
		
		
		
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






