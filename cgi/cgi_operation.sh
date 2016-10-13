#!/bin/bash

DESTDIR=/huishoubao/cgi
cgiArr="api-cancel_order  api-get_channel    api-get_order  api-login 
            api-place_order   api-raw_products   api-shipments  api-download_orderlist  
            api-evaluate      api-get_logistics  api-get_product_param  
            api-modify_pwd    api-products       api-receiving 
            help_page         login_page         home_page     order_page"

#CGI 对应的端口
port_login_page=9001
port_home_page=9002
port_order_page=9003
port_help_page=9004
port_login=9011
port_products=9012
port_get_product_param=9013
port_evaluate=9014
port_place_order=9015
port_get_order=9016
port_shipments=9017
port_receiving=9018
port_cancel_order=9020
port_get_logistics=9021
port_download_orderlist=9022
port_raw_products=9023
port_get_channel=9024
port_modify_pwd=9025

# kill 掉cgi
function stopCgi()
{
    PROCESSNAME=$1
    echo killall -9 $PROCESSNAME
    killall -9 $PROCESSNAME
    echo "=====killed====="
}

# 启动cgi
function startCgi()
{
    PROCESSNAME=$1
    HOST='127.0.0.1'
    PORT=$2
    fcgiCount=$3
    RESTART="/usr/local/bin/spawn-fcgi -a $HOST -p $PORT -f $DESTDIR/$PROCESSNAME -F ${fcgiCount}"
    echo $RESTART
    $RESTART
    echo "======done======"
    ps -ef | grep $PROCESSNAME
}

# 重启cgi
function restartCgi()
{
    processName=$1
    stopCgi "${processName}"
    startCgi "${processName}" "$2" "$3"
}

# 帮助信息
function helpInfo()
{
   echo " 用法1：./cgi_operation.sh -[o start/stop/restart] -[a] "
   echo " 用法2：./cgi_operation.sh -[o start/stop/restart] -[n cginame] "
   echo ""
   echo " 停止、启动、重启，某个或者所有的cgi."
   echo "-o -[o start/stop/restart]"
   echo "-n -[n cginame]"
   echo "    cginame:${cgiArr}"
   echo "-a -[a] start/stop/restart all cgi"
   echo "-h -[h] help info"
   echo ""
   echo "全部启动：./cgi_operation.sh -o start -a"
   echo "启动某个：./cgi_operation.sh -o start -n home_page"
}


function findCgiName()
{
    for name in ${cgiArr}
    do
          if [ ${name} == "$1" ];then
              echo 1
              exit;
          fi
    done
    echo 0
}

#操作动作、操作的某个cgi名称、是否操作全部的cgi
opera="error"
cgiName=""
isAll=0

cmd=""
allCgiName=""
port=0
cgiCount=0

while getopts :o:n:ah opt
do
    #echo +++++++++++++ ${opt}
    case $opt in
        o) opera=$OPTARG 
           ;;
        n) cgiName=$OPTARG 
           ;;
        a) isAll=1 
           ;;
        h) helpInfo
           exit
           ;;
        \?)  # usage statement 
            helpInfo 
            exit
           ;;
    esac
done

# 验证cgi 名字
if [ ${cgiName} ];then
     res=0
     res=$(findCgiName "${cgiName}")

     if [ ${res} -eq 0 ];then
        helpInfo
        exit;
     fi
fi

#验证操作类型
if [ ${opera} == "start" ];then
    cmd="startCgi"
elif [ ${opera} == "restart" ];then
    cmd="restartCgi"
elif [ ${opera} == "stop" ];then
    cmd="stopCgi"
else
     helpInfo
     exit;
fi

#是否是操作所有CGI
    if [ ${isAll} -eq 0 ];then
       if [ -z ${cgiName} ];then
           helpInfo
           exit;
       fi
    elif [ ${isAll} -eq 1 ];then
       if [ ${cgiName} ];then
           helpInfo
           exit;
       fi
       allCgiName=${cgiArr}
    else
        helpInfo
        exit;
    fi

#是否操作单个CGI 
if [ ${cgiName} ];then
    allCgiName=${cgiName}
fi

#执行操作
for name in ${allCgiName}
do
    if [ ${opera} != "stop" ];then
       cgiPortStr="port_${name//api-/}"
       port=$(eval echo \${$cgiPortStr})
       cgiStr="fcgiCount_${name//api-/}"
       cgiCount=`/huishoubao/config/readConf.sh "${cgiStr}"`
    fi
    
    $cmd "${name}" "${port}" "${cgiCount}"
    sleep 1
done
