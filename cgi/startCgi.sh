#!/bin/bash
#create by mickey at 2014-11-3
#description: 用于启动cgi进程
#param:varible_name 要启动的cgi 的目录名
#usage: 调用方式 ./startCgi.sh "api-cancel_order"

#参数个数获取
#shell start
if [ $# -eq 1 ];then
        if [ $1 == "api-cancel_order" ];then
                cd "api-cancel_order"
        fi

        if [ $1 == "api-download_orderlist" ];then
                cd "api-download_orderlist"
        fi

        if [ $1 == "api-evaluate" ];then
                cd "api-evaluate"
        fi

        if [ $1 == "api-get_channel" ];then
                cd "api-get_channel"
        fi

        if [ $1 == "api-get_logistics" ];then
                cd "api-get_logistics"
        fi

        if [ $1 == "api-get_order" ];then
                cd "api-get_order"
        fi

        if [ $1 == "api-get_product_param" ];then
                cd "api-get_product_param"
        fi

        if [ $1 == "api-login" ];then
                cd "api-login"
        fi

        if [ $1 == "api-modify_pwd" ];then
                cd "api-modify_pwd"
        fi

        if [ $1 == "api-place_order" ];then
                cd "api-place_order"
        fi

        if [ $1 == "api-products" ];then
                cd "api-products"
        fi

        if [ $1 == "api-raw_products" ];then
                cd "api-raw_products"
        fi

        if [ $1 == "api-receiving" ];then
                cd "api-receiving"
        fi

        if [ $1 == "api-shipments" ];then
                cd "api-shipments"
        fi

        if [ $1 == "help_page" ];then
                cd "help_page"
        fi

        if [ $1 == "home_page" ];then
                cd "home_page"
        fi

        if [ $1 == "login_page" ];then
                cd "login_page"
        fi
		
        if [ $1 == "order_page" ];then
                cd "order_page"
        fi

        echo "run $1/restart.sh"
	chmod 755 "restart.sh"
	./restart.sh
else
        echo "The number of parameters in error! like: ./startCgi api-products"
fi
#shell end
