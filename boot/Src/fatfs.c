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
#include "string.h"
//static uint8_t buffer[_MAX_SS]; /* a work buffer for the f_mkfs() */
static uint8_t buffer[_MAX_SS]; 
char filename[256];
void fileErrProcess(uint8_t a)
{
	if(a==0x0d)
	{
		if(f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, buffer, sizeof(buffer)) != FR_OK)
		{
			/* FatFs Format Error */
		}
	}
}
uint8_t fatfsTeat(FATFS *FatFs,char *Path,FIL *MyFile,char *fname)
{
  FRESULT res;                                          /* FatFs function common result code */
  uint32_t byteswritten, bytesread;                     /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
  uint8_t rtext[100]; 
	 
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
			return 1;
		}
		else
		{
			/*##-5- Write data to the text file ################################*/
			res = f_write(MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
			if((byteswritten == 0) || (res != FR_OK))
			{
				/* 'STM32.TXT' file Write or EOF Error */
				f_close(MyFile);
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
							/* Success of the demo: no error occurrence */
							
						}
					}
				}
			}
		}
		
	}
	return 0;
}
/* USER CODE END Variables */    
uint8_t SD_STATUS=0;
void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
	if(retSD==0){
		if(0!=fatfsTeat(&SDFatFS,SDPath,&SDFile,"TEST.txt"))
		{
			SD_STATUS = 1;
		}
	}
  /* additional user code for init */     
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
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
