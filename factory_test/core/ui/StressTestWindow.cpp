#include "../../inc/ui.h"

StressTestWindow* StressTestWindow::_stress_test_window = NULL;
StressTestWindow* StressTestWindow::get_stress_test_window()
{
    if (!_stress_test_window) {
        _stress_test_window = new StressTestWindow();
    }
    return _stress_test_window;
}

StressTestWindow* StressTestWindow::g_get_stress_test_window()
{
    return _stress_test_window;
}

#if 0
static void eos_cb (GstBus *bus, GstMessage *msg, CustomData *data) {
    g_print ("End-Of-Stream reached.\n");
    int ret = 0;

    gst_element_set_state (data->playbin, GST_STATE_READY);

    ret = gst_element_set_state (data->playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (data->playbin);
    }
}



void StressTestWindow::mediaPlay()
{
    char file_buf[512];
    int ret = 0;
    GstBus *bus;
    static CustomData data;
    memset(file_buf,0,sizeof(file_buf));
    strncpy(file_buf,"file:",strlen("file:"));
    strcat(file_buf,"/home/rcd/qtfac/movie.mp4");

    gst_init (0, NULL);
    GstElement *playbin = gst_element_factory_make ("playbin", "playbin");
    if (!playbin) {
        g_printerr ("Not all elements could be created.\n");
    }
    g_object_set (playbin, "uri", file_buf, NULL);
    data.playbin = playbin;
    data.window = _lb_video;
    del_data = data;
    WId xwinid = _lb_video->winId();
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY (playbin),xwinid);
    bus = gst_element_get_bus (playbin);
    gst_bus_add_signal_watch (bus);
    g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, &data);

    ret = gst_element_set_state (playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (playbin);
    }
}
#endif

StressTestWindow::StressTestWindow(QWidget *parent)
    : QWidget(parent)
{
    if (this->objectName().isEmpty()) {
        this->setObjectName(QString::fromUtf8("StressTestWindow"));
    }
    st_w = MainTestWindow::get_main_test_window()->get_current_res_w;
    st_h = MainTestWindow::get_main_test_window()->get_current_res_h;
    this->setGeometry(0, 0, st_w, st_h);

    _frame = new QFrame(this);
    _frame->setObjectName(QString::fromUtf8("StressTestFrame"));
    _frame->setGeometry(QRect(0, 0, st_w, st_h));

    // video layout
    _lb_video = new QLabel(_frame);
    _v_a.video_start_x = 0;
    _v_a.video_start_y = st_h/3;
    _v_a.video_w = st_w/2;
    _v_a.video_h = st_h - st_h/3;
    _lb_video->setObjectName(QString::fromUtf8("lb_video"));
    _lb_video->setGeometry(QRect(_v_a.video_start_x, _v_a.video_start_y, _v_a.video_w, _v_a.video_h));
    //_lb_video->installEventFilter(this);

    // image layout
    _lb_image_frame = new QLabel(_frame);
    _im_a.image_start_x = 0;
    _im_a.image_start_y = 0;
    _im_a.image_w = st_w;
    _im_a.image_h = st_h/3;
    _lb_image_frame->setObjectName(QString::fromUtf8("lb_image"));
    _lb_image_frame->setGeometry(QRect(_im_a.image_start_x, _im_a.image_start_y, _im_a.image_w, _im_a.image_h));

    for (int i = 0; i < 4; i++) {
        QLabel* label = new QLabel(_lb_image_frame);
        label->setObjectName(QString::fromUtf8("_lb_image"));
        label->setGeometry(QRect(st_w/4*i, 0, st_w/4, st_h/3));
        image_layout_attr tmp;
        tmp.lb_image = label;
        _image_label_list.append(tmp);
    }

    // info layout
    _lb_info = new QFrame(_frame);
    _if_a.info_start_x = st_w/2;
    _if_a.info_start_y = st_h/3;
    _if_a.info_w = st_w/2/2;
    _if_a.info_h = st_h - st_h/3;
    _lb_info->setObjectName(QString::fromUtf8("lb_info"));
    _lb_info->setGeometry(QRect(_if_a.info_start_x, _if_a.info_start_y, _if_a.info_w, _if_a.info_h));

    _frame_check_pass_fail = new QFrame(_frame);
    _frame_check_pass_fail->setObjectName(QString::fromUtf8("_frame_check_pass_fail"));
    _frame_check_pass_fail->setGeometry(QRect(st_w/2+st_w/2/2, st_h/3, st_w/2/2, st_h - st_h/3));
    _lb_pass_fail = new QLabel(_frame_check_pass_fail);
    _lb_pass_fail->setObjectName(QString::fromUtf8("_lb_pass_fail"));
    _lb_pass_fail->setGeometry(QRect(0, (st_h - st_h/3)/5, st_w/2/2/2, (st_h - st_h/3)/2));

    _group_box = new QGroupBox(_lb_info);
    _group_box->setObjectName(QString::fromUtf8("_group_box"));
    _group_box->setStyleSheet("QGroupBox{border:none}");
    _group_box->setGeometry(QRect(0, 0, _if_a.info_w, _if_a.info_h));
    _form_box = new QFormLayout;
    QFont font;
    if ((st_h <= 1080 && st_h > 1050) && (st_w <= 1920 && st_w > 1680)) {
        font.setPointSize(12);
    } else if ((st_h <= 1050 && st_h > 1024) && (st_w <= 1680 && st_w > 1440)) {
       font.setPointSize(10);
    } else if ((st_h <= 1024 && st_h >= 900) && (st_w <= 1440 && st_w >= 1280)) {
       font.setPointSize(9);
    } else if ((st_h < 900 && st_h >= 720) && (st_w <= 1280 && st_w > 1024)) {
       font.setPointSize(8);
    } else {
       font.setPointSize(10);
    }
    font.setWeight(QFont::Black);

    for (int i = 0; i < MainTestWindow::get_main_test_window()->stress_test_item_list.count(); i++) {
        QString item = MainTestWindow::get_main_test_window()->stress_test_item_list.at(i).itemname;
        QLabel* st_lab = new QLabel(item+" : ");
        st_lab->setFont(font);
        QLabel* st_lab_value = new QLabel;
        st_lab_value->setFont(font);
        _form_box->addRow(st_lab, st_lab_value);
        Stress_Test_Info info;
        info.name = item;
        info.label = st_lab_value;
        stress_test_info_list.append(info);
    }
    _form_box->setSpacing(20);
    _form_box->setMargin(20);
    _group_box->setLayout(_form_box);

    //connect and start thread
    connect(ImageTestThread::get_image_test_thread(), SIGNAL(sig_send_one_pixmap(QPixmap)), this, SLOT(slot_get_one_pixmap(QPixmap)));
#if 0
    connect(VideoTestThread::get_video_test_thread(), SIGNAL(sig_send_one_frame(QImage)), this, SLOT(slot_get_one_frame(QImage)));
#endif
    ImageTestThread::get_image_test_thread()->start_run();
    VideoTestThread::get_video_test_thread()->start_play();
    connect(this, SIGNAL(sig_finish_video_test_thread()), VideoTestThread::get_video_test_thread(), SLOT(slot_finish_video_test_thread()), Qt::QueuedConnection);
    connect(this, SIGNAL(sig_finish_image_test_thread()), ImageTestThread::get_image_test_thread(), SLOT(slot_finish_image_test_thread()), Qt::QueuedConnection);
    connect(this, SIGNAL(sig_finish_show_stress_window()), MainTestWindow::get_main_test_window(), SLOT(slot_finish_show_stress_window()), Qt::QueuedConnection);
}

StressTestWindow::~StressTestWindow()
{
    qDebug()<<"~StressTestWindow";
    //gst_element_set_state (del_data.playbin, GST_STATE_NULL);
    //gst_object_unref (del_data.playbin);
}

QPixmap StressTestWindow::_text2Pixmap(QString text, QColor color)
{
    QFont font;
    font.setPointSize(100);
    QFontMetrics fmt(font);
    QPixmap result(fmt.width(text), fmt.height());
    QRect rect(0,0,fmt.width(text), fmt.height());
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setFont(font);
    painter.setPen(color);
    painter.drawText((const QRectF)(rect),text);
    return result;
}

void StressTestWindow::finish_stress_window()
{
    if (NULL != _stress_test_window) {
        hide();
        this->deleteLater();
        _stress_test_window = NULL;
    }
}

void StressTestWindow::update_stress_test_pass_or_fail(QString result)
{
    if (result.compare("PASS") == 0) {
        _pm_pass_fail = _text2Pixmap(result, QColor(0, 255, 0));
    } else {
        _pm_pass_fail = _text2Pixmap(result, QColor(255, 0, 0));
    }
    _pm_pass_fail = _pm_pass_fail.scaled(st_w/2/2/2, (st_h - st_h/3)/2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _lb_pass_fail->setPixmap(_pm_pass_fail);
}

void StressTestWindow::update_stress_label_value(QString item, QString result)
{
    if (stress_test_info_list.isEmpty()) {
        return ;
    }

    foreach (Stress_Test_Info item_info, stress_test_info_list) {

        if (item.compare(item_info.name) == 0) {

            QLabel* label = item_info.label;
            label->setText(result);
            label->update();
        }
    }
}
#if 0
bool StressTestWindow::eventFilter(QObject *obj, QEvent *event)
{

    if (obj == _lb_video && event->type() == QEvent::Paint) {

        QPainter painter(_lb_video);
        painter.setBrush(Qt::black);
        painter.drawRect(0,0,_lb_video->width(),_lb_video->height()); //先画成黑色

        if (mImage.size().width() <= 0)
            return false;

        //将图像按比例缩放成和窗口一样大小
        QImage img = mImage.scaled(_lb_video->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = _lb_video->width() - img.width();
        int y = _lb_video->height() - img.height();

        x /= 2;
        y /= 2;

        painter.drawImage(QPoint(x,y),img);

    }

    return QObject::eventFilter(obj,event);
}
#endif

void StressTestWindow::start_exec()
{
    show();
}
#if 0
void StressTestWindow::slot_get_one_frame(QImage img)
{
    mImage = img.copy();
    update();
}
#endif

void StressTestWindow::slot_get_one_pixmap(QPixmap pix)
{
    QPixmap fixtmp = pix.scaled(_lb_image_frame->width()/4, _lb_image_frame->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    _set_picture(fixtmp);
}

void StressTestWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        if (NULL != _stress_test_window) {
            emit sig_finish_video_test_thread();
            emit sig_finish_image_test_thread();
            emit sig_finish_show_stress_window();
        }
    }
}

void StressTestWindow::_set_picture(QPixmap& pix)
{
    if (_image_label_list.isEmpty()) {
        return ;
    }

    foreach (image_layout_attr item, _image_label_list) {
        QLabel* lb = item.lb_image;
        lb->setPixmap(pix);
        lb->update();
    }
}

void StressTestWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (NULL != _stress_test_window) {
            emit sig_finish_video_test_thread();
            emit sig_finish_image_test_thread();
            emit sig_finish_show_stress_window();
        }
    }
}

bool start_stress_ui()
{
    StressTestWindow::get_stress_test_window()->start_exec();
    #if 0
    StressTestWindow::get_stress_test_window()->mediaPlay();
    #endif
    return true;
}
