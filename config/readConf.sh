#!/bin/bash

confPath="/huishoubao/config/cgiConfig.conf"
function getFcgiCount()
{
	isReadEnv=0
	isInEnv=0
	env_head=""
	while read line
	do
		# 非空行
		if [ -n "${line}" ];then
			firstWord=`echo ${line} |cut -c 1`

			# 非注释行
			if [ "${firstWord}" != "#" ];then

				#没有读取到使用的环境 dev or idc
				if [ "${isReadEnv}" -eq 0 ];then
					env_key=${line%"="*}

					#获取环境参数值
					if [ "${env_key}" == "useEnv" ];then
						#echo "env_key:"${env_key}
						env_val=${line##*"="}
						env_head="["${env_val}"]"
						#echo "----->env_val:"${env_val}
						
						isReadEnv=1
					fi
				else
					#不在环境参数定义内
					if [ "${isInEnv}" -eq 0 ];then

						#获取环境参数
						if [ "${line}" == "${env_head}" ];then
							#echo "--->env_head:"${line}
							
							isInEnv=1
						fi
					else
						firstWord=`echo ${line} |cut -c 1`
						# 非注释行
						if [ "${firstWord}" == "[" ];then
							echo "not get the word!"
							return
						else
							param_key=${line%"="*}
							# 取到要获取的变量
							if [ $1 == ${param_key} ];then
								param_val=${line##*"="}
								echo ${param_val}
								return 
							fi
						fi
					fi
				fi
			#else
				#echo "--->firstWord"${line}
			fi
			#env=${varible##*string}
		#else
			#echo "------>>>>is null line"
		fi
	done < ${confPath}
}

#参数个数获取
if [ $# -eq 1 ];then
	result=0

        # SP域
	if [ $1 == "fcgiCount_cancel_order" ];then
		result=$(getFcgiCount "fcgiCount_cancel_order")
	elif [ $1 == "fcgiCount_get_channel" ];then
		result=$(getFcgiCount "fcgiCount_get_channel")
	elif [ $1 == "fcgiCount_download_orderlist" ];then
		result=$(getFcgiCount "fcgiCount_download_orderlist")
	elif [ $1 == "fcgiCount_evaluate" ];then
		result=$(getFcgiCount "fcgiCount_evaluate")
	elif [ $1 == "fcgiCount_get_product_param" ];then
		result=$(getFcgiCount "fcgiCount_get_product_param")
	elif [ $1 == "fcgiCount_get_logistics" ];then
		result=$(getFcgiCount "fcgiCount_get_logistics")
	elif [ $1 == "fcgiCount_get_order" ];then
		result=$(getFcgiCount "fcgiCount_get_order")
	elif [ $1 == "fcgiCount_modify_pwd" ];then
		result=$(getFcgiCount "fcgiCount_modify_pwd")
	elif [ $1 == "fcgiCount_login" ];then
		result=$(getFcgiCount "fcgiCount_login")
	elif [ $1 == "fcgiCount_raw_products" ];then
		result=$(getFcgiCount "fcgiCount_raw_products")
	elif [ $1 == "fcgiCount_place_order" ];then
		result=$(getFcgiCount "fcgiCount_place_order")
	elif [ $1 == "fcgiCount_products" ];then
		result=$(getFcgiCount "fcgiCount_products")
	elif [ $1 == "fcgiCount_receiving" ];then
		result=$(getFcgiCount "fcgiCount_receiving")
	elif [ $1 == "fcgiCount_shipments" ];then
		result=$(getFcgiCount "fcgiCount_shipments")
	elif [ $1 == "fcgiCount_help_page" ];then
		result=$(getFcgiCount "fcgiCount_help_page")
	elif [ $1 == "fcgiCount_home_page" ];then
		result=$(getFcgiCount "fcgiCount_home_page")
	elif [ $1 == "fcgiCount_login_page" ];then
		result=$(getFcgiCount "fcgiCount_login_page")
	elif [ $1 == "fcgiCount_order_page" ];then
		result=$(getFcgiCount "fcgiCount_order_page")

        # M 域
        elif [ $1 == "fcgiCount_m_cancel_order" ];then
                result=$(getFcgiCount "fcgiCount_m_cancel_order")
        elif [ $1 == "fcgiCount_m_evaluate" ];then
                result=$(getFcgiCount "fcgiCount_m_evaluate")
        elif [ $1 == "fcgiCount_m_get_order" ];then
                result=$(getFcgiCount "fcgiCount_m_get_order")
        elif [ $1 == "fcgiCount_m_get_payways" ];then
                result=$(getFcgiCount "fcgiCount_m_get_payways")
        elif [ $1 == "fcgiCount_m_get_product_param" ];then
                result=$(getFcgiCount "fcgiCount_m_get_product_param")
        elif [ $1 == "fcgiCount_m_place_order" ];then
                result=$(getFcgiCount "fcgiCount_m_place_order")
        elif [ $1 == "fcgiCount_m_products" ];then
                result=$(getFcgiCount "fcgiCount_m_products")
        elif [ $1 == "fcgiCount_m_sendmsg_passive" ];then
                result=$(getFcgiCount "fcgiCount_m_sendmsg_passive")
        elif [ $1 == "fcgiCount_m_set_payways" ];then
                result=$(getFcgiCount "fcgiCount_m_set_payways")
        elif [ $1 == "fcgiCount_m_shipments" ];then
                result=$(getFcgiCount "fcgiCount_m_shipments")
        elif [ $1 == "fcgiCount_m_evaluate_page" ];then
                result=$(getFcgiCount "fcgiCount_m_evaluate_page")
        elif [ $1 == "fcgiCount_m_home_page" ];then
                result=$(getFcgiCount "fcgiCount_m_home_page")
        elif [ $1 == "fcgiCount_m_place_order_page" ];then
                result=$(getFcgiCount "fcgiCount_m_place_order_page")
        elif [ $1 == "fcgiCount_m_submitted_page" ];then
                result=$(getFcgiCount "fcgiCount_m_submitted_page")
        elif [ $1 == "fcgiCount_m_usercenter_page" ];then
                result=$(getFcgiCount "fcgiCount_m_usercenter_page")

	else
		echo "Parameter is error!"
	fi

	#获取最后 return 值 PS:因为有字符串所以不可取
	#result1=$?
	#echo ${result1}
	echo ${result}
else
	echo "The number of parameters in error!"
fi
