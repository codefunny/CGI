#!/bin/bash

DESTDIR=/huishoubao/cgi/m

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
PORT='9120'
PROCESSNAME='m-api-cancel_order'

killall -9 $PROCESSNAME
echo "=====killed====="
RESTART="/usr/local/bin/spawn-fcgi -a $HOST -p $PORT -f $DESTDIR/$PROCESSNAME -F 1"
echo $RESTART
$RESTART
echo "======done======"
ps -ef | grep $PROCESSNAME
