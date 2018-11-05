#!/bin/bash
#
log_file=/var/log/factory_test/factory.log
ssd_status_file=/tmp/ssd.status
ssd_exist_result=0
ssd_cap_result=0
min_mount_point_cap=$[$1*9/10]     #unit:G
max_mount_point_cap=$1     #unit:G
mount_point=''

if_ssd_inside()
{
    local devpath
    local devlist
    local ssdflag
    local ret=1
    devpath=`ls /sys/block/sd[a-z] -d 2>/dev/null`
    if [ -n "$devpath" ]; then
        for path in `realpath $devpath  | grep -v -E "/usb[0-9]+/"`
        do
            devlist="$devlist `basename $path`"
        done
    fi
    
    if [ -n $devlist ]; then
        for sdev in $devlist
        do
            ssdflag=`LC_ALL=C lsblk -d -o name,rota /dev/$sdev | grep $sdev | awk '{print $2}'`
            ret=$?
            if [ ! $ret -eq 0 ]; then
                echo 'failed to run cmd: lsblk' >> $log_file
            elif [ $ssdflag -eq 0 ]; then
                if [[ $sdev =~ sd ]]; then
                    mount_point=$sdev
                    ssd_exist_result=1
                    break
                fi
            fi
        done
    fi
    if [ -z ${mount_point} ];then
        echo 'no ssd' >> $log_file
        echo "ssd不存在" > $ssd_status_file
        exit
    fi
}

rm -rf /tmp/ssd.status

if_ssd_inside

mount_point_unit=`fdisk -l /dev/${mount_point} | awk '/\/dev\/'"${mount_point}"'/ {print $4}' | head -1`
mount_point_cap=`fdisk -l /dev/${mount_point} | awk '/\/dev\/'"${mount_point}"'/ {print $3}' | head -1`
mount_point_cap=${mount_point_cap%.*}
cap_unit="GiB"
echo "mount_point:${mount_point},mount_point_unit:${mount_point_unit},mount_point_cap:${mount_point_cap}" >> $log_file
if [[ $mount_point_unit =~ $cap_unit ]];then
    if [ $mount_point_cap -gt $min_mount_point_cap -a $mount_point_cap -lt $max_mount_point_cap ];then
        echo "ssd cap:PASS" >> $log_file
        ssd_cap_result=1
    else
        echo 'ssd cap is wrong' >> $log_file
        echo "ssd容量错误" > $ssd_status_file
        exit
    fi
else
    echo 'cap unit is wrong' >> $log_file
    echo "ssd容量单位错误" > $ssd_status_file
    exit
fi

echo "ssd_exist_result:${ssd_exist_result},ssd_cap_result:${ssd_cap_result}" >> $log_file
if [ ${ssd_exist_result} -eq 1 -a ${ssd_cap_result} -eq 1 ];then    ##${bcache_result} -eq 1 -a 
    echo "ssd test:PASS" >> $log_file
    echo "SUCCESS" > $ssd_status_file
fi

