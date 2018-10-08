#include "../../inc/ImageTestThread.h"

ImageTestThread* ImageTestThread::_image_test_thread = NULL;
ImageTestThread* ImageTestThread::get_image_test_thread()
{
    if (!_image_test_thread) {
        _image_test_thread = new ImageTestThread();
    }
    return _image_test_thread;
}

ImageTestThread::ImageTestThread(QThread *parent) : QThread(parent)
{
    this->filepath1 = "girl.jpg";
    this->filepath2 = "car.jpg";
    this->_m_stopped = false;
}

ImageTestThread::~ImageTestThread()
{

}

void ImageTestThread::slot_finish_image_test_thread()
{
    if (_image_test_thread->isRunning()) {

        if (NULL != _image_test_thread) {
            disconnect(_image_test_thread);
            _m_stopped = true;
            //quit();
            wait();
            delete _image_test_thread;
            _image_test_thread = NULL;
        }
    }
}

void ImageTestThread::start_run()
{
    this->start();
}

void ImageTestThread::run()
{
    int i = 0;

    QString filepath = filepath1;
    QPixmap _image_file_1;

    while(!_m_stopped) {

        if (i == 0) {
            filepath = filepath2;
            i = 1;
        } else {
            filepath = filepath1;
            i = 0;
        }
        _image_file_1.load(filepath);
        emit sig_send_one_pixmap(_image_file_1);
        QThread::sleep(2);
    }
}
