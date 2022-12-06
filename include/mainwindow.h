#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "camprotocol.h"
#include <windows.h>
#include "mylabel.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    uchar *predata = new uchar[20];

    QImage Matimgtoqt(const cv::Mat &src);

    void initwindow();

    void updatebbox(QPoint startpoint,int bbox_w,int bbox_h);

    void getSelfCheck(uchar status2,uchar status1,uchar status0);
    void getOnceDistance(uchar status,qint16 dist);



//    void mousepress(QObject *obj,QEvent* event);

//    void mousemove(QObject *obj,QMouseEvent* event);

//    void mouserelease(QObject *obj,QMouseEvent* event);

    //void keypressEvent(QKeyEvent *event);

    AVPacket *pAVPacket = av_packet_alloc();

    QString cur_ip;

    int cur_port;

    //void paintEvent(QPaintEvent *event);


    void rtsp_open();

    ~MainWindow();


signals:
    void sendtcpdata(uchar *data);
    void deleteShowerThreath();
    void CloseTCPConnect();

private slots:
    void getmsg(uchar num,qint16 x,qint16 y,quint16 dis,quint8 dis1);

    void getbbox(QPoint start,int w,int h);

    void getshowbuff(uchar *buff,int len);

    void Showpic(QImage image,int w,int h);

    //bool eventFilter(QObject *obj,QEvent *event);

    void sendcmd();

    void on_showvideo_comboBox_currentIndexChanged(int index);

//    void on_camfocalen_lineEdit_editingFinished();

//    void on_fusion_checkBox_clicked(bool checked);

    void on_laser_checkBox_clicked(bool checked);

    void on_detect_checkBox_clicked(bool checked);

//    void on_track_checkBox_clicked(bool checked);
    void on_laser_combox_currentIndexChanged(int index);


    void on_ip_lineEdit_editingFinished();

    void on_port_lineEdit_editingFinished();

    void on_connect_clicked();

    void on_sysmode_combox_currentIndexChanged(int index);

    void on_btnOpenNetworkVideo_clicked();

    void set_btnOpenNetworkVideo_open(QString);
    void t_set_videolabel_clear();




    void on_camfocalen_lineEdit_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;

   // QTimer *timer1 = new QTimer(this);

    void initApplication(void);

    camprotocol *camhandle;
    CH264Decoder *decoder;

//    QPainter m_painter;
 //   QPoint startpoint;
//    QPoint finishpoint;
//    QRectF rectangle;
//    bool m_isMousePress;
//    bool m_isdraw;
//    int bbox_w;
//    int bbox_h;
};

#endif // MAINWINDOW_H
