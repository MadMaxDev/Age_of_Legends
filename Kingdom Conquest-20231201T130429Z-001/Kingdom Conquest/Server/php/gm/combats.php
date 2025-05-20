<?php
include "auth.php";
include "header.php";
include "cmn.php";

$sql	= "select c.combat_id,c.combat_type,c.attack_name,c.defense_name,from_unixtime(c.begin_time) begin_time,c.use_time,c.attack_data,c.defense_data,c.account_id,c.obj_id from $g_webdb.combats c";
$rst	= DoQuery($sql);
if( $rst )
{
	echo "<table border=2><tr><th>类型</th><th>发起用户</th><th>目标用户</th><th>开始时刻</th><th>时间</th><th>攻击布阵</th><th>攻击方数据</th><th>防守方数据</th>\n";
	while( ($row=mysql_fetch_assoc($rst))!=NULL )
	{
        $combat_id      = $row['combat_id'];
		$type	    	= $row['combat_type'];
		$attack_name 	= $row['attack_name'];
		$defense_name	= $row['defense_name'];
		$begin_time     = $row['begin_time'];
		$use_time       = $row['use_time'];
		$attack_data    = $row['attack_data'];
		$defense_data   = $row['defense_data'];
        $accid          = $row['account_id'];
        $objid          = $row['obj_id'];
		echo("<tr><th>$type</th><th><a href=user.php?action=search&accid=$accid>$attack_name</a></th><th><a href=user.php?action=search&accid=$objid>$defense_name</a></th><th>$begin_time</th><th>$use_time</th><th><a href=combatform.php?id=$combat_id>查看</a></th><th>$attack_data</th><th>$defense_data</th>\n");
	}
	echo "</table>";
}

?>
