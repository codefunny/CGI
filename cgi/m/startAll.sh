#!/bin/bash

chmod 755 startCgi.sh

echo "==================== CGI START ========================"
./startCgi.sh "api-cancel_order"
sleep 3
./startCgi.sh "api-evaluate"
sleep 3
./startCgi.sh "api-get_order"
sleep 3
./startCgi.sh "api-get_payways"
sleep 3
./startCgi.sh "api-get_product_param"
sleep 3
./startCgi.sh "api-place_order"
sleep 3
./startCgi.sh "api-products"
sleep 3
./startCgi.sh "api-sendmsg_passive"
sleep 3
./startCgi.sh "api-set_payways"
sleep 3
./startCgi.sh "api-shipments"
sleep 3
./startCgi.sh "evaluate_page"
sleep 3
./startCgi.sh "home_page"
sleep 3
./startCgi.sh "place_order_page"
sleep 3
./startCgi.sh "submitted_page"
sleep 3
./startCgi.sh "usercenter_page"

echo "==================== CGI END ========================"
