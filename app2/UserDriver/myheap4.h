#ifndef __MYHEAP4_H
#define __MYHEAP4_H
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

void *mypvPortMalloc( size_t xWantedSize );
void myvPortFree( void *pv );
size_t myxPortGetFreeHeapSize( void );
size_t myxPortGetMinimumEverFreeHeapSize( void );
void myvPortInitialiseBlocks( void );
#endif
