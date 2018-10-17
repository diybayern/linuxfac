#!/bin/sh
#
log_file=/var/log/qt.log
pidof xawtv | xargs kill -9
xawtv &

while :
do
    sleep 0.5
    if xwininfo -name "Welcome to xawtv!" -children >> $log_file 2>&1; then
        break
    fi
done

xwininfo -name "1920x1080, 32 bit TrueColor (LE: bgr-)" -children | awk '/Window id/ {print $4}' > /tmp/xawtv.winid
echo -n "xawtv window ID: " >> $log_file
cat /tmp/xawtv.winid >> $log_file

xwininfo -name "Welcome to xawtv!" -children | awk '/Window id/ {print $4}' > /tmp/xawtv_welcome.winid
echo -n "xawtv welcome window ID: " >> $log_file
cat /tmp/xawtv_welcome.winid >> $log_file

sync
sleep 0.5
