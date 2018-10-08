#ifndef IMAGETESTTHREAD_H
#define IMAGETESTTHREAD_H

#include <QThread>
#include <QString>
#include <QImage>
#include <QDebug>

#include "ui.h"

class ImageTestThread : public QThread
{
    Q_OBJECT

public:
    explicit ImageTestThread(QThread *parent = 0);
    ~ImageTestThread();
    static ImageTestThread* get_image_test_thread();
    void start_run();

private:
    void run();
    static ImageTestThread* _image_test_thread;

    QString filepath1;
    QString filepath2;
    bool _m_stopped;

signals:
    void sig_send_one_pixmap(QPixmap);

public slots:
    void slot_finish_image_test_thread();
};





#endif//IMAGETESTTHREAD_H
