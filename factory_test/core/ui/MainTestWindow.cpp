#include "../../inc/ui.h"
#include "../../inc/messageform.h"

MainTestWindow::MainTestWindow(QWidget *parent)
        : QDialog(parent)
{
    _desktopWidget = QApplication::desktop();
     connect(&updatetimer, SIGNAL(timeout()), this, SLOT(_record_play_audio()));
}

MainTestWindow::~MainTestWindow()
{
    if (_main_test_window)
    {
        delete _main_test_window;
        _main_test_window = NULL;
    }
}

MainTestWindow* MainTestWindow::_main_test_window = NULL;
MainTestWindow* MainTestWindow::get_main_test_window()
{
    if (!_main_test_window) {
        _main_test_window = new MainTestWindow();
    }
    return _main_test_window;
}

void MainTestWindow::add_interface_test_button(QString item)
{
    InterfaceTestItem listitem;
    listitem.itemname = item;
    _interface_test_list.append(listitem);
}

void MainTestWindow::add_main_label(QString item, QString result)
{
    MainLabelItem listitem;
    listitem.itemname = item;
    listitem.result = result;
    _main_label_item_list.append(listitem);
}

void MainTestWindow::add_main_test_button(QString item)
{
    MainTestItem listitem;
    listitem.itemname = item;
    _main_test_item_list.append(listitem);
}

void MainTestWindow::add_stress_test_label(QString item)
{
    StressTestItem listitem;
    listitem.itemname = item;
    stress_test_item_list.append(listitem);
}

void MainTestWindow::add_complete_or_single_test_label(QString config)
{
    QPalette pa;
    QFont ft;
    ft.setPointSize(18);
    pa.setColor(QPalette::WindowText,Qt::red);

    _lab_complete_or_single_test = new QLabel(config);
    _lab_complete_or_single_test->setFont(ft);
    _lab_complete_or_single_test->setPalette(pa);
}

void MainTestWindow::confirm_test_result_dialog(QString title)
{	
    MessageBox(MessageForm::Message, title, title + "结果确认", "请确认"+title+"结果是PASS 还是 FAIL", 0);
}

void MainTestWindow::confirm_test_result_warning(QString title)
{
    MessageBox(MessageForm::Warnning, title, "警告", title, 0);
}

void MainTestWindow::confirm_test_result_success(QString title)
{
    MessageBox(MessageForm::Success, title, "提示", title, 0);
}

void MainTestWindow::show_sn_mac_message_box(QString sn_mac)
{
    MessageBox(MessageForm::SNMAC, sn_mac,  sn_mac + "测试", sn_mac, 0);
}

void MainTestWindow::confirm_test_result_waiting(QString title)
{
    MessageBox(MessageForm::DOING, title,  "提示", title, 0);
}

void MainTestWindow::get_result_string(QString func, QString result)
{
    if (itemlist.isEmpty())
    {
        return ;
    }

    QObject *obj;
    for (int i = 0 ; i < itemlist.count(); i++)
    {
        QString objname = itemlist.at(i).name;
        if (objname == func)
        {
            obj = itemlist.at(i).label;
            QLabel *b = qobject_cast<QLabel*>(obj);
            b->setPalette(_get_label_color(result));
            b->setText(result);
            break;
        }
    }
}

QPalette MainTestWindow::_get_label_color(QString result)
{
    QPalette pa;
    if (result.compare("PASS") == 0 || result.compare("pass") == 0)
    {
        pa.setColor(QPalette::WindowText,Qt::green);
    }
    else
    {
        pa.setColor(QPalette::WindowText,Qt::red);
    }
    return pa;
}

void MainTestWindow::update_screen_log(QString textInfo)
{
    if (_editInfo == NULL)
    {
        return ;
    }

    _editloglist.append(textInfo+"\n");
    _editInfo->setText(_editloglist);
}

void MainTestWindow::setupUI()
{
    _hbox_bottom_left_layout = new QHBoxLayout;
    _hbox_bottom_left_layout->addLayout(_hbox_main_test_layout);
    _hbox_bottom_left_layout->addStretch();
    _hbox_bottom_left_layout->addLayout(_vbox_test_count_auto_upload_layout);

    _vbox_function_layout = new QVBoxLayout;
    _vbox_function_layout->addLayout(_hbox_main_label_layout);
    _vbox_function_layout->addWidget(_spilter_line);
    _vbox_function_layout->addLayout(_hbox_bottom_left_layout);

    _grid_main_test_window_layout = new QGridLayout(this);
    _grid_main_test_window_layout->addLayout(_vbox_function_layout, 0, 0);
    _grid_main_test_window_layout->addLayout(_vbox_screenlog_layout, 0, 1);
    _grid_main_test_window_layout->setColumnStretch(0, 3);
    _grid_main_test_window_layout->setColumnStretch(1, 4);
}

void MainTestWindow::_create_main_label_layout()
{
    int i;

    if (_main_label_item_list.isEmpty())
    {
        return ;
    }

    _grid_main_label_layout = new QGridLayout;
    _hbox_main_label_layout = new QHBoxLayout;
    for (i = 0 ; i < _main_label_item_list.count(); i++)
    {
        QString label = _main_label_item_list.at(i).itemname;
        QString result = _main_label_item_list.at(i).result;
        _grid_main_label_layout->addWidget(new QLabel(label), i, 0);
        _grid_main_label_layout->addWidget(new QLabel(result), i, 1);
        if (label.contains("SN")) {
            _grid_main_label_layout->addWidget(&_lb_sn_pass_fail, i, 2);
        } else if (label.contains("MAC")) {
            _grid_main_label_layout->addWidget(&_lb_mac_pass_fail, i, 2);
        }
    }

    if (_lab_complete_or_single_test != NULL)
    {
        _grid_main_label_layout->addWidget(_lab_complete_or_single_test, i , 1);
    }

    _hbox_checkbox_auto_upload_log = new QHBoxLayout;
    _checkbox_auto_upload_log = new QCheckBox(tr("自动上传"));
    _checkbox_auto_upload_log->setChecked(true);
    connect(_checkbox_auto_upload_log, SIGNAL(stateChanged(int)), this, SLOT(_auto_upload_state_changed(int)));

    _hbox_checkbox_auto_upload_log->addWidget(_checkbox_auto_upload_log);
    _hbox_main_label_layout->addLayout(_grid_main_label_layout);
    _hbox_main_label_layout->addStretch();
    _hbox_main_label_layout->addLayout(_hbox_checkbox_auto_upload_log);
}

void MainTestWindow::update_sn_mac_state(QString sn_mac, QString result)
{
    if (sn_mac.contains("SN")) {
        _lb_sn_pass_fail.setPalette(_get_label_color(result));
        _lb_sn_pass_fail.setText(result);
    } else {
        _lb_mac_pass_fail.setPalette(_get_label_color(result));
        _lb_mac_pass_fail.setText(result);
    }
}

void MainTestWindow::on_state_changed(int state)
{

    foreach (ItemCheck item, itemlist) {

        if (item.name == sender()->objectName()) {

            emit sig_check_state_changed(item.name, state == Qt::Checked ? true:false);
        }
    }
}

/***********************progressbar dialog begin***********************/
CustomProgressDialog::CustomProgressDialog(QWidget *parent)
        : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::ApplicationModal);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_DeleteOnClose, false);

    if (this->objectName().isEmpty()) {
        this->setObjectName(QString::fromUtf8("CustomProgressDialog"));
    }

    resize(400, 60);
    frame = new QFrame(this);
    frame->setObjectName(QString::fromUtf8("CustomProgressDialogFrame"));
    frame->setGeometry(QRect(0, 0, 400, 60));

    lb_title = new QLabel(frame);
    lb_title->setObjectName(QString::fromUtf8("lb_title"));
    lb_title->setGeometry(QRect(0, 0, 400, 20));
    lb_title->setStyleSheet("background-color: rgb(0, 255, 255);");
    lb_title->setAlignment(Qt::AlignCenter);
    lb_title->setText("正在录音中....");

    progressbar = new QProgressBar(frame);
    progressbar->setObjectName(QString::fromUtf8("progressbar"));
    progressbar->setGeometry(QRect(0, 20, 400, 40));
    progressbar->setStyleSheet("background-color: rgb(214, 214, 214);");
    progressbar->setMaximum(3);
    progressbar->setMinimum(0);
    progressbar->setValue(0);
    progressbar->setFormat("00:0%v");
}

CustomProgressDialog::~CustomProgressDialog()
{
    qDebug()<<"~CustomProgressDialog";
}

void CustomProgressDialog::startExec()
{
    show();
}

/************************progressbar dialog end****************/


void MainTestWindow::start_audio_progress_dialog()
{
    _custom_progress_dialog = new CustomProgressDialog();
    _custom_progress_dialog->startExec();
    this->_status = RECORD;
    updatetimer.start(1000);
}

void MainTestWindow::_record_play_audio()
{
    if (this->_status == RECORD) {

        if (_custom_progress_dialog->progressbar->value()+1 <= _custom_progress_dialog->progressbar->maximum()) {
            _custom_progress_dialog->progressbar->setValue(_custom_progress_dialog->progressbar->value()+1);
        } else {
            this->_status = PLAY;
            _custom_progress_dialog->lb_title->setText("正在播放中....");
            _custom_progress_dialog->progressbar->setValue(0);
            _custom_progress_dialog->lb_title->update();
        }
    } else if (this->_status == PLAY) {
        if (_custom_progress_dialog->progressbar->value()+1 <= _custom_progress_dialog->progressbar->maximum()) {
            _custom_progress_dialog->progressbar->setValue(_custom_progress_dialog->progressbar->value()+1);
        } else {
            this->_status = PLAY_END;
            if (NULL != _custom_progress_dialog) {
                delete _custom_progress_dialog;
                _custom_progress_dialog = NULL;
            }
            updatetimer.stop();
        }
    }

}

void MainTestWindow::_create_main_test_layout()
{
    if (_main_test_item_list.isEmpty())
    {
        return ;
    }

    _grid_main_test_layout = new QGridLayout;
    _vbox_main_test_layout = new QVBoxLayout;
    _hbox_main_test_layout = new QHBoxLayout;

    if (_interface_test_list.isEmpty())
    {
        for (int i = 0 ; i < _main_test_item_list.count(); i++)
        {
            QString name = _main_test_item_list.at(i).itemname;

            iteminfo.name = name;

            QPushButton *button = new QPushButton(name);
            QLabel *label = new QLabel;
            _grid_main_test_layout->addWidget(button, i, 0);
            _grid_main_test_layout->addWidget(label, i, 1);
            iteminfo.button = button;
            iteminfo.label = label;
            _insert_item_record(iteminfo);
        }

    } else {
         QPushButton *button;
         QLabel *label;

         for (int i = 0 ; i < _main_test_item_list.count(); i++)
         {
             if (i == 0)
             {
                button = new QPushButton(_main_test_item_list.at(0).itemname);
                label = new QLabel;
                _grid_main_test_layout->addWidget(button, 0, 0);
                _grid_main_test_layout->addWidget(label, 0, 1);
                iteminfo.name = _main_test_item_list.at(0).itemname;
                iteminfo.button = button;
                iteminfo.label = label;
                _insert_item_record(iteminfo);

                for (int j = 0 ; j < _interface_test_list.count(); j++)
                {
                    QCheckBox*checkbox = new QCheckBox(_interface_test_list.at(j).itemname);
                    checkbox->setChecked(true);
                    checkbox->setObjectName(_interface_test_list.at(j).itemname);
                    connect(checkbox, SIGNAL(stateChanged(int)), this, SLOT(on_state_changed(int)));
                    label = new QLabel;

                    _grid_main_test_layout->addWidget(checkbox, j+1, 1);
                    _grid_main_test_layout->addWidget(label, j+1, 2);
                    iteminfo.name = _interface_test_list.at(j).itemname;
                    iteminfo.checkbox = checkbox;
                    iteminfo.label = label;
                    _insert_item_record(iteminfo);

                    InterfaceTestItemPri if_item;
                    if_item.checkbox = checkbox;
                    if_test_pri_list.append(if_item);
                }

             } else {

                 button = new QPushButton(_main_test_item_list.at(i).itemname);
                 label = new QLabel;
                 _grid_main_test_layout->addWidget(button, i + _interface_test_list.count(), 0);
                 _grid_main_test_layout->addWidget(label, i + _interface_test_list.count(), 1);
                 iteminfo.name = _main_test_item_list.at(i).itemname;
                 iteminfo.button = button;
                 iteminfo.label = label;
                 _insert_item_record(iteminfo);
             }
         }
    }

    _vbox_main_test_layout->addLayout(_grid_main_test_layout);
    _vbox_main_test_layout->addStretch();
    _hbox_main_test_layout->addLayout(_vbox_main_test_layout);
    _hbox_main_test_layout->addStretch();
}

void MainTestWindow::_create_screen_log_layout()
{
    _vbox_screenlog_layout = new QVBoxLayout;
    QFont font;
    font.setWeight(QFont::Light);
    font.setPointSize(10);
    _editInfo = new QTextEdit;
    _editInfo->setReadOnly(true);
    _editInfo->setFont(font);
    _editInfo->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    connect(_editInfo, SIGNAL(textChanged()), this, SLOT(_slot_text_changed()));
    _vbox_screenlog_layout->addWidget(_editInfo);
    _vbox_screenlog_layout->setContentsMargins(0, 0, 0, 20);
}

void MainTestWindow::_slot_text_changed()
{
    _editInfo->moveCursor(QTextCursor::End);
}

bool MainTestWindow::get_auto_upload_check_state()
{
    return _is_auto_upload_checked;
}


void MainTestWindow::_auto_upload_state_changed(int state)
{
    if (state == Qt::Checked) {
        _is_auto_upload_checked = true;

    } else {
        _is_auto_upload_checked = false;
    }
}

void MainTestWindow::_create_test_count_and_upload_layout()
{
    _lab_test_count = new QLabel(tr("测试次数:"));
    _lineedit_test_count = new QLineEdit;
    _lineedit_test_count->setText("1");
    _lineedit_test_count->setValidator(new QIntValidator(0, 1000, this));
    _lineedit_test_count->setFixedWidth(50);

    _hbox_test_count_layout = new QHBoxLayout;
    _vbox_test_count_auto_upload_layout = new QVBoxLayout;
    _hbox_test_count_layout->addStretch();
    _hbox_test_count_layout->addWidget(_lab_test_count);
    _hbox_test_count_layout->addWidget(_lineedit_test_count);

    _vbox_test_count_auto_upload_layout->addLayout(_hbox_test_count_layout);
    _vbox_test_count_auto_upload_layout->addStretch();
}

void MainTestWindow::_create_spilter_line_layout()
{
    _spilter_line = new QFrame;
    _spilter_line->setFixedSize(_desktopWidget->width()*3/7, 3);
    _spilter_line->setFrameShape(QFrame::HLine);
    _spilter_line->setFrameShadow(QFrame::Sunken);
}

void MainTestWindow::_insert_item_record(ItemCheck &record)
{
    itemlist.append(record);
}

void MainTestWindow::draw_main_test_window()
{
    _create_screen_log_layout();
    _create_test_count_and_upload_layout();
    _create_main_test_layout();
    _create_main_label_layout();
    _create_spilter_line_layout();
}

void MainTestWindow::show_main_test_window()
{
    setupUI();
    this->setWindowTitle("工程测试程序");
    get_current_res_h = _desktopWidget->height();
    get_current_res_w = _desktopWidget->width();
    this->resize(_desktopWidget->width(), _desktopWidget->height());
    this->show();
}

QString MainTestWindow::ui_get_test_count()
{
    if (NULL != _lineedit_test_count) {
        return _lineedit_test_count->text();
    }

    return "";
}

void MainTestWindow::compute_result()
{
    emit sig_get_message_from_scangun(g_sn_mac_message);
}

void MainTestWindow::show_stress_test_window()
{
    start_stress_ui();
}

void MainTestWindow::show_display_test_window()
{
    start_display_test_ui();
}

void MainTestWindow::slot_finish_show_stress_window()
{
    if (StressTestWindow::g_get_stress_test_window() != NULL) {
        StressTestWindow::g_get_stress_test_window()->finish_stress_window();
        while(true) {
            if (NULL != StressTestWindow::g_get_stress_test_window()) {
                eventloop.exec();
            } else {
                if (eventloop.isRunning()) {
                    eventloop.exit();
                }
                break;
            }
        }
        emit to_quit_test_window("拷机测试");
    }

}

void MainTestWindow::slot_finish_show_display_window(bool state)
{
    if (NULL != DisplayTestWindow::g_get_display_test_window()) {
        DisplayTestWindow::g_get_display_test_window()->finish_display_window();
        while(true) {
            if (NULL != DisplayTestWindow::g_get_display_test_window()) {
                eventloop.exec();
            } else {
                if (eventloop.isRunning()) {
                    eventloop.exit();
                }
                break;
            }
        }
        if (state == true) {
            emit to_quit_test_window("显示测试");
        }
    }
}

QPushButton* MainTestWindow::_get_interface_test_button()
{
    if (itemlist.isEmpty()) {
        return NULL;
    }

    foreach (ItemCheck item, itemlist) {
        if (0 == item.name.compare("接口测试")) {
            return (QPushButton*)item.button;
        }
    }
    return NULL;
}

void MainTestWindow::_set_interface_test_item_enable(bool state)
{
    if (if_test_pri_list.isEmpty()) {
        return ;
    }

    foreach (InterfaceTestItemPri item, if_test_pri_list) {
        if (state == true) {
            item.checkbox->setEnabled(true);
        } else {
            item.checkbox->setEnabled(false);
        }
    }
}

void MainTestWindow::slot_set_interface_test_state(int state)
{
    if (NULL == _get_interface_test_button()) {
        return ;
    }

    if (UI_INF_RUNNING == state) {
        _get_interface_test_button()->setText("结束");
        _set_interface_test_item_enable(false);

    } else if (UI_INF_RUNEND == state) {
        _get_interface_test_button()->setEnabled(true);
        _get_interface_test_button()->setText("接口测试");
        _set_interface_test_item_enable(true);

    } else { //UI_INF_BREAK
        _get_interface_test_button()->setEnabled(false);
        _set_interface_test_item_enable(false);
    }
}

void MainTestWindow::slot_show_sn_mac_comparison_result(QString sn_mac, QString result)
{
    if (result.compare("PASS") == 0) {
        MessageBox(MessageForm::SNMAC_Success, sn_mac, "扫描成功", sn_mac + "比对成功", 1000);

    } else {
        MessageBox(MessageForm::SNMAC_Error, sn_mac, "警告", sn_mac + "比对失败,请重试！", 0);

    }
}

void MainTestWindow::slot_update_stress_test_pass_or_fail(QString result)
{
    if (StressTestWindow::g_get_stress_test_window() != NULL) {
        StressTestWindow::g_get_stress_test_window()->update_stress_test_pass_or_fail(result);
    }
}

void MainTestWindow::update_stress_label_value(QString item, QString result)
{
    if (StressTestWindow::g_get_stress_test_window() != NULL) {
        StressTestWindow::g_get_stress_test_window()->update_stress_label_value(item, result);
    }

}
