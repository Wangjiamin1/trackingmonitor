#include "camprotocol.h"


// 回调函数的参数，时间和有无流的判断
typedef struct {
    time_t lasttime;
    bool connected;
} Runner;

// 回调函数
int interrupt_callback(void *p) {
    Runner *r = (Runner *)p;
    if (r->lasttime > 0) {
        if (time(NULL) - r->lasttime > 2 && !r->connected) {
            // 等待超过1s则中断
            return 1;
        }
    }
    return 0;
}

void rtsp::run()
    {
    //定义FFMPEG参数指针
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    const AVCodec *pCodec = NULL;
    AVFrame *pFrame,*pFrameRGB;
    AVPacket *packet;
    struct SwsContext *img_convert_ctx,*img_convert_ctx_;

    unsigned char *out_buffer;
    int i,videoIndex;
    int ret;
    char errors[1024] = "";

    //rstp地址设置
    char url[] = "rtsp://192.168.1.112:8554/live";
    QString url1 = "rtsp://"+m_ip+":"+m_RTSPport+"/"+m_stream;
    qDebug()<<url1;
    QByteArray byte = url1.toLocal8Bit();
    strcpy_s(url,byte.size()+1,byte.data());

    //初始化FFMPEG  调用了这个才能正常适用编码器和解码器
    pFormatCtx = avformat_alloc_context();  //init FormatContext

    //初始化FFmpeg网络模块
    avformat_network_init();    //init FFmpeg network

    //判断RTSP超时断开
    Runner input_runner = { 0 };
    pFormatCtx->interrupt_callback.callback = interrupt_callback;
    pFormatCtx->interrupt_callback.opaque = &input_runner;
    input_runner.lasttime = time(NULL);
    input_runner.connected = false;
    AVDictionary* options = nullptr;
    //实时播放使用udp，减小带宽并防止断线
    av_dict_set(&options, "rtsp_transport", "udp", 0);





    //open Media File
     qDebug()<<"+++++++++++++++++++++++++++++++++++++";
    ret = avformat_open_input(&pFormatCtx,byte,NULL,&options);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        qDebug() <<"Failed to open video: ["<< ret << "]"<< errors << endl;
        emit open_rtsp_fail("Failed to open video");
        return;
    }
    else{
        input_runner.connected = true;
    }

    //Get audio information
    ret = avformat_find_stream_info(pFormatCtx,NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        //cout <<"Failed to get audio info: ["<< ret << "]"<< errors << endl;
        return;
    }

    videoIndex = -1;

    ///循环查找视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 videoIndex
    ///这里我们现在只处理视频流  音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
        }
    }

    ///videoIndex-1 说明没有找到视频流
    if (videoIndex == -1) {
        printf("Didn't find a video stream.\n");
        return;
    }

    //配置编码上下文，AVCodecContext内容
    //1.查找解码器
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoIndex]->codecpar->codec_id);
    //2.初始化上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    //3.配置上下文相关参数
    avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[videoIndex]->codecpar);
    //4.打开解码器
    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        //cout <<"Failed to open Codec Context: ["<< ret << "]"<< errors << endl;
        return;
    }

    //初始化视频帧
    pFrame = av_frame_alloc();
//    pFrameRGB = av_frame_alloc();
    //为out_buffer申请一段存储图像的内存空间
    out_buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32,pCodecCtx->width,pCodecCtx->height,1));
    //实现AVFrame中像素数据和Bitmap像素数据的关联
//    av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize, out_buffer,
//                   AV_PIX_FMT_RGB32,pCodecCtx->width, pCodecCtx->height,1);
    //为AVPacket申请内存
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //打印媒体信息
    av_dump_format(pFormatCtx,0,url,0);
    //初始化一个SwsContext
//    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
//                pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
//                AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    img_convert_ctx_ = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                AVPixelFormat::AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    //AVPixelFormat::AV_PIX_FMT_BGR24
    cv::Mat frame(pCodecCtx->height, pCodecCtx->width, CV_8UC3);

    //设置视频label的宽高为视频宽高
    //ui->label->setGeometry(0, 0, pCodecCtx->width, pCodecCtx->height);

    //读取帧数据，并通过av_read_frame的返回值确认是不是还有视频帧
    QImage img;
    while(!Run_stopped && av_read_frame(pFormatCtx,packet) >=0){
        //判断视频帧
        if(packet->stream_index == videoIndex){
            //解码视频帧
            ret = avcodec_send_packet(pCodecCtx, packet);
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if(ret != 0){
                av_strerror(ret,errors,sizeof(errors));
                //cout <<"Failed to decode video frame: ["<< ret << "]"<< errors << endl;
            }
            if (ret == 0) {
                //处理图像数据
//                sws_scale(img_convert_ctx,
//                                        (const unsigned char* const*) pFrame->data,
//                                        pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
//                                        pFrameRGB->linesize);

                int cvLinesizes[1];
                cvLinesizes[0] = frame.step1();
                sws_scale(img_convert_ctx_,
                                        (const unsigned char* const*) pFrame->data,
                                        pFrame->linesize, 0, pCodecCtx->height, &frame.data,
                                        cvLinesizes);



                img=QImage((const uchar*)frame.data,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB888);
                //img=QImage((uchar*)pFrameRGB->data[0],pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
                if(!img.isNull()&&pCodecCtx->width>200&&pCodecCtx->height>200){
                    emit sendimg(img,pCodecCtx->width, pCodecCtx->height);
                }
                //释放前需要一个延时
            }
        }
        //释放packet空间
        msleep(1);
        av_packet_unref(packet);
    }
//    //disconnect(this->parent(), SIGNAL(sendimg(QImage,int,int)), 0, 0);

//    //close and release resource
    av_free(out_buffer);
    av_free(pFrameRGB);

    sws_freeContext(img_convert_ctx);
    avcodec_close(pCodecCtx);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);

    emit s_set_videolabel_clear();
}

class cam : public mythread
{

public:
    void run() Q_DECL_OVERRIDE
    {
        char *Rec_Temp = new char[65535];
        int Rec_len = 0;
        Run_stopped = false;

        //        tcpsocket->abort();
        tcpsocket->connectToHost(m_ip,m_port);
        if(tcpsocket->waitForConnected(3000))
        {
            while (!Run_stopped) {
                /*接收数据*/
                if(tcpsocket->waitForReadyRead(1))
                {
                    Rec_len = tcpsocket->read(Rec_Temp, 13);

                    qDebug()<<"rec_len:"<<Rec_len;

                    if(Rec_Temp[0]==(char)0xEE){
                        if(Rec_Temp[4]==(char)0x01)
                        {
                            emit sendSelfCheck(Rec_Temp[6],Rec_Temp[7],Rec_Temp[8]);
                        }
                        if(Rec_Temp[4]==(char)0x02)
                        {

                            qint16 dict = Rec_Temp[7]|Rec_Temp[6]<<8;
                            emit sendOnceDistance(Rec_Temp[5],dict);
                        }

                    }
                }

            }

        }

        qDebug()<<"连接失败";
        CloseTCPConnect();
        tcpsocket->close();
        //                delete tcpsocket;
        delete [] Rec_Temp;
        //                Run_stopped = true;
        qDebug()<<"disconnect tcp";

    }

//public slots:
//    void CloseTCPConnect(){
//        Run_stopped = true;
//        qDebug()<<"mantual disconnect";

//    }
};

//class decode : public mythread
//{
//public:
//    void run() Q_DECL_OVERRIDE
//    {
//        uchar *Pre_Decoder = new uchar[13];
//        uchar detect_num = 0;
//        qint16 tracker_x = 0;
//        qint16 tracker_y = 0;
//        quint16 distance = 0;
//        quint8 distance1 = 0;

//        Run_stopped = false;

//        while(!Run_stopped)
//        {

//            if(m_usedBytes->tryAcquire(13,1))
//            {

//                Pre_reader_Ptr = Reader_Ptr;
//                Pre_Decoder[0] = m_Rec_Buffer[Pre_reader_Ptr++];

//                if (Pre_Decoder[0] == (uchar)0x55)
//                {
//                    Pre_Decoder[1] = m_Rec_Buffer[Pre_reader_Ptr++];
//                    Pre_Decoder[2] = m_Rec_Buffer[Pre_reader_Ptr++];

//                    if ((Pre_Decoder[1] == (uchar)0xAA)&&(Pre_Decoder[2]==0x01))
//                    {
//                        //for (int i=3;i<12;i++)
//                        for (int i=3;i<11;i++)
//                        {
//                            Pre_Decoder[i] = m_Rec_Buffer[Pre_reader_Ptr++];
//                        }
//                        detect_num = Pre_Decoder[3];
//                        tracker_x = Pre_Decoder[5]|Pre_Decoder[4]<<8;
//                        tracker_y = Pre_Decoder[7]|Pre_Decoder[6]<<8;

//                        distance = Pre_Decoder[9]|Pre_Decoder[8]<<8;
//                        distance1 = Pre_Decoder[10];

//                        Reader_Ptr = Pre_reader_Ptr;
//                        m_freeBytes->release(13);

//                    }
//                    else {
//                        Reader_Ptr++;

//                        m_usedBytes->release(10);
//                        m_freeBytes->release(3);
//                    }
//                 }

//                 else {
//                     Reader_Ptr++;

//                     m_usedBytes->release(10);
//                     m_freeBytes->release(1);
//                 }
//            }
//        }

//        delete [] Pre_Decoder;
//        Run_stopped = false;

//    }
//};

camprotocol::camprotocol()
{
    Rec_Buffer_ptr = new uchar[BUFFER_SIZE];
    freeBytes = new QSemaphore(BUFFER_SIZE);
    usedBytes = new QSemaphore(0);

    const QString ip = "192.168.1.114";

    camera = new cam();
//    camera->set_Rec_Buffer_ptr(Rec_Buffer_ptr);
//    camera->set_freeBytes_ptr(freeBytes);
//    camera->set_usedBytes_ptr(usedBytes);
    camera->m_ip.clear();
    camera->m_ip = ip;
    camera->m_port = 6666;

//    decoder = new decode();
//    decoder->set_Rec_Buffer_ptr(Rec_Buffer_ptr);
//    decoder->set_freeBytes_ptr(freeBytes);
//    decoder->set_usedBytes_ptr(usedBytes);

    shower = new rtsp();
}

camprotocol::~camprotocol()
{
    camera->Run_stopped = true;
    camera->wait();
//    decoder->Run_stopped = true;
//    decoder->wait();

    delete camera;
//    delete decoder;
    delete shower;

    delete freeBytes;
    delete usedBytes;
    delete [] Rec_Buffer_ptr;
}

void camprotocol::stopRun()
{
    camera->Run_stopped = true;
    camera->wait();
//    decoder->Run_stopped = true;
//    decoder->wait();
//    shower->Run_stopped = true;
}

void camprotocol::setMai(QObject *qMain, const QString ip, quint16 port)
{
    camera->m_ip.clear();
    camera->m_ip = ip;
    camera->m_port = port;
//    shower->m_ip.clear();
//    shower->m_ip = ip;
    this->connectToMain(qMain);
    this->startRun();
}

//void camprotocol::deteleshowthreath(){
//    shower->Run_stopped = true;
//}



void camprotocol::startRun()
{
    camera->start(QThread::TimeCriticalPriority);
//    decoder->start(QThread::TimeCriticalPriority);

}

void camprotocol::connectToMain(QObject *Main_Obj)
{
        QObject::connect(Main_Obj,SIGNAL(sendtcpdata(uchar *)),camera,SLOT(gettcpdata(uchar *)),Qt::QueuedConnection);
//        QObject::connect(decoder,SIGNAL(sendmsgtomain(uchar,qint16,qint16,quint16,quint8)),Main_Obj,SLOT(getmsg(uchar,qint16,qint16,quint16,quint8))
//                         ,Qt::QueuedConnection);
//        QObject::connect(decoder,SIGNAL(sendshowbuff(uchar*,int)),Main_Obj,SLOT(getshowbuff(uchar *,int))
//                         ,Qt::QueuedConnection);
        QObject::connect(Main_Obj,SIGNAL(CloseTCPConnect()),camera,SLOT(CloseTCPConnect()),Qt::QueuedConnection);
        QObject::connect(camera,SIGNAL(sendSelfCheck(uchar status2,uchar status1,uchar status0)),Main_Obj,SLOT(getSelfCheck(uchar status2,uchar status1,uchar status0)),Qt::QueuedConnection);
        QObject::connect(camera,SIGNAL(sendOnceDistance(uchar status,qint16 dist)),Main_Obj,SLOT(getOnceDistance(uchar status,qint16 dist)),Qt::QueuedConnection);

}

void camprotocol::open_rtsp(QObject *qMain, const QString rtsp_ip,QString port,QString stream,qint16 flag)
{
    if(flag){
        shower->m_ip.clear();
        shower->m_ip = rtsp_ip;
        shower->m_RTSPport.clear();
        shower->m_RTSPport = port;
        shower->m_stream.clear();
        shower->m_stream = stream;
        QObject::connect(shower,SIGNAL(sendimg(QImage,int, int)),qMain,SLOT(Showpic(QImage ,int ,int ))
                         ,Qt::QueuedConnection);
        QObject::connect(shower,SIGNAL(s_set_videolabel_clear()),qMain,SLOT(t_set_videolabel_clear())
                         ,Qt::QueuedConnection);
        shower->Run_stopped = false;
        shower->start(QThread::TimeCriticalPriority);
    }
    else{
        QObject::disconnect(shower,SIGNAL(sendimg(QImage,int,int)),qMain,SLOT(Showpic(QImage ,int ,int )));
        shower->Run_stopped = true;
    }



}

//void camprotocol::RTSPConnectToMain(QObject *Main_Obj){
//    QObject::connect(shower,SIGNAL(sendimg(QImage,int,int)),Main_Obj,SLOT(Showpic(QImage ,int ,int ))
//                     ,Qt::QueuedConnection);

//}

//void camprotocol::RTSPConnectToMain(QObject *Main_Obj){
//    QObject::disconnect(shower,SIGNAL(sendimg(QImage,int,int)),Main_Obj,SLOT(Showpic(QImage ,int ,int )));

//}
