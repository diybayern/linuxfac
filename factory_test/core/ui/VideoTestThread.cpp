#include "../../inc/VideoTestThread.h"

VideoTestThread* VideoTestThread::_video_test_thread = NULL;
VideoTestThread* VideoTestThread::get_video_test_thread()
{
    if (!_video_test_thread) {
        _video_test_thread = new VideoTestThread();
    }
    return _video_test_thread;
}

VideoTestThread::VideoTestThread(QThread *parent) : QThread(parent)
{
    this->_m_stopped = false;
    this->videoindex = -1;
    this->filepath = "movie.mp4";
    this->pFormatCtx        = NULL;
    this->pCodec            = NULL;
    this->pCodecCtx         = NULL;
    this->img_convert_ctx   = NULL;
    this->sws_width    = MainTestWindow::get_main_test_window()->get_current_res_w/2;
    this->sws_height   = MainTestWindow::get_main_test_window()->get_current_res_h/3*2;

    this->video_type   = VIDEO_INIT;
    this->ffmpeg_init();
}

VideoTestThread::~VideoTestThread()
{
    this->uninit();
}

int VideoTestThread::ffmpeg_read_stream()
{
    //(0) alloc storage to save the info of the video
    pFormatCtx = avformat_alloc_context();
    if (NULL == pFormatCtx) {
        qDebug()<<"malloc format failed.";
        return _FAIL;
    }

    //(1) open video file
    if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0)
    {
           qDebug()<<"Couldn't open input stream.";
           return _FAIL;
    }

    //(2) to get which format of the video
    /*
    videoindex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoindex < 0) {
        qDebug()<<"not video find in " <<filepath;
        return _FAIL;
    }*/
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        printf("Couldn't find stream information.\n");
        return _FAIL;
    }

    //(3) to get which format of the video
    for(uint i=0; i<pFormatCtx->nb_streams; i++)
    {
       if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
       {
           videoindex=i;
           break;
       }
    }
    //printf("the videoindex is :%d\n",videoindex);
    if(videoindex==-1)
    {
        printf("Didn't find a video stream.\n");
        return _FAIL;
    }

    //(3) to find the decoder
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    pCodecCtx->codec_id = pCodec->id;

    if(pCodec == NULL)
    {
        qDebug()<<"Codec not found.";
        return _FAIL;
    }

    //(4) open the decoder
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        qDebug()<<"Could not open codec.";
        return _FAIL;
    }

    av_dump_format(pFormatCtx, 0, filepath, 0);

    return _SUCCESS;
}

int VideoTestThread::ffmpeg_video_change_format(AVFrame* frame, int dst_w, int dst_h)
{
    int numBytes = 0;
    AVFrame* pFrameRGB = NULL;
    unsigned char* buffer = NULL;

    pFrameRGB = avcodec_alloc_frame();
    if (NULL == pFrameRGB) {
        return _FAIL;
    }

    img_convert_ctx = sws_getCachedContext(img_convert_ctx, frame->width,
                frame->height, AV_PIX_FMT_YUV420P, dst_w, dst_h, AV_PIX_FMT_RGBA,
                SWS_BILINEAR, NULL, NULL, NULL);

    if (NULL == img_convert_ctx) {
        return _FAIL;
    }

    numBytes = avpicture_get_size(AV_PIX_FMT_RGBA, dst_w, dst_h);


    buffer = (unsigned char *) av_malloc(numBytes * sizeof(unsigned char));
    if (NULL == buffer) {
        return _FAIL;
    }

    avpicture_fill((AVPicture *) pFrameRGB, buffer, AV_PIX_FMT_RGBA, dst_w, dst_h);

    sws_scale(img_convert_ctx, (const unsigned char* const *) frame->data,
                        frame->linesize, 0, frame->height, pFrameRGB->data,
                        pFrameRGB->linesize);

    QImage tmpImg((uchar *)buffer, dst_w, dst_h, QImage::Format_RGBA8888);
    QPixmap pixmap2(QPixmap::fromImage (tmpImg));

    if (NULL != StressTestWindow::g_get_stress_test_window()) {
        StressTestWindow::g_get_stress_test_window()->_lb_video->setPixmap(pixmap2);
    }

    av_free(buffer);
    av_frame_unref(pFrameRGB);
    av_free(pFrameRGB);

    return _SUCCESS;
}

int VideoTestThread::ffmpeg_video_decode(unsigned char* buf, int size)
{
    int ret = 0;
    int decoded = 0;

    AVPacket pkt;
    AVFrame frame;
    av_init_packet(&pkt);
    pkt.data = buf;
    pkt.size = size;

    avcodec_get_frame_defaults(&frame);

    ret = avcodec_decode_video2(pCodecCtx, &frame, &decoded, &pkt);
    if (ret < 0) {
	av_free_packet(&pkt);
        qDebug()<<"avcodec_decode_video failed ret = "<<ret;
        return _FAIL;
    } else if (!decoded) {
	av_free_packet(&pkt);
        qDebug()<<"no frame is decoded.";
        return _FAIL;
    }

    ret = ffmpeg_video_change_format(&frame, sws_width, sws_height);

    if (_OUT == ret){
        qDebug()<<"exit video test.";
    }
    return ret;
}

void VideoTestThread::ffmpeg_decode_deinit()
{
    if (NULL != pCodecCtx) {
        avcodec_close(pCodecCtx);
        pCodecCtx = NULL;
    }

    if (NULL != pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
    }
}

void VideoTestThread::ffmpeg_init()
{
    av_register_all();
}

void VideoTestThread::start_play()
{
    this->start();
}

void VideoTestThread::stop_play()
{
    if (_video_test_thread->isRunning()) {
        if (NULL != _video_test_thread) {
            disconnect(_video_test_thread);
            _m_stopped = true;
            quit();
            wait();
            delete _video_test_thread;
            _video_test_thread = NULL;
        }
    }
}

void VideoTestThread::slot_finish_video_test_thread()
{
    stop_play();
}

void VideoTestThread::run()
{
    int ret = 0;
    int err = -1;

    AVPacket pkt;

    //(1) here we want to get frame
    while(!_m_stopped)
    {
        if (video_type == VIDEO_INIT) {
            video_type = VIDEO_LOOP;
            ffmpeg_decode_deinit();

            ret = ffmpeg_read_stream();
            if (ret == _FAIL) {
                qDebug()<<"read ffmpeg stream fail.";
                break;
            }
        }

        av_init_packet(&pkt);
        err = av_read_frame(pFormatCtx, &pkt);
        if (err < 0) {
            if (err == AVERROR_EOF) {
                qDebug()<<"read data end.";
                video_type = VIDEO_INIT;
                continue;
            }
        }

        if (pkt.stream_index == videoindex) {
            ret = ffmpeg_video_decode(pkt.data, pkt.size);
            if (_OUT == ret) {
                break;
            }
            usleep(40000);
        }

        av_free_packet(&pkt);
     }
}


void VideoTestThread::uninit()
{
    if (NULL != img_convert_ctx) {
        sws_freeContext(img_convert_ctx);
    }

    if (NULL != pCodecCtx) {
        avcodec_close(pCodecCtx);
    }

    if (NULL != pFormatCtx) {
        avformat_close_input(&pFormatCtx);
    }
}
