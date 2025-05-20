<?php
include "auth.php";
$rst	= DoQuery("delete from session where uid=$g_uid and sessionid='$g_sid'");
?>
<html>
<meta HTTP-EQUIV=REFRESH CONTENT="4; URL=.">
返回<a href=.>首页</a>
</html>
