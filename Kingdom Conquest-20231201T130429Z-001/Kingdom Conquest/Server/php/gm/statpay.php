<?php
include "auth.php";
include "header.php";
include "cmn.php";

echo
"
<script type=\"text/javascript\">
</script>
";

// 判断是否有action
$action		= $_REQUEST['action'];

if( $action=='search' )
{
}
$sql	= "select m.serial,m.accountid,m.accountname,from_unixtime(m.createTime) ctime,from_unixtime(m.overTime) otime, m.GoodsCount,c.name from $g_webdb.pay91_done m join $g_webdb.common_characters c on m.accountid=c.account_id order by m.overTime desc limit 100";
$rst	= DoQuery($sql);
if( $rst )
{
	echo "<table border=2><tr><th>序列号</th><th>帐号ID</th><th>帐号名</th><th>角色名</th><th>创建时间</th><th>结束时间</th><th>钻石数量</th>\n";
	while( ($row=mysql_fetch_row($rst))!=NULL )
	{
		$serial	    	= $row[0];
		$account_id 	= $row[1];
		$accountname	= $row[2];
		$ctime        	= $row[3];
		$otime        	= $row[4];
		$count        	= $row[5];
		$name   		= $row[6];
		echo("<tr><th>$serial</th><th>$account_id</th><th>$accountname</th><th><a href=user.php?action=detail&accid=$account_id>$name</a></th><th>$ctime</th><th>$otime</th><th>$count</th>\n");
	}
	echo "</table>";
}

?>
