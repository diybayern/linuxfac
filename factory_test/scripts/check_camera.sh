#!/bin/sh
#
log_file=/var/log/factory.log

if xawtv -hwscan 2>&1 | grep OK >> $log_file 2>&1; then
    echo "Starting xawtv ..." >> $log_file
    echo "VIDEOOK"
else
    echo "Failed to start xawtv, for no camera devices exist!" >> $log_file
    echo "NOTFOUND"
fi
