<?php
include "auth.php";
include "header.php";
include "cmn.php";

$id     = $_REQUEST['id'];
$sql	= "select ch.*,h.name from $g_webdb.combat_heros ch join $g_webdb.hire_heros h on h.hero_id=ch.hero_id where combat_id=$id";
$rst	= DoQuery($sql);
if( $rst )
{
	echo "<table border=2><tr><th>战道</th><th>将领</th>\n";
	while( ($row=mysql_fetch_assoc($rst))!=NULL )
	{
        $slot   = $row['slot_idx'];
		$name 	= $row['name'];
        $hid    = $row['hero_id'];
		echo("<tr><th>$slot</th><th><a href=user.php?action=herodetail&id=$hid>$name</a></th>\n");
	}
	echo "</table>";
}

?>
