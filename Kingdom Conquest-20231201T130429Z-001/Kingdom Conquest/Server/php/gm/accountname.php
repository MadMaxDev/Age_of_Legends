<?php
include "auth.php";
include "header.php";
include "cmn.php";

$accid	= $_REQUEST['accid'];
if( !empty($accid) )
{
	$sql	= "select name from $g_webims.common_accounts where account_id=$accid";
	$name	= DoQueryGetOne($sql);
	if( $name )
	{
		echo "Accountid:[$accid] 对应帐号名为: [$name]";
	}
	else
	{
		echo "Accountid:[$accid] NOT FOUND !";
	}
}
?>
