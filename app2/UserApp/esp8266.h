#ifndef __NET_H
#define __NET_H
#include "stdint.h"
typedef enum{
	NET_ERR=-1,/*入口参数非法*/
	NET_OK,
	NET_TIME_OUT,/*超时无响应*/
	NET_ACK_ERR,/*有非期待的响应*/
}_NET_ERR;

typedef struct{
	uint8_t localSta;//本地网络  0未配置参数  1初始化连接中  2连接上 3重连中 0xff禁止
	uint8_t linkNum;//远端链接数	
	uint8_t linkSta[5];//连接状态 最多5个链接
											//0未配置参数 1初始化连接中 2连接上 3重连中 0xff禁止
}_NET_STA;



#endif