#!/bin/sh
log_file=/var/log/qt.log
xawtv_welcome="/tmp/xawtv_welcome.winid"
if [ -f $xawtv_welcome ];then
	winid=`cat /tmp/xawtv_welcome.winid`
	xdotool key --window $winid Return
else
	echo 'xawtv_welcome.winid is not found'>>$log_file
	exit
fi 


