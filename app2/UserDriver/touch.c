#include "touch.h"
#include "24cxx.h"

//Ĭ��Ϊtouchtype=0������
uint8_t CMD_YPYN_XP=0X90;//YP + YN -   XP����   ���Ϊ Y����ADC
uint8_t CMD_XPXN_YP=0XD0;//XP + XN -   YP����   ���Ϊ X����ADC

//SPIд����
//������ICд��1byte����    
//num:Ҫд�������
void TP_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN_1;  
		else TDIN_0;   
		num<<=1;    
		TCLK_0; 	 
		TCLK_1;		//��������Ч	        
	}		 			    
}

//SPI������ 
//�Ӵ�����IC��ȡadcֵ
//CMD:ָ��
//����ֵ:����������	   
uint16_t TP_Read_AD(uint8_t CMD)	  
{ 	 
	uint8_t count=0; 	  
	uint16_t Num=0; 
	TCLK_0;		//������ʱ�� 	 
	TDIN_0; 	//����������
	TCS_0; 		//ѡ�д�����IC
	TP_Write_Byte(CMD);//����������
	delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
	TCLK_0; 	     	    
	delay_us(1);    	   
	TCLK_1;		//��1��ʱ�ӣ����BUSY
	delay_us(1);    
	TCLK_0; 	     	    
	for(count=0;count<16;count++)//����16λ����,ֻ�и�12λ��Ч 
	{ 				  
		Num<<=1; 	 
		TCLK_0;	//�½�����Ч  	    	   
		delay_us(1);    
 		TCLK_1;
 		if(DOUT_DATA_IN_1)Num++; 		 
	}  	
	Num>>=4;   	//ֻ�и�12λ��Ч.
	TCS_1;		//�ͷ�Ƭѡ	 
	return(Num);   
}

//��ȡһ������ֵ(x����y)
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ 
//xy:ָ�CMD_RDX/CMD_RDY��
//����ֵ:����������
#define READ_TIMES 5 	//��ȡ����
#define LOST_VAL 1	  	//����ֵ
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//����
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//��������
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
	//if(xtemp<100||ytemp<100)return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}

#define ERR_RANGE 50 //��Χ 
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}

//������EEPROM����ĵ�ַ�����ַ,ռ��13���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)

/*4����*/
_LogPhy px0={0,155};
_LogPhy px1={240,3903};
_LogPhy py0={0,188};
_LogPhy py1={320,3935};

//����У׼����										    
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
	temp=0X0A;//���У׼����
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
	tempfac=AT24CXX_ReadOneByte(TOUCH_SAVE_ADDR_BASE+16);//��ȡ�����,���Ƿ�У׼���� 		 
	if(tempfac==0X0A)//�������Ѿ�У׼����			   
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



