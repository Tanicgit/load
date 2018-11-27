#ifndef __COMM_H
#define __COMM_H
#include "stm32f4xx_hal.h"
#pragma  pack(1)
typedef struct{
	uint16_t 	len;
	uint16_t 	mid;
	uint8_t		dir;//
	uint16_t	ver;
	uint32_t	addr;
	uint8_t		msg;
}_MSG;

typedef struct{
	_MSG head;
	uint16_t loadsta;
	uint8_t sum;
}_MSG01_t;
typedef struct{
	_MSG head;
	uint16_t ver;
	uint8_t  upTp;
	uint32_t fileSize;
	uint16_t packSize;	
	uint16_t  crc16;
	uint8_t sum;
}_MSG01_r;


typedef struct{
	_MSG head;
	uint16_t packId;
	uint16_t packSize;
	uint8_t sum;
}_MSG02_t;

#define MSG02_R_OFF	6
#define BIN_PACK_MAX_SIZE	2048
#if (BIN_PACK_MAX_SIZE<128)|(BIN_PACK_MAX_SIZE>2048) 
	#error "BIN_PACK_NAX_SIZE"
#endif
#define COMM_RX_BUF_SIZE	2*(sizeof(_MSG02_r)+BIN_PACK_MAX_SIZE+2)
typedef struct{
	_MSG head;
	uint16_t packId;
	uint16_t packSize;
	uint16_t ver; 
//	uint8_t  *binData;
//	uint8_t 	sum;
}_MSG02_r;
#pragma  pack()

typedef enum{
	CM_ERR=-1,/*��ڲ�������*/
	CM_OK,
	CM_RAM,/*RAM����*/
	CM_FILE,/*�ļ��������� �򿪶�д��*/
	CM_DATA,/*���ݴ���,�ļ���������ݺ�Ԥ�ڲ�ͬ*/
	CM_PACK,/*������ ���� У��� ��*/
	CM_NOACK_TIME_OUT,/*��ʱ����Ӧ*/
	CM_NONE,/*�����ڴ��Ľ��*/
	CM_RELOAD,/*��Ҫ��������*/
	CM_COPY/*���ƴ���*/
}_CERR;


void CommTask(void);

#endif 
