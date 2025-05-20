<?php
header('Content-Type: text/html; charset=UTF-8');
// 用于在最上面显示菜单
// 设置默认的选项
?>
<html>

<?php echo "<form method=post action=changedb.php>
<font size=3><a href=logout.php>[注销]</a></font> Welcome back comrade <B>
<font size=4> \"$g_name\"</font>";
echo "[db:<select name=dbname>";
echo "<option value=$g_webdb style=\"color:red;\">$g_webdb</option>";
foreach($g_webdb_arr as $key=>$val)
{
    if( $val!=$g_webdb )
    {
        echo "<option value=$val>$val</option>";
    }
}
echo "</select> <input type=submit value='切换DB'>]</form>";
?>
</B><br>
<font size=3>
<br>----
<a href=.>首页</a>
<?php
echo '&nbsp;<a href=passwd.php>修改密码</a>';
echo '&nbsp;|';
echo '&nbsp;<a href=mail.php>GM邮件</a>';
echo '&nbsp;|';
echo '&nbsp;<a href=user.php>用户管理</a>';
echo '&nbsp;|';
echo '&nbsp;<a href=stat.php>登录统计</a>';
echo '&nbsp;|';
if( $g_isapple )
{
echo '&nbsp;<a href=statpay_apple.php>充值统计</a>';
}
else
{
echo '&nbsp;<a href=statpay.php>充值统计</a>';
}
echo '&nbsp;|';
echo '&nbsp;<a href=device.php>设备统计</a>';
echo '&nbsp;|';
echo '&nbsp;<a href=combats.php>当前战争</a>';
echo '&nbsp;|';
echo '&nbsp;<a href=logview.php>玩家日志</a>';
echo '&nbsp;|';
echo '&nbsp;<a href=toolkit.php>工具箱</a>';
echo '&nbsp;|';
?>
&nbsp;<a href=help.txt>帮助</a>
----
</font>
<br>
<br>
<HR>
</html>
