#!/bin/bash
#create by Zhang Shibo at 2014-11-7
#description: 用于启动微信cgi进程
#param:varible_name 要启动的cgi 的目录名
#usage: 调用方式 ./makeAndStartCgi.sh "api-cancel_order"

#参数个数获取
#shell start
if [ $# -eq 1 ];then
        if [ $1 == "api-cancel_order" ];then
                cd "api-cancel_order"
        fi

        if [ $1 == "api-evaluate" ];then
                cd "api-evaluate"
        fi

        if [ $1 == "api-get_order" ];then
                cd "api-get_order"
        fi

        if [ $1 == "api-get_payways" ];then
                cd "api-get_payways"
        fi

        if [ $1 == "api-get_product_param" ];then
                cd "api-get_product_param"
        fi

        if [ $1 == "api-place_order" ];then
                cd "api-place_order"
        fi

        if [ $1 == "api-products" ];then
                cd "api-products"
        fi

        if [ $1 == "api-sendmsg_passive" ];then
                cd "api-sendmsg_passive"
        fi

        if [ $1 == "api-set_payways" ];then
                cd "api-set_payways"
        fi

        if [ $1 == "api-shipments" ];then
                cd "api-shipments"
        fi

        if [ $1 == "evaluate_page" ];then
                cd "evaluate_page"
        fi

        if [ $1 == "home_page" ];then
                cd "home_page"
        fi

        if [ $1 == "place_order_page" ];then
                cd "place_order_page"
        fi
        
        if [ $1 == "submitted_page" ];then
                cd "submitted_page"
        fi

        if [ $1 == "usercenter_page" ];then
                cd "usercenter_page"
        fi

        echo "run $1/restart.sh"
    chmod 755 "restart.sh"
    make mem; chmod +x ./restart.sh; make; make install; make install; ./restart.sh; sleep 1; ./restart.sh
else
        echo "The number of parameters in error! like: ./makeAndStartCgi api-products"
fi
#shell end
