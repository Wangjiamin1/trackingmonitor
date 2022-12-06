#ifndef CAMPROTOCOL_H
#define CAMPROTOCOL_H

#include <QApplication>

#include "mythread.h"
#include "h264_decoder.h"

#define BUFFER_SIZE 13
#define NUM_SIZE 1600

class cam;
class decode;
class img_decode;

class rtsp : public mythread
{
    Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;




signals:
    void open_rtsp_fail(QString);
    void s_set_videolabel_clear();

};

class camprotocol : public QObject
{
    Q_OBJECT
public:

    camprotocol();
    ~camprotocol();

    void stopRun(void);
    void setMai(QObject *main ,const QString ip,quint16 port);
    void open_rtsp(QObject *qMain, const QString rtsp_ip,QString port,QString stream,qint16 flag);
//    void deteleshowthreath();
    QSemaphore *freeBytes;
    QSemaphore *usedBytes;
    uchar *Rec_Buffer_ptr;

    QSemaphore *img_freeBytes;
    QSemaphore *img_usedBytes;
    uchar *img_Rec_Buffer_ptr;

    cam *camera;
    cam *img;
    decode *decoder;
    img_decode *img_decoder;

    rtsp *shower;

private:
     void startRun(void);
     void connectToMain(QObject *Main_Obj);
     void RTSPConnectToMain(QObject *Main_Obj);



};

#endif // CAMPROTOCOL_H
