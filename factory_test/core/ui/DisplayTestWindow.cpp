#include "../../inc/ui.h"

DisplayTestWindow* DisplayTestWindow::_display_test_window = NULL;
DisplayTestWindow* DisplayTestWindow::get_display_test_window()
{
    if (!_display_test_window) {
        _display_test_window = new DisplayTestWindow();
    }
    return _display_test_window;
}

DisplayTestWindow* DisplayTestWindow::g_get_display_test_window()
{
    return _display_test_window;
}

DisplayTestWindow::DisplayTestWindow(QWidget *parent)
    : QWidget(parent)
{
    if (this->objectName().isEmpty()) {
        this->setObjectName(QString::fromUtf8("DisplayTestWindow"));
        this->setFocus();
    }
    _st_w = MainTestWindow::get_main_test_window()->get_current_res_w;
    _st_h = MainTestWindow::get_main_test_window()->get_current_res_h;
    this->setGeometry(0, 0, _st_w, _st_h);
    _frame = new QFrame(this);
    _frame->setObjectName(QString::fromUtf8("_frame"));
    _frame->setGeometry(QRect(0,0,_st_w,_st_h));

    _lb_red = new QLabel(_frame);
    _lb_red->setObjectName(QString::fromUtf8("_lb_red"));
    _lb_red->setGeometry(QRect(_st_w/3/2/2, _st_h/3, _st_w/3/2, _st_h/3));

    _lb_green = new QLabel(_frame);
    _lb_green->setObjectName(QString::fromUtf8("_lb_green"));
    _lb_green->setGeometry(QRect(_st_w/3/2/2 + _st_w/3, _st_h/3, _st_w/3/2, _st_h/3));

    _lb_blue = new QLabel(_frame);
    _lb_blue->setObjectName(QString::fromUtf8("_lb_blue"));
    _lb_blue->setGeometry(QRect(_st_w/3/2/2 + _st_w/3*2, _st_h/3, _st_w/3/2, _st_h/3));

    QString str_red = QString::fromLocal8Bit("红");
    QString str_green = QString::fromLocal8Bit("绿");
    QString str_blue = QString::fromLocal8Bit("蓝");

    _pm_red = _text2Pixmap(str_red);
    _pm_green = _text2Pixmap(str_green);
    _pm_blue = _text2Pixmap(str_blue);

    _pm_red = _pm_red.scaled(_st_w/3/2, _st_h/3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _pm_green = _pm_green.scaled(_st_w/3/2, _st_h/3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _pm_blue = _pm_blue.scaled(_st_w/3/2, _st_h/3, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    this->_state = 0;

    connect(this, SIGNAL(sig_finish_show_display_window(bool)), MainTestWindow::get_main_test_window(), SLOT(slot_finish_show_display_window(bool)));
}

DisplayTestWindow::~DisplayTestWindow()
{

}

QPixmap DisplayTestWindow::_text2Pixmap(QString text)
{
    QFont font;
    font.setPointSize(100);
    QFontMetrics fmt(font);
    QPixmap result(fmt.width(text), fmt.height());
    QRect rect(0,0,fmt.width(text), fmt.height());
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setFont(font);
    painter.setPen(QColor(0,0,0));
    painter.drawText((const QRectF)(rect),text);
    return result;
}

void DisplayTestWindow::finish_display_window()
{
    if (NULL != _display_test_window) {
        hide();
        this->deleteLater();
        _display_test_window = NULL;
    }
}

void DisplayTestWindow::start_exec()
{
    show();
}

void DisplayTestWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        switch(_state) {
           case 0:{
                _state = RGB;
                update();
                break;
           }
           case 1: {
                _state = BLACK;
                update();
                break;
           }
           case 2: {
                if (NULL != _display_test_window) {
                    emit sig_finish_show_display_window(true);
                }
                break;
           }
           default:
           break;
        }

    } else if (event->button() == Qt::RightButton) {
        if (NULL != _display_test_window) {
            emit sig_finish_show_display_window(false);
        }
    }

}

void DisplayTestWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        switch(_state) {
            case 0:{
                 _state = RGB;
                 update();
                 break;
            }
            case 1: {
                 _state = BLACK;
                 update();
                 break;
            }
            case 2: {
                 if (NULL != _display_test_window) {
                     emit sig_finish_show_display_window(true);
                 }
                 break;
            }
            default:
            break;
        }

    } else if (event->key() == Qt::Key_Escape) {
        if (NULL != _display_test_window) {
            emit sig_finish_show_display_window(false);
        }
    }

}

void DisplayTestWindow::paintEvent(QPaintEvent* event)
{
    if (event->type() == QEvent::Paint) {
        QPainter painter(this);
        switch(_state) {
          case 0:{
              painter.setBrush(QColor(255, 0, 0)); //red
              painter.drawRect(0, 0, _st_w/3, _st_h);
              painter.setBrush(QColor(0, 255, 0));//green
              painter.drawRect(_st_w/3, 0, _st_w/3, _st_h);
              painter.setBrush(QColor(0, 0, 255));//blue
              painter.drawRect(_st_w/3*2, 0, _st_w/3, _st_h);
              _lb_red->setPixmap(_pm_red);
              _lb_green->setPixmap(_pm_green);
              _lb_blue->setPixmap(_pm_blue);
              break;
          }
          case 1:{
              painter.setBrush(QColor(0, 0, 0)); //black
              painter.drawRect(0, 0, _st_w, _st_h);
              _lb_red->hide();
              _lb_green->hide();
              _lb_blue->hide();
              break;
          }
          case 2:{
              painter.setBrush(QColor(255, 255, 255)); //white
              painter.drawRect(0, 0, _st_w, _st_h);
              break;
          }
        }
    }
}

void start_display_test_ui()
{
    DisplayTestWindow::get_display_test_window()->start_exec();
}
