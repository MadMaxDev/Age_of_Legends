<?php
$filename=$_GET['file'];
if( !isset($filename) )
{
	die('filename must be set!'.time());
}
// 判断里面是否下载了自己的内容
if( strstr($filename,'.php') )
{
	header('Content-Type: text/html; charset=UTF-8');
	die('这个是不允许的，骚年');
}

ini_set('display_errors','Off');
$filesize = filesize($filename);
if( $filesize<=0 )
{
	die('WTF');
}

header('Cache-Control: no-cache, no-store, must-revalidate'); // HTTP 1.1. 
header('Pragma: no-cache'); // HTTP 1.0. 
header('Expires: 0'); // Proxies. 
header("Content-Type: application/force-download");
header("Content-Type: application/download");
header("content-disposition: attachment; filename= $filename".time());
header("Content-Transfer-Encoding: binary "); 
header("Content-Length: ".$filesize);

    $file = fopen($filename, "rb");
    while(!feof($file))
    {
	$buf = fread($file, 4096);
	if( $buf )
	{
	    echo $buf;
	}
	else
	{
	    break;
	}
    }
    fclose($file);



?>
