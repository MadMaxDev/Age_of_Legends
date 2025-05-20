<?php
	$ref_array[1]	= 'https://itunes.apple.com/app/id648847943';
	
	$appid=$_GET['appid'];
	if( !isset($appid) )
	{
		$appid		= 0;
	}
	header("location:$ref_array[$appid]");
?>