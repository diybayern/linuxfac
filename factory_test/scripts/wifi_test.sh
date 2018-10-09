#! /bin/bash

#
#  wifi factory test bash
#  1. test wifi scan
#  2. test wifi connect
#  3. dhcp wifi ip address
#  4. test wifi send/recv packet(not in this bash)
#

TEST_WIFI="sfc-test"

SIGNAL_THRESHOLD=70
VORBOSE_LOG=0

SCAN_SUCCESS=0
CONNECT_SUCCESS=0
IP_SUCCESS=0
IS_WEP_PASSWD=0
IS_WPA_PASSWD=0

FACOTORY_DIR=/usr/local/bin/factory
WIFI_SCAN_AWK=$FACOTORY_DIR/wifi_scan.sh
WIFI_SCAN_RESULT=/tmp/wifi_scan.tmp
WIFI_TEST_CONFIG_INFO=/tmp/wifi_test_info.tmp
WIFI_TEST_RESULT=/tmp/wifi.status
WIFI_SSID_MAC=/tmp/ssid.mac
SSID=$1
PASSWD=$2
ENP=$3
# WIFI_CONFIG_FILE=/tmp/wifi_config.ini
FACTORY_SHELL_LOG=/var/log/factory_test_bash.log
WPA_CONFIG=/tmp/wpa.conf
WPA_SUPPLICANT_EXIST=0
FACTORY_WIFI_TEST_LOCK=/tmp/.wifi_test.lock

SCAN_CONNECT_TIME=3
CONNECT_WAIT_TIME=5

function close_wifi()
{
    sudo ifconfig wlan0 down
}

function open_wifi()
{
    sudo ifconfig wlan0 up
}

function check_wifi_connect()
{
    WIFI_NAME=`iw dev wlan0 link | awk '/SSID:/{print $2}'`

    if [ $VORBOSE_LOG -eq 1 ]; then
        iw dev wlan0 link
        echo "try dump wifi name is :" $WIFI_NAME | tee -a $FACTORY_SHELL_LOG
    fi

    if [ "$WIFI_NAME" = "$TEST_WIFI" ]; then
        echo "now connect wifi $TEST_WIFI success" | tee -a $FACTORY_SHELL_LOG
        CONNECT_SUCCESS=1
    fi
}

function check_ip_addr()
{
    IP=`ifconfig wlan0 | sed -n '2p' | cut -d: -f2 | awk '{print $1}'`
    echo "wlan ip address is :" $IP

    # check ip addr is correct
    if echo $IP | grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$" > /dev/null; then
        VALID_CHECK=$(echo $IP | awk -F. '$1<=255&&$2<=255&&$3<=255&&$4<=255{print "yes"}')
        if [ ${VALID_CHECK:-no} = "yes" ]; then
            echo "IP $IP available." | tee -a $FACTORY_SHELL_LOG
            IP_SUCCESS=1
        fi
    fi
}

function start_wpa_supplicant_process()
{
    echo "ctrl_interface=/var/run/wpa_supplicant" > $WPA_CONFIG 
    echo "ctrl_interface_group=0" >> $WPA_CONFIG 
    echo "update_config=1" >> $WPA_CONFIG
    wpa_supplicant -i wlan0 -c $WPA_CONFIG  -B
}

function connect_wifi_with_wpacli()
{
    NETWORK_ID=`wpa_cli -i wlan0 add_network`
    echo "network id is :"$NETWORK_ID | tee -a $FACTORY_SHELL_LOG
    wpa_cli -i wlan0 set_network $NETWORK_ID ssid "\"$TEST_WIFI\""
    if [ $IS_WPA_PASSWD -eq 1 ]; then
        wpa_cli -i wlan0 set_network $NETWORK_ID psk "\"$PASSWD\""
    elif [ $IS_WEP_PASSWD -eq 1 ]; then
        wpa_cli -i wlan0 set_network $NETWORK_ID key_mgmt NONE
        wpa_cli -i wlan0 set_network $NETWORK_ID wep_key0 "\"$PASSWD\""
    else
        wpa_cli -i wlan0 set_network $NETWORK_ID key_mgmt NONE
    fi

    wpa_cli -i wlan0 enable_network $NETWORK_ID 
    wpa_cli -i wlan0 select_network $NETWORK_ID
}

echo "start wifi factory test...."

rm -f $WIFI_SCAN_RESULT $WIFI_TEST_RESULT $WPA_CONFIG $WIFI_TEST_CONFIG_INFO

open_wifi

while getopts "n:s:v" opt
do
    case $opt in
        n)TEST_WIFI=$OPTARG;;
        s)SIGNAL_THRESHOLD=$OPTARG;;
        v)VORBOSE_LOG=1;;
        ?)echo "invaild arg";;
    esac
done

# try to find wifi config ini and parse ssid & passwd
# write wifi test config info to tmp file for factory test could show them on screan

if [ -z "$SSID" || -z "$ENP" || -z "$PASSWD" ]; then
    echo "NOT FOUNT WIFI CONFIG FILE. USE THE DEFAULT CONFIG INFO. SSID : $TEST_WIFI ." | tee $WIFI_TEST_CONFIG_INFO
else
    if [ -n "$SSID" ]; then
        TEST_WIFI=$SSID
    fi

    if [ "$ENP" = "WPA" ]; then
        IS_WPA_PASSWD=1
        echo "now use wpa wifi connect"
    elif [ "$ENP" = "WEP" ]; then
        IS_WEP_PASSWD=1
        echo "now use wep wifi connect"
    else
        echo "now use none enp wifi connect"
    fi

    echo "parse netword ssid is : $SSID ,password is: $PASSWD , enp is $ENP" | tee -a $FACTORY_SHELL_LOG
    echo "DETECTED WIFI CONFIG FILE. SSID : $TEST_WIFI  PASSWORD : $PASSWD  ENP : $ENP ." | tee $WIFI_TEST_CONFIG_INFO
fi

# test wifi scan function

echo "start wlan scan $TEST_WIFI ....  signal threshold is : $SIGNAL_THRESHOLD" | tee -a $FACTORY_SHELL_LOG

for i in `seq 1 $SCAN_CONNECT_TIME`
do
    sudo iw wlan0 scan | awk -f $WIFI_SCAN_AWK > $WIFI_SCAN_RESULT

    if [ $VORBOSE_LOG -eq 1 ]; then
        cat $WIFI_SCAN_RESULT | tee -a $FACTORY_SHELL_LOG
    else
        cat $WIFI_SCAN_RESULT >> $FACTORY_SHELL_LOG
    fi

    echo "finish wlan scan !"

    WIFI_NAME=`grep -w $TEST_WIFI $WIFI_SCAN_RESULT | awk '{print $5}' | head -1`

    if [ $VORBOSE_LOG -eq 1 ]; then
        echo "try scan wifi name is :" $WIFI_NAME
    fi

    if [ "$WIFI_NAME" = "$TEST_WIFI" ]; then
        echo "scan wifi $TEST_WIFI success" | tee -a $FACTORY_SHELL_LOG
    else
        sleep 3
        continue
    fi

    # get test_wifi ssid signal in wlan0 scan result
    SCAN_SIGNAL=`grep -w $TEST_WIFI $WIFI_SCAN_RESULT | awk '{print $3}' | sort  | head -1`
    SCAN_SIGNAL=`echo ${SCAN_SIGNAL%.*} | egrep -o '[0-9]+'`

    echo "scan wifi signal is : -$SCAN_SIGNAL dBm" | tee -a $FACTORY_SHELL_LOG

    if [ $SCAN_SIGNAL -le $SIGNAL_THRESHOLD ]; then
        echo "wifi signal test psss !" | tee -a $FACTORY_SHELL_LOG
        SCAN_SUCCESS=1
        break
    else
        sleep 3
        continue
    fi
done

# check wifi scan test pass or not
if [ $SCAN_SUCCESS -eq 0 ]; then
    if [ $SCAN_SIGNAL -gt 0 ]; then
        echo "wifi scan signal test failed ! signal too weak : - $SCAN_SIGNAL dbm" | tee $WIFI_TEST_RESULT
    else
        echo "wifi scan signal test failed ! cannot scan the wifi : $TEST_WIFI" | tee $WIFI_TEST_RESULT
    fi
    close_wifi
    exit 1
fi

# check if need to start wpa_supplicant process

WPA_PID=`ps x | grep "wpa_supplicant" | grep -v grep | awk '{print $1}'`

if [ -n "$WPA_PID" ]; then
    echo "wpa supplicant process has already up"
    WPA_SUPPLICANT_EXIST=1
elif [ $IS_WPA_PASSWD -eq 1 ]; then
    echo "wpa supplicant not exist"
    start_wpa_supplicant_process
fi

touch $FACTORY_WIFI_TEST_LOCK
# test wifi connect function
echo "start wlan connect $TEST_WIFI...." | tee -a $FACTORY_SHELL_LOG
for i in `seq 1 $SCAN_CONNECT_TIME`
do
    sudo iw wlan0 disconnect
    wpa_cli -i wlan0 disconnect

    sleep 1
        
    # check if wpa or wep enp mode,and connect wifi with different function
    if [ $WPA_SUPPLICANT_EXIST -eq 1 ]; then
        connect_wifi_with_wpacli
    else
        if [ $IS_WPA_PASSWD -eq 1 ]; then
            connect_wifi_with_wpacli
        elif [ $IS_WEP_PASSWD -eq 1 ]; then
            sudo iw wlan0 connect $TEST_WIFI key 0:$PASSWD
        else
            sudo iw wlan0 connect $TEST_WIFI
        fi
    fi

    for j in `seq 1 $CONNECT_WAIT_TIME`
    do
        sleep 1
        check_wifi_connect
        if [ $CONNECT_SUCCESS -eq 1 ]; then
            break
        fi
    done

    if [ $CONNECT_SUCCESS -eq 1 ]; then
        break
    fi
done

# check wifi connected or not
if [ $CONNECT_SUCCESS -ne 1 ];  then
    echo "connect wifi test failed!" | tee $WIFI_TEST_RESULT
    rm $FACTORY_WIFI_TEST_LOCK
    close_wifi
    exit 1
fi

# get wlan ip address
echo "get wifi ip address..."

dhclient wlan0
check_ip_addr

if [ $IP_SUCCESS -eq 1 ]; then
    echo "wifi factory shell test pass!" | tee -a $FACTORY_SHELL_LOG
    echo "SUCCESS" > $WIFI_TEST_RESULT
    ssid_mac=`iw wlan0 station dump | awk '/Station/ {print $2}'`
    echo $ssid_mac > $WIFI_SSID_MAC
else
    echo "IP not available!" | tee $WIFI_TEST_RESULT
    rm $FACTORY_WIFI_TEST_LOCK
    exit 1
fi

rm $FACTORY_WIFI_TEST_LOCK
exit 0
