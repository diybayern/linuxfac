#!/bin/bash
#
fan_rpm_total=0
fan_run_number=5
fan_rpm_aver=0
fan_rpm_norm=$1

/usr/local/bin/cpufanctl 0 255 > /dev/null
sleep 5 #make sure fan speed up
for((i=0;i<$fan_run_number;i++));do
    fan_rpm=`/usr/local/bin/cpufanctl | grep "cpufan_rpm:"`
    fan_rpm=${fan_rpm#*:}
    fan_rpm_total=$(($fan_rpm_total+$fan_rpm))
    sleep 1
done
/usr/local/bin/cpufanctl 3 > /dev/null

fan_rpm_aver=$(($fan_rpm_total / $fan_run_number))
if [ $fan_rpm_aver -gt $fan_rpm_norm ];then
    echo "SUCCESS"
else
    echo $fan_rpm_aver
fi
