#ifndef _EDID_TEST_H
#define _EDID_TEST_H

#include "Control.h"
#include "FuncBase.h"
#include "libx86.h"

typedef struct LRMI_regs reg_frame;

extern int edid_read_i2c_test(int edid_num);

class EdidTest : public FuncBase
{
public:
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);

private:
    static string screen_log_black;
    static string screen_log_red;
    static bool edid_test_all(unsigned int num);
    static bool read_edid(unsigned int controller, char* output);
    static bool do_vbe_ddc_service(unsigned BX, reg_frame* regs);
    static bool do_vbe_service(unsigned int AX, unsigned int BX, reg_frame* regs);
    static bool parse_edid(char* buf);
    static bool lcd_info_test(BaseInfo *baseInfo);
};

#endif
