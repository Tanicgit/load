#ifndef APP_H
#define APP_H

#include <QObject>
#include "tcp_server.h"
#include "tcp_server_private.h"
#include "comm.h"
class app : public QObject
{
    Q_OBJECT

    public:
        app(QString name, uint16_t binV, uint16_t pkSize);
        ~app();
        bool tcpListen(int port);
    signals:
        void ServerRecved(qintptr, QTcpSocket*, const QByteArray &);
    private slots:
        void ClientConnected(qintptr handle, QTcpSocket *socket);
        void ClientDisconnected(qintptr handle);
        /**
         * @brief 服务端收到消息的信号
         *    若想要统一管理或做日志处理可连接此信号
         * @param 收到消息的连接句柄
         * @param 收到消息的socket指针
         * @param 收到的消息内容
         */
        void ServerRecvedSlot(qintptr handle, QTcpSocket *socket, const QByteArray &data);
    private:
        TcpServer *tcp_server_;
        comm commpro;
        QString binName;
        int binVersion;
        int binPackSize;
        int binSize;
};

#endif // APP_H
