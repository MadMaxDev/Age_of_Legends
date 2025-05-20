<?php
include "auth.php";
include "header.php";
include "cmn.php";

echo
"
<script type=\"text/javascript\">
function del(accid,mailid,name,time)
{
	if( confirm(\"删除(\"+time+\")来自\"+name+\"的GM邮件\") )
	{
		document.URL	= \"?action=del&accid=\"+accid+\"&mailid=\"+mailid;
	}
}
</script>
";

// 判断是否有action
$action		= $_REQUEST['action'];
$account_id	= $_REQUEST['accid'];
$mail_id	= $_REQUEST['mailid'];
if( $action=='del' )
{

	$sql	= "delete from $g_webdb.gm_mail where account_id=$account_id and mail_id=$mail_id";
	$rst	= DoQuery($sql);
	$sql	= "delete from $g_webdb.gm_mail_reply where account_id=$account_id and mail_id=$mail_id";
	$rst	= DoQuery($sql);
}
else if( $action=='reply' )
{
	// 先取出原来的内容
	$sql	= "SELECT from_unixtime(m.time),uncompress(m.text),c.name FROM $g_webdb.gm_mail m JOIN $g_webdb.common_characters c ON m.account_id=c.account_id where m.account_id=$account_id and m.mail_id=$mail_id";
	$rst	= DoQuery($sql);
	if( $rst )
	{
		$row		= mysql_fetch_array($rst);
		$time		= $row[0];
		$content	= $row[1];
		$name		= $row[2];
		// 过滤一下content内容，里面不能有';"
		$content	= str_replace(array("'",'"'),'x',$content);
	}
	echo "To: $name <br>";
	echo "<form method=post action='?action=doreply&accid=$account_id&mailid=$mail_id'>";
	echo "<textarea name=msg rows=10 cols=100>";
    echo "Dear lord:";
    echo "\n\n\n<----$time---->\n$content";
	echo "</textarea><br>";
	echo "<input type=submit value='提交'>";
	echo "</form><br><br>";
}
else if( $action=='doreply' )
{
	// 发送邮件到数据库
	$text	= AntiSQLInjection($_REQUEST['msg']);
	$sql	= "CALL $g_webdb.add_private_mail(0,$account_id,4,1,0,'$text',NULL,0,@result,@mail_id)";
	$rst	= DoQuery($sql);
	{
		//mysql_free_result($rst);
		echo	'发送完毕<br>';
		// 标记邮件为已处理
		$sql	= "UPDATE $g_webdb.gm_mail set status=".g_gm_mail_status_over." where account_id=$account_id and mail_id=$mail_id";
		$rst	= DoQuery($sql);
        // 加入回复历史
		$sql	= "insert into $g_webdb.gm_mail_reply (account_id,mail_id,time,text) values ($account_id,$mail_id,unix_timestamp(),compress('$text'))";
        $rst    = DoQuery($sql);
	}
}
else if( $action=='history' )
{
    $sql    = "select from_unixtime(time),uncompress(text) from $g_webdb.gm_mail_reply where account_id=$account_id and mail_id=$mail_id";
    $rst    = DoQuery($sql);
    if( $rst )
    {
	    echo '<table border=2><tr><th>回复时间</th><th>内容</th>';
	    while( $row=mysql_fetch_array($rst) )
        {
            $time   = $row[0];
            $content= $row[1];

            echo "<tr>";
            echo "<th>$time</th><th>$content</th>";
        }
	    echo '</table>';
        return;
    }
}

// 列表
$sql	= "SELECT m.account_id,from_unixtime(m.time),uncompress(m.text),c.name,m.status,m.mail_id FROM $g_webdb.gm_mail m JOIN $g_webdb.common_characters c ON m.account_id=c.account_id order by m.time desc limit 50";
$rst	= DoQuery($sql);

if( $rst )
{
	echo '<table border=2><tr><th>帐号ID</th><th>角色名</th><th>提交时间</th><th>内容</th><th>状态</th><th>操作</th>';
	while( $row=mysql_fetch_array($rst) )
	{
		$account_id	= $row[0];
		$time		= $row[1];
		$content	= $row[2];
		$name		= $row[3];
		$status		= $row[4];
		$mail_id	= $row[5];
		$statusstr	= $g_gm_mail_status_STR[$status];
		if( $status==g_gm_mail_status_pending )
		{
			echo "<tr bgcolor=gold>";
		}
		else
		{
			echo "<tr>";
		}
		echo "<th><a href=\"accountname.php?accid=$account_id\">$account_id</a></th><th><a href=\"user.php?action=search&accid=$account_id\" target=\"_blank\">$name</a></th><th>$time</th><th width=50%>$content</th><th>$statusstr</th>"
			."<th>"
			."<a href=# onclick=\"del($account_id,$mail_id,'$name','$time'); return false;\">删除</a>"
			."&nbsp; <a href=?action=reply&accid=$account_id&mailid=$mail_id>回复</a>"
			."&nbsp; <a href=?action=history&accid=$account_id&mailid=$mail_id>回复历史</a>"
			."</th>";
	}
	echo '</table>';
}
