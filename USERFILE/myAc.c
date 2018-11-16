#include "myAc.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "myAcData.h"

#include "usart.h"
static char buffer[256];
int Ac_pf(char *format,...)
{
	va_list aptr;
	int ret;
	va_start(aptr,format);
	ret = vsprintf(buffer, format, aptr);
  va_end(aptr);
	HAL_UART_Transmit(&huart1,(uint8_t*)buffer,strlen(buffer),0xffff);
  return(ret);
}

uint16_t Crc16_ModbusFarst(uint8_t *puchMsg,uint16_t usDataLen)
{
	uint8_t ucCRCHi = 0xFF;
	uint8_t ucCRCLo = 0xFF;
	int iIndex;
	while( usDataLen-- )
	{
			iIndex = ucCRCLo ^ *( puchMsg++ );
			ucCRCLo = ( uint8_t )( ucCRCHi ^ aucCRCHi[iIndex] );
			ucCRCHi = aucCRCLo[iIndex];
	}
	return ( uint16_t )( ucCRCHi << 8 | ucCRCLo );	
	
}
uint16_t Crc16_Modbus(uint8_t *puchMsg,uint16_t usDataLen)
{
	uint16_t i=0;
	uint16_t j=0;
	uint16_t crc16=0xFFFF;
	for (i = 0; i < usDataLen; i++)
	{
		crc16 ^= puchMsg[i];
		for (j = 0; j < 8; j++)
		{		
			if ((crc16 & 0x01) == 1)
			{
				crc16 = (crc16 >> 1) ^ 0xA001;
			}
			else
			{
				crc16 = crc16 >> 1;
			}
		}
	}
	return crc16;	
}


uint8_t U8checkSum(uint8_t *a,uint8_t len)
{
	uint8_t i=0,sum=0;
	for(i=0;i<len;i++)
	{
		sum += a[i];
	}
	return sum;
}


/*************************
0x30   0x0
0x41   0xa
0x61   0xa
*************************/
uint8_t NumChar2Bin(char a)
{
	
	if(a>=0x30 && a<=0x39)
	{
		return a-0x30;
	}
	if(a>0x40&&a<0x47)
	{
		return a-0x41 + 10;
	}
	if(a>0x60&&a<0x67)
	{
		return a-0x61 +10;
	}
	return 0xff;
}

char Bin2HexChar(uint8_t a)
{
	if(a<10)return a+0x30;
	if(a>=10 ||a<16) return (a-10)+0x41;
	return 0;
}
/*
bits = 1 2 3 4 5 6 7 8 
*/
_myAc Bin2HexStr(uint32_t bin,uint8_t bits,char* ret)
{
	int i=0;
	if(bits==0||bits>8)return AC_ERR;
	for(i=0;i<bits;i++)
	{
		if((ret[i] = Bin2HexChar(bin>>((bits-i+1)*4))&0xf)==0)return AC_ERR;
	}
	return AC_OK;
}

/*
bits = 1 2 3 4 5 6 7 8 
*/
_myAc HexStr2Bin(char *a ,uint8_t bits,void *ret)
{
	int i;
	uint32_t temp=0;
	uint8_t t;
	for(i=0;i<bits;i++)
	{
		t = NumChar2Bin(a[i]);
		if(t==0xff)return AC_ERR;
		temp |= t<<(i*4); 
	}
	return AC_OK;
}


/*
*source 目标串
*match 匹配串
*c 匹配结果  每一个结果在*a中的偏移地址
*d 期待匹配的结果个数   实际匹配的个数

0 匹配成功

*/
_myAc str_find(char *source, char *match,uint16_t *c,uint8_t *d)
{
	uint16_t i,j,flag=0,m=0;
	uint16_t lena  = strlen(source);
	uint16_t lenb  = strlen(match);
	for(i=0;i<lena;i++)
	{
		   if(match[0]==source[i] && (i+lenb<=lena))
			 {
					for(j=0;j<lenb;j++)
				 {				  
						if(match[j]!=source[i+j] && match[j]!='?')
						{
							flag=1;
							break;
						}										
				 }
				 if(flag==0)
				 {
						c[m++]=i;
					  if(m>*d)break;
				 }
				 else
				 {
						flag = 0;
				 }
			 }
	}
	*d=m-1;
	if(m>0)return AC_OK;
	else return AC_ERR;
}



/*
字符串分割 不可重入
返回的_stringList 用完后要调用 FreeStringList释放
*/
_stringList *mtStrDiv(char *a, char *b)
{
	char *c,*d[LIST_MAX_NUM];
	_stringList *list;
	
	uint8_t i=0;
	c = strtok(a,b);
	while(c!=NULL)
	{
		d[i] = malloc(strlen(c));	
		if(d[i]==NULL)return NULL;
		strcpy(d[i],c);
		c = strtok(NULL,b);
		i++;
		if(i==LIST_MAX_NUM)break;
	}
	if(i==0)return NULL;
	list=malloc(sizeof(_stringList));
	if(list==NULL)return NULL;
	list->num=i;
	memcpy(list->d,d,sizeof(char*)*i);
	return list;
}


void FreeStringList(_stringList *list)
{
	uint8_t i=0;
	for(i=0;i<list->num;i++)
	free(list->d[i]);
	free(list);
}

static uint32_t fifo_size=0;
static uint8_t *PAfifo;
static uint16_t tail=0;
static uint16_t head=0;
void fifo_init(uint8_t *p,uint32_t size)
{
	fifo_size = fifo_size;
	tail = 0;
	head = fifo_size-1;
	PAfifo = p;
}
int8_t in(uint8_t a)
{
	uint16_t p;
	p=tail;
	if(p==head)return -1;
	else
	{
		PAfifo[p] = a;
		tail = p<fifo_size-1 ? p+1 : 0;
		return 0;
	}
}
uint8_t Fifoout(uint8_t *a)
{
	uint16_t p;
	p= head;
	p= p<fifo_size-1 ? p+1:0;
	if(p==tail)
	{
		return 1;
	}
	else
	{
		*a = PAfifo[p];
		head = p;
		return 0;
	}
}
