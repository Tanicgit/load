#include "stm32f4xx_hal.h"
#include "flash.h"
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
 if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else
  {
    sector = FLASH_SECTOR_7;  
  }
  return sector;
}

uint16_t Flash_If_Erase(uint32_t sector)
{
  uint32_t startsector = 0, sectorerror = 0;
  
  /* Variable contains Flash operation status */
  HAL_StatusTypeDef status;
  FLASH_EraseInitTypeDef eraseinitstruct;
  
  /* Get the number of sector */
  startsector = sector;
  eraseinitstruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  eraseinitstruct.Banks = FLASH_BANK_1;
  eraseinitstruct.Sector = startsector;
  eraseinitstruct.NbSectors = 1;
  eraseinitstruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  status = HAL_FLASHEx_Erase(&eraseinitstruct, &sectorerror);
  
  if (status != HAL_OK)
  {
    return 1;
  }
  return 0;
}

uint16_t Flash_If_Write(uint8_t *src, uint32_t dest, uint32_t Len)
{
  uint32_t i = 0;
  
  for(i = 0; i < Len; i+=4)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by byte */ 
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest+i, *(uint32_t*)(src+i)) == HAL_OK)
    {
     /* Check the written value */
      if(*(uint32_t *)(src + i) != *(uint32_t*)(dest+i))
      {
        /* Flash content doesn't match SRAM content */
        return 2;
      }
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return 1;
    }
  }
  return 0;
}

/*
3*16=48K¿Õ¼ä boot
*/
uint8_t EraseSpace(uint32_t size)
{
	uint32_t Saddr = ADDR_FLASH_SECTOR_2;
	uint32_t Eaddr = ADDR_FLASH_SECTOR_2+size;
	uint32_t sector1,sector2;
	sector1 = GetSector(Saddr);
	sector2 = GetSector(Eaddr);
	for(int i=sector1;i<sector2+1;i++)
	{
		if(0!=Flash_If_Erase(i))return 1;
	}
	return 0;
}
