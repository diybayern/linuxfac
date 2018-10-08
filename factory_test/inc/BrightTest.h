#ifndef _BRIGHT_TEST_H
#define _BRIGHT_TEST_H

#include "fac_utils.h"
#include "Control.h"
#include "FuncBase.h"

class Control;

class BrightTest : public FuncBase
{
public:
    BrightTest();
    static int brightness_is_set(const int* const array, int array_cout, int value);
    static void bright_test_all(string bright_level);
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);

private:
    Control* _control;
};

#endif

