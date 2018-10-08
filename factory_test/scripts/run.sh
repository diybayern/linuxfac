#!/bin/bash
#

export DISPLAY=:0
openbox="openbox"
Client_Name=""
retry_num=30
retry=0
test_status="whole"
lock_status=""

rm -f /tmp/whole_test

#get rg_w_e
if [ -f /etc/sethwinfo ]; then
    . /etc/sethwinfo
    client_type=`echo ${rg_w_e}`
    if [ "$client_type" = "w" ]; then
        Client_Name="IDV_Client"
    elif [ "$client_type" = "e" ]; then
        Client_Name="RCC_Client"
    else
        echo "client_type is error" >> /tmp/run.log
        exit
    fi
    echo "client_type is :$client_type" >> /tmp/run.log
fi

if [ -f /tmp/factory_test.lock ];then
    lock_status=`cat /usr/local/bin/factory/lock`
    if [[ $lock_status =~ $test_status ]];then
        touch /tmp/whole_test
    fi
    while [ $retry -le $retry_num ]
    do
        openbox_status=`ps -ef | grep openbox | grep -v grep`
        Client_Name_status=`ps -ef | grep $Client_Name | grep -v grep`
        echo "openbox:$openbox_status" >> /tmp/run.log
        echo "Client_Name_STATUS:$Client_Name_status" >> /tmp/run.log
        if [[ $openbox_status =~ $openbox ]] && [[ $Client_Name_status =~ $Client_Name ]]; then
            sleep 5
            rm -rf /tmp/factory_test.lock
            echo "ready" >> /tmp/run.log
            break
        else
            #wait openbox and Client_Name satrt
            echo "no ready" >> /tmp/run.log
            sleep 1
        fi
        retry=`expr $retry + 1`
        echo "retry num:$retry" >> /tmp/run.log
    done
fi
    
killall -s 9 factory_test

sleep 0.1

if [ -n "$1" ]; then
    if [ "$1" = "whole" ]; then
        # factory whole_test
        touch /tmp/whole_test
        shift
    fi
fi

cd /usr/local/bin/factory
./factory_test $1 > /var/log/factorytest_stdout.log 2>&1
rm -f /tmp/whole_test
