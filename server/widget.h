#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "tcp_server.h"
#include "app.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
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

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::Widget *ui;
    TcpServer *tcp_server_;
    app commpro;
};

#endif // WIDGET_H
