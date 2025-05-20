<?php
include "auth.php";
include "header.php";
include "cmn.php";

echo
"
<script type=\"text/javascript\">
</script>
";

// 显示用户查询界面
echo
'
<form method=post action=?action=search>
输入账号名（可用%通配符）：<input name=name size=20> &nbsp;&nbsp; <input type=submit value=检索><br>
</form>
<HR>
';

// 判断是否有action
$action		= $_REQUEST['action'];
$account_id	= $_REQUEST['accid'];
function doSearchPlayer($sql)
{
	$rst	= DoQuery($sql);
	if( $rst )
	{
		echo "<table border=2><tr><th>帐号ID</th><th>账号</th><th>登陆时间</th><th>设备串</th><th>操作</th>";
		while( $row=mysql_fetch_array($rst) )
		{
			$account_id	= $row[0];
			$name		= $row[1];
			$lastlogin	= $row[2];
			$device_id	= $row[3];
			echo "<tr><th>$account_id</th><th>$name</th><th>$lastlogin</th><th>$device_id</th>"
				."<th><a href=?action=pass&accid=$account_id>修改密码</a>"
				."</th>";
		}
		echo "</table>";
	}
}
if( $action=='search' )
{
	$name	= $_REQUEST['name'];
    if( $name )
    {
        $sql	= "select account_id,name,from_unixtime(last_login_time),device_id from $g_webims.common_accounts where name like '$name' limit $g_gm_page_size";
    }
    else if( $account_id )
    {
        $sql	= "select account_id,name,from_unixtime(last_login_time),device_id from $g_webims.common_accounts where account_id=$account_id";
    }
	doSearchPlayer($sql);
}
else if( $action=='pass' )
{
	// 发送gm邮件
	$sql	= "SELECT c.name FROM $g_webims.common_accounts c where c.account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	echo "账号	: $name <br>";
	echo "账号ID: $account_id <br>";
	echo "<form method=post action='?action=change_pass&accid=$account_id'>";
	echo "新密码:<input name=new_pass size=20>";
	echo "<br>";
	echo "<input type=submit value='提交'>";
	echo "</form><br><br>";
}
else if( $action=='change_pass' )
{
	// 发送邮件到数据库
	$new_pass	= AntiSQLInjection($_REQUEST['new_pass']);
	$sql	= "UPDATE $g_webims.common_accounts SET pass=md5('$new_pass') WHERE account_id=$account_id";
	$rst	= DoQuery($sql);
	echo	'发送完毕<br>';
}
?>
