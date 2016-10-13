#!/bin/bash

chmod 755 makeAndStartCgi.sh

echo "==================== CGI START ========================"
./makeAndStartCgi.sh "api-cancel_order"
sleep 3
./makeAndStartCgi.sh "api-evaluate"
sleep 3
./makeAndStartCgi.sh "api-get_order"
sleep 3
./makeAndStartCgi.sh "api-get_payways"
sleep 3
./makeAndStartCgi.sh "api-get_product_param"
sleep 3
./makeAndStartCgi.sh "api-place_order"
sleep 3
./makeAndStartCgi.sh "api-products"
sleep 3
./makeAndStartCgi.sh "api-sendmsg_passive"
sleep 3
./makeAndStartCgi.sh "api-set_payways"
sleep 3
./makeAndStartCgi.sh "api-shipments"
sleep 3
./makeAndStartCgi.sh "evaluate_page"
sleep 3
./makeAndStartCgi.sh "home_page"
sleep 3
./makeAndStartCgi.sh "place_order_page"
sleep 3
./makeAndStartCgi.sh "submitted_page"
sleep 3
./makeAndStartCgi.sh "usercenter_page"

echo "==================== CGI END ========================"
