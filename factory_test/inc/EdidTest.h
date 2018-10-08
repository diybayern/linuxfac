#ifndef _EDID_TEST_H
#define _EDID_TEST_H

#include "Control.h"
#include "FuncBase.h"
#include "libx86.h"

#define PRINT_RESULT(x)  (x == SUCCESS ? "OK" : "FAIL")
#define PRINT_RESULT1(x) (x == SUCCESS ? "PASS" : "FAIL")

typedef struct LRMI_regs reg_frame;

class Control;

class EdidTest : public FuncBase
{
public:
    EdidTest();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);

private:
    Control* _control;
    static int edid_test_all(unsigned int num);
    static int read_edid(unsigned int controller, char* output);
    static int do_vbe_ddc_service(unsigned BX, reg_frame* regs);
    static int do_vbe_service(unsigned int AX, unsigned int BX, reg_frame* regs);
    static int parse_edid(char* buf);
    static int get_edid_num(BaseInfo* baseInfo);
};

#endif
