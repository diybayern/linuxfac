#!/bin/bash
#
hdd_exist_result=0
hdd_cap_result=0
health_result=0
bcache_result=0
dd_result=0
min_mount_point_cap=450     #unit:G
max_mount_point_cap=$1     #unit:G
dd_test_file_size=1024      #unit:M
block_dir="/sys/block/"
whole_test_file="/tmp/whole_test"
mount_dev=''
dd_file_unit=''             #unit:M
mount_point=''
hdd_base=''

if_hdd_inside()
{
    local base=`basename $1`
    local dev=`echo ${base} | sed 's/[0-9]*$//g'`
    ls -l ${block_dir}${dev} | grep "ata" > /dev/null 2>&1
    if [ $? -ne 0 ];then
        return 1
    fi
    return 0
}

judge_bcache_constitute_mmc_hdd()
{
    BCACHE_DEVNUM=$(lsblk | grep '/opt/lessons' | wc -l)
    if [ $BCACHE_DEVNUM -ne 2 ]; then
        echo "Bcache setting error!" >> /var/log/factory_test.log
        return 1
    fi

    lsblk | grep -B1 '/opt/lessons' > /tmp/bcache.list
    num=0
    SDDEV=$(cat /tmp/bcache.list | awk '/sd/ {print $1}' | sed 's/[^a-z0-9]//g')
    MMCBLKDEV=$(cat /tmp/bcache.list | awk '/mmcblk0/ {print $1}' | sed 's/[^a-z0-9]//g')
    if [ -n "$SDDEV" ]; then
        num=$((num + 1))
    fi
    if [ -n "$MMCBLKDEV" ]; then
        num=$((num + 1))
    fi

    if [ $num -ne $BCACHE_DEVNUM ]; then
        echo "Bcache setting error!" >> /var/log/factory_test.log
        return 1
    fi

    echo "Bcache setting OK." >> /var/log/factory_test.log
    return 0
}

rm -rf /tmp/hdd.status

#if hdd exist
mount_point=`python3 /etc/bcache-status -s | awk '/\/dev\/sd/ {print $2}'`
if [ -z "$mount_point" ]; then
    echo 'hdd has bcache info' >> /var/log/factory_test.log
    #hdd has bcache info
    for element in `ls ${block_dir}`
    do
        if [[ ${element} =~ sd* ]];then
            if_hdd_inside ${element}
            if [ $? -ne 0 ];then
                continue
            fi
            #only support one hdd inside
            hdd_base=${element}
            mount_point=${element}
            hdd_exist_result=1
            break
        fi
    done
    if [ -z ${hdd_base} ];then
        echo 'no /dev/sd' >> /var/log/factory_test.log
        echo "hdd is not exist" > /tmp/hdd.status
        exit
    fi
else
    hdd_exist_result=1
fi

#if hdd 500G
#TODO mount_point fix
mount_point=${mount_point##*/}
mount_point=${mount_point:0:3}
mount_point_unit=`fdisk -l /dev/${mount_point} | awk '/\/dev\/'"${mount_point}"'/ {print $4}' | head -1`
mount_point_cap=`fdisk -l /dev/${mount_point} | awk '/\/dev\/'"${mount_point}"'/ {print $3}' | head -1`
mount_point_cap=${mount_point_cap%.*}
cap_unit="GiB"
echo "mount_point:${mount_point},mount_point_unit:${mount_point_unit},mount_point_cap:${mount_point_cap}" >> /var/log/factory_test.log
if [[ $mount_point_unit =~ $cap_unit ]];then
    if [ $mount_point_cap -gt $min_mount_point_cap -a $mount_point_cap -lt $max_mount_point_cap ];then
        echo "hdd cap:PASS" >> /var/log/factory_test.log
        hdd_cap_result=1
    else
        echo 'hdd cap is wrong' >> /var/log/factory_test.log
        echo "the capacity of hdd is wrong" > /tmp/hdd.status
        exit
    fi
else
    echo 'cap unit is wrong' >> /var/log/factory_test.log
    echo "the cap unit of hdd is wrong" > /tmp/hdd.status
    exit
fi

#smartctl test
smartctl --smart=on /dev/${mount_point}
smartctl_result=`smartctl -H /dev/${mount_point} | grep "PASSED"`
echo "smartctl_result:${smartctl_result}" >> /var/log/factory_test.log
if [ -n "$smartctl_result" ];then
    echo "hdd health:PASS" >> /var/log/factory_test.log
    health_result=1
else
    echo 'hdd is not health' >> /var/log/factory_test.log
    echo "hdd is not health" > /tmp/hdd.status
    exit
fi

#judge whole test or PCBA test
if [ ! -f "$whole_test_file" ];then
    echo "PCBA test" >> /var/log/factory_test.log
    bcache_result=1
    dd_result=1
else
    echo "whole test" >> /var/log/factory_test.log
    judge_bcache_constitute_mmc_hdd
    if [ $? -ne 0 ];then
        bcache_result=0
        echo 'bcache is not ready' >> /var/log/factory_test.log
        echo "bcache is not ready" > /tmp/hdd.status
        exit
    else
        bcache_result=1
        #dd test
        echo 'start dd test' >> /var/log/factory_test.log

        #if file size greater than the cache_size,dd writes HDD directly
        cache_size=`cat /sys/block/bcache0/bcache/sequential_cutoff`
        cache_cap=${cache_size%.*}
        cache_unit=`echo ${cache_size} | tr -d '[0-9\.]'`

        if [[ $cache_unit =~ "M" ]]; then
            cache_cap=$[${cache_cap}+1]
            dd_file_unit=${cache_cap}M
        else
            cache_cap=1
            dd_file_unit=1M
        fi
        file_count=`echo $((file_count=${dd_test_file_size}/${cache_cap}))`
        file_last_id=$[${file_count}-1]

        echo "file_count:$file_count;dd_file_unit:$dd_file_unit;file_last_id:$file_last_id" >> /var/log/factory_test.log
        
        rm -f /tmp/.norm_file
        rm -f /opt/lessons/.tmp_factory_test*
        dd if=/dev/urandom of=/tmp/.norm_file bs=${dd_file_unit} count=1
        
        for ((i=0; i<${file_count}; i++))
        do
            dd if=/tmp/.norm_file of=/opt/lessons/.tmp_factory_test bs=${dd_file_unit} seek=$i count=1
        done

        for ((i=0; i<${file_count}; i++))
        do
            dd if=/opt/lessons/.tmp_factory_test of=/opt/lessons/.tmp_factory_test_norm$i bs=${dd_file_unit} skip=$i count=1
            diff /tmp/.norm_file /opt/lessons/.tmp_factory_test_norm$i
            if [ $? -ne 0 ];then
                echo "dd diff error at $i compare." >> /var/log/factory_test.log
                echo "hdd read and write error" > /tmp/hdd.status
                break
            fi

            if [ $i -eq ${file_last_id} ];then
                echo "dd diff success!" >> /var/log/factory_test.log
                dd_result=1
            fi
        done

        rm -rf /tmp/.norm_file
        rm -rf /opt/lessons/.tmp_factory_test*
    fi
fi

echo "hdd_exist_result:${hdd_exist_result},hdd_cap_result:${hdd_cap_result},health_result:${health_result},bcache_result:${bcache_result},dd_result:${dd_result}" >> /var/log/factory_test.log

if [ ${hdd_exist_result} -eq 1 -a ${hdd_cap_result} -eq 1 -a ${health_result} -eq 1 -a ${bcache_result} -eq 1 -a ${dd_result} -eq 1 ];then
    echo "hdd test:PASS" >> /var/log/factory_test.log
    echo "SUCCESS" > /tmp/hdd.status
fi

