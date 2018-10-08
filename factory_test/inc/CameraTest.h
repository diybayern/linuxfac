#ifndef _CAMERA_TEST_H
#define _CAMERA_TEST_H

#include "fac_utils.h"
#include "Control.h"
#include "FuncBase.h"

class Control;

class CameraTest : public FuncBase
{
public:
    CameraTest();
    static bool camera_test_all();
    static void *test_all(void*);
    void start_test(BaseInfo* baseInfo);
    static void start_camera_xawtv_on_stress();
    static void close_xawtv_window();

private: 
    Control* _control;
    static void start_camera_xawtv();
    static unsigned long get_window_id(const char *winid_file);
    static bool check_if_xawtv_started();
    static void move_xawtv_window(int new_x, int new_y);
    static void move_xawtv_window_on_func_test(void);
};

#endif

