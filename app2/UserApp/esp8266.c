#include "esp8266.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "myheap4.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/**/
#include "usart.h"
/**/
char SSID[]="TP-LINK_2052";
char KEY[]="ysjoowxf0713";
//char IP[]="47.107.228.113";
//char port[]="3200";
char IP[]="192.168.0.100";
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
	_Log("%s",buffer);
  return(ret);
}

static char *RX_buffer; 
static uint16_t RX_buf_size=0;
static uint16_t Rx_p=0;
static uint8_t _r_n=0;
static uint8_t mode=0;//0 set  1 trans


void getDataSetMode(uint8_t a)
{
	if(Rx_p<RX_buf_size)
	{
		RX_buffer[Rx_p++] = a;
		if(a=='\n')_r_n=1;
	}
}

void getTransMode(uint8_t a)
{
	if(Rx_p<RX_buf_size && _r_n==0)
	{
		RX_buffer[Rx_p++] = a;
		if(a=='\n')_r_n=1;
	}
}

void clearRxbuf()
{
	memset(RX_buffer,0,RX_buf_size);
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
_NET_ERR waitEspAck(_ESP_RE *esp,uint16_t ms)
{
	uint16_t offset,i=0,j=0;
	uint8_t num=0;
	if(esp->endStrNum==0)return NET_ERR;
	if(esp->endStrNum>=MAX_END_STR)return NET_ERR;
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
	if(i>=ms/100)return NET_TIME_OUT;
	else return NET_OK;
}


/*
0 ok
1 timeout
*/
_NET_ERR ESP_sendMsg(char *msg,_ESP_RE *esp)
{
	uint8_t i=0;
	do{
	clearRxbuf();
	Ac_pf3(msg);
	if(NET_OK==waitEspAck(esp,esp->ms))return NET_OK;
	}while(i++<esp->cnt);
	return NET_TIME_OUT;
}

_NET_ERR ESP_mode_set()
{
	uint16_t offset;
	uint8_t num,mode=0;
	_ESP_RE re;
	_NET_ERR err=NET_OK;
	re.endStrNum=1;
	re.cnt=3;
	re.ms=500;
	strcpy(re.endStr[0].res,"OK");
	err=ESP_sendMsg("AT+CWMODE?\r\n",&re);
	if(err==NET_OK)
	{
		num=1;
		if(AC_OK==str_find(RX_buffer,"+CWMODE:",&offset,&num))
		{
			mode = RX_buffer[offset+8]-0x30;
			if(mode==1)err = NET_OK;
			else
			{
				re.endStrNum=1;
				re.cnt=3;
				re.ms=200;
				strcpy(re.endStr[0].res,"+CWMODE:1");
				ESP_sendMsg("AT+CWMODE=1\r\n",&re);
				err = NET_ACK_ERR;
			}
		}
		else err = NET_ACK_ERR;
	}
	return err;
}

_NET_ERR ESP_CIPMUX()
{
	
	_ESP_RE re;
	_NET_ERR err=NET_OK;
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
_NET_ERR ESP_ssid_key_set()
{
	char *tbuf=NULL;
	_ESP_RE re;
	_NET_ERR err=NET_OK;
	tbuf = mypvPortMalloc(128);
	sprintf(tbuf,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",SSID,KEY);
	re.endStrNum=2;
	re.cnt=3;
	re.ms=20000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	strcpy(re.endStr[1].res,"FAIL");
	err=ESP_sendMsg(tbuf,&re);
	if(err==NET_OK)
	{
		if(re.result==0)err = NET_OK;	
		else err = NET_ACK_ERR;		
	}
	myvPortFree(tbuf);
	return err;
}

_NET_ERR ESP_AT_reset()
{
	_ESP_RE re;
	_NET_ERR err=NET_OK;
	re.endStrNum=1;
	re.cnt=1;
	re.ms=8000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"ready");
	err = ESP_sendMsg("AT+RST\r\n",&re);
	return err;
}

/**/
_NET_ERR ESP_creat_TCP_socket()
{
	char *tbuf=NULL;
	_ESP_RE re;
	_NET_ERR err=NET_OK;
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
_NET_ERR ESP_into_transMode()
{
	_ESP_RE re;
	re.endStrNum=1;
	re.cnt=2;
	re.ms=1000;
	re.result=0xff;
	strcpy(re.endStr[0].res,"OK");
	return ESP_sendMsg("AT+CIPMODE=1\r\n",&re);
	
}

_NET_ERR ESP_start_trans()
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
_NET_STA net_sta;
void ESP_init()
{
	ESP_exit_TransMode();
	clearRxbuf();
	UART3_Callback = getDataSetMode;
	StartUART_Rx(&huart3);
	if(net_sta.net_en==0)
	{
		if( SSID[0]==0 || KEY[0]==0)net_sta.localSta=0;		
		else net_sta.localSta=1;		
		net_sta.linkNum=0;
	}
	/**/
	mode = 0;
	re:
	if(0!=ESP_AT_reset())goto re;
	
	if(0!=ESP_mode_set())goto re;
	
	if(0!=ESP_ssid_key_set())goto re;
	
	if(0!=ESP_CIPMUX())goto re;
	
	if(0!=ESP_creat_TCP_socket())goto re;
	
	if(0!=ESP_into_transMode())goto re;
	
	if(0!=ESP_start_trans())goto re;
	
	_Log("net ok");
	clearRxbuf();
	UART3_Callback = getTransMode;
	mode = 1;
}
