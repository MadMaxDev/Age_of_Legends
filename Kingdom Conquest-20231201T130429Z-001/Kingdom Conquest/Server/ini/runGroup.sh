#/bin/sh
. $HOME/sh.cfg

XDBS4Web/XDBS4Web_d dbs4web_cfg.txt
XLP4Web/XLP4Web_d lp4web_cfg.txt
sleep 2
XCAAFS4Web/XCAAFS4Web_d caafs4web_cfg1.txt
XCLS4Web/XCLS4Web_d cls4web_cfg1.txt
XCombatSvr/XCombatSvr_d combatsvr_cfg.txt

cd $IAPPath
if [ -f /tmp/stop_iap_check-$group_id.txt ]
then
	rm /tmp/stop_iap_check-$group_id.txt
fi
nohup php verify_receipt.php $group_id >/dev/null 2>/dev/null &
cd -

if [ -f /tmp/autocheck-$group_id.stop ]
then
	rm /tmp/autocheck-$group_id.stop
fi
nohup sh autocheck.sh $group_id >/dev/null 2>/dev/null &
