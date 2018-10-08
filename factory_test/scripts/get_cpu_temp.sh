#!/bin/bash
#
temp_dir=/sys/devices/platform/coretemp.0/hwmon/hwmon*
temp_file=`ls -l ${temp_dir} | awk '/_input/ {print $9}'`
temp_file_num=`ls -l ${temp_dir} | awk '/_input/ {print $9}' | wc -l`
max_temp=0
for file in ${temp_file}
do
    #cat ${temp_dir}/${file}
    tmp_temp=`cat ${temp_dir}/${file}`
    if [[ ${tmp_temp} -gt ${max_temp} ]];then
        max_temp=${tmp_temp}
    fi
done
max_temp=`expr ${max_temp} / 1000`
echo "${max_temp} ℃"
