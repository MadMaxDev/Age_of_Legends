<?php
include "auth.php";
include "header.php";
include "cmn.php";
// 包含item数组
include "data/item.php";

echo
"
<script type=\"text/javascript\">
</script>
";

// 显示用户查询界面
echo
'
<form method=post action=?action=search>
输入角色名（可用%通配符）：<input name=name size=20> &nbsp;&nbsp; <input type=submit value=检索><br>
</form>
<form method=post action=?action=showtoday>
<input type=submit value=显示最近登录用户><br>
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
		echo "<table border=2><tr><th>帐号ID</th><th>角色名</th><th>操作</th><th>信息</th>";
		while( $row=mysql_fetch_array($rst) )
		{
			$account_id	= $row[0];
			$name		= $row[1];
			$lastlogin	= $row[2];
			$level		= $row[3];
			echo "<tr><th><a href=accountname.php?accid=$account_id>$account_id</a></th><th>$name</th><th><a href=?action=mail&accid=$account_id>GM邮件</a>"
				." &nbsp;&nbsp; <a href=?action=rewardmail&accid=$account_id>奖励邮件</a>"
				." &nbsp;&nbsp; <a href=?action=fakemail&accid=$account_id>伪造邮件</a>"
				."<br><br><a href=?action=item&accid=$account_id>道具发放</a>"
				." &nbsp;&nbsp; <a href=?action=gold&accid=$account_id>金币发放</a>"
				." &nbsp;&nbsp; <a href=?action=diamond&accid=$account_id>水钻修改</a>"
				."<br><br><a href=?action=fakecharge&accid=$account_id>伪装充值</a>"
				."</th><th>"
				." Lv:$level Login:$lastlogin "
				." &nbsp;&nbsp; <a href=?action=detail&accid=$account_id>详情</a>"
				." &nbsp;&nbsp; <a href=?action=hero&accid=$account_id>将领</a>"
				." &nbsp;&nbsp; <a href=?action=itemlist&accid=$account_id>道具列表</a>"
				." &nbsp;&nbsp; <a href=?action=get_receive_mail&accid=$account_id>收件箱</a>"
				." &nbsp;&nbsp; <a href=?action=get_send_mail&accid=$account_id>发件箱</a>"
                ." &nbsp;&nbsp; <a href=logview.php?action=search&accid=$account_id>日志</a>"
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
        $sql	= "select account_id,name,from_unixtime(last_login_time),level from $g_webdb.common_characters where name like '$name' limit $g_gm_page_size";
    }
    else if( $account_id )
    {
        $sql	= "select account_id,name,from_unixtime(last_login_time),level from $g_webdb.common_characters where account_id=$account_id";
    }
	doSearchPlayer($sql);
}
else if( $action=='showtoday' )
{
	$sql	= "select account_id,name,from_unixtime(last_login_time),level from $g_webdb.common_characters order by last_login_time desc limit $g_gm_page_size";
	doSearchPlayer($sql);
}
else if( $action=='detail' )
{
	$sql	= "select name,from_unixtime(create_time) ctime,from_unixtime(last_login_time) intime,from_unixtime(last_logout_time) outtime,level,exp,development,population,diamond,crystal,gold,vip,pos_x,pos_y,total_diamond,inet_ntoa(ip) as ip from $g_webdb.common_characters where account_id=$account_id";
	$rst	= DoQuery($sql);
	if( $rst )
	{
		$row	= mysql_fetch_assoc($rst);
		if( $row )
		{
			echo "玩家详细信息：<br><table border=2>\n";
			foreach($row as $key=>$val)
			{
				$keystr	= $g_gm_pinfo_field_STR[$key];
				if( !$keystr )
				{
					$keystr	= $key;
				}
				if ( $keystr == "ip")
				{
					$token	= strtok($val, ".");
					$newval	= $token;
					while ($token !== false)
					{
						$token	= strtok(".");
						$newval	= $token.".".$newval;
					}
					$val	= substr($newval,1);
				}
				echo("<tr><th>$keystr</th><th>$val</th>\n");
			}
			echo "</table>\n<br>";
            {
                $sql    = "select device_type from $g_webims.common_accounts where account_id=$account_id";
                $rst    = DoQuery($sql);
                if( $rst )
                {
                    $row    = mysql_fetch_array($rst);
                    $dev    = $row[0];
                    $dev    = $g_gm_device_type[$dev]."[$dev]";
                    echo "设备: $dev<br><br>";
                }
            }
            {
                echo "城防:<br>";
                $sql    = "select * from $g_webdb.citydefenses where account_id=$account_id";
                $rst    = DoQuery($sql);
                if( $rst )
                {
                    $row    = mysql_fetch_array($rst);
                    if( $row )
                    {
                        echo "<table border=2><tr><th>战道</th><th>将领名</th><th>级别</th><th>兵类型</th><th>兵级别</th><th>兵数量</th>\n";
                        for($i=1;$i<6;$i++)
                        {
                            $hid    = $row[$i];
                            $sql    = "select name,level,army_type,army_level,army_num from $g_webdb.hire_heros where hero_id=$hid";
                            $rst    = DoQuery($sql);
                            if( $rst )
                            {
                                $row2   = mysql_fetch_array($rst);
                                if( $row2 )
                                {
                                    $name   = $row2[0];
                                    $level  = $row2[1];
                                    $atype  = $g_gm_army_type[$row2[2]];
                                    $alevel = $row2[3];
                                    $anum   = $row2[4];
                                    echo "<tr><th>$i</th><th><a href=?action=herodetail&id=$hid>$name</a></th><th>$level</th><th>$atype</th><th>$alevel</th><th>$anum</th>\n";
                                }
                            }
                        }
                        echo "</table>";
                    }
                }
            }
            {
                echo "兵营:<br>";
                $sql    = "select excel_id,level,num from $g_webdb.soldiers where account_id=$account_id";
                $rst    = DoQuery($sql);
                if( $rst )
                {
                    echo "<table border=2><tr><th>类型</th><th>级别</th><th>数量</th>";
                    while( $row=mysql_fetch_array($rst) )
                    {
                        $type   = $g_gm_army_type[$row[0]];
                        $lvl    = $row[1];
                        $num    = $row[2];
                        echo    "<tr><th>$type</th><th>$lvl</th><th>$num</th>\n";
                    }
                    echo "</table>";
                }
            }
            {
                echo "科技:<br>";
                $sql    = "select excel_id,level from $g_webdb.technologys where account_id=$account_id";
                $rst    = DoQuery($sql);
                if( $rst )
                {
                    echo "<table border=2><tr><th>类型</th><th>级别</th>";
                    while( $row=mysql_fetch_array($rst) )
                    {
                        $type   = $g_gm_army_type[$row[0]];
                        $lvl    = $row[1];
                        echo    "<tr><th>$type</th><th>$lvl</th>\n";
                    }
                    echo "</table>";
                }
            }
            echo " &nbsp;&nbsp; <a href=?action=hero&accid=$account_id>将领</a>";
            echo " &nbsp;&nbsp; <a href=?action=itemlist&accid=$account_id>道具列表</a>";
            echo " &nbsp;&nbsp; <a href=logview.php?action=search&accid=$account_id>日志</a>";
		}
	}
}
else if( $action=='itemlist' )
{
    // 获取名字
	$sql	= "SELECT name FROM $g_webdb.common_characters where account_id=$account_id";
	$name	= DoQueryGetOne($sql);
    echo "$name 的装备列表:<br>";
    // 包裹中的
    $sql    = "select item_id,excel_id,num from $g_webdb.items where account_id=$account_id order by excel_id";
    $rst    = DoQuery($sql);
    if( $rst )
    {
		echo '包裹中的:<br><table border=2><tr><th>id</th><th>装备名</th><th>数量</th>';
        while( $row=mysql_fetch_array($rst) )
        {
            $itemid     = $row[0];
            $itemname   = $g_item[$row[1]];
            $num        = $row[2];
            echo "<tr><th>$itemid</th><th>$itemname</th><th>$num</th>";
        }
        echo '</table>';
    }
    // 武将身上的
    // 先选取武将到列表
	$sql	= "select hero_id,name from $g_webdb.hire_heros where account_id=$account_id";
    $idx        = 0;
    $heroid     = array();
    $heroname   = array();
	$rst	= DoQuery($sql);
	if( $rst )
	{
		while( $row=mysql_fetch_array($rst) )
		{
            $heroid[$idx]       = $row[0];
            $heroname[$idx]     = $row[1];
            $idx    ++;
        }
        echo "武将身上的:<br>";
        // 列表武将及物品
        for($i=0;$i<$idx;$i++)
        {
            $id     = $heroid[$i];
            $name   = $heroname[$i];
            $sql    = "select item_id,excel_id from $g_webdb.hero_items where account_id=$account_id and hero_id=$id order by equip_type";
            $rst    = DoQuery($sql);
            if( $rst )
            {
                echo "$name:<br><table border=2><tr><th>id</th><th>装备名</th><th>镶嵌</th>";
                $itemid_arr     = array();
                $itemanme_arr   = array();
                $slotsize       = 0;
                while( $row=mysql_fetch_array($rst) )
                {
                    $itemid_arr[$slotsize]      = $row[0];
                    $itemname_arr[$slotsize]    = $g_item[$row[1]];
                    $slotsize   ++;
                }
                for($j=0;$j<$slotsize;$j++)
                {
                    $itemid     = $itemid_arr[$j];
                    $itemname   = $itemname_arr[$j];
                    echo "<tr><th>$itemid</th><th>$itemname</th>";
                    $sql    = "select slot_idx,slot_type,excel_id,src_id from $g_webdb.item_slots where account_id=$account_id and item_id=$itemid order by slot_idx";
                    $rst2   = DoQuery($sql);
                    if( $rst2 )
                    {
                        echo '<th><table border=1><tr><th>slot_idx</th><th>slot_type</th><th>宝石名</th><th>物品id</th>';
                        
                        while( $row2=mysql_fetch_array($rst2) )
                        {
                            $slot_idx   = $row2[0];
                            $slot_type  = $row2[1];
                            $itemname   = $g_item[$row2[2]];
                            $itemid     = $row2[3];
                            echo "<tr><th>$slot_idx</th><th>$slot_type</th><th>$itemname</th><th>$itemid</th>";
                        }
                        echo '</table></th>';
                    }
                }
                echo '</table>';
            }
        }
    }
}
else if( $action=='hero' )
{
	// 将领查询
	$sql	= "select hero_id,name,level,status,army_type,army_level,army_num from $g_webdb.hire_heros where account_id=$account_id";
	$rst	= DoQuery($sql);
	if( $rst )
	{
		echo '将领列表：<br><table border=2><tr><th>将领ID</th><th>名字</th><th>级别</th><th>状态</th><th>带兵,lvl,num</th>';
		while( $row=mysql_fetch_array($rst) )
		{
			$id		= $row[0];
			$name	= $row[1];
			$level	= $row[2];
			$status	= $g_gm_hero_status_STR[$row[3]]."[".$row[3]."]";
            $atype  = $g_gm_army_type[$row[4]];
            $alvl   = $row[5];
            $anum   = $row[6];
			echo "<tr><th>$id</th><th><a href=?action=herodetail&id=$id>$name</a></th><th>$level</th><th>$status</th><th>$atype,$alvl,$anum</th>";
		}
		echo '</table>';
	}
}
else if( $action=='herodetail' )
{
    $id     = $_REQUEST['id'];
    $sql    = "select * from $g_webdb.hire_heros where hero_id=$id";
    $rst    = DoQuery($sql);
    if( $rst )
    {
        $row    = mysql_fetch_assoc($rst);
        if( $row )
        {
            echo "<table border=2>";
            foreach($row as $key=>$val)
            {
                /*
                $keystr	= $g_gm_pinfo_field_STR[$key];
                if( !$keystr )
                {
                    $keystr	= $key;
                }
                */
                if( $key=='army_type' )
                {
                    echo("<tr><th>带兵</th><th>".$g_gm_army_type[$val]."</th>\n");
                }
                else
                {
                    echo("<tr><th>$key</th><th>$val</th>\n");
                }
            }
            echo "</table>";
        }
    }
}

else if( $action=='mail' )
{
	// 发送gm邮件
	$sql	= "SELECT c.name FROM $g_webdb.common_characters c where c.account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	echo "发送GM邮件 To: $name <br>";
	echo "<form method=post action='?action=domail&accid=$account_id'>";
	echo "<textarea name=msg rows=10 cols=100>\n尊敬的玩家 $name 您好：\n";
	echo "</textarea><br>";
	echo "<input type=submit value='提交'>";
	echo "</form><br><br>";
}
else if( $action=='domail' )
{
	// 发送邮件到数据库
	$text	= AntiSQLInjection($_REQUEST['msg']);
	$sql	= "CALL $g_webdb.add_private_mail(0,$account_id,4,1,0,'$text',NULL,0,@result,@mail_id)";
	$rst	= DoQuery($sql);
	echo	'发送完毕<br>';
}
else if( $action=='rewardmail' )
{
	$sql	= "SELECT c.name FROM $g_webdb.common_characters c where c.account_id=$account_id";
	$name	= DoQueryGetOne($sql);
echo
'
<script language="javascript">
String.prototype.trim=function(){ 
    return this.replace(/(^\s*)|(\s*$)/g, ""); 
} 

function hideElementsById(Id,bHide){
    var strDisplay = "";
    if(bHide) strDisplay = "none";
    var obj = document.getElementById(Id);
    obj.style.display = strDisplay;
}       
function onRewardTypeSelect()
{
    var Val=document.getElementById("reward_type").value;
    if( Val==1 )
    {
        hideElementsById("itemid",false);
    }
    else
    {
        hideElementsById("itemid",true);
    }
}
function onAdd()
{
    var obj = document.getElementById("reward_id");
    obj.value = obj.value.trim();
    var type = document.getElementById("reward_type").value;
    obj.value   = obj.value + type + "*" + document.getElementById("reward_num").value + "*";
    if( type==1 )
    {
        obj.value   = obj.value + document.getElementById("itemid").value;
    }
    else
    {
        obj.value   = obj.value + "0";
    }
    obj.value = obj.value+",";
}
</script>
<body onLoad=onRewardTypeSelect();>
';
	echo "发送奖励邮件 To: $name <br>";
	echo "<form method=post action='?action=dorewardmail&accid=$account_id'>";
    echo "文字：<br>";
	echo "<textarea name=msg id=msg_id rows=5 cols=100>\n尊敬的玩家 $name 您好：\n\n鉴于您对口袋帝国的杰出贡献，特奖励您如下奖品：";
	echo "</textarea><br>";
    echo "奖品：<br>";
	echo "<textarea name=reward id=reward_id rows=5 cols=100>";
	echo "</textarea> 奖励类型:<select name=reward_type id=reward_type onclick='onRewardTypeSelect()'>";
    foreach($g_gm_mail_reward_type as $key=>$val)
    {
        echo    "<option value=$key>$val</option>\n";
    }
    echo "</select> &nbsp;&nbsp; 数量:<input name=reward_num id=reward_num size=10 value=1> &nbsp;&nbsp; <select name=itemid id=itemid>";
	foreach($g_item as $key=>$val)
	{
		echo("<option value=$key>$val</option>\n");
	}
    echo "</select> &nbsp; <input type=button value='Add' onclick='onAdd()' ";
	echo "<br><br><br><input type=submit value='发送'>";
	echo "</form><br><br>";
}
else if( $action=='dorewardmail' )
{
	// 发送邮件到数据库
	$text	= AntiSQLInjection($_REQUEST['msg']);
	$ext	= trim($_REQUEST['reward']);
    // 检查一下ext格式，如果是空串就不发送，如果是","开头，则删除
    if( $ext=='' )
    {
        echo    '奖励内容不能为空!!!!<br>';
    }
    else
    {
        $sql	= "CALL $g_webdb.add_private_mail(0,$account_id,4,3,0,'$text','$ext',0,@result,@mail_id)";
        $rst	= DoQuery($sql);
        echo	'发送完毕<br>';
    }
}
else if( $action=='fakemail' )
{
	// 发送假冒用户邮件
	$sql	= "SELECT c.name FROM $g_webdb.common_characters c where c.account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	echo "发送伪造邮件 To: $name <br>";
	echo "<form method=post action='?action=fakemail1&accid=$account_id'>";
	echo "输入发信人角色名：<input name=fakename size=20> &nbsp;&nbsp; <input type=submit value='检索'>";
	echo "</form><br><br>";
}
else if( $action=='fakemail1' )
{
	// 目标人姓名
	$sql	= "SELECT c.name FROM $g_webdb.common_characters c where c.account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	$fakename	= $_REQUEST['fakename'];
	echo "发送伪造邮件 To: $name <br>请从下面发件人中选取：<br>";
	// 检索发送人姓名
	echo "<form method=post action='?action=fakemail2&accid=$account_id'>";
	$sql	= "SELECT account_id,name FROM $g_webdb.common_characters where name like '$fakename'";
	$rst	= DoQuery($sql);
	if( $rst )
	{
		echo "<table border=2><tr><th>角色名</th>";
		while( $row=mysql_fetch_array($rst) )
		{
			$fakeid	= $row[0];
			$name	= $row[1];
			echo "<tr><th><a href='?action=fakemail2&accid=$account_id&fakeid=$fakeid'>$name</a></th>";
		}
		echo "</table>";
	}
	echo "</form><br><br>";
}
else if( $action=='fakemail2' )
{
	// 显示发送界面
	// 目标人姓名
	$sql	= "SELECT c.name FROM $g_webdb.common_characters c where c.account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	$fakeid	= $_REQUEST['fakeid'];
	$sql	= "SELECT c.name FROM $g_webdb.common_characters c where c.account_id=$fakeid";
	$fakename	= DoQueryGetOne($sql);
	echo "<form method=post action='?action=dofakemail&accid=$account_id&fakeid=$fakeid'>";
	echo "发送伪造邮件 To: $name &nbsp;&nbsp; From: $fakename &nbsp;&nbsp; <select name=mailtype id=mailtype_sel><option value=1>私人邮件</option><option value=2>联盟邮件</option><select><br>";
	echo "<textarea name=msg rows=10 cols=100>\n$name 您好：\n";
	echo "</textarea><br>";
	echo "<input type=submit value='提交'>";
	echo "</form><br><br>";
}
else if( $action=='dofakemail' )
{
	// 发送邮件到数据库
	$fakeid	= $_REQUEST['fakeid'];
	$text	= AntiSQLInjection($_REQUEST['msg']);
	$mailtype	= $_REQUEST['mailtype'];
	$sql	= "CALL $g_webdb.add_private_mail($fakeid,$account_id,$mailtype,1,0,'$text',NULL,0,@result,@mail_id)";
	$rst	= DoQuery($sql);
	echo	'发送完毕<br>';
}
else if( $action=='item' )
{
	// 查找用户角色名
	$sql	= "select name from $g_webdb.common_characters where account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	echo "-----------------------------------------------------<br>Send item To: $name <br>";
	echo '<form method=post action="?action=doitem&accid='.$account_id.'"><select name=itemlist id=itemlist_sel>';
	foreach($g_item as $key=>$val)
	{
		echo("<option value=$key>$val</option>\n");
	}
	echo '</select>';
	echo ' &nbsp;&nbsp; 数量:<input size=10 name=itemnum value=1>';
	echo ' &nbsp;&nbsp; <input type=submit value="发送">';
	echo '</form>';
}
else if( $action=='doitem' )
{
	$itemid	= $_REQUEST['itemlist'];
	$num	= $_REQUEST['itemnum'];
	echo "Adding item:$itemid num:$num to $account_id ... "; 
	$sql	= "CALL $g_webdb.game_add_item($account_id,0,$itemid,$num,0,@result);";
	DoQuery($sql);
	echo "OVER.";

}
else if( $action=='gold' )
{
	// 查找用户角色名
	$sql	= "select name from $g_webdb.common_characters where account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	echo "-----------------------------------------------------<br>增加 金币 To: $name <br>";
	echo '<form method=post action="?action=dogold&accid='.$account_id.'">';
	echo ' &nbsp;&nbsp; 数量:<input size=10 name=num value=10000>';
	echo ' &nbsp;&nbsp; <input type=submit value="发送">';
	echo '</form>';
}
else if( $action=='dogold' )
{
	$num	= $_REQUEST['num'];
	echo "Adding 金币:$num to $account_id ... "; 
	$sql	= "update $g_webdb.common_characters set gold=gold+$num where account_id=$account_id";
	DoQuery($sql);
	echo "OVER.";

}
else if( $action=='diamond' )
{
	// 查找用户角色名
	$sql	= "select name from $g_webdb.common_characters where account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	echo '<form method=post action="?action=dodiamond&accid='.$account_id.'">';
	echo "-----------------------------------------------------<br>增加 <select name=stone><option value=\"diamond\">钻石</option><option value=\"crystal\">水晶</option></select> To: $name <br>";
	echo ' &nbsp;&nbsp; 数量:<input size=10 name=num value=10>';
	echo ' &nbsp;&nbsp; <input type=submit value="发送">';
	echo '</form>';
}
else if( $action=='dodiamond' )
{
	$num	= $_REQUEST['num'];
    $stone  = $_REQUEST['stone'];
    if( $stone!='diamond' )
    {
        $stone  = 'crystal';
        $name   = '水晶';
    }
    else
    {
        $name   = '钻石';
    }
	echo "Adding $name:$num to $account_id ... "; 
	$sql	= "update $g_webdb.common_characters set $stone=$stone+$num where account_id=$account_id";
	DoQuery($sql);
	echo "OVER.";

}
else if( $action=='fakecharge' )
{
	// 查找用户角色名
	$sql	= "select name from $g_webdb.common_characters where account_id=$account_id";
	$name	= DoQueryGetOne($sql);
	echo "-----------------------------------------------------<br>假装充值 钻石 To: $name <br>";
	echo '<form method=post action="?action=dofakecharge&accid='.$account_id.'">';
	echo ' &nbsp;&nbsp; 数量:<input size=10 name=num value=50>';
	echo ' &nbsp;&nbsp; <input type=submit value="发送">';
	echo '</form>';
}
else if( $action=='dofakecharge' )
{
	$num	= $_REQUEST['num'];
	echo "假装充值 钻石:$num to $account_id ... "; 
	$sql	= "insert into $g_webdb.pay91_procqueue values(substr(concat('FAKE-',UUID()),36), $account_id, 0, $num)";
	DoQuery($sql);
	echo "OVER.";

}
else if( $action=='get_receive_mail' )
{
	$sql    = "CALL $g_webdb.get_receive_mails($account_id)";
    $rst    = DoQuery($sql);
	if( $rst )
	{
		echo    "$name 接收的邮件:<br><table border=2><tr><th>发送者</th><th>时间</th><th>内容</th>\n";
		while( $row=mysql_fetch_assoc($rst) )
		{
			foreach( $row as $key=>$val )
			{
				$$key   = $val;
			}
			echo    "<tr><th>$name</th><th>$time</th><th>$text</th>";
		}
	}
}
else if( $action=='get_send_mail' )
{
	$sql    = "CALL $g_webdb.get_send_mails($account_id)";
    $rst    = DoQuery($sql);
	if( $rst )
	{
		echo    "$name 发送的邮件:<br><table border=2><tr><th>接收者</th><th>时间</th><th>内容</th>\n";
		while( $row=mysql_fetch_assoc($rst) )
		{
			foreach( $row as $key=>$val )
			{
				$$key   = $val;
			}
			echo    "<tr><th>$name</th><th>$time</th><th>$text</th>";
		}
	}
}
?>
