#!/bin/sh
#

if xawtv -hwscan 2>&1 | grep OK >> /var/log/factory_test.log 2>&1; then
    echo "Starting xawtv ..." >> /var/log/factory_test.log
    echo "VIDEOOK"
else
    echo "Failed to start xawtv, for no camera devices exist!" >> /var/log/factory_test.log
    echo "NOTFOUND"
fi
