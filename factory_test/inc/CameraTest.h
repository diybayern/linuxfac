#ifndef _CAMERA_TEST_H
#define _CAMERA_TEST_H

#include "Control.h"
#include "FuncBase.h"

class CameraTest : public FuncBase
{
public:
    static void *test_all(void*);
    void start_test(BaseInfo* baseInfo);
    void start_camera_xawtv_on_stress();
    void close_xawtv_window();

private: 
    static bool camera_test_all();
    static void start_camera_xawtv();
    static unsigned long get_window_id(string winid_file);
    static bool check_if_xawtv_started();
    static void move_xawtv_window(int new_x, int new_y);
    static void move_xawtv_window_on_func_test();
};

#endif

