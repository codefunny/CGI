#!/bin/bash

DESTDIR=/huishoubao/cgi/m

HOST='127.0.0.1'
PORT='9126'
PROCESSNAME='m-api-get_payways'

killall -9 $PROCESSNAME
echo "=====killed====="
RESTART="/usr/local/bin/spawn-fcgi -a $HOST -p $PORT -f $DESTDIR/$PROCESSNAME -F 1"
echo $RESTART
$RESTART
echo "======done======"
ps -ef | grep $PROCESSNAME
