#ifndef _FUNCBASE_H
#define _FUNCBASE_H

#include "fac_log.h"

class FuncBase
{
public:
    virtual ~FuncBase(){
        LOG_INFO("~FuncBase()");
    }
    virtual void start_test(BaseInfo* baseInfo) = 0;
};

#endif
