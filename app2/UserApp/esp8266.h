#ifndef __NET_H
#define __NET_H
#include "stdint.h"
typedef enum{
	NET_ERR=-1,/*��ڲ����Ƿ�*/
	NET_OK,
	NET_TIME_OUT,/*��ʱ����Ӧ*/
	NET_ACK_ERR,/*�з��ڴ�����Ӧ*/
}_NET_ERR;

typedef struct{
	uint8_t localSta;//��������  0δ���ò���  1��ʼ��������  2������ 3������ 0xff��ֹ
	uint8_t linkNum;//Զ��������	
	uint8_t linkSta[5];//����״̬ ���5������
											//0δ���ò��� 1��ʼ�������� 2������ 3������ 0xff��ֹ
}_NET_STA;



#endif