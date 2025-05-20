<?php
include_once "config.inc.php";
include_once "func/mysql_func.php";

$mustrelogin	= 1;
if( !empty($_COOKIE['WH_LOGINFO_UID']) && !empty($_COOKIE['WH_LOGINFO_SID']) )
{
	if( !OpenDB() )
	{
		die("Can not open mysql DB!");
	}
	$g_uid	= $_COOKIE['WH_LOGINFO_UID'];
	$g_sid	= $_COOKIE['WH_LOGINFO_SID'];
	$rst	= DoQuery("select a.uid ,a.user ,a.name, s.dbname from session as s left join account as a on s.uid=a.uid where s.uid=$g_uid and s.sessionid='$g_sid'");
	if( $rst )
	{
		if( ($l_rowobj=mysql_fetch_array($rst)) )
		{
			// 校验成功，可以继续了
			$mustrelogin	= 0;
			$g_user			= $l_rowobj['user'];
			$g_name			= $l_rowobj['name'];
            $g_webdb        = $l_rowobj['dbname'];
		}
	}
}
if( $mustrelogin )
{
	if( !empty($g_isindex) && $g_isindex )
	{
	// 是首页，则直接转到login.php
	?>
	<html>
	<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
	<meta HTTP-EQUIV=REFRESH CONTENT="0; URL=login.php">
	</html>
	<?php
	}
	else
	{
	// 重新登录
	?>
	<html>
	<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
	<meta HTTP-EQUIV=REFRESH CONTENT="4; URL=login.php">
	请<a href=login.php>重新登录(4秒之后自动转换页面)</a>!
	</html>
	<?php
	}
	// 后面的就不用继续了
	exit(0);
}

// 今后这个需要是在登录时选定的
require_once "db-array-gm.php";
if( !$g_webdb )
{
    $g_webdb	= 'web_cur';
}
$g_weblog	= $g_db2log[$g_webdb];
$g_webims	= $g_db2ims[$g_webdb];
$g_isapple	= $g_dbisapple[$g_webdb];

?>
