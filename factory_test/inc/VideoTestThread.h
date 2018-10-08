#ifndef VIDEOTESTTHREAD_H
#define VIDEOTESTTHREAD_H

#include <QThread>
#include <QString>
#include <QImage>
#include <QLabel>
#include <QDebug>
#include <QMutexLocker>
#include "ui.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include <libavutil/time.h>
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
}

class VideoTestThread : public QThread
{
    Q_OBJECT
public:
    enum {
        VIDEO_INIT = 0,
        VIDEO_LOOP,
    };

    enum {
        _SUCCESS = 0,
        _FAIL,
        _OUT,
        _AGAIN,
    };
    explicit VideoTestThread(QThread *parent = 0);
    ~VideoTestThread();
    const char* filepath;
    int videoindex;
    int error;
    int sws_width;
    int sws_height;
    int video_type;
    AVFormatContext *pFormatCtx;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrameRGB;
    AVPixelFormat    pixFormat;
    struct SwsContext *img_convert_ctx;
    int ffmpeg_read_stream();
    int ffmpeg_video_decode(unsigned char* buf, int size);
    void ffmpeg_decode_deinit();
    void ffmpeg_init();
    int ffmpeg_video_change_format(AVFrame* frame, int dst_w, int dst_h);
    void run();
    void uninit();
    static VideoTestThread* get_video_test_thread();
    void start_play();
    void stop_play();
    char* filename;

private:
    static VideoTestThread* _video_test_thread;
    bool _m_stopped;

signals:
#if 0
    void sig_send_one_frame(QImage);
#endif
public slots:
    void slot_finish_video_test_thread();
};

#endif//VIDEOTESTTHREAD_H
