#include <QApplication>
#include "inc/Control.h"
#include "inc/fac_utils.h"
#include "inc/fac_log.h"

void* semi_auto_test_control(void*)
{
    Control* control = Control::get_control();
    while(1){
		int testStep = control->get_test_step();
		FuncFinishStatus* funcFinishStatus = control->get_func_finish_status();
        usleep(500000);
		
        if(testStep != STEP_IDLE){
            if (funcFinishStatus->interface_finish 
				&& funcFinishStatus->sound_finish 
				&& funcFinishStatus->display_finish
				&& funcFinishStatus->bright_finish
				&& funcFinishStatus->camera_finish) {
				if (control->get_auto_upload_mes_status()) {
					control->start_upload_log();
			    }
			}
            
            switch(testStep){
                case STEP_INTERFACE:
                {
                    if(funcFinishStatus->interface_finish && !funcFinishStatus->sound_finish){				
                        LOG_INFO("interface_finish OK.\n");
						control->set_test_step(STEP_SOUND);
                        control->start_sound_test();
                    }
                }break;
				case STEP_SOUND:
                {
                    if(funcFinishStatus->sound_finish && !funcFinishStatus->display_finish){
                        LOG_INFO("sound_finish OK.\n");
						control->set_test_step(STEP_DISPLAY);
                        control->start_display_test();
                    }
                }break;
				case STEP_DISPLAY:
                {
                    if(funcFinishStatus->display_finish && !funcFinishStatus->bright_finish) {
						if (control->get_base_info()->bright_level != "0" || control->get_base_info()->bright_level != "") {
                           LOG_INFO("display_finish OK.\n");
						   control->set_test_step(STEP_BRIGHTNESS);
                           control->start_bright_test();
						}
                        
                    }
                }break;
				case STEP_BRIGHTNESS:
                {
                    if(funcFinishStatus->bright_finish && !funcFinishStatus->camera_finish) {
						if (control->get_base_info()->camera_exist != "0" || control->get_base_info()->camera_exist != "") {
                           LOG_INFO("bright_finish OK.\n");
						   control->set_test_step(STEP_CAMERA);
                           control->start_camera_test();
						}
                        
                    }
                }break;
                default:
                    break;
            }
        }
    }
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

	Control* control = Control::get_control();
	control->init_func_test();
    control->show_main_test_ui();
    

	pthread_t tid;
    pthread_create(&tid,NULL,semi_auto_test_control,NULL);

    return a.exec();
}
