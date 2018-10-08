#include "../../inc/CameraTest.h"
#include "../../inc/fac_log.h"
#include "../../inc/fac_utils.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define XAWTV_MAX_FAIL_COUNT   (5)

CameraTest::CameraTest()
{
}

unsigned long CameraTest::get_window_id(const char *winid_file)
{
    char winidbuf[CMD_BUF_SIZE];
    unsigned long winid;
	int size = 0;

	memset(winidbuf, 0, CMD_BUF_SIZE);
	if(!get_file_size(winid_file, &size))
		return 0;
	LOG_INFO("%s file size %d\n", winid_file, size);
	
	if(!read_local_data(winid_file, winidbuf, size))
		return 0;

    winid = strtoul(winidbuf, NULL, 16);
    LOG_INFO("%s: xawtv window ID: [0x%x]\n", winid_file, winid);

    /* check if the window exists */
    memset(winidbuf, 0, CMD_BUF_SIZE);
    snprintf(winidbuf, CMD_BUF_SIZE, "xwininfo -id 0x%lx 2>&1", winid);
	string str = execute_command(winidbuf);
	if (strstr(str.c_str(), "X Error")) {
		LOG_ERROR("%s: xawtv window does not exist!\n", winid_file);
		winid = 0;
	}
	
    return winid;
}
 
void CameraTest::move_xawtv_window(int new_x, int new_y)
{
    Display *display;
    unsigned long winid;

    winid = get_window_id("/tmp/xawtv.winid");
    if (winid == 0) {
        LOG_ERROR("Failed to move xawtv window to right-top!\n");
        return;
    }

    display = XOpenDisplay(getenv("DISPLAY"));
    XMoveWindow(display, winid, new_x, new_y);
    XRaiseWindow(display, winid);
    XFlush(display);
    usleep(20000);
    XCloseDisplay(display);
    LOG_ERROR("Move xawtv window to (%d)x(%d) location.\n", new_x, new_y);
}

void CameraTest::move_xawtv_window_on_func_test(void)
{
    int screen_width;
    int new_x, new_y;

    screen_width = Control::get_control()->get_screen_width();

    /* xawtv window size: 384 x 288 */
    new_x = screen_width - 395;
    new_y = 50;
    move_xawtv_window(new_x, new_y);
} 

void CameraTest::start_camera_xawtv()
{
    if (system("/usr/local/bin/factory/start_xawtv.sh") < 0) {
        LOG_ERROR("system run start_xawtv.sh error!\n");
        return ;
    }
    usleep(50000);

    if (system("/usr/local/bin/factory/close_xawtv.sh") < 0) {
        LOG_ERROR("system run close_xawtv.sh error!\n");
        return ;
    }
    usleep(5000);
	move_xawtv_window_on_func_test();
}

bool CameraTest::check_if_xawtv_started()
{
    unsigned long winid;
	Control* control = Control::get_control();

    winid = get_window_id("/tmp/xawtv.winid");
    if (winid == 0) {
        LOG_ERROR("Failed to start xawtv window!\n");
		control->update_screen_log("Failed to start xawtv window!\n");
        return false;
    }

    LOG_INFO("xawtv window started OK.\n");
	control->update_screen_log("xawtv window started OK.\n");
    return true;
}

bool CameraTest::camera_test_all()
{
    int failed_count = 0;
    bool xawtv_ok = false;
	Control* control = Control::get_control();

    /* check if camera device exists */
	string result = execute_command("sh " + CAMERA_CHECK_SCRIPT);
	if (result == "error"){
    	LOG_ERROR("system run error!\n");
    }
    usleep(50000);
    if (result != "VIDEOOK") {
        return false;
    }

    do {
        start_camera_xawtv();
        if (check_if_xawtv_started()) {
            /* xawtv started, show dialog */
            xawtv_ok = true;
            break;
        } else {
            usleep(50000);
            failed_count++;
            LOG_ERROR("xawtv started failed count: %d\n", failed_count);
			control->update_screen_log("xawtv started failed count: " + to_string(failed_count) + "\n");
        }
    } while (failed_count < XAWTV_MAX_FAIL_COUNT);

    if (!xawtv_ok && failed_count >= XAWTV_MAX_FAIL_COUNT) {
        /* xawtv started failed, just report FAIL result */
        LOG_ERROR("ERROR: Failed to start xawtv, GPU fault may be detected!\n");
		control->update_screen_log("ERROR: Failed to start xawtv, GPU fault may be detected!\n");
    }
    return false;
}

void* CameraTest::test_all(void*)
{
	Control* control = Control::get_control();
	control->update_screen_log("==================== camera test ====================\n");
	camera_test_all();	
	control->confirm_test_result(CAMERA_TEST_NAME);
	return NULL;
}

void CameraTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}

void CameraTest::start_camera_xawtv_on_stress()
{
    /* check if camera device exists */
	string result = execute_command("bash " + CAMERA_CHECK_SCRIPT);
	if (result == "error"){
    	LOG_ERROR("system run error!\n");
    }
    usleep(50000);
    if (result != "VIDEOOK") {
        LOG_ERROR("ERROR: Camera device is not found!\n");
        return;
    }

	if (system("/usr/local/bin/factory/start_xawtv.sh") < 0) {
        LOG_ERROR("system run start_xawtv.sh error!\n");
        return ;
    }
    usleep(50000);
	
	if (system("/usr/local/bin/factory/close_xawtv.sh") < 0) {
		LOG_ERROR("system run close_xawtv.sh error!\n");
		return ;
	}
	usleep(5000);
	move_xawtv_window_on_func_test();
}


void CameraTest::close_xawtv_window()
{
    Display *display;
    unsigned long winid;

    winid = get_window_id("/tmp/xawtv.winid");
    if (winid == 0) {
        LOG_ERROR("Failed to close xawtv window!\n");
        return;
    }
	
    display = XOpenDisplay(getenv("DISPLAY"));
    XDestroyWindow(display, winid);
    XFlush(display);
    usleep(20000);
    XCloseDisplay(display);
    LOG_ERROR("xawtv window has been closed.\n");
}

