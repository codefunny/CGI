#!/bin/bash

chmod 755 startCgi.sh

echo "==================== CGI START ========================"
./startCgi.sh "api-cancel_order"
sleep 3
./startCgi.sh "api-download_orderlist"
sleep 3
./startCgi.sh "api-evaluate"
sleep 3
./startCgi.sh "api-get_channel"
sleep 3
./startCgi.sh "api-get_logistics"
sleep 3
./startCgi.sh "api-get_order"
sleep 3
./startCgi.sh "api-get_product_param"
sleep 3
./startCgi.sh "api-login"
sleep 3
./startCgi.sh "api-modify_pwd"
sleep 3
./startCgi.sh "api-place_order"
sleep 3
./startCgi.sh "api-products"
sleep 3
./startCgi.sh "api-raw_products"
sleep 3
./startCgi.sh "api-receiving"
sleep 3
./startCgi.sh "api-shipments"
sleep 3
./startCgi.sh "help_page"
sleep 3
./startCgi.sh "home_page"
sleep 3
./startCgi.sh "login_page"
sleep 3
./startCgi.sh "order_page"

echo "==================== CGI END ========================"
