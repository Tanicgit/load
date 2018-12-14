#include "app.h"
#include <QDebug>
#include "comm.h"

app::app(QString fileName,uint16_t binV,uint16_t pkSize)
{
    if(fileName.isEmpty())
   {
       qDebug()<<"warning: " << fileName << "empty";
       return;
   }
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"warning: open" << fileName <<file.errorString();
        return;
    }
    binSize = file.size();
    binVersion = binV;
    binPackSize = pkSize;
    commpro.updataBinFile(file.readAll(),binVersion,file.size(),binPackSize);
    file.close();
}

app::~app()
{
    tcp_server_->close();
}

bool app::tcpListen(int port)
{
      connect(this,SIGNAL(ServerRecved(qintptr,QTcpSocket*,QByteArray)),
              this,SLOT(ServerRecvedSlot(qintptr,QTcpSocket*,QByteArray)));
        tcp_server_ = new TcpServer(this);
        if(!tcp_server_->listen(QHostAddress::Any, port)) {
            qDebug()<<tcp_server_->errorString();    //错误信息
            return false;
        }
        qDebug()<<"listening";    //错误信息
        connect(tcp_server_,
            &TcpServer::ClientConnected,
            this,
            &app::ClientConnected);//监听
        connect(tcp_server_,
            &TcpServer::ClientDisconnected,
            this,
            &app::ClientDisconnected);//监听
        return true;
}

void app::ClientConnected(qintptr handle, QTcpSocket *socket)
{
    connect(socket,&QTcpSocket::readyRead,
            [=]() {
       emit ServerRecved(handle,socket,socket->readAll());
    });

}

void app::ClientDisconnected(qintptr handle)
{
    qDebug() << QString::number(handle)<< " disconnected";
}

void app::ServerRecvedSlot(qintptr handle, QTcpSocket *socket, const QByteArray &data)
{

    Q_UNUSED(handle);
    QByteArray msg;
    msg = commpro.process(data);
    /**/
    if(!msg.isEmpty())
    {
        socket->write(msg);
    }
   // socket->write(data);

    qDebug()<<socket->peerAddress().toString()<<socket->peerPort()<<data;
}



