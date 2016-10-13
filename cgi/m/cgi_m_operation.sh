#!/bin/bash

DESTDIR=/huishoubao/cgi/m
cgiArr="m-api-cancel_order       m-api-evaluate     m-api-get_order  m-api-get_payways
        m-api-get_product_param  m-api-place_order  m-api-products   m-api-sendmsg_passive
        m-api-set_payways        m-api-shipments    m-evaluate_page  m-home_page
        m-place_order_page       m-submitted_page   m-usercenter_page"

#CGI 对应的端口
port_m_cancel_order=9120
port_m_evaluate=9114
port_m_get_order=9116
port_m_get_payways=9126
port_m_get_product_param=9113
port_m_place_order=9115
port_m_products=9112
port_m_sendmsg_passive=9154
port_m_set_payways=9127
port_m_shipments=9117
port_m_evaluate_page=9150
port_m_home_page=9102
port_m_place_order_page=9153
port_m_submitted_page=9151
port_m_usercenter_page=9152

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
   echo " 用法1：./cgi_m_operation.sh -[o start/stop/restart] -[a] "
   echo " 用法2：./cgi_m_operation.sh -[o start/stop/restart] -[n cginame] "
   echo ""
   echo " 停止、启动、重启，某个或者所有的cgi."
   echo "-o -[o start/stop/restart]"
   echo "-n -[n cginame]"
   echo "    cginame:${cgiArr}"
   echo "-a -[a] start/stop/restart all cgi"
   echo "-h -[h] help info"
   echo ""
   echo "全部启动：./cgi_m_operation.sh -o start -a"
   echo "启动某个：./cgi_m_operation.sh -o start -n home_page"
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
       cgiPortStr="port_${name//-api-/_}"
       cgiPortStr="${cgiPortStr//-/_}"
       port=$(eval echo \${$cgiPortStr})
       cgiStr="fcgiCount_${name//-api-/_}"
       cgiStr="${cgiStr//-/_}"
       cgiCount=`/huishoubao/config/readConf.sh "${cgiStr}"`
    fi
    
    $cmd "${name}" "${port}" "${cgiCount}"
    sleep 1
done
