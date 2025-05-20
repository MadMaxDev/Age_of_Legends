<?php

// 显示错误并显示返回链接
function disp_str_and_back($err)
{
	echo	$err." <a href='javascript:history.back(1);'>返回</a><br>";
}
// 然后再退出
function disp_str_and_back_exit($err)
{
	disp_str_and_back($err);
	exit(0);
}
function get_time_str($t)
{
	if( $t==0 )
	{
		return	'';
	}
	return	date('Y-m-d H:i:s', $t);
}
function GetValue($varname)
{
	global $rowobj;
	if( isset($_REQUEST[$varname]) )
	{
		echo ' value="'.$_REQUEST[$varname].'"';
		return	1;
	}
	if( isset($rowobj) && isset($rowobj[$varname]) )
	{
		echo ' value="'.$rowobj[$varname].'"';
		return	1;
	}
	if( isset($_COOKIE['WH_LOGINFO_DFT_'.$varname]) )
	{
		echo ' value='.$_COOKIE['WH_LOGINFO_DFT_'.$varname];
		return	1;
	}
	return	0;
}
function GetValueDft($varname, $dftval)
{
	if( GetValue($varname) )
	{
		return	1;
	}
	echo ' value="'.$dftval.'"';
	return	1;
}
function GetValue0($varname)
{
	global $rowobj;

	if( isset($_REQUEST[$varname]) )
	{
		echo $_REQUEST[$varname];
		return	1;
	}
	if( isset($rowobj) && isset($rowobj[$varname]) )
	{
		echo $rowobj[$varname];
		return	1;
	}
	if( isset($_COOKIE['WH_LOGINFO_DFT_'.$varname]) )
	{
		echo $_COOKIE['WH_LOGINFO_DFT_'.$varname];
		return	1;
	}
	return	0;
}
function GetValue0Dft($varname, $dftval)
{
	if( GetValue0($varname) )
	{
		return	1;
	}
	echo $dftval;
	return	1;
}
function GetRequest($varname, $default)
{
	if( isset($_REQUEST[$varname]) )
	{
		return	trim($_REQUEST[$varname]);
	}
	else
	{
		return	$default;
	}
}
// 打印mysql返回的表格
class	PrintMySQLReturnTable
{
	var $query_rst;
	var $row;
	// 表格的第一行
	// 返回为真假
	function	Hdr()
	{
		$rst	= '';
		for($i=0; $i<mysql_num_fields($this->query_rst); $i++)
		{
			$l_varname  = mysql_field_name($this->query_rst, $i);
			$rst	.= "<td>$l_varname&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
		}
		return	$rst;
	}
	// 表格的后面的各个行
	// 返回为真假
	function	NextLine()
	{
		$this->row    = mysql_fetch_object($this->query_rst);
		if( !$this->row )
		{
			return	'';
		}
		$rst	= '';
		for($i=0; $i<mysql_num_fields($this->query_rst); $i++)
		{
			$l_varname  = mysql_field_name($this->query_rst, $i);
			if( !isset($this->row->$l_varname) )
			{
				 $rst	.= "<td></td>";
			}
			else
			{
				 $rst	.= "<td>".htmlspecialchars($this->row->$l_varname)."</td>";
			}
		}
		return	$rst;
	}
}
?>
