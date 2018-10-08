#!/bin/sh

xawtv_welcome="/tmp/xawtv_welcome.winid"
if [ -f $xawtv_welcome ];then
	winid=`cat /tmp/xawtv_welcome.winid`
	xdotool key --window $winid Return
else
	echo 'xawtv_welcome.winid is not found'>>/var/log/qt.log
	exit
fi 


