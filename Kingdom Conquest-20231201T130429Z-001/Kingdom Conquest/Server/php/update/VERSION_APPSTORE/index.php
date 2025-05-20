<?php
// 这个是为了兼容最老版本,后续版本只需要使用bigversion_appid
$g_updatesite['1.0_1']='www.nutgame.net/battleforempire/update/down.php?file=';

$cliver=$_GET['cliver'];
if( !isset($cliver) )
{
	die('client version must be set!'.time());
}
$appid=$_GET['appid'];
if (empty($appid))
{
	$site=$g_updatesite[$cliver];
}
else
{
	$site=$g_updatesite[$cliver.'_'.$appid];
}
if(empty($site))
{
	die('client version error!'.time());
}
echo $site;
?>