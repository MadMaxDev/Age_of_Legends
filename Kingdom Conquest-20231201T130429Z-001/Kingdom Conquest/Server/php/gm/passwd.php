<?php
include "auth.php";
include "header.php";
include "cmn.php";

if( !empty($_POST['act']) )
{
	switch( $_POST['act'] )
	{
		case	'change':
		{
			// 比较新密码是否正确
			if( $_POST['newpass'] != $_POST['newpass2'] )
			{
				disp_str_and_back_exit('两次输入的新密码不匹配!');
			}
			$rst	= DoQuery('select func_passwd('.$g_uid.', "'.md5($_POST['oldpass']).'", "'.md5($_POST['newpass']).'")');
			if( $rst )
			{
				if( ($rowobj=mysql_fetch_array($rst)) )
				{
					$rst	= $rowobj[0];
					if( $rst==0 )
					{
						disp_str_and_back_exit('修改成功^__^');
					}
					else
					{
						disp_str_and_back_exit('修改失败，错误代码:'.$rst);
					}
				}
			}
		}
		break;
	}
}
?>
<html>
<form action="" method=post>
<input type=hidden name=act value=change>
旧密码：<input type=password name=oldpass><br>
新密码：<input type=password name=newpass><br>
确　认：<input type=password name=newpass2><br>
<input type=submit value='提  交'><input type=reset value='取  消'>
</form>
<hr>
<a href='javascript:history.back(1);'>返回</a>
</html>
