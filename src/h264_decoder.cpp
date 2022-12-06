#include "h264_decoder.h"

CH264Decoder::CH264Decoder()
{
    initial();
    //readfile();
}

CH264Decoder::~CH264Decoder()
{
    unInitial();
}




int CH264Decoder::initial()
{
    //avcodec_register_all();//新版本应该不需要这句话
    av_init_packet(&packet);

    avformat_network_init(), //初始化网络，支持rtsp

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec)
    {
        printf("avcodec_find_encoder failed");
        return -1;
    }

    context = avcodec_alloc_context3(codec);
    if (!context)
    {
        printf("avcodec_alloc_context3 failed");
        return -2;
    }

    context->codec_type = AVMEDIA_TYPE_VIDEO;
    context->pix_fmt = AV_PIX_FMT_YUV420P;

//    int err;

//    //pFormatCtx = avformat_alloc_context();
//    err = avformat_open_input(nullptr, url.toStdString().c_str(), NULL,
//                                  NULL);

    if (avcodec_open2(context, codec, NULL) < 0)
    {
        printf("avcodec_open2 failed");
        return -3;
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        return -4;
    }

    return 0;
}

void CH264Decoder::unInitial()
{
    avcodec_close(context);
    av_free(context);
    av_frame_free(&frame);
}

int CH264Decoder::decode(unsigned char *pDataIn, int nInSize, cv::Mat& res)
{
//    av_init_packet(&packet);
    packet.size = nInSize;
    packet.data = pDataIn;

    if (packet.size > 0)
    {
        int got_picture=0;
        //int ret= avcodec_decode_video2(context, frame, &got_picture, &packet);
        //新版用法
        int ret = avcodec_send_packet(context, &packet);
        if (ret == 0) got_picture = avcodec_receive_frame(context, frame); //got_picture = 0 success, a frame was returned
        if (ret < 0)
        {
            printf("avcodec_encode_video2 failed");
            return -2;
        }

        if (got_picture==0)//采用avcodec_decode_video2时,此处为if (got_picture)
        {
             avframe_to_cvmat(frame,res);
        }
    }
    else
    {
        printf("no data to decode");
        return -1;
    }

    return 0;
}

int CH264Decoder::avframe_to_cvmat(AVFrame *frame,cv::Mat& res)
{
    int width = frame->width, height = frame->height;
    res.create(height*3/2, width, CV_8UC1);
    memcpy( res.data, frame->data[0], width*height );
    memcpy( res.data + width*height, frame->data[1], width*height/4 );
    memcpy( res.data + width*height*5/4, frame->data[2], width*height/4 );

    //cv::imshow( "yuv_show", res );//yuv格式
    cv::cvtColor( res, res, cv::COLOR_YUV2BGR_I420 );//bgr格式
    //cv::imshow( "bgr_show", bgr );
    return 0;

}

void CH264Decoder::readfile()
{
    const QString filename = "testfile.h264";
    QFile file(filename);
    //QByteArray bytes;
    cv::Mat res;
//    int size = 0;
//    if(!file.exists()) //文件不存在则退出
//    {
//        qDebug()<<"file not exist";
//        return;
//    }
//    if(file.open(QFile::ReadOnly))
//    {
//        bytes = file.readAll();
//    }

//    file.close();
//    size = bytes.size();
//    char *buf;
//    buf = bytes.data();
//    decode((unsigned char*)buf,size,res);
//    cv::imshow("test",res);

      FILE *fp = NULL;
      unsigned char buff[65535];

      fp = fopen("./testfile.h264", "rb+"); //第一个逗号前是文件位置。逗号之后是打开文件方式
      int len =  fread(buff,1,65535,fp);
      fclose(fp);  //记得用完关闭文件
      decode(buff,len,res);
      cv::imshow("test",res);

}

//QImage CH264Decoder::rtsp_open()
void CH264Decoder::rtsp_open()
{
       AVFormatContext* format_ctx = avformat_alloc_context();
       AVFormatContext* ptr = NULL;
       AVCodecContext *pAVCodecContext_video = nullptr;
       const AVCodec *pAVCodec_video = nullptr;

       AVCodecParameters *pAVCodePar_video = avcodec_parameters_alloc();
       AVPacket *pAVPacket = av_packet_alloc(); ;                                  // ffmpeg单帧数据包
       AVFrame *pAVFrame_video = av_frame_alloc();                                 // ffmpeg单帧缓存
       AVFrame *pAVFrameRGB32_video = av_frame_alloc();                            // ffmpeg单帧缓存转换颜色空间后的缓存
       AVCodecParserContext *pAVCodeParseContext_video = nullptr;
       struct SwsContext *pSwsContext_video = nullptr;                             // ffmpeg编码数据格式转换
       AVDictionary * opts = nullptr;

       int ret = -1;
       int numBytes = 0;                                                           // 解码后的数据长度
       uchar *outBuffer = nullptr;                                                // 解码后的数据存放缓存区
       //av_register_all();

    // open rtsp: Open an input stream and read the header. The codecs are not opened
       //const char* url = "rtsp://admin:genepoint2020@192.168.100.14:554/cam/realmonitor?channel=1&subtype=0";
       av_dict_set(&opts, "rtsp_transport", "tcp", 0);
       av_dict_set(&opts, "stimeout", "2000000", 0);
       // audio/video stream index
       int video_stream_index = -1;
      ret = avformat_open_input(&format_ctx, "rtsp://192.168.1.108:8554/live", nullptr, nullptr);
           if (ret != 0) {
               printf("Can not open this file");
               //return;
           }
           // Read packets of a media file to get stream information
           ret = avformat_find_stream_info(format_ctx, nullptr);
           if (ret < 0) {
               printf("Unable to get stream info");
              // return ;
           }

           fprintf(stdout, "Number of elements in AVFormatContext.streams: %d\n", format_ctx->nb_streams);
           for (int i = 0; i < format_ctx->nb_streams; ++i) {
               const AVStream *stream = format_ctx->streams[i];
               fprintf(stdout, "type of the encoded data: %d\n", stream->codecpar->codec_id);
               if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                   video_stream_index = i;
                   // 对找到的视频流寻解码器
                   pAVCodePar_video = stream->codecpar;
                   pAVCodec_video = avcodec_find_decoder(pAVCodePar_video->codec_id);
                   if (!pAVCodec_video) {
                       video_stream_index = -1;
                       break;
                   }
                   pAVCodeParseContext_video = av_parser_init(pAVCodec_video->id);
                   if (!pAVCodeParseContext_video) {
                       video_stream_index = -1;
                       break;
                   }
                   pAVCodecContext_video = avcodec_alloc_context3(pAVCodec_video);
                   if (!pAVCodecContext_video) {
                   }
                   if (avcodec_open2(pAVCodecContext_video, pAVCodec_video, NULL) < 0) {
                       video_stream_index = -1;
                       break;
                   }
                   fprintf(stdout, "dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
                           stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);

               }
//               AVFrame *pFrame = av_frame_alloc();    //创建  存储解码器信息*/
//                AVFrame *pFrameRGB = av_frame_alloc(); //创建  存储解码器信息*/

                                                 //解码后的h264数据转换成RGB32
                 SwsContext *img_convert_ctx = sws_getContext(stream->codecpar->width, stream->codecpar->height,
                               pAVCodecContext_video->pix_fmt,stream->codecpar->width, stream->codecpar->height,
                                       AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
           }
           numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, pAVCodecContext_video->width,pAVCodecContext_video->height,1);
           outBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

           av_image_fill_arrays(pAVFrameRGB32_video->data, pAVFrameRGB32_video->linesize, outBuffer, AV_PIX_FMT_RGB32, pAVCodecContext_video->width, pAVCodecContext_video->height, 1);

           int av_size =  pAVCodecContext_video->width * pAVCodecContext_video->height;

           pAVPacket = (AVPacket *) malloc(sizeof(AVPacket));
           av_new_packet(pAVPacket,av_size);

           int as = 0;
           while(av_read_frame(format_ctx,pAVPacket)>=0)
           {
               if(pAVPacket->stream_index==video_stream_index)
               {
                   qDebug("a == %d\n",++as);

                   int ret = avcodec_send_packet(pAVCodecContext_video, pAVPacket);
                   int got_picture = avcodec_receive_frame(pAVCodecContext_video, pAVFrame_video);
                   if (ret<0)
                   {
                       qDebug()<<"decode failed！！！";
                   }

                   if(!got_picture)
                   {
                       sws_scale(pSwsContext_video,(const unsigned char* const*)pAVFrame_video->data,pAVFrame_video->linesize
                                 ,0,pAVCodecContext_video->height,pAVFrameRGB32_video->data,pAVFrameRGB32_video->linesize);

                       QImage img((uchar*)pAVFrameRGB32_video->data[0],pAVCodecContext_video->width,pAVCodecContext_video->height
                               ,QImage::Format_RGB32);

                       //return img;

                   }

               }
           }


           if (video_stream_index == -1) {
               fprintf(stderr, "no video stream\n");
               return;
           }
           //outBuffer = (uchar *) av_malloc(numBytes);


       av_parser_close(pAVCodeParseContext_video);
       av_frame_free(&pAVFrame_video);
       av_frame_free(&pAVFrameRGB32_video);
       av_free(outBuffer);
       av_free(pSwsContext_video);
       avcodec_free_context(&pAVCodecContext_video);
       avformat_close_input(&format_ctx);
}
AVFrame * CH264Decoder::cvmat2avframe(cv::Mat mat) {

    // alloc avframe
    AVFrame *avframe = av_frame_alloc();
    if (avframe && !mat.empty()) {

        avframe->format = AV_PIX_FMT_YUV420P;
        avframe->width = mat.cols;
        avframe->height = mat.rows;
        av_frame_get_buffer(avframe, 0);
        av_frame_make_writable(avframe);
        cv::Mat yuv; // convert to yuv420p first
        cv::cvtColor(mat, yuv, cv::COLOR_BGR2YUV_I420);
        // calc frame size
        int frame_size = mat.cols * mat.rows;
        unsigned char *pdata = yuv.data;
        // fill yuv420
        // yyy yyy yyy yyy
        // uuu
        // vvv
        avframe->data[0] = pdata; // fill y
        avframe->data[1] = pdata + frame_size; // fill u
        avframe->data[2] = pdata + frame_size * 5 / 4; // fill v
    }
    return avframe;
}
