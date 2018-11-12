#ifndef _BRIGHT_TEST_H
#define _BRIGHT_TEST_H

#include "Control.h"
#include "FuncBase.h"

class BrightTest : public FuncBase
{
public:
    static int brightness_is_set(const int* const array, int array_cout, int value);
    static void bright_test_all(string bright_level);
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
    static int inotify_fd;
    static int wd;
};

#endif

