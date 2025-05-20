<?php

define('CLIENT_MULTI_RESULTS', 131072);

function sqllog_error($str, $file)
{
	$fp = fopen($file, "a");
	if( $fp )
	{
		fputs($fp, $str);
	}
	fclose($fp);
}

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
		sqllog_error("Query error:".mysql_error()."Query=".$query,"/tmp/IAP_sqlerr.txt");
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

// 一般要用这个肯定是要打开DB的了
OpenDB();

?>
