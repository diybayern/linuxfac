#!/bin/sh
#

pidof xawtv | xargs kill -9
xawtv &

while :
do
    sleep 0.5
    if xwininfo -name "Welcome to xawtv!" -children >> /var/log/qt.log 2>&1; then
        break
    fi
done

xwininfo -name "1920x1080, 32 bit TrueColor (LE: bgr-)" -children | awk '/Window id/ {print $4}' > /tmp/xawtv.winid
echo -n "xawtv window ID: " >> /var/log/qt.log
cat /tmp/xawtv.winid >> /var/log/qt.log

xwininfo -name "Welcome to xawtv!" -children | awk '/Window id/ {print $4}' > /tmp/xawtv_welcome.winid
echo -n "xawtv welcome window ID: " >> /var/log/qt.log
cat /tmp/xawtv_welcome.winid >> /var/log/qt.log

sync
sleep 0.5
