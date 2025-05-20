#!/bin/sh 
 
. $HOME/sh.cfg
exe_suffix="_d" 
 
check_global() 
{ 
	# XGDB4Web 
	row_num=`ps fx | grep XGDB4Web | grep -v grep | wc -l` 
	if [ $row_num -eq 0 ]  
	then 
		# 重启该进程 
		cd $HOME/bin/ 
		XGDB4Web/XGDB4Web$exe_suffix gdb4web_cfg.txt 
		cd - 
	fi 
	 
	# XGS4Web 
	row_num=`ps fx | grep XGS4Web | grep -v grep | wc -l` 
	if [ $row_num -eq 0 ]  
	then 
		# 重启该进程 
		cd $HOME/bin/ 
		XGS4Web/XGS4Web$exe_suffix gs4web_cfg.txt 
		cd - 
	fi 
} 
 
check_group() 
{ 
	# XCombatSvr 
	row_num=`ps fx | grep XCombatSvr | grep -v grep | wc -l`
	if [ $row_num -eq 0 ] 
	then 
		# 重启该进程 
		cd $HOME/bin/ 
		XCombatSvr/XCombatSvr$exe_suffix combatsvr_cfg.txt 
		cd - 
	fi 
	
	# XDBS4Web 
	row_num=`ps fx | grep XDBS4Web | grep -v grep | wc -l`
	if [ $row_num -eq 0 ] 
	then 
		# 重启该进程 
		cd $HOME/bin/ 
		XDBS4Web/XDBS4Web$exe_suffix dbs4web_cfg.txt 
		cd - 
	fi 
	 
	# XLP4Web 
	row_num=`ps fx | grep XLP4Web | grep -v grep | wc -l` 
	if [ $row_num -eq 0 ] 
	then 
		# 重启该进程 
		cd $HOME/bin/ 
		XLP4Web/XLP4Web$exe_suffix lp4web_cfg.txt 
		cd - 
	fi 
	 
	# XCAAFS4Web 
	row_num=`ps fx | grep XCAAFS4Web | grep -v grep | wc -l` 
	if [ $row_num -eq 0 ] 
	then 
		# 重启该进程 
		cd $HOME/bin/ 
		XCAAFS4Web/XCAAFS4Web$exe_suffix caafs4web_cfg1.txt 
		cd - 
	fi 
	 
	# XCLS4Web 
	row_num=`ps fx | grep XCLS4Web | grep -v grep | wc -l`  
	if [ $row_num -eq 0 ] 
	then 
		# 重启该进程 
		cd $HOME/bin/ 
		XCLS4Web/XCLS4Web$exe_suffix cls4web_cfg1.txt 
		cd - 
	fi 
} 
 
if [ -f /tmp/autocheck-$group_id.start ] 
then 
	echo "autocheck-$group_id.start EXISTS!" 
	exit 0 
fi 
 
touch /tmp/autocheck-$group_id.start 
 
while [ 1 -eq 1 ] 
do 
	if [ -f /tmp/autocheck-$group_id.stop ] 
	then 
		if [ -f /tmp/autocheck-$group_id.start ] 
		then 
			rm /tmp/autocheck-$group_id.start 
		fi 
		echo "STOP NOW!" 
		exit 0 
	fi 
	 
	if [ $is_global -eq 1 ]
	then
		check_global 
	fi
	if [ $is_group -eq 1 ]
	then
		check_group
	fi
	 
	sleep 10 
done 
 
 
