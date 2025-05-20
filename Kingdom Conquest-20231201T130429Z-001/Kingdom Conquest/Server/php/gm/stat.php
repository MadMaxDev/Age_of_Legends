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
$sql	= "call $g_webdb.calc_stat_login_today";
$rst	= DoQuery($sql);
$sql	= "select from_unixtime(day) day,todaynew,todaylogin,player7,player30,total,remain1,remain3 from $g_webdb.stat_login order by day desc limit $g_gm_page_size";
$rst	= DoQuery($sql);
if( $rst )
{
	echo "<table border=2><tr><th>日期</th><th>今日新增</th><th>今日登录</th><th>7日活跃</th><th>30日活跃</th><th>次日留存</th><th>三日留存</th><th>总用户量</th>\n";
	while( ($row=mysql_fetch_row($rst))!=NULL )
	{
		$day		= $row[0];
		$todaynew	= $row[1];
		$todaylogin	= $row[2];
		$player7	= $row[3];
		$player30	= $row[4];
		$total		= $row[5];
		$remain1	= $row[6];
		$remain3	= $row[7];
		echo("<tr><th>$day</th><th>$todaynew</th><th>$todaylogin</th><th>$player7</th><th>$player30</th><th>$remain1</th><th>$remain3</th><th>$total</th>\n");
	}
	echo "</table>";
}

?>
