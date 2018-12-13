/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "ff.h"
#include "comm.h"
#include "GUI.h"
#include "gpio.h"
#include "ui.h"
#include "touch.h"
#include "load.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static uint8_t phy_key1=0;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTaskHeartHandle;
osThreadId myTaskUiHandle;
osThreadId myTaskKeyHandle;
osThreadId myTaskTouchHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTaskHeart(void const * argument);
void StartTaskUi(void const * argument);
void StartTaskKey(void const * argument);
void StartTaskTouch(void const * argument);

extern void MX_FATFS_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityBelowNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTaskHeart */
  osThreadDef(myTaskHeart, StartTaskHeart, osPriorityHigh, 0, 128);
  myTaskHeartHandle = osThreadCreate(osThread(myTaskHeart), NULL);

  /* definition and creation of myTaskUi */
  osThreadDef(myTaskUi, StartTaskUi, osPriorityLow, 0, 2048);
  myTaskUiHandle = osThreadCreate(osThread(myTaskUi), NULL);

  /* definition and creation of myTaskKey */
  osThreadDef(myTaskKey, StartTaskKey, osPriorityNormal, 0, 256);
  myTaskKeyHandle = osThreadCreate(osThread(myTaskKey), NULL);

  /* definition and creation of myTaskTouch */
  osThreadDef(myTaskTouch, StartTaskTouch, osPriorityHigh, 0, 128);
  myTaskTouchHandle = osThreadCreate(osThread(myTaskTouch), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	ReadNetSet();
	_Log("APP:-----");
  /* USER CODE END RTOS_QUEUES */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for FATFS */
  MX_FATFS_Init();

  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for(;;)
  {
    osDelay(200);		
		CommTask();
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTaskHeart */
/**
* @brief Function implementing the myTaskHeart thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskHeart */
void StartTaskHeart(void const * argument)
{
  /* USER CODE BEGIN StartTaskHeart */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
		LED1_GPIO_Port->ODR ^= LED1_Pin;
  }
  /* USER CODE END StartTaskHeart */
}

/* USER CODE BEGIN Header_StartTaskUi */
/**
* @brief Function implementing the myTaskUi thread.
* @param argument: Not used
* @retval None
*/
static void _cbBkWindow(WM_MESSAGE* pMsg) 
{
	switch (pMsg->MsgId) 
	{
		case WM_PAINT: 
			{				
				GUI_SetBkColor(GUI_BLACK);
				GUI_SetColor(GUI_BLACK);
				GUI_Clear();
			} 
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}
/* USER CODE END Header_StartTaskUi */
void StartTaskUi(void const * argument)
{
  /* USER CODE BEGIN StartTaskUi */
	GUI_Init();
	WM_SetCreateFlags(WM_CF_MEMDEV);
	/* 使能控件的皮肤色 */
//	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
//	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
//	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
//	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
//	CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
//	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
//	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
//	SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
//	HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
//	RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
	
	/* 设置桌面窗口的回调函数 */
	WM_SetCallback(WM_HBKWIN, &_cbBkWindow);
	
	
//	TEXT_SetDefaultFont(&GUI_Font10_ASCII);
//	BUTTON_SetDefaultFont(&GUI_Font10_ASCII);
//	LISTVIEW_SetDefaultFont(&GUI_Font10_ASCII);
//	FRAMEWIN_SetDefaultFont(&GUI_Font10_ASCII);
//	HEADER_SetDefaultFont(&GUI_Font10_ASCII);
//	EDIT_SetDefaultFont(&GUI_Font10_ASCII);
//	DROPDOWN_SetDefaultFont(&GUI_Font10_ASCII);
//	CHECKBOX_SetDefaultFont(&GUI_Font10_ASCII);
	
	if(0==TP_Get_Adjdata())
	{
		TP_Adjust();
	}
	GUI_TOUCH_Calibrate(GUI_COORD_X,px0.Log,px1.Log,px0.Phy,px1.Phy);
	GUI_TOUCH_Calibrate(GUI_COORD_Y,py0.Log,py1.Log,py0.Phy,py1.Phy);
	/* 进入主界面 */
	CreateMainWindow();
	WM_HideWin(CreateKeyWindow());

  /* Infinite loop */
  for(;;)
  {
		GUI_Delay(50);
		if(phy_key1)
		{
			returnDefaultBin();
			phy_key1=0;
		}
  }
  /* USER CODE END StartTaskUi */
}

/* USER CODE BEGIN Header_StartTaskKey */
/**
* @brief Function implementing the myTaskKey thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskKey */
void StartTaskKey(void const * argument)
{
  /* USER CODE BEGIN StartTaskKey */
	uint8_t  cnt=0;
  /* Infinite loop */
  for(;;)
  {
    osDelay(20);	
		if(KEY_1)
		{
			osDelay(10);
			if(KEY_1)
			{
				if(cnt<50)cnt++;
				else phy_key1=1;
			}
		}	
  }
  /* USER CODE END StartTaskKey */
}

/* USER CODE BEGIN Header_StartTaskTouch */
/**
* @brief Function implementing the myTaskTouch thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskTouch */
void StartTaskTouch(void const * argument)
{
  /* USER CODE BEGIN StartTaskTouch */
  /* Infinite loop */
  for(;;)
  {
    osDelay(5);
		GUI_TOUCH_Exec();
  }
  /* USER CODE END StartTaskTouch */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
