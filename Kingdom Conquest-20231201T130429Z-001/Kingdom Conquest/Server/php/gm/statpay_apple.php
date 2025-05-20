<?php
include "auth.php";
include "header.php";
include "cmn.php";

// 判断是否有action
$action		= $_REQUEST['action'];

if( $action=='hack' )
{
    $sql	= "select m.id,m.transaction_id,m.account_id,m.val,c.name,'' region,'' ip,from_unixtime(m.client_time) client_time,from_unixtime(m.server_time) server_time,from_unixtime(m.donetime) donetime,from_unixtime(m.purchase_date) purchase_date from $g_webdb.billslog m join $g_webdb.common_characters c on m.account_id=c.account_id where m.status!=0 order by m.donetime desc";}
else
{
    $sql	= "select m.id,m.transaction_id,m.account_id,m.val,c.name,IFNULL(t.region,'') region,IFNULL(t.ip,'') ip,from_unixtime(m.client_time) client_time,from_unixtime(m.server_time) server_time,from_unixtime(m.donetime) donetime,from_unixtime(m.purchase_date) purchase_date from $g_webdb.billslog m join $g_webdb.common_characters c on m.account_id=c.account_id left join $g_webdb.account_ip_region t on t.account_id=c.account_id where m.status=0 order by m.donetime desc";
}
$rst	= DoQuery($sql);
if( $rst )
{
	echo "<table border=2><tr><th>序号</th><th>事务ID</th><th>帐号ID</th><th>钻石数量</th><th>角色名</th><th>地区</th><th>ip</th><th>客户端时间</th><th>服务器时间</th><th>结束时间</th><th>Apple购买时间</th>\n";
	while( ($row=mysql_fetch_assoc($rst))!=NULL )
	{
		foreach($row as $key=>$v) $$key=$v;
		echo("<tr><th>$id</th><th>$transaction_id</th><th><a href=accountname.php?accid=$account_id>$account_id</a></th><th>$val</th><th><a href=user.php?action=detail&accid=$account_id>$name</a></th><th>$region</th><th>$ip</th><th>$client_time</th><th>$server_time</th><th>$donetime</th><th>$purchase_date</th>\n");
	}
	echo "</table>";
}

// 可以显示一下黑客的记录
echo '
<br><br>
<form method=post action=><input type=hidden name=action value=hack><input type=submit value=黑客记录></form>
';
?>
