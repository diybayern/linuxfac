#ifndef _MEM_TEST_H
#define _MEM_TEST_H

#include "Control.h"
#include "FuncBase.h"

class MemTest : public FuncBase
{
public:
    static bool compare_men_cap(int mem_cap);
    static bool mem_stability_test();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
    static string screen_log_black;
    static string screen_log_red;
};

#endif
