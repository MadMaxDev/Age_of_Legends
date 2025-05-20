#!/bin/sh
. $HOME/sh.cfg

touch /tmp/autocheck-$group_id.stop

if [ $is_group -eq 1 ]
then
	echo stopping XLP4Web ...
	XLP4Web/XLP4Web_d -stop lp4web_cfg.txt

	echo stopping XDBS4Web ...
	XDBS4Web/XDBS4Web_d -stop dbs4web_cfg.txt

	echo stopping XCombatSvr ...
	XCombatSvr/XCombatSvr_d -stop combatsvr_cfg.txt
fi

if [ $is_global -eq 1 ]
then
	echo stopping XGS4Web ...
	XGS4Web/XGS4Web_d -stop gs4web_cfg.txt

	echo stopping XGDB4Web ...
	XGDB4Web/XGDB4Web_d -stop gdb4web_cfg.txt
fi

# 关闭glogger_d进程,同时清理glogger的pid文件(glogger_d关闭时pid文件被pidfile的析构函数删除)
killall glogger_d

touch /tmp/stop_iap_check-$group_id.txt
if [ -f /tmp/verify_receipt-$group_id.pid ]
then
	rm /tmp/verify_receipt-$group_id.pid
fi

