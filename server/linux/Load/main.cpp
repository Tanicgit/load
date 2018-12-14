#include <QCoreApplication>
#include"app.h"
#include <QDebug>
/*
argv[0]
argv[1]   .bin's  filename  
argv[2]   .bin's  version
argv[3]   .bin's  packsize
*/
int main(int argc, char *argv[])/*binName  Version packSize*/
{
    QCoreApplication a(argc, argv);

    QString name;
    int ver=-1;
    int pksize=-1;
    bool ok=true;
    if(argc != 4)
    {
        qDebug() << "err: argc !=4" <<endl;
        exit(0);
    }


    name = QString(argv[1]);



    ver = QString(argv[2]).toInt(&ok,10);
    if(!ok)
    {
        qDebug() << "err argv[2]" << QString(argv[2]) <<endl;
        exit(0);
    }


    pksize = QString(argv[3]).toInt(&ok,10);
    if(!ok)
    {
        qDebug() << "err argv[3]" << QString(argv[3]) <<endl;
        exit(0);
    }

    app b(name,ver,pksize);
    if(!b.tcpListen(3201))
    {
       qDebug() << "listen 3201 err";
    }

    qDebug() << name << ver << pksize;
    return a.exec();
}

