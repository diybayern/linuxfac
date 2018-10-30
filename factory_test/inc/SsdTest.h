#ifndef _SSD_TEST_H
#define _SSD_TEST_H

#include "fac_utils.h"
#include "Control.h"
#include "FuncBase.h"

class SsdTest : public FuncBase
{
public:
    static string screen_log_black;
    static string screen_log_red;
    static bool ssd_test_all(string ssd_cap);
    static bool check_if_ssd_pass();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
};

#endif

