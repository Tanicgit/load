#ifndef __LOAD_H
#define __LOAD_H
#include "stm32f4xx_hal.h"

#pragma  pack(1) 
typedef struct{
	uint8_t  sta;//0û������ 2���ذ���  3�ϳɰ��� 4�����ļ�׼�� 5app֪ͨboot�������� 
							//6boot֪ͨapp���������ɹ�   7app֪ͨboot�ָ������̼�  8 boot֪ͨapp�ָ������̼��ɹ�
	uint8_t  nowBin;//��ǰ���е�bin�ļ�  1.bin  2.bin  �ϵ����� ���sta==3 ���л��̼����޸ı�λ
	uint16_t	nowVersion;//��ǰ���еİ汾 
	uint16_t	VersionToLoad;//׼�����صİ汾
	uint8_t	 bootErrCnt;//�°汾�Ѿ�boot��ʧ�ܴ��� ����һ������sta��Ϊ0 boot������+1
	uint32_t FileSize;
	uint16_t FilePackNum;
	uint16_t PackSize;
	uint16_t lastPackSize;
	uint8_t  packListSum;//pack ���ļ���У���
	uint16_t  crc16;
	uint8_t  sum;//���ṹ���ļ�У���
}_LOAD;

#pragma  pack() 
struct listPack{
	uint8_t id;//˳��
	uint8_t sta;//״̬0δ���� ������
	uint8_t checkSum;//�ð�bin��У���
	uint16_t size;//�ð��Ĵ�С
	struct listPack *next;
};
extern struct listPack *packHead;
uint8_t NeedReLoad(void);
uint8_t savePackInfo(void);
uint8_t readPackInfo(void);
uint8_t saveLoadInfo(void);
void readLoadInfo(void);
void returnDefaultBin(void);
void loadTask(void);
void loadInit(void);
extern _LOAD load;
#endif 
