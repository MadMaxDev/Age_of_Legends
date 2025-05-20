<?php
include "auth.php";
include "header.php";
include "cmn.php";
// 包含item数组
include "data/item.php";
// 建筑名称
include "data/building.php";

// 显示玩家查询
echo
'
<form method=post action=?action=search>
方法1.  输入角色名（可用%通配符）：<input name=name size=20> &nbsp;&nbsp; <input type=submit value=检索><br>
</form>
<form method=post action=?action=showtoday>
<HR>
方法2.  <input type=submit value=显示最近登录用户><br>
</form>
<form method=post action=?action=showluxury>
<HR>
方法3.  输入抽奖价值高于: <input type=text name=value value=100> &nbsp;&nbsp; 
<input type=submit value=显示抽中高价物品的用户><br>
</form>
<HR>
';

function doSearchPlayer($sql)
{
	$rst	= DoQuery($sql);
	if( $rst )
	{
		echo "<table border=2><tr><th>帐号ID</th><th>角色名</th><th>登录时间</th><th>日志操作</th>";
		while( $row=mysql_fetch_array($rst) )
		{
			$account_id	= $row[0];
			$name		= $row[1];
            $logintime  = $row[2];
			echo "<tr><th><a href=accountname.php?accid=$account_id>$account_id</a></th><th><a href=user.php?action=search&accid=$account_id>$name</a></th><th>$logintime</th><th>"
				." &nbsp;&nbsp; <a href=?action=stone_consume&accid=$account_id>钻石/水晶消费统计</a>"
				." &nbsp;&nbsp; <a href=?action=item_sell&accid=$account_id>道具售出记录</a>"
				." &nbsp;&nbsp; <a href=?action=lottery_gain&accid=$account_id>抽奖获得记录</a>"
				."</th>";
		}
		echo "</table>";
	}
}

// 判断是否有action
$action		= $_REQUEST['action'];
$account_id	= $_REQUEST['accid'];
if( $action=='search' )
{
	$name	= $_REQUEST['name'];
    if( $name )
    {
        $sql	= "select account_id,name,from_unixtime(last_login_time) from $g_webdb.common_characters where name like '$name' limit $g_gm_page_size";
    }
    else if( $account_id )
    {
        $sql	= "select account_id,name,from_unixtime(last_login_time) from $g_webdb.common_characters where account_id=$account_id";
    }
	doSearchPlayer($sql);
}
else if( $action=='showtoday' )
{
	$sql	= "select account_id,name,from_unixtime(last_login_time) from $g_webdb.common_characters order by last_login_time desc limit $g_gm_page_size";
	doSearchPlayer($sql);
}
else if( $action=='showluxury' )
{
    $value  = $_REQUEST['value'];
	$sql	= "select c.account_id,c.name,from_unixtime(c.last_login_time) from $g_webdb.common_characters c join (select account_id from $g_weblog.log_lottery where value>=$value group by account_id order by value) l on c.account_id=l.account_id";
	doSearchPlayer($sql);
}
else if( isset($action) && isset($account_id) )
{
    // 先获取一下用户角色名
    $sql	= "SELECT name FROM $g_webdb.common_characters where account_id=$account_id";
    $name	= DoQueryGetOne($sql);
    if( $name )
    {
        $name   = "<a href=logview.php?action=search&accid=$account_id>$name</a>";
    }
    // 如果是以do开头的
    $startt = $_REQUEST['start_date'];
    $endt   = $_REQUEST['end_date'];
    if( $action=='do_stone_consume' )
    {
        $sql    = "select log_datetime,use_type,money_type,money,ext_data_0,ext_data_1,ext_data_2,ext_data_3 from $g_weblog.log_money_cost where account_id=$account_id and log_datetime>='$startt' and log_datetime<'$endt' order by log_datetime desc";
        $rst    = DoQuery($sql);
        if( $rst )
        {
            echo    "$name 的钻石水晶消费:<br><table border=2><tr><th>时间</th><th>用途</th><th>货币类型</th><th>使用数量</th><th>参数0</th><th>参数1</th><th>参数2</th><th>参数3</th>\n";
            while( $row=mysql_fetch_assoc($rst) )
            {
                foreach( $row as $key=>$val )
                {
                    $$key   = $val;
                }
                $use_type_str   = $g_gm_money_use_type_STR[$use_type];
                $money_type     = $g_gm_money_type[$money_type];
                switch( $use_type )
                {
                    case    g_gm_money_use_type_buy_item:
                        $ext_data_0 = $g_item[$ext_data_0];
                        $ext_data_1 = "num:$ext_data_1";
                        break;
                    case    g_gm_money_use_type_accelerate_build:
                        $ext_data_0 = "$ext_data_0 秒";
                        $ext_data_1 = $g_building[$ext_data_1];
                        break;
                    case    g_gm_money_use_type_accelerate_goldore:
                        $ext_data_0 = "$ext_data_0 秒";
                        break;
                    case    g_gm_money_use_type_accelerate_research:
                        $ext_data_0 = "$ext_data_0 秒";
                        break;
                    case    g_gm_money_use_type_accelerate_march:
                        $ext_data_0 = "$ext_data_0 秒";
                        break;
                    case    g_gm_money_use_type_lottery:
                        break;
                    case    g_gm_money_use_type_change_char_name:
                        break;
                    case    g_gm_money_use_type_change_hero_name:
                        break;
                    case    g_gm_money_use_type_change_alliance_name:
                        break;
                    case    g_gm_money_use_type_store_buy_gold:
                        break;
                }
                echo "<tr><th>$log_datetime</th><th>$use_type_str</th><th>$money_type</th><th>$money</th><th>$ext_data_0</th><th>$ext_data_1</th><th>$ext_data_2</th><th>$ext_data_3</th>";
            }
            echo    "</table>\n";
        }
    }
    else if( $action=='do_lottery_gain' )
    {
        $value  = $_REQUEST['value'];
        if( $value )
        {
            $valuesql   = "value>=$value and";
        }
        else
        {
            $valuesql   = '';
        }
        $sql    = "select log_datetime,lottery_type,type,data,value from $g_weblog.log_lottery where $valuesql account_id=$account_id and log_datetime>='$startt' and log_datetime<'$endt' order by log_datetime desc";
        $rst    = DoQuery($sql);
        if( $rst )
        {
            echo    "$name 的抽奖日志:<br><table border=2><tr><th>时间</th><th>投币类型</th><th>获奖类型</th><th>参数</th><th>价值</th>\n";
            while( $row=mysql_fetch_assoc($rst) )
            {
                foreach( $row as $key=>$val )
                {
                    $$key   = $val;
                }
                $money_type = $g_gm_money_type[$lottery_type];
                $rst_type   = $g_gm_lottery_rst_type_STR[$type];
                switch( $type )
                {
                    case    g_gm_lottery_rst_type_item:
                        $data   = $g_item[$data];
                        $value  = '价值:'.$value;
                        break;
                    case    g_gm_lottery_rst_type_crystal:
                        break;
                    case    g_gm_lottery_rst_type_gold:
                        break;
                }
                echo    "<tr><th>$log_datetime</th><th>$money_type</th><th>$rst_type</th><th>$data</th><th>$value</th>";
            }
        }
    }
    else if( $action=='do_item_sell' )
    {
        $sql    = "select log_datetime,item_id,excel_id,num,gold from $g_weblog.log_item where account_id=$account_id and log_datetime>='$startt' and log_datetime<'$endt' order by log_datetime desc";
        $rst    = DoQuery($sql);
        if( $rst )
        {
            echo    "$name 的物品卖出日志:<br><table border=2><tr><th>时间</th><th>物品ID</th><th>物品类型</th><th>数量</th><th>获取金币</th>\n";
            while( $row=mysql_fetch_assoc($rst) )
            {
                foreach( $row as $key=>$val )
                {
                    $$key   = $val;
                }
                $excel_id   = $g_item[$excel_id];
                echo    "<tr><th>$log_datetime</th><th>$item_id</th><th>$excel_id</th><th>$num</th><th>$gold</th>";
            }
            echo    "</table>";
        }
    }
    else
    {
        echo '<script language="javascript" type="text/javascript" src="js/calendar2.js"></script>';
        $queryname  = $g_gm_logtype_STR[$action];
        // 查询角色名
        $startt = date("Y-m-d H", strtotime("-1 day"));
        $endt   = date("Y-m-d H");
        echo "$name 的 $queryname 查询:<br>";
        //
        // 显示日期输入框
        echo "<form method=post action=?action=do_$action><input type=hidden name=accid value=$account_id>";
        echo '开始时间：<input type="text" size=20 name="start_date" value="'.$startt.'"onClick="setDayH(this);"> &nbsp;&nbsp; ';
        echo '结束时间：<input type="text" size=20 name="end_date" value="'.$endt.'"  onClick="setDayH(this);"> ';
        if( $action=='lottery_gain' )
        {
            echo '价值高于:<input name=value size=10>';
        }
        echo '<br>';
        echo '<input type=submit value=提交>';
        echo "</form>\n";
    }
}

?>
