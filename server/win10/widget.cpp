#include "widget.h"
#include "ui_widget.h"
#include <QTcpSocket>
#include <QDebug>

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QTime>
#include <QDir>
#include <QFile>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(this, &Widget::ServerRecved, this, Widget::ServerRecvedSlot);

}

Widget::~Widget()
{
    tcp_server_->close();
    delete ui;
}

void Widget::ClientConnected(qintptr handle, QTcpSocket* socket) {
    ui->plainTextEdit->insertPlainText(QString::number(handle)+"connected");
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    connect(socket, &QTcpSocket::readyRead,
            [=]() {
        emit ServerRecved(handle, socket, socket->readAll());
    });
}

void Widget::ClientDisconnected(qintptr handle) {
    ui->plainTextEdit->insertPlainText(QString::number(handle)+"disconnected");
    ui->plainTextEdit->moveCursor(QTextCursor::End);
}

void Widget::ServerRecvedSlot(qintptr handle,
                                  QTcpSocket *socket,
                                  const QByteArray &data) {
    Q_UNUSED(handle);

    QByteArray msg;

    QString send_data = QString("%1 %2 %3").
            arg(socket->peerAddress().toString()).
            arg(socket->peerPort()).
            arg(QString(data));

    ui->plainTextEdit->insertPlainText(send_data);
    ui->plainTextEdit->moveCursor(QTextCursor::End);

    /**/
    msg = commpro.process(data);
    /**/
    if(!msg.isEmpty())
    {
        socket->write(msg);
    }
   // socket->write(send_data.toLatin1());
}
void Widget::on_pushButton_2_clicked()
{
   ui->plainTextEdit->clear();
}

void Widget::on_pushButton_clicked()
{
    bool ok=true;
    int port = ui->lineEdit->text().toInt(&ok,10);
    if(ui->pushButton->text()=="listen")
    {
        if(!ok)return;
        tcp_server_ = new TcpServer(this);
        if(!tcp_server_->listen(QHostAddress::Any, port)) {
            qDebug()<<tcp_server_->errorString();    //错误信息
            return;
        }
        connect(tcp_server_,
            &TcpServer::ClientConnected,
            this,
            &Widget::ClientConnected);//监听
        connect(tcp_server_,
            &TcpServer::ClientDisconnected,
            this,
            &Widget::ClientDisconnected);//监听

        ui->pushButton->setText("stop");
        ui->pushButton->setStyleSheet("color:red");
    }
    else
    {
        tcp_server_->close();
        ui->pushButton->setText("listen");
        ui->pushButton->setStyleSheet("color:black");
    }

}

void Widget::on_pushButton_3_clicked()
{
    bool ok,a;
    uint16_t ver = 0,pac;
    ver = ui->lineEdit_2->text().toInt(&ok,10);
    pac = ui->lineEdit_3->text().toInt(&a,10);
    if(ok&&a)
    {
        /*打开文件*/
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("open image file"),
                                                        "./",
                                                        tr("All files (*.*)"));
        if(fileName.isEmpty())
       {
           return;
       }
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox mesg;
            mesg.warning(this,"警告",file.errorString());
            return;
        }
        ui->plainTextEdit->appendPlainText(QString("%1").arg(file.size()));
        commpro.updataBinFile(file.readAll(),ver,file.size(),pac);
        file.close();
    }
}
