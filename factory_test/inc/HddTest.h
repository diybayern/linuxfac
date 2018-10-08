#ifndef _HDD_TEST_H
#define _HDD_TEST_H

#include "fac_utils.h"
#include "Control.h"
#include "FuncBase.h"

class Control;

class HddTest : public FuncBase
{
public:
    HddTest();
    static string hdd_test_all(string hdd_cap);
    static bool check_if_hdd_pass();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);

private:
    Control* _control;
};

#endif

