<?php
include "auth.php";
include "cmn.php";

$dbname = $_POST['dbname'];

if( !empty($dbname) && $dbname!=$g_webdb )
{
    // 更新session中的db名
    $sql    = "update session set dbname='$dbname' where sessionid='$g_sid'";
    $rst    = DoQuery($sql);
}
// 跳转到index
echo "
<html>
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\">
<meta HTTP-EQUIV=REFRESH CONTENT=\"1; URL=index.php\">
切换DB到 $dbname 中...
</html>
";
?>
