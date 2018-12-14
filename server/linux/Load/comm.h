#ifndef COMM_H
#define COMM_H

#include <QtCore>
class comm
{
    #define     APP_ERR     -1 /*参数错误  数据不符合协议 未初始化等 不返回*/
    #define     APP_OK      0 /*正常  返回正常数据*/
    #define     APP_DATA    1/*数据包 符合协议  但是  数据有错误*/
    #define     APP_UNKNOW  2/*符合协议  消息类型未知 */
    #pragma pack(1)
    typedef struct {
        uint16_t 	len;
        uint16_t 	mid;
        uint8_t		dir;
        uint16_t	ver;
        uint32_t	addr;
        uint8_t		msg;
    }_MSG;

    typedef struct{
        _MSG head;
        uint16_t loadsta;
    }_MSG01_r;
    typedef struct{
        _MSG head;
        uint16_t ver;
        uint8_t  upTp;
        uint32_t fileSize;
        uint16_t packSize;
        uint16_t  crc16;
    }_MSG01_t;

    typedef struct{
        _MSG head;
        uint16_t packId;
        uint16_t packSize;
    }_MSG02_r;

    typedef struct{
        _MSG head;
        uint16_t packId;
        uint16_t packSize;
        uint16_t ver;
    }_MSG02_t;

    /*错误响应*/
    typedef struct{
        _MSG head;
        uint8_t msgId;
        uint8_t errId;
    }_MSG7f_t;
    #pragma pack()
public:
    comm();
    ~comm();
    QByteArray process(const QByteArray &data);
    bool HexStr2Bins(uchar *HexStr, uint16_t StrLen, uint8_t *re);
    uchar _09_af_AF_ToBin(char a, bool *ok);
    uint8_t U8checkSum(uint8_t *a,uint8_t len);
    uint16_t Crc16_Modbus(uint8_t *puchMsg,uint16_t usDataLen);
    QByteArray MsgPacked(uint8_t *a,uint16_t len);
    char BinTo_09_AF(uint8_t a);
    QByteArray ErrAck(int e, uint8_t *data);

    bool updataBinFile(QByteArray bindata, uint16_t ver, uint32_t size, uint16_t pSize);
private:
    int MsgPro(uint8_t *data,uint16_t len,QByteArray *re);
    uint16_t binVersion;
    uint32_t binSize;
    uint16_t packSize;
    uint16_t packNum;
    uint8_t  *binFileBuf=NULL;
};

#endif // COMM_H
