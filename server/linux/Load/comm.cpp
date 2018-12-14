#include "comm.h"



comm::comm()
{

}

comm::~comm()
{
    delete binFileBuf;
}

QByteArray comm::process(const QByteArray &data)
{
    uchar da[2048],tp=0,bin[1024];
    uint16_t j=0;
    QByteArray msg;
    int err=APP_OK;
    int len=data.length();
    for(int i=0;i<len;i++)
    {
        if(data.at(i)=='\n')
        {
            tp=0;
        }
        if(tp==1)
        {
            da[j++] = data.at(i);
        }
        if(data.at(i)=='@')
        {
            tp=1;
        }
    }
    if(HexStr2Bins(da,j,bin))
    {
       if(bin[j/2-1]==U8checkSum(bin,j/2-1))
       {
           err = MsgPro(bin,j/2,&msg);
           if(err!=APP_ERR)
           {
               if(err!=APP_OK)
               {
                   msg = ErrAck(err,bin);
               }
           }
       }
    }
    return msg;
}

QByteArray comm::ErrAck(int e, uint8_t *data)
{
    _MSG7f_t msg7f_t;
    msg7f_t.head.len = sizeof(_MSG01_t)+1;//+1个 CheckSum长度
    msg7f_t.head.mid = ((_MSG01_r*)data)->head.mid;
    msg7f_t.head.dir = 0x81;
    msg7f_t.head.ver = ((_MSG01_r*)data)->head.ver;
    msg7f_t.head.addr = ((_MSG01_r*)data)->head.addr;
    msg7f_t.head.msg = 0x7f;
    msg7f_t.msgId = ((_MSG01_r*)data)->head.msg;
    msg7f_t.errId = e;
   return  MsgPacked((uint8_t*)&msg7f_t,sizeof(_MSG7f_t));
}

int comm::MsgPro(uint8_t *data,uint16_t len,QByteArray *re)
{

    _MSG *tHead = (_MSG*)data;

    if(len<sizeof(_MSG)) return APP_ERR;
    if(0!=(tHead->dir &0x80)) return APP_ERR;
    /*addr 合法检查 略*/
    switch(tHead->dir)
    {
        //订阅端
        case 1:
            switch(tHead->msg)
            {
                //检查更新
                case 1:
                    /*如果有更新,在这之前应该已经初始化好了需要的数据、指针等*/
                    if(len<sizeof(_MSG01_r)) return APP_ERR;
                    _MSG01_t msg01_t;
                    msg01_t.head.len = sizeof(_MSG01_t)+1;//+1个 CheckSum长度
                    msg01_t.head.mid = ((_MSG01_r*)data)->head.mid;
                    msg01_t.head.dir = 0x81;
                    msg01_t.head.ver = ((_MSG01_r*)data)->head.ver;
                    msg01_t.head.addr = ((_MSG01_r*)data)->head.addr;
                    msg01_t.head.msg = 0x01;
                    msg01_t.ver = binVersion;
                    if(msg01_t.head.ver!=binVersion && binFileBuf!=NULL)
                    {
                       msg01_t.upTp = 1;
                       msg01_t.fileSize = binSize;
                       msg01_t.packSize = packSize;
                       msg01_t.crc16 = Crc16_Modbus(binFileBuf,binSize);

                    }
                    else
                    {
                      msg01_t.upTp = 0;
                    }
                    *re = MsgPacked((uint8_t*)&msg01_t,sizeof(_MSG01_t));
                    return APP_OK;
                break;
                //下载更新
                case 2:
                    if(len<sizeof(_MSG02_r)) return APP_ERR;
                    uint16_t tsize=0;
                    _MSG02_t msg02_t;

                    if(((_MSG02_r*)data)->packId <packNum-1)
                    {
                        tsize = packSize;
                    }
                    else if(((_MSG02_r*)data)->packId == packNum-1)
                    {
                        tsize = binSize%packSize;
                    }
                    else
                    {
                        return APP_DATA;
                    }
                    uint8_t *msg02_buf = new uint8_t[tsize + sizeof(_MSG02_t)];

                    msg02_t.head.len = tsize + sizeof(_MSG02_t)+1;//+1个 CheckSum长度
                    msg02_t.head.mid = ((_MSG02_r*)data)->head.mid;
                    msg02_t.head.dir = 0x81;
                    msg02_t.head.ver = ((_MSG02_r*)data)->head.ver;
                    msg02_t.head.addr = ((_MSG02_r*)data)->head.addr;
                    msg02_t.head.msg = 0x02;

                    msg02_t.packId = ((_MSG02_r*)data)->packId;
                    msg02_t.packSize = tsize;
                    msg02_t.ver = binVersion;

                    memcpy(msg02_buf,&msg02_t,sizeof(_MSG02_t));
                    memcpy(msg02_buf+sizeof(_MSG02_t),binFileBuf+packSize*msg02_t.packId,tsize);
                    *re = MsgPacked(msg02_buf,tsize + sizeof(_MSG02_t));
                    delete msg02_buf;
                    return APP_OK;
                break;
            }
        break;
        //发布端
        case 2:

        break;

    }


    return APP_UNKNOW;
}

QByteArray comm::MsgPacked(uint8_t *a,uint16_t len)
{
    QByteArray re;
    uint8_t sum=0;
    re.append('@');
    for(int i=0;i<len;i++)
    {
      re.append(BinTo_09_AF((a[i]&0xf0)>>4));
      re.append(BinTo_09_AF(a[i]&0xf));
      sum += a[i];
    }
    re.append(BinTo_09_AF((sum&0xf0)>>4));
    re.append(BinTo_09_AF(sum&0x0f));
    re.append('\n');
    return re;
}


bool comm::updataBinFile(QByteArray bindata, uint16_t ver, uint32_t size, uint16_t pSize)
{
    if(size==0)return false;
    binVersion = ver;
    binSize = size;
    packSize = pSize;
    packNum = (binSize%packSize==0) ? (binSize/packSize) : (binSize/packSize +1);
    delete binFileBuf;
    binFileBuf = new uint8_t[binSize];
    memcpy(binFileBuf,bindata.data(),binSize);
    return true;
}

char comm::BinTo_09_AF(uint8_t a)
{
    if(a<10)return a+0x30;
    if(a>=10 ||a<16) return (a-10)+0x41;
    return 0;
}

uchar comm::_09_af_AF_ToBin(char a,bool *ok)
{
    *ok=true;
    if(a>=0x30 && a<=0x39)
    {
        return a-0x30;
    }
    else if(a>0x40&&a<0x47)
    {
        return a-0x41 + 10;
    }
    else if(a>0x60&&a<0x67)
    {
        return a-0x61 +10;
    }
    else
    {
        *ok=false;
    }
    return 0;
}

bool comm::HexStr2Bins(uchar *HexStr,uint16_t StrLen,uint8_t *re)
{
    bool ok;
    for(int i=0;i<StrLen/2;i++)
    {
        re[i] =  _09_af_AF_ToBin(HexStr[2*i],&ok)<<4;
        re[i] |= _09_af_AF_ToBin(HexStr[2*i+1],&ok);
        if(ok!=true)return false;
    }
    return true;
}
uint8_t comm::U8checkSum(uint8_t *a,uint8_t len)
{
    uint8_t i=0,sum=0;
    for(i=0;i<len;i++)
    {
        sum += a[i];
    }
    return sum;
}

uint16_t comm::Crc16_Modbus(uint8_t *puchMsg,uint16_t usDataLen)
{
    uint16_t i=0;
    uint16_t j=0;
    uint16_t crc16=0xFFFF;
    for (i = 0; i < usDataLen; i++)
    {
        crc16 ^= puchMsg[i];
        for (j = 0; j < 8; j++)
        {
            if ((crc16 & 0x01) == 1)
            {
                crc16 = (crc16 >> 1) ^ 0xA001;
            }
            else
            {
                crc16 = crc16 >> 1;
            }
        }
    }
    return crc16;
}
