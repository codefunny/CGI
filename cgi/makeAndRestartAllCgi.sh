#!/bin/bash

chmod 755 makeAndRestartCgi.sh

echo "==================== CGI START ========================"
./makeAndRestartCgi.sh "api-cancel_order"
sleep 3
./makeAndRestartCgi.sh "api-download_orderlist"
sleep 3
./makeAndRestartCgi.sh "api-evaluate"
sleep 3
./makeAndRestartCgi.sh "api-get_channel"
sleep 3
./makeAndRestartCgi.sh "api-get_logistics"
sleep 3
./makeAndRestartCgi.sh "api-get_order"
sleep 3
./makeAndRestartCgi.sh "api-get_product_param"
sleep 3
./makeAndRestartCgi.sh "api-login"
sleep 3
./makeAndRestartCgi.sh "api-modify_pwd"
sleep 3
./makeAndRestartCgi.sh "api-place_order"
sleep 3
./makeAndRestartCgi.sh "api-products"
sleep 3
./makeAndRestartCgi.sh "api-raw_products"
sleep 3
./makeAndRestartCgi.sh "api-receiving"
sleep 3
./makeAndRestartCgi.sh "api-shipments"
sleep 3
./makeAndRestartCgi.sh "help_page"
sleep 3
./makeAndRestartCgi.sh "home_page"
sleep 3
./makeAndRestartCgi.sh "login_page"
sleep 3
./makeAndRestartCgi.sh "order_page"

echo "==================== CGI END ========================"
