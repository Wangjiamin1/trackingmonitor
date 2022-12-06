#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QSemaphore>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QTime>
#include <QApplication>
#include <QImage>
#include <windows.h>
//#include <QtNetwork>


class mythread : public QThread
{
    Q_OBJECT
public:
    mythread();
    ~mythread();

//    void Delay(int msec);

//    void set_Rec_Buffer_ptr(uchar *x_Rec_Buffer);
//    void set_freeBytes_ptr(QSemaphore *x_freeBytes);
//    void set_usedBytes_ptr(QSemaphore *x_usedBytes);

    QSemaphore *m_freeBytes;
    QSemaphore *m_usedBytes;
    uchar *m_Rec_Buffer;

    quint16 Writer_Ptr;
    quint16 Reader_Ptr;
    quint16 Pre_reader_Ptr;

    bool Run_stopped;
    volatile bool Run;
    volatile bool Tcp_send;
    volatile bool ctr_send;

    uchar *data = new uchar[18];
    QTcpSocket *tcpsocket;
    QString ctrcmd;
    QString m_ip;
    quint16 m_port;
    QString m_RTSPport;
    QString m_stream;
    QString m_PortName;


public slots:
    void gettcpdata(uchar *pdata);
    void CloseTCPConnect();

signals:
    void sendSelfCheck(uchar status2,uchar status1,uchar status0);
    void sendOnceDistance(uchar status,qint16 dist);

    void sendimg(QImage img , int w,int h);

};

#endif // MYTHREAD_H
