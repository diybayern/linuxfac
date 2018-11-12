#include "CameraTest.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

unsigned long CameraTest::get_window_id(string winid_file)
{
    char winidbuf[CMD_BUF_SIZE] = {0, };    //TODO: char[] winidbuf
    unsigned long winid = 0;
    int size = 0;

    if (!get_file_size(winid_file, &size)) {
        LOG_ERROR("get %s size failed", winid_file.c_str());
        return 0;
    }
    LOG_INFO("%s file size %d\n", winid_file.c_str(), size);
    
    if (!read_local_data(winid_file, winidbuf, size)) {
        LOG_ERROR("read %s date failed", winid_file.c_str());
        return 0;
    }

    winid = strtoul(winidbuf, NULL, 16);
    LOG_INFO("%s: xawtv window ID: [0x%x]\n", winid_file.c_str(), winid);

    /* check if the window exists */
    memset(winidbuf, 0, CMD_BUF_SIZE);
    snprintf(winidbuf, CMD_BUF_SIZE, "xwininfo -id 0x%lx 2>&1", winid);
    string str = execute_command(winidbuf, true);
    if (strstr(str.c_str(), "X Error")) {
        LOG_ERROR("%s: xawtv window does not exist!\n", winid_file.c_str());
        winid = 0;
    }
    
    return winid;
}

/* move camera xawtv window to the top right corner (new_x, new_y)*/
void CameraTest::move_xawtv_window(int new_x, int new_y)
{
    Display *display = NULL;
    unsigned long winid = 0;

    winid = get_window_id(CAMERA_WINID_FILE);
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

void CameraTest::move_xawtv_window_on_func_test()
{
    int screen_width;
    int new_x, new_y;

    screen_width = Control::get_control()->get_screen_width();

    /* xawtv camera window size: 384 x 288 */
    new_x = screen_width - 395; // move to the right
    new_y = 50;    // move to the top
    move_xawtv_window(new_x, new_y);
} 

void CameraTest::start_camera_xawtv()
{
    if (system(CAMERA_START_SCRIPT.c_str()) < 0) {  // start camera xawtv window
        LOG_ERROR("system run start_xawtv.sh error!\n");
        return ;
    }
    usleep(50000);

    if (system(CAMERA_CLOSE_SCRIPT.c_str()) < 0) {  // close xawtv welcome window
        LOG_ERROR("system run close_xawtv.sh error!\n");
        return ;
    }
    usleep(5000);

    move_xawtv_window_on_func_test();
}

bool CameraTest::check_if_xawtv_started()
{
    unsigned long winid = 0;
    Control* control = Control::get_control();

    winid = get_window_id(CAMERA_WINID_FILE);
    if (winid == 0) {
        LOG_ERROR("Failed to start xawtv window!\n");
        control->update_color_screen_log("Failed to start xawtv window!\n", "black");
        control->update_color_screen_log("\t错误：摄像头启动失败\n", "red");
        return false;
    }

    LOG_INFO("xawtv window started OK.\n");
    control->update_color_screen_log("xawtv window started OK.\n", "black");
    return true;
}

bool CameraTest::camera_test_all()
{
    int failed_count = 0;
    bool xawtv_ok = false;
    Control* control = Control::get_control();

    /* check if camera device exists */
    string result = execute_command("sh " + CAMERA_CHECK_SCRIPT, true);
    if (result == "error"){
        LOG_ERROR("system run error!\n");
        return false;
    }
    usleep(50000);
    if (result != "VIDEOOK") {
        LOG_ERROR("not found camera devices");
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
            control->update_color_screen_log("xawtv started failed count: " + to_string(failed_count) + "\n", "black");
            control->update_color_screen_log("\t错误：摄像头启动失败" + to_string(failed_count) + "次\n", "red");
        }
    } while (failed_count < XAWTV_MAX_FAIL_COUNT); // if xawtv start failed, try 5 times totally

    if (!xawtv_ok && failed_count >= XAWTV_MAX_FAIL_COUNT) {
        /* xawtv started failed, just report FAIL result */
        LOG_ERROR("ERROR: Failed to start xawtv, GPU fault may be detected!\n");
        control->update_color_screen_log("ERROR: Failed to start xawtv, GPU fault may be detected!\n", "black");
        control->update_color_screen_log("\t错误: xawtv启动失败, 可能存在GPU故障!\n", "red");
    }

    return false;
}

void* CameraTest::test_all(void*)
{
    Control* control = Control::get_control();
    control->update_color_screen_log("==================== " + FUNC_TEST_NAME[F_CAMERA] + " ====================\n", "black");
    camera_test_all();    
    control->show_test_confirm_dialog(FUNC_TEST_NAME[F_CAMERA]);
    return NULL;
}

void CameraTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

/* stress test camera if camera exists */
void CameraTest::start_camera_xawtv_on_stress()
{
    /* check if camera device exists */
    string result = execute_command("bash " + CAMERA_CHECK_SCRIPT, true);
    if (result == "error"){
        LOG_ERROR("system run error!\n");
    }
    usleep(50000);
    if (result != "VIDEOOK") {
        LOG_ERROR("ERROR: Camera device is not found!\n");
        return;
    }

    if (system(CAMERA_START_SCRIPT.c_str()) < 0) {
        LOG_ERROR("system run start_xawtv.sh error!\n");
        return;
    }
    usleep(50000);
    
    if (system(CAMERA_CLOSE_SCRIPT.c_str()) < 0) {
        LOG_ERROR("system run close_xawtv.sh error!\n");
        return;
    }
    usleep(5000);
    move_xawtv_window_on_func_test();
}

/* close camera window When the test result is confirmed or the stress test is exited*/
void CameraTest::close_xawtv_window()
{
    Display *display = NULL;
    unsigned long winid = 0;
    
    winid = get_window_id(CAMERA_WINID_FILE);
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

