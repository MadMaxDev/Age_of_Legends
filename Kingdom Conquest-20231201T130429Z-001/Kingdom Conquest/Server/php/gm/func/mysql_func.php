<?php

define('CLIENT_MULTI_RESULTS', 131072);

function OpenDB()
{
	global $g_dbh;
	if( !isset($g_dbh) )
	{
		$g_dbh    = mysql_connect(g_dbhost, g_dbuser, g_dbpass, true, CLIENT_MULTI_RESULTS);
		if( !$g_dbh )
		{
			echo "Can not connect to g_dbhost";
			return  false;
		}
		if( !mysql_select_db(g_dbname,$g_dbh) )
		{
			return  false;
		}
		DoQuery("SET NAMES ".g_dbcharset);
	}
	return  true;
}

function DoQuery($query)
{
	global $g_dbh;
	global $g_mysql_lastrst;
	if( isset($g_mysql_lastrst) )
	{
		mysql_free_result($g_mysql_lastrst);
	}
	$g_mysql_lastrst    = mysql_query($query, $g_dbh);
	if( !$g_mysql_lastrst )
	{
		echo "Query error:".mysql_error()."<br>Query=".$query."<br>";
	}
	if( !is_resource($g_mysql_lastrst) )
	{
		$g_mysql_lastrst	= NULL;
	}
	return  $g_mysql_lastrst;
}

// 这个只用来获取select到一个结果的用途
function DoQueryGetOne($query)
{
	$rst	= DoQuery($query);
	if( $rst )
	{
		$row	= mysql_fetch_row($rst);
		if( $row )
		{
			return	$row[0];
		}
	}
	return	NULL;
}

function AntiSQLInjection($value)
{
	// 去除斜杠
	if (get_magic_quotes_gpc())
	{
		// get_magic_quotes_gpc取得PHP环境变量magic_quotes_gpc的值
		// 返回0表示关闭本功能；
	// 返回1表示本功能打开。
	// 当magic_quotes_gpc打开时,所有的'(单引号),"(双引号),\(反斜线)以及空字符会自动转为含有反斜线的转义字符
		$value	= stripslashes($value);
	}
	
	// 如果不是数字则转义
	if (!is_numeric($value))
	{
		$value = mysql_real_escape_string($value);
	}
	return $value;
}

// 一般要用这个肯定是要打开DB的了
OpenDB();

?>
