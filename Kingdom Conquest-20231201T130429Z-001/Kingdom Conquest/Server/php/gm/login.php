<?php
include "config.inc.php";
include "func/mysql_func.php";

header('Content-Type: text/html; charset=UTF-8');
if( !empty($_POST['act']) )
{
	if( $_POST['act']=='login' )
	{
		// 判断一下密码，如果成功就生成session串。
		$passwdmd5	= md5($_POST['passwd']);
		$_POST['user']	= AntiSQLInjection($_POST['user']);
		if( !OpenDB() )
		{
			die("Can not open mysql DB!");
		}
		$rst	= DoQuery("select uid from account where user='".$_POST['user']."' and pass='".$passwdmd5."'");
		if( $rst )
		{
			if( ($rowobj=mysql_fetch_array($rst)) )
			{
				$uid		= $rowobj[0];
				// 校验成功，生成session串
				$sessionid	= md5($_POST['user'].time());
                $g_webdb    = $_REQUEST['dbname'];
				// 把sessionid加入session表中
				$rst	= DoQuery("insert into session values($uid, '$sessionid', unix_timestamp(), '$g_webdb') on duplicate key update sessionid='$sessionid',startime=unix_timestamp(),dbname='$g_webdb'");
				// 更新在线IP
				//$rst	= DoQuery('update account set lastip="'.$_SERVER['REMOTE_ADDR'].'" where uid='.$uid);
				setcookie("WH_LOGINFO_UID", $uid);
				setcookie("WH_LOGINFO_SID", $sessionid);
				//header('Content-Type: text/html; charset=UTF-8');
				echo '<html>
				<meta http-equiv="Content-Type" content="text/html; charset=utf8">
				<meta HTTP-EQUIV=REFRESH CONTENT="2; URL=.">
				登录成功，2秒内进入<a href=.>工作页面</a>...
				</html>
				';
				exit(0);
			}
			else
			{
				// 说明没有用户名密码
				echo "用户名密码错误。请重新输入：<br><br>";
			}
		}
		else
		{
			// 这里不知道是为什么
			echo "用户名密码错误。请重新输入：<br><br>";
		}
	}
}
?>
<html>
<style>input{font-family: "宋体";}</style>
<B><font size=6><?php echo g_sysname ?></font></B>&nbsp;<B>[<?php echo g_sysver ?>]</B>&nbsp;<font color=white size=0>written by weihua, xixi :)</font><br>
<br>
<form action="" method=post>
数据库：<select name=dbname>
<?php
require_once "db-array-gm.php";
foreach ($g_webdb_arr as $key=>$val)
{
    echo "<option value='$val'>$val</option>";
}
?>
</select>
<br>
<br>
<input type=hidden name=act value=login>
用户名：<input name=user type=text size=22><br>
密　码：<input name=passwd type=password size=22><br>
<br>
<input type=submit value=登录> <input type=reset value=重填><br>
</form>
</html>

