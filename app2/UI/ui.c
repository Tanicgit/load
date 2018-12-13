
#include "ui.h"
#include "lcd9325.h"
#include "touch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#define  R	3
#define  D	10
GUI_POINT  logPoint[4]={
	{D,D},
	{USER_LCD_X_SIZE-D,D},
	{D,USER_LCD_Y_SIZE-D},
	{USER_LCD_X_SIZE-D,USER_LCD_Y_SIZE-D},
};
GUI_RECT logRect[4];
GUI_POINT phyPoint[4];

extern _LogPhy px0;
extern _LogPhy px1;
extern _LogPhy py0;
extern _LogPhy py1;
static char str[32];
void TP_Adjust()
{
	uint8_t cnt=0,tp_down=0,i,tp=0,time=0;
	uint32_t l1=0,l2=0;
	double k;
	for(i=0;i<4;i++)
	{
		logRect[i].x0 = logPoint[i].x-R;
		logRect[i].y0 = logPoint[i].y-R;
		logRect[i].x1 = logPoint[i].x+R;
		logRect[i].y1 = logPoint[i].y+R;
	}
	
	GUI_SetBkColor(GUI_WHITE);
	GUI_SetColor(GUI_RED);
	re:
	
	if(time!=0)
	{
		sprintf(str,"err %d",time);
		GUI_DispStringHCenterAt(str,120,25);
		osDelay(1000);
		if(time>5)return;
		GUI_DispStringHCenterAt("EXIT",120,45);
		osDelay(1000);
	}
	time++;
	GUI_Clear();
	cnt=0;tp_down=0;tp=0;
	GUI_DispStringHCenterAt("TOUCH CALIBRATW",120,5);
	GUI_FillCircle(logPoint[cnt].x,logPoint[cnt].y,R);
	while(1)
	{
		osDelay(50);
		if(GPIO_PIN_RESET==PE_DATA_IN_1)
		{
				if(tp_down==0)
				{				
					phyPoint[cnt].x = GUI_TOUCH_GetxPhys();
					phyPoint[cnt].y = GUI_TOUCH_GetyPhys();						
					if(phyPoint[cnt].x!=0&&phyPoint[cnt].y!=0)tp_down=1;
				}
				else
				{
					
				}
		}
		else
		{
				if(tp_down==1)
				{
					tp_down=0;
					tp=1;

				}
		}
		if(tp==1)
		{
			  tp=0;
				cnt++;
				switch(cnt)
				{
					case 1:
					case 2:
					case 3:
						GUI_ClearRectEx(&logRect[cnt-1]);
						GUI_FillCircle(logPoint[cnt].x,logPoint[cnt].y,R);	
						sprintf(str,"x Log:%03d Phy:%04d",logPoint[cnt-1].x,phyPoint[cnt-1].x);
						GUI_DispStringAt(str,5,50);
						sprintf(str,"y Log:%03d Phy:%04d",logPoint[cnt-1].y,phyPoint[cnt-1].y);
						GUI_DispStringAt(str,5,70);
						break;
					case 4:
						GUI_ClearRectEx(&logRect[cnt-1]);
						sprintf(str,"x Log:%03d Phy:%04d",logPoint[cnt-1].x,phyPoint[cnt-1].x);
						GUI_DispStringAt(str,5,50);
						sprintf(str,"y Log:%03d Phy:%04d",logPoint[cnt-1].y,phyPoint[cnt-1].y);
						GUI_DispStringAt(str,5,70);
					
						l1=myAbs(phyPoint[0].x,phyPoint[1].x);
						l2=myAbs(phyPoint[2].x,phyPoint[3].x);
						if(l1==0||l2==0)goto re;
						k=(float)l1/l2;
						if(k<0.97 ||k>1.03)goto re;
					
						l1=myAbs(phyPoint[0].y,phyPoint[2].y);
						l2=myAbs(phyPoint[1].y,phyPoint[3].y);
						if(l1==0||l2==0)goto re;
						k=(float)l1/l2;
						if(k<0.97 ||k>1.03)goto re;
						
						
					
						px0.Log = logPoint[0].x;
						px0.Phy = phyPoint[0].x;
						px1.Log = logPoint[1].x;
						px1.Phy = phyPoint[1].x;
						
						py0.Log = logPoint[0].y;
						py0.Phy = phyPoint[0].y;
						py1.Log = logPoint[2].y;
						py1.Phy = phyPoint[2].y;
					
//						GUI_TOUCH_Calibrate(GUI_COORD_X,px0.Log,px1.Log,px0.Phy,px1.Phy);
//						GUI_TOUCH_Calibrate(GUI_COORD_Y,py0.Log,py1.Log,py0.Phy,py1.Phy);
					
						TP_Save_Adjdata();
						
						GUI_DispStringHCenterAt("OK",120,25);
						GUI_DispStringHCenterAt("EXIT",120,45);
						osDelay(1000);
						return;
				}
		}
		
	}
}
