#!/bin/bash

DESTDIR=/huishoubao/cgi

: << !
PROCESSNAME="$1"

if [ "$PROCESSNAME" = "test" ];then
    PORT="9000"
elif [ "$PROCESSNAME" = "user" ];then
    PORT="9001"
elif [ "$PROCESSNAME" = "logic" ];then
    PORT="9002"
elif [ "$PROCESSNAME" = "account" ];then
    PORT="9003"
else
    PROCESSNAME=test
    PORT="9000"
fi
!

HOST='127.0.0.1'
PORT='9012'
PROCESSNAME='api-products'
fcgiCount=`/huishoubao/config/readConf.sh "fcgiCount_products"`

killall -9 $PROCESSNAME
echo "=====killed====="
RESTART="/usr/local/bin/spawn-fcgi -a $HOST -p $PORT -f $DESTDIR/$PROCESSNAME -F ${fcgiCount}"
echo $RESTART
$RESTART
echo "======done======"
ps -ef | grep $PROCESSNAME
