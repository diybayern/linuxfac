#include <QApplication>
#include <execinfo.h>
#include <signal.h>
#include "Control.h"

void* semi_auto_test_control(void*)
{
    Control* control = Control::get_control();
    bool* funcFinishStatus = control->get_func_finish_status();
    while (1) {
        int testStep = control->get_test_step();
        usleep(500000);
        
        if (testStep != STEP_IDLE) {
            bool tmp_func_finish = true;
            for (int i = 0; i < F_STRESS; i++) {
                tmp_func_finish &= funcFinishStatus[i];
                if (!tmp_func_finish) {
                    break;
                }

            }
            if (tmp_func_finish) {
                if (control->get_auto_upload_mes_status() && !control->get_third_product_state()) {
                    control->upload_mes_log();
                }
            }
            
            switch (testStep) {
                case STEP_INTERFACE: {
                    if (funcFinishStatus[F_INTERFACE]) {                
                        LOG_INFO("interface_finish OK.\n");
                        control->set_test_step(STEP_SOUND);
                        if (!funcFinishStatus[F_SOUND]) {
                            control->start_sound_test();
                        }
                    }
                } break;
                
                case STEP_SOUND: {
                    if (funcFinishStatus[F_SOUND]) {
                        LOG_INFO("sound_finish OK.\n");
                        control->set_test_step(STEP_DISPLAY);
                        if (!funcFinishStatus[F_DISPLAY]) {
                            control->start_display_test();
                        }
                    }
                } break;
                
                case STEP_DISPLAY: {
                    if (funcFinishStatus[F_DISPLAY]) {
                        if (control->get_base_info()->bright_level != "0" && control->get_base_info()->bright_level != "") {
                            LOG_INFO("display_finish OK.\n");
                            control->set_test_step(STEP_BRIGHTNESS);
                            if (!funcFinishStatus[F_BRIGHT]) {
                                control->start_bright_test();
                            }
                        }
                    }
                } break;
                
                case STEP_BRIGHTNESS: {
                    if (funcFinishStatus[F_BRIGHT]) {
                        if (control->get_base_info()->camera_exist != "0" && control->get_base_info()->camera_exist != "") {
                            LOG_INFO("bright_finish OK.\n");
                            control->set_test_step(STEP_CAMERA);
                            if (!funcFinishStatus[F_CAMERA]) {
                                control->start_camera_test();
                            }
                        }
                    }
                } break;
                
                default:
                    break;
            }
        }
    }
}

static void print_stack()
{   
    size_t i;
    size_t size;
    char **strings;
    void *array[200] = {0, };

    size = backtrace(array, 200);
    strings = backtrace_symbols(array, size);

    if (!strings) {
        LOG_ERROR("Failed to get backtrace symbols!\n");
        return;
    }
    
    for (i = 0; i < size; i++) {
        LOG_ERROR("%s\n", strings[i]);
    }

    free (strings);
}


void mmr_defalut_handle_signo(int signo)
{  
    LOG_ERROR("signo=%d\n", signo);
    print_stack();
    exit(1);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setWeight(QFont::DemiBold);

    QDesktopWidget* dtw = QApplication::desktop();

    if ((dtw->height() <= 1080 && dtw->height() > 1050)
           && (dtw->width() <= 1920 && dtw->width() > 1680)) {
       font.setPointSize(10);
    } else if ((dtw->height() <= 1050 && dtw->height() > 1024)
           && (dtw->width() <= 1680 && dtw->width() > 1440)) {
       font.setPointSize(9);
    } else if ((dtw->height() <= 1024 && dtw->height() >= 900)
              && (dtw->width() <= 1440 && dtw->width() >= 1280)) {
       font.setPointSize(10);
    } else if ((dtw->height() < 900 && dtw->height() >= 720)
              && (dtw->width() <= 1370 && dtw->width() > 1024)) {
       font.setPointSize(8);
    } else {
       font.setPointSize(9);
    }
    a.setFont(font);

    atexit(print_stack);
    signal(SIGPIPE, SIG_IGN);    
    signal(SIGKILL, mmr_defalut_handle_signo);
    signal(SIGTERM, mmr_defalut_handle_signo);
    signal(SIGSEGV, mmr_defalut_handle_signo);

    LOG_INFO("************************************************************");
    LOG_INFO("******************** start factory test ********************");
    LOG_INFO("************************************************************");
    Control* control = Control::get_control();
    control->init_func_test();
    control->show_main_test_ui();

    pthread_t tid;
    pthread_create(&tid, NULL, semi_auto_test_control, NULL);

    return a.exec();
}
