#!/bin/bash
#

g_fac_path=/usr/local/bin/factory

rm -rf /opt/lessons/.tmp_factory_test*

if [ -f $g_fac_path/lock ] && [ -f $g_fac_path/run.sh ];then
    touch /tmp/factory_test.lock
    $g_fac_path/run.sh &
fi
