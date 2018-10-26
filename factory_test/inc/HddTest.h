#ifndef _HDD_TEST_H
#define _HDD_TEST_H

#include "fac_utils.h"
#include "Control.h"
#include "FuncBase.h"

class HddTest : public FuncBase
{
public:
    static bool hdd_test_all(string hdd_cap);
    static bool check_if_hdd_pass();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
    static string screen_log_black;
    static string screen_log_red;
};

#endif

