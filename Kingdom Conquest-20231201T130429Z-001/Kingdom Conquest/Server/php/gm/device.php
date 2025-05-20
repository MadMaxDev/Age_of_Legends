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
$sql	= "select device_type,count(1) c from $g_webims.common_accounts group by device_type order by c desc";
$rst	= DoQuery($sql);
if( $rst )
{
	echo "<table border=2><tr><th>设备类型</th><th>数量</th>\n";
	while( ($row=mysql_fetch_array($rst))!=NULL )
	{
		$dev		= $g_gm_device_type[$row[0]];
		$count	    = $row[1];
		echo("<tr><th>$dev</th><th>$count</th>\n");
	}
	echo "</table>";
}

?>
