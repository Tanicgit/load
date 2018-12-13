#include "touch.h"
#include "24cxx.h"

//默认为touchtype=0的数据
uint8_t CMD_YPYN_XP=0X90;//YP + YN -   XP测量   结果为 Y坐标ADC
uint8_t CMD_XPXN_YP=0XD0;//XP + XN -   YP测量   结果为 X坐标ADC

//SPI写数据
//向触摸屏IC写入1byte数据    
//num:要写入的数据
void TP_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN_1;  
		else TDIN_0;   
		num<<=1;    
		TCLK_0; 	 
		TCLK_1;		//上升沿有效	        
	}		 			    
}

//SPI读数据 
//从触摸屏IC读取adc值
//CMD:指令
//返回值:读到的数据	   
uint16_t TP_Read_AD(uint8_t CMD)	  
{ 	 
	uint8_t count=0; 	  
	uint16_t Num=0; 
	TCLK_0;		//先拉低时钟 	 
	TDIN_0; 	//拉低数据线
	TCS_0; 		//选中触摸屏IC
	TP_Write_Byte(CMD);//发送命令字
	delay_us(6);//ADS7846的转换时间最长为6us
	TCLK_0; 	     	    
	delay_us(1);    	   
	TCLK_1;		//给1个时钟，清除BUSY
	delay_us(1);    
	TCLK_0; 	     	    
	for(count=0;count<16;count++)//读出16位数据,只有高12位有效 
	{ 				  
		Num<<=1; 	 
		TCLK_0;	//下降沿有效  	    	   
		delay_us(1);    
 		TCLK_1;
 		if(DOUT_DATA_IN_1)Num++; 		 
	}  	
	Num>>=4;   	//只有高12位有效.
	TCS_1;		//释放片选	 
	return(Num);   
}

//读取一个坐标值(x或者y)
//连续读取READ_TIMES次数据,对这些数据升序排列,
//然后去掉最低和最高LOST_VAL个数,取平均值 
//xy:指令（CMD_RDX/CMD_RDY）
//返回值:读到的数据
#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//排序
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//升序排列
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(0XD0);
	ytemp=TP_Read_XOY(0X90);	  												   
	//if(xtemp<100||ytemp<100)return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}

#define ERR_RANGE 50 //误差范围 
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}

//保存在EEPROM里面的地址区间基址,占用13个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)

/*4个点*/
_LogPhy px0={0,155};
_LogPhy px1={240,3903};
_LogPhy py0={0,188};
_LogPhy py1={320,3935};

//保存校准参数										    
void TP_Save_Adjdata(void)
{
	int temp;			 
	//X	   							  
  AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE,px0.Log,2);
	AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE+2,px0.Phy,2);
	AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE+4,px1.Log,2);
	AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE+6,px1.Phy,2);
	//Y
	AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE+8,py0.Log,2);
	AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE+10,py0.Phy,2);
	AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE+12,py1.Log,2);
	AT24CXX_WriteLenByte(TOUCH_SAVE_ADDR_BASE+14,py1.Phy,2);	
	temp=0X0A;//标记校准过了
	AT24CXX_WriteOneByte(TOUCH_SAVE_ADDR_BASE+16,temp); 
}

void ClearAdjdata()
{
	int temp;	
	temp=0;
	AT24CXX_WriteOneByte(TOUCH_SAVE_ADDR_BASE+16,temp);	
}
/**/
uint8_t TP_Get_Adjdata(void)
{					  
	int tempfac;
	tempfac=AT24CXX_ReadOneByte(TOUCH_SAVE_ADDR_BASE+16);//读取标记字,看是否校准过！ 		 
	if(tempfac==0X0A)//触摸屏已经校准过了			   
	{    												 
		px0.Log=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE,2);		   
		px0.Phy=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE+2,2);
		px1.Log=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE+4,2);
		px1.Phy=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE+6,2);
		
		py0.Log=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE+8,2);		   
		py0.Phy=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE+10,2);
		py1.Log=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE+12,2);
		py1.Phy=AT24CXX_ReadLenByte(TOUCH_SAVE_ADDR_BASE+14,2);	
		
		return 1;	 
	}
	return 0;
}	 



