#include "inc/messageform.h"

QPointer<MessageForm> g_form = NULL;
QString g_sn_mac_message;
static int g_mode = 0;

MessageForm::MessageForm(QWidget *parent, const int mode, const int timeout) : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::ApplicationModal);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_DeleteOnClose, false);
    effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(8);
    effect->setColor(Qt::gray);
    effect->setOffset(-5, 5);

    _main_w = MainTestWindow::get_main_test_window()->get_current_res_w;
    _main_h = MainTestWindow::get_main_test_window()->get_current_res_h;

    if (this->objectName().isEmpty()) {
        this->setObjectName(QString::fromUtf8("MessageFormClass"));
    }
    QPalette pa;
    pa.setColor(QPalette::Background,QColor(0xD6, 0xD6, 0xD6));
    this->setPalette(pa);

    if (mode == Message) {
        _dialog_w = 900;
        _dialog_h = 220;
        _lb_text_w = 500;

    } else if (mode == Warnning || mode == Success || mode == SNMAC_Success || mode == SNMAC_Error || mode == DOING) {
        _dialog_w = 400;
        _dialog_h = 220;
        _lb_text_w = 200;
    } else if (mode == SNMAC) {
        _dialog_w = 500;
        _dialog_h = 170;
    }

    resize(_dialog_w, _dialog_h);
    frame = new QFrame(this);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setGeometry(QRect(0, 0, _dialog_w, _dialog_h));
    frame->setGraphicsEffect(effect);

    lb_title = new QLabel(frame);
    lb_title->setObjectName(QString::fromUtf8("lb_title"));
    lb_title->setGeometry(QRect(0, 0, _dialog_w, 20));
    QFont lb_font;
    lb_font.setPointSize(8);

    QFont font;
    font.setPointSize(18);
    font.setWeight(QFont::Bold);
    lb_title->setFont(lb_font);
    lb_title->setStyleSheet("background-color: rgb(0, 255, 255);");
    lb_title->setAlignment(Qt::AlignCenter);
    groupBox = new QGroupBox(frame);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    groupBox->setStyleSheet("QGroupBox{border:none}");

    this->mode = mode;

    if (mode != NOICON) {
        groupBox->setGeometry(QRect(0, 50, _dialog_w, 80));
        groupBox->setFont(font);

        lb_icon = new QLabel(groupBox);
        lb_icon->setObjectName(QString::fromUtf8("lb_icon"));
        lb_icon->setGeometry(QRect(20, 0, 80, 80));

        if (mode == Message) {
            lb_icon->setPixmap(QPixmap("./img/message.png"));

        } else if (mode == Warnning || mode == DOING) {
            lb_icon->setPixmap(QPixmap("./img/warning.png"));

        } else if (mode == SNMAC_Error) {
            lb_icon->setPixmap(QPixmap("./img/failure.png"));

        } else if (mode == SNMAC_Success || mode == Success) {
            lb_icon->setPixmap(QPixmap("./img/correct.png"));
        }
    }

    if (mode != SNMAC) {
        lb_text = new QLabel(groupBox);
        lb_text->setObjectName(QString::fromUtf8("lb_text"));
        lb_text->setGeometry(QRect(100, 0, _lb_text_w, 80));
        lb_text->setFont(font);
        lb_text->setAlignment(Qt::AlignLeading|Qt::AlignCenter);
        lb_text->setWordWrap(true);
    } else {

        fl_snmac = new QFormLayout;
        fl_snmac->setObjectName(QString::fromUtf8("fl_snmac"));
        lb_snmac = new QLabel;
        lb_snmac->setObjectName(QString::fromUtf8("lb_snmac"));
        lb_snmac->setFont(font);
        le_snmac = new QLineEdit;
        le_snmac->setFont(font);
        le_snmac->setObjectName(QString::fromUtf8("le_snmac"));
        le_snmac->setFocusPolicy(Qt::StrongFocus);
        le_snmac->installEventFilter(this);
        fl_snmac->addRow(lb_snmac, le_snmac);
        fl_snmac->setSpacing(10);
        fl_snmac->setMargin(10);
        groupBox->setLayout(fl_snmac);
    }

    if (mode == Message || mode == SNMAC_Error || mode == SNMAC_Success || mode == Warnning || mode == NOICON || mode == SNMAC || mode == Success || mode == DOING)
    {
        if (mode == Warnning || mode == DOING) {
            lb_title->setStyleSheet("background-color: rgb(255, 255, 0);");
        } else if (mode == SNMAC_Error) {
            lb_title->setStyleSheet("background-color: rgb(255, 0, 0);");        
        } else if (mode == Message || mode == SNMAC) {
            lb_title->setStyleSheet("background-color: rgb(174, 238, 238);");
        } else if (mode == SNMAC_Success || mode == Success) {
            lb_title->setStyleSheet("background-color: rgb(188, 238, 104);");
        }

        if (mode == Message) {
            bt_ok = new QPushButton(frame);
            bt_ok->setObjectName(QString::fromUtf8("bt_ok"));
            if (mode == NOICON) {
                bt_ok->setGeometry(QRect(600, 150, 100, 40));
            } else {
                bt_ok->setGeometry(QRect(600, 150, 100, 40));
            }

            bt_ok->setFont(font);
            bt_ok->setText(tr("PASS"));

            bt_fail = new QPushButton(frame);
            bt_fail->setObjectName(QString::fromUtf8("bt_fail"));
            if (mode == NOICON) {
                bt_fail->setGeometry(QRect(750, 150, 100, 40));
            } else {
                bt_fail->setGeometry(QRect(750, 150, 100, 40));
            }

            bt_fail->setFont(font);
            bt_fail->setText(tr("FAIL"));
            connect(bt_ok, SIGNAL(clicked()), this, SLOT(proButtonOK()));
            connect(bt_fail, SIGNAL(clicked()), this, SLOT(proButtonFail()));
            connect(this, SIGNAL(sig_handled_test_result(QString, QString)), UiHandle::get_uihandle(), SLOT(slot_handled_test_result(QString, QString)));

        } else if (mode == SNMAC) {
            QFont font_snmac;
            font_snmac.setPointSize(16);
            bt_snmac = new QPushButton(frame);
            bt_snmac->setObjectName(QString::fromUtf8("bt_snmac"));
            bt_snmac->setGeometry(QRect(400, 120, 80, 30));
            bt_snmac->setFont(font_snmac);
            bt_snmac->setText(tr("取消"));
            connect(bt_snmac, SIGNAL(clicked()), this, SLOT(proButtonCancel()));

        } else if (mode == Warnning) {
            bt_cancle = new QPushButton(frame);
            bt_cancle->setObjectName(QString::fromUtf8("bt_cancel"));
            if (mode == NOICON) {
                bt_cancle->setGeometry(QRect(150, 150, 100, 40));
            } else {
                bt_cancle->setGeometry(QRect(150, 150, 100, 40));
            }

            bt_cancle->setFont(font);
            bt_cancle->setText(tr("退出"));
            connect(bt_cancle, SIGNAL(clicked()), this, SLOT(proButtonQuit()));

        } else if (mode == Success) {
            bt_confirm = new QPushButton(frame);
            bt_confirm->setObjectName(QString::fromUtf8("bt_confirm"));
            bt_confirm->setGeometry(QRect(150, 150, 100, 40));
            bt_confirm->setFont(font);
            if (UiHandle::get_uihandle()->get_is_complete_test()) {
                bt_confirm->setText(tr("关机"));
            } else {
                bt_confirm->setText(tr("下道工序"));
            }
            connect(bt_confirm, SIGNAL(clicked()), this, SLOT(proButtonConfirm()));
            connect(this, SIGNAL(sig_confirm_shut_down_or_next_process(QString)), UiHandle::get_uihandle(), SLOT(slot_confirm_shut_down_or_next_process(QString)));

        } else if (mode == SNMAC_Error) {
            QFont font_snmac_state;
            font_snmac_state.setPointSize(16);

            bt_snmac_cancel = new QPushButton(frame);
            bt_snmac_cancel->setObjectName(QString::fromUtf8("bt_snmac_cancel"));
            bt_snmac_cancel->setGeometry(QRect(250, 150, 80, 30));
            bt_snmac_cancel->setFont(font_snmac_state);
            bt_snmac_cancel->setText(tr("取消"));
            connect(bt_snmac_cancel, SIGNAL(clicked()), this, SLOT(proButtonSnmacCancel()));

            bt_snmac_retry = new QPushButton(frame);
            bt_snmac_retry->setObjectName(QString::fromUtf8("bt_snmac_retry"));
            bt_snmac_retry->setGeometry(QRect(70, 150, 80, 30));
            bt_snmac_retry->setFont(font_snmac_state);
            bt_snmac_retry->setText(tr("重试"));
            connect(bt_snmac_retry, SIGNAL(clicked()), this, SLOT(proButtonSNMACRetry()));
            connect(this, SIGNAL(sig_retry_sn_mac_test()), UiHandle::get_uihandle(), SLOT(slot_retry_sn_mac()));

        } else if (mode == SNMAC_Success) {
            connect(this, SIGNAL(sig_send_sn_mac_test_result(QString, QString)), UiHandle::get_uihandle(), SLOT(slot_recv_sn_mac_test_result(QString, QString)));

        }
    }
    this->timeout = timeout;
    timerId = 0;
}

MessageForm::~MessageForm()
{
    disconnect(this);
    qDebug()<<"~MessageForm()";
}

bool MessageForm::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == le_snmac && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
        {
            QString str = le_snmac->text();
            g_sn_mac_message = str;
            QTimer::singleShot(1000, MainTestWindow::get_main_test_window(), SLOT(compute_result()));
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

void MessageForm::timerEvent(QTimerEvent *evt)
{
    if (evt->timerId() == timerId)
    {
        if (g_mode != SNMAC) {

            killTimer(timerId);
            accept();
        }
        emit sig_send_sn_mac_test_result(_m_test_item, _m_snmac_state);
    }
}

void MessageForm::proButtonOK()
{
    emit sig_handled_test_result(_m_test_item, "PASS");
    accept();
}

void MessageForm::proButtonFail()
{
    emit sig_handled_test_result(_m_test_item, "FAIL");
    reject();
}

void MessageForm::proButtonQuit()
{
    reject();
}

void MessageForm::proButtonCancel()
{
    reject();
}

void MessageForm::proButtonConfirm()
{
    emit sig_confirm_shut_down_or_next_process(bt_confirm->text());
    accept();
}

void MessageForm::proButtonSnmacCancel()
{
    accept();
}

void MessageForm::proButtonSNMACRetry()
{
    emit sig_retry_sn_mac_test();
}

int MessageForm::startExec()
{
    if (timeout)
    {
        timerId = startTimer(timeout);
    }

    show();
    return exec();
}

bool MessageBox(const int mode,const QString &test_item, const QString &title,const QString &text,const int timeout)
{

    int timeoutTemp = timeout;

    g_mode = mode;
    if (g_form != NULL)
    {
        delete g_form;
        g_form = NULL;
    }

    QPointer<MessageForm> form = new MessageForm(NULL, mode, timeoutTemp);
    g_form = form;

    form->setTitle(title);
    if (mode != MessageForm::SNMAC) {
        form->setText(text);
    } else {
        form->setLabel(text);
    }

    if (mode == MessageForm::SNMAC_Success) {
        form->setSNMACState("PASS");
    } else if (mode == MessageForm::SNMAC_Error) {
        form->setSNMACState("FAIL");
    }

    form->setTestItem(test_item);

    int ret = form->startExec();

    delete form;
    form = NULL;
    return !!ret;
}


