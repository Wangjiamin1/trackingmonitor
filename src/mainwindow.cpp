#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    this->initApplication();

    ui->ip_lineEdit->setText("192.168.2.119");
    ui->cmbNetworkVideoURL->setText("192.168.2.119");
    ui->port_lineEdit->setText("6665");
    ui->RTSP_port->setText("554");
    ui->Stream->setText("stream0");
    connect(ui->videoDispLabel,SIGNAL(emitbboox(QPoint,int,int)),this,SLOT(getbbox(QPoint,int,int)));
    camhandle = new camprotocol();
    decoder = new CH264Decoder();
    connect(camhandle->shower,SIGNAL(open_rtsp_fail(QString)),this,SLOT(set_btnOpenNetworkVideo_open(QString)));
    initwindow();
    //this->opencvrtsp();
    //decoder->rtsp_open();
    //this->rtsp_open();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete camhandle;
    delete decoder;
    //timer1->stop();
    //delete timer1;
}



QImage MainWindow::Matimgtoqt(const cv::Mat &src)
{
    //Mat转成QImage
    {
        //CV_8UC1 8位无符号的单通道---灰度图片
        if(src.type() == CV_8UC1)
        {
            //使用给定的大小和格式构造图像
            //QImage(int width, int height, Format format)
            QImage qImage(src.cols,src.rows,QImage::Format_Indexed8);
            //扩展颜色表的颜色数目
            qImage.setColorCount(256);

            //在给定的索引设置颜色
            for(int i = 0; i < 256; i ++)
            {
                //得到一个黑白图
                qImage.setColor(i,qRgb(i,i,i));
            }
            //复制输入图像,data数据段的首地址
            uchar *pSrc = src.data;
            //
            for(int row = 0; row < src.rows; row ++)
            {
                //遍历像素指针
                uchar *pDest = qImage.scanLine(row);
                //从源src所指的内存地址的起始位置开始拷贝n个
                //字节到目标dest所指的内存地址的起始位置中
                memcmp(pDest,pSrc,src.cols);
                //图像层像素地址
                pSrc += src.step;
            }
            return qImage;
        }
        //为3通道的彩色图片
        else if(src.type() == CV_8UC3)
        {
            //得到图像的的首地址
            const uchar *pSrc = (const uchar*)src.data;
            //以src构造图片
            QImage qImage(pSrc,src.cols,src.rows,src.step,QImage::Format_RGB888);
           // QImage qImage(pSrc,src.cols,src.rows,src.cols*src.channels(),QImage::Format_RGB888);
            //在不改变实际图像数据的条件下，交换红蓝通道
            return qImage.rgbSwapped();
        }
        //四通道图片，带Alpha通道的RGB彩色图像
        else if(src.type() == CV_8UC4)
        {
            const uchar *pSrc = (const uchar*)src.data;
            QImage qImage(pSrc, src.cols, src.rows, src.step, QImage::Format_ARGB32);
            //返回图像的子区域作为一个新图像
            return qImage.copy();
        }
        else
        {
            return QImage();
        }
    }

}

void MainWindow::initwindow()
{
    this->setMouseTracking(true);
    //this->setWindowFlags(Qt::FramelessWindowHint);
    setWindowState(Qt::WindowActive);
}

void MainWindow::updatebbox(QPoint startpoint,int bbox_w,int bbox_h)
{
    predata[10] = uchar(startpoint.x()>>8);
    predata[11] = startpoint.x()&0xff;

    predata[12] = uchar(startpoint.y()>>8);
    predata[13] = startpoint.y()&0xff;

    predata[14] = uchar(bbox_w>>8);
    predata[15] = bbox_w&0xff;

    predata[16] = uchar(bbox_h>>8);
    predata[17] = bbox_h&0xff;

    emit sendtcpdata(predata);
}

void MainWindow::sendcmd()
{
    emit sendtcpdata(predata);
}

void MainWindow::set_btnOpenNetworkVideo_open(QString str){
    ui->output_brower->insertPlainText(str);

    ui->btnOpenNetworkVideo->setText("Open");
}

void MainWindow::Showpic(QImage image,int w,int h)
{

    if(!image.isNull()){
        ui->videoDispLabel->setPixmap(QPixmap::fromImage(image));
    }

}

void MainWindow::getmsg(uchar num, qint16 x, qint16 y,quint16 dis,quint8 dis1)
{
    QString text;
    text = "检测数目： "+QString::number(num)+"跟踪靶坐标："+QString::number(x)+","+QString::number(y)
            +"距离："+QString::number(dis)+"."+QString::number(dis1);
    //ui->videoDispLabel->setText(text);
    ui->statusBar->showMessage(text);
}

void MainWindow::getbbox(QPoint start, int w, int h)
{
    updatebbox(start,w,h);
}

void MainWindow::getshowbuff(uchar *buff,int len)
{
    cv::Mat res;
    QImage image;
    qDebug()<<len;
    decoder->decode(buff,len,res);
    image = this->Matimgtoqt(res);
    qDebug()<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}


void MainWindow::initApplication()
{
    predata[0] = uchar(0x55);
    predata[1] = uchar(0xAA);
    predata[2] = uchar(0x02);
    predata[3] = uchar(0x00);
    predata[4] = uchar(0x01);
    predata[5] = uchar(0x00);
    predata[6] = uchar(0x00);
    predata[7] = uchar(0x00);
    predata[8] = uchar(0x00);
    predata[9] = uchar(0x00);
    predata[10] = uchar(0x00);
    predata[11] = uchar(0x00);
    predata[12] = uchar(0x00);
    predata[13] = uchar(0x00);
    predata[14] = uchar(0x00);
    predata[15] = uchar(0x00);
    predata[16] = uchar(0x00);
    predata[17] = uchar(0x00);
    predata[18] = uchar(0x00);    //激光测距数据返回
    predata[19] = uchar(0x00);
    emit sendtcpdata(predata);
}

void MainWindow::on_showvideo_comboBox_currentIndexChanged(int index)
{
    qDebug()<<index;
    switch (index) {
    case 0:
        predata[3] = 0x00;
        ui->showvideostatus_label->setText("可见光");
        ui->ircam_status_label->setText("关闭");
        break;
    case 1:
        predata[3] = 0x01;
         ui->showvideostatus_label->setText("红外");
         ui->ircam_status_label->setText("打开");

        break;
    case 2:
        predata[3] = 0x03;
         ui->showvideostatus_label->setText("融合");
         ui->ircam_status_label->setText("打开");
        break;
    }

    emit sendtcpdata(predata);
}

//void MainWindow::on_camfocalen_lineEdit_editingFinished()
//{
//    short focal_len = ui->camfocalen_lineEdit->text().toShort();
//    QString text = QString::number(focal_len,10);
//    predata[4] = uchar(focal_len);
//    qDebug("%x",predata[4]);

//    emit sendtcpdata(predata);
//    ui->cam_focal_lenth_status_label->setText("x"+text);
//}

//void MainWindow::on_fusion_checkBox_clicked(bool checked)
//{
//    //qDebug()<<checked;
//    if(checked){
//        predata[8] = 0x01;
//        emit sendtcpdata(predata);
//    }
//    else {
//        predata[8] = 0x00;
//        emit sendtcpdata(predata);
//    }
//}

void MainWindow::on_laser_checkBox_clicked(bool checked)
{
    if(checked)
    {
        predata[7] = 0x01;
        emit sendtcpdata(predata);
        ui->laser_status_label->setText("打开");
    }
    else {
        predata[7] = 0x00;
        emit sendtcpdata(predata);
        ui->laser_status_label->setText("关闭");
    }
}

void MainWindow::on_detect_checkBox_clicked(bool checked)
{
    if(checked)
    {
        predata[5] = 0x01;
        emit sendtcpdata(predata);
        ui->detect_status_label->setText("打开");
    }
    else {
        predata[5] = 0x00;
        emit sendtcpdata(predata);
        ui->detect_status_label->setText("关闭");

    }
}

//void MainWindow::on_track_checkBox_clicked(bool checked)
//{
//    if(checked)
//    {
//        predata[6] = 0x01;
//        emit sendtcpdata(predata);
//    }
//    else {
//        predata[6] = 0x00;
//        emit sendtcpdata(predata);
//    }
//}

void MainWindow::on_sysmode_combox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        predata[5] = 0x00;
        ui->sysmode_status_label->setText("定位");
        break;
    case 1:
        predata[5] = 0x01;
        ui->sysmode_status_label->setText("检测");
        break;
    case 2:
        predata[5] = 0x02;
        ui->sysmode_status_label->setText("自动跟踪");
        break;
    case 3:
        predata[5] = 0x03;
        ui->sysmode_status_label->setText("手动跟踪");
        break;
    }

    emit sendtcpdata(predata);
}

void MainWindow::on_laser_combox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        predata[7] = 0x00;
        ui->laser_status_label->setText("无");
        break;
    case 1:
        predata[7] = 0x01;
        ui->laser_status_label->setText("自检");
        break;
    case 2:
        predata[7] = 0x02;
        ui->laser_status_label->setText("测距");
        break;
    case 3:
        predata[7] = 0x03;
        ui->laser_status_label->setText("查询设备版本");
        break;
    }

    emit sendtcpdata(predata);
}

void getOnceDistance(uchar status,qint16 dist){
    switch (status) {
    case 0:
        qDebug()<<"单目标";
        break;
    case 1:
        qDebug()<<"前目标";
        break;
    case 2:
        qDebug()<<"后目标";
        break;
    case 4:
        qDebug()<<"超距离";
        break;
    default:
        qDebug()<<"测距失效";
    }

    qDebug()<<dist;

}

void getSelfCheck(uchar status2,uchar status1,uchar status0){
    int i=0;
    vector<int> temp_bit;
    while(status1){
        if(status1/2==0){
            temp_bit.push_back(i);
        }
        status1>>1;
        i++;
    }
    qDebug()<<"回波强度: "<<status2;
    if(status0==1){
        qDebug()<<"电源状态正常";
    }
    else{
        qDebug()<<"电源状态异常";
    }
    if(temp_bit.size()==0)
    {
        qDebug()<<"状态正常";
    }
    else{
        for (auto i:temp_bit)
        {
            switch (i) {
            case 0:
                qDebug()<<"FPGA系统异常";
                break;
            case 1:
                qDebug()<<"激光出光异常";
                break;
            case 2:
                qDebug()<<"主波检测异常";
                break;
            case 3:
                qDebug()<<"回波检测异常";
                break;
            case 4:
                qDebug()<<"偏压开关关闭";
                break;
            case 5:
                qDebug()<<"偏压输出异常";
                break;
            case 6:
                qDebug()<<"温度异常";
                break;
            case 7:
                qDebug()<<"出光关断无效";
                break;
            }
        }
    }


}



void MainWindow::on_ip_lineEdit_editingFinished()
{
    cur_ip = ui->ip_lineEdit->text();
}

void MainWindow::on_port_lineEdit_editingFinished()
{
    cur_port = ui->port_lineEdit->text().toInt();
}

void MainWindow::on_connect_clicked()
{
    if(ui->connect->text()=="Connect"){
        cur_ip = ui->ip_lineEdit->text();
        cur_port = ui->port_lineEdit->text().toInt();
        camhandle->setMai(this,cur_ip,cur_port);
        ui->connect->setText("Disconnect");
    }
    else{
        emit CloseTCPConnect();
        ui->connect->setText("Connect");
    }
}

void MainWindow::on_btnOpenNetworkVideo_clicked()
{
    if(ui->btnOpenNetworkVideo->text()=="Open"){
        camhandle->open_rtsp(this,ui->cmbNetworkVideoURL->text(),ui->RTSP_port->text(),ui->Stream->text(),1);
        ui->btnOpenNetworkVideo->setText("Close");
    }
    else{
        camhandle->open_rtsp(this,ui->cmbNetworkVideoURL->text(),ui->RTSP_port->text(),ui->Stream->text(),0);
        ui->btnOpenNetworkVideo->setText("Open");
        ui->videoDispLabel->clear();
    }
}

void MainWindow::t_set_videolabel_clear(){
    ui->btnOpenNetworkVideo->setText("Open");
    ui->videoDispLabel->setText("NO SIGNAL");
    qDebug()<<"test";
}


void MainWindow::on_camfocalen_lineEdit_currentIndexChanged(int index)
{
    predata[4] = (uchar)index+1;
    qDebug("%x",predata[4]);

    emit sendtcpdata(predata);
}
