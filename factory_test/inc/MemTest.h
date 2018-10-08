#ifndef _MEM_TEST_H
#define _MEM_TEST_H

#include "Control.h"
#include "FuncBase.h"

class Control;

class MemTest : public FuncBase
{
public:
    MemTest();
    static bool compare_men_cap(int mem_cap);
    static bool mem_stability_test();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);

private:
    Control* _control;
};

#endif