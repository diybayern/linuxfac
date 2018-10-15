#!/bin/bash
#
mem_result_file=/usr/local/bin/factory/mem_result_file
mem_ui_log=/usr/local/bin/factory/mem_ui_log

rm -f $mem_ui_log

result=`memtester 10M 1 | grep -v "Loop" | grep ":" > $mem_result_file`

resultStuckAddress=`cat $mem_result_file | grep "ok" | awk '/Stuck Address/ {print $1}'`
resultRandomValue=`cat $mem_result_file | grep "ok" | awk '/Random Value/ {print $1}'`
resultCompareXOR=`cat $mem_result_file | grep "ok" | awk '/Compare XOR/ {print $1}'`
resultCompareSUB=`cat $mem_result_file | grep "ok" | awk '/Compare SUB/ {print $1}'`
resultCompareMUL=`cat $mem_result_file | grep "ok" | awk '/Compare MUL/ {print $1}'`
resultCompareDIV=`cat $mem_result_file | grep "ok" | awk '/Compare DIV/ {print $1}'`
resultCompareOR=`cat $mem_result_file | grep "ok" | awk '/Compare OR/ {print $1}'`
resultCompareAND=`cat $mem_result_file | grep "ok" | awk '/Compare AND/ {print $1}'`
resultSequentialIncrement=`cat $mem_result_file | grep "ok" | awk '/Sequential Increment/ {print $1}'`
resultSolidBits=`cat $mem_result_file | grep "ok" | awk '/Solid Bits/ {print $1}'`
resultBlockSequential=`cat $mem_result_file | grep "ok" | awk '/Block Sequential/ {print $1}'`
resultCheckerboard=`cat $mem_result_file | grep "ok" | awk '/Checkerboard/ {print $1}'`
resultBitSpread=`cat $mem_result_file | grep "ok" | awk '/Bit Spread/ {print $1}'`
resultBitFlip=`cat $mem_result_file | grep "ok" | awk '/Bit Flip/ {print $1}'`
resultWalkingOnes=`cat $mem_result_file | grep "ok" | awk '/Walking Ones/ {print $1}'`
resultWalkingZeroes=`cat $mem_result_file | grep "ok" | awk '/Walking Zeroes/ {print $1}'`
result8bitWrites=`cat $mem_result_file | grep "ok" | awk '/8-bit Writes/ {print $1}'`
result16bitWrites=`cat $mem_result_file | grep "ok" | awk '/16-bit Writes/ {print $1}'`

if [ -n "$resultStuckAddress" ]; then
    echo "Stuck Address:\t\tok" >> $mem_ui_log
else
    echo "Stuck Address:\t\tfail" >> $mem_ui_log
fi

if [ -n "$resultRandomValue" ]; then
    echo "Random Value:\t\tok" >> $mem_ui_log
else 
    echo "Random Value:\t\tfail" >> $mem_ui_log
fi

if [ -n "$resultCompareXOR" ]; then
    echo "Compare XOR:\t\tok" >> $mem_ui_log
else 
    echo "Compare XOR:\t\tfail" >> $mem_ui_log
fi

if [ -n "$resultCompareSUB" ]; then 
    echo "Compare SUB:\t\tok" >> $mem_ui_log 
else 
    echo "Compare SUB:\t\tfail" >> $mem_ui_log 
fi

if [ -n "$resultCompareMUL" ]; then 
    echo "Compare MUL:\t\tok" >> $mem_ui_log 
else 
    echo "Compare MUL:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultCompareDIV" ]; then 
    echo "Compare DIV:\t\tok" >> $mem_ui_log 
else 
    echo "Compare DIV:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultCompareOR" ]; then 
    echo "Compare OR:\t\tok" >> $mem_ui_log 
else 
    echo "Compare OR:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultCompareAND" ]; then 
    echo "Compare AND:\t\tok" >> $mem_ui_log 
else 
    echo "Compare AND:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultSequentialIncrement" ]; then 
    echo "Sequential Increment:\t\tok" >> $mem_ui_log 
else
    echo "Sequential Increment:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultSolidBits" ]; then 
    echo "Solid Bits:\t\t\tok" >> $mem_ui_log 
else
    echo "Solid Bits:\t\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultBlockSequential" ]; then 
    echo "Block Sequential:\t\tok" >> $mem_ui_log 
else
    echo "Block Sequential:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultCheckerboard" ]; then 
    echo "Checkerboard:\t\tok" >> $mem_ui_log 
else
    echo "Checkerboard:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultBitSpread" ]; then 
    echo "Bit Spread:\t\t\tok" >> $mem_ui_log 
else
    echo "Bit Spread:\t\t\tfail" >> $mem_ui_log 
fi

if [ -n "$resultBitFlip" ]; then 
    echo "Bit Flip:\t\t\tok" >> $mem_ui_log 
else
    echo "Bit Flip:\t\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultWalkingOnes" ]; then 
    echo "Walking Ones:\t\tok" >> $mem_ui_log 
else
    echo "Walking Ones:\t\tfail" >> $mem_ui_log 
fi
if [ -n "$resultWalkingZeroes" ]; then 
    echo "Walking Zeroes:\t\tok" >> $mem_ui_log 
else
    echo "Walking Zeroes:\t\tfail" >> $mem_ui_log 
fi

if [ -n "$result8bitWrites" ]; then 
    echo "8-bit Writes:\t\t\tok" >> $mem_ui_log 
else
    echo "8-bit Writes:\t\t\tfail" >> $mem_ui_log 
fi

if [ -n "$result16bitWrites" ]; then 
    echo "16-bit Writes:\t\tok" >> $mem_ui_log 
else
    echo "16-bit Writes:\t\tfail" >> $mem_ui_log 
fi

rm -f $mem_result_file

if [ -n "$resultStuckAddress" -a -n "$resultRandomValue" -a -n "$resultCompareXOR" -a -n "$resultCompareSUB" \
     -a -n "$resultCompareMUL" -a -n "$resultCompareDIV" -a -n "$resultCompareOR" -a -n "$resultCompareAND" \
     -a -n "$resultSequentialIncrement" -a -n "$resultSolidBits" -a -n "$resultBlockSequential" -a -n "$resultCheckerboard" \
     -a -n "$resultBitSpread" -a -n "$resultBitFlip" -a -n "$resultWalkingOnes" -a -n "$resultWalkingZeroes" \
     -a -n "$result8bitWrites" -a -n "$result16bitWrites" ];then
    echo SUCCESS
else
    echo FAIL
fi
