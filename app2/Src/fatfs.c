/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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

#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */
#include "main.h"
#include "string.h"
#include "myheap4.h"
static uint8_t buffer[_MAX_SS]; /* a work buffer for the f_mkfs() */
char filename[256];
void fileErrProcess(uint8_t a)
{
	_Log(FRESULT_err[a]);
	if(a==0x0d)
	{
		_Log("f_mkfs ");
		if(f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, buffer, sizeof(buffer)) != FR_OK)
		{
			/* FatFs Format Error */
			_Log("err");
			Error_Handler();
		}
		_Log("ok");
	}
}

uint8_t fatfsTeat(FATFS *FatFs,char *Path,FIL *MyFile,char *fname)
{
  FRESULT res;                                          /* FatFs function common result code */
  uint32_t byteswritten, bytesread;                     /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
  uint8_t rtext[100]; 
	 
	_Log("f_mount \"%s\"",Path);
	if((res=f_mount(FatFs, (TCHAR const*)Path, 0)) != FR_OK)
	{
		/* FatFs Initialization Error */
		fileErrProcess(res);
		return 1;
	}
	else
	{
		/*##-4- Create and Open a new text file object with write access #####*/
		sprintf(filename,"%s%s",Path,fname);
		if((res=f_open(MyFile, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
		{
			/* 'STM32.TXT' file Open for write Error */
			fileErrProcess(res);
			return 1;
		}
		else
		{
			/*##-5- Write data to the text file ################################*/
			_Log("f_write:");
			res = f_write(MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
			_Log("%s",wtext)
			if((byteswritten == 0) || (res != FR_OK))
			{
				/* 'STM32.TXT' file Write or EOF Error */
				return 1;
			}
			else
			{
				/*##-6- Close the open text file #################################*/
				f_close(MyFile);
				
				/*##-7- Open the text file object with read access ###############*/
				if(f_open(MyFile, filename, FA_READ) != FR_OK)
				{
					/* 'STM32.TXT' file Open for read Error */
					return 1;
				}
				else
				{
					/*##-8- Read data from the text file ###########################*/
					_Log("f_read:");
					res = f_read(MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);
					
					if((bytesread == 0) || (res != FR_OK))
					{
						/* 'STM32.TXT' file Read or EOF Error */
						return 1;
					}
					else
					{
						/*##-9- Close the open text file #############################*/
						f_close(MyFile);
						
						/*##-10- Compare read data with the expected data ############*/
						if ((bytesread != byteswritten))
						{                
							/* Read data is different from the expected data */
							return 1;
						}
						else
						{
							_Log("%s",rtext);
							/* Success of the demo: no error occurrence */
							
						}
					}
				}
			}
		}
		
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//文件复制 b 复制 到 a
uint8_t exf_copy(char *a,char *b,uint8_t fwmode)
{
	uint8_t res=0;
  uint16_t br=0;
	uint16_t bw=0;
	FIL *fa=0;
	FIL *fb=0;
	uint8_t *fbuf=0;

 	fa=(FIL*)mypvPortMalloc(sizeof(FIL));//申请内存
 	fb=(FIL*)mypvPortMalloc(sizeof(FIL));
	fbuf=(uint8_t*)mypvPortMalloc(4096);
  if(fa==NULL||fb==NULL||fbuf==NULL)res=100;//前面的值留给fatfs
	else
	{   
		if(fwmode==0)fwmode=FA_CREATE_NEW;//不覆盖
		else fwmode=FA_CREATE_ALWAYS;	  //覆盖存在的文件
		 
	 	res=f_open(fb,b,FA_READ|FA_OPEN_EXISTING);	//打开只读文件
	 	if(res==0)res=f_open(fa,a,FA_WRITE|fwmode); 	//第一个打开成功,才开始打开第二个
		if(res==0)//两个都打开成功了
		{
			while(res==0)//开始复制
			{
				res=f_read(fb,fbuf,4096,(UINT*)&br);	//源头读出512字节
				if(res||br==0)break;
				res=f_write(fa,fbuf,(UINT)br,(UINT*)&bw);	//写入目的文件					     
				if(res||bw<br)break;       
			}
		  f_close(fa);
		  f_close(fb);
		}
	}
	myvPortFree(fa);//释放内存
	myvPortFree(fb);
	myvPortFree(fbuf);
	return res;
}


uint8_t SD_sta = 0;
/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
	if(retSD==0){
		if(0!=fatfsTeat(&SDFatFS,SDPath,&SDFile,"TEST.txt"))
		{
			SD_sta = 1;
		}
	}
//	if(retUSER==0){
//		fatfsTeat(&USERFatFS,USERPath,&USERFile,"TEST.txt");
//	}
  /* additional user code for init */ 
	_Log("");
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC 
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */  
}

/* USER CODE BEGIN Application */
 char *FRESULT_err[] = {
"(0) Succeeded",
"(1) A hard error occurred in the low level disk I/O layer",
"(2) Assertion failed",
"(3) The physical drive cannot work",
"(4) Could not find the file",
"(5) Could not find the path",
"(6) The path name format is invalid ",
"(7) Access denied due to prohibited access or directory full ",
"(8) Access denied due to prohibited access ",
"(9) The file/directory object is invalid ",
"(10) The physical drive is write protected ",
"(11) The logical drive number is invalid ",
"(12) The volume has no work area ",
"(13) There is no valid FAT volume ",
"(14) The f_mkfs() aborted due to any problem ",
"(15) Could not get a grant to access the volume within defined period ",
"(16) The operation is rejected according to the file sharing policy ",
"(17) LFN working buffer could not be allocated ",
"(18) Number of open files > _FS_LOCK ",
"(19) Given parameter is invalid "
};
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
