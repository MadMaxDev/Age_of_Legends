<?php
// 使用方法
// nohup php verify_receipt.php grpid >/dev/null 2>/dev/null &
//
require_once "db-config.inc.php";
require_once "mysql_func.php";

if( $argc<2 )
{
    die("必须给出大区号. e.g. php verify_receipt.php 10\n");
}

$grpid      = $argv[1];
$g_webdb    = "web_$grpid";
$pidfile	= "/tmp/verify_receipt-$grpid.pid";

$g_bid_array[1]	= 'com.yueyue.AgeofKingdom';
$g_prefix_array[1]	= 'com.AgeofKingdom.diamonds.';

// 如果之前文件存在，则不能运行
if( file_exists($pidfile) )
{
	die("之前程序可能未正常退出，请检查 $pidfile\n");
}
// 把本脚本的进程号写入tmp
function str2file($str, $file)
{
	$fp = fopen($file, "w");
	if( $fp )
	{
		fputs($fp, $str);
	}
	fclose($fp);
}
str2file(getmypid(), $pidfile);

function wlog($str, $file)
{
	$fp = fopen($file, "a");
	if( $fp )
	{
		fputs($fp, $str);
	}
	fclose($fp);
}

date_default_timezone_set("UTC");

    /**
     * Verify a receipt and return receipt data
     *
     * @param   string  $receipt    Base-64 encoded data
     * @param   bool    $isSandbox  Optional. True if verifying a test receipt
     * @throws  Exception   If the receipt is invalid or cannot be verified
     * @return  array       Receipt info (including product ID and quantity)
     */
    function getReceiptData($receipt, $isSandbox = false)
    {
        // determine which endpoint to use for verifying the receipt
        if ($isSandbox) {
            $endpoint = 'https://sandbox.itunes.apple.com/verifyReceipt';
        }
        else {
            $endpoint = 'https://buy.itunes.apple.com/verifyReceipt';
        }
 
        // build the post data
        $postData = json_encode(
            array('receipt-data' => $receipt)
        );
 
        // create the cURL request
        $ch = curl_init($endpoint);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $postData);
 
        // execute the cURL request and fetch response data
        $response = curl_exec($ch);
        $errno    = curl_errno($ch);
        $errmsg   = curl_error($ch);
        curl_close($ch);
 
        // ensure the request succeeded
        if ($errno != 0) {
			// 应该是网络有问题
            return "curl err:$errmsg code:$errno";
        }
 
        // parse the response data
        $data = json_decode($response);
 
        // ensure response data was a valid JSON string
        if (!is_object($data)) {
            return	'Invalid response data';
        }
 
        // ensure the expected data is present
        if (!isset($data->status) || $data->status != 0) {
            return	'Invalid receipt';
        }
 
        // build the response array with the returned data
        return array(
            'quantity'       =>  $data->receipt->quantity,
            'product_id'     =>  $data->receipt->product_id,
            'transaction_id' =>  $data->receipt->transaction_id,
            'purchase_date'  =>  $data->receipt->purchase_date,
//            'app_item_id'    =>  $data->receipt->app_item_id,
            'bid'            =>  $data->receipt->bid,
            'bvrs'           =>  $data->receipt->bvrs
        );
    }
 

// status定义：
// -1000	太久远的（超过30天了）
// -8		黑客(不匹配）
// 0		正常成功充值
// 1		之前已经有过的记录了
// 从mysql中读出一条待校验数据
function doCheck()
{
    global $g_webdb;
	global $grpid;
	$sql	= "select id,account_id,client_time,server_time,uncompress(bill) as bill,appid from $g_webdb.bills limit 1";
	$rst	= DoQuery($sql);
	if( $rst )
	{
		$row	= mysql_fetch_assoc($rst);
		if( $row )
		{
			foreach($row as $key=>$val) $$key=$val;
			// 解析bill内容
			$binfo	= explode(' ', $bill);
			// fetch the receipt data and sandbox indicator from the post data
			$product_id= $binfo[1];
			$transid   = $binfo[0];
			$receipt   = $binfo[2];
			$isSandbox = true;
			//$isSandbox = false;
		 
			// verify the receipt
            echo "verifying to apple svr ...\n";
			$info = getReceiptData($receipt, $isSandbox);
            echo "done.\n";
			$redo = false;
			redo:
			if( is_string($info) )
			{
				if ($isSandbox && !$redo)
				{
					$info = getReceiptData($receipt, false);
					$redo = true;
					goto redo;
				}
				echo "err $info\n";
				echo "receipt: $receipt\n";
                // 删除之
                wlog("$id $account_id $client_time $server_time $bill $appid\n", "/tmp/IAP_fail-$grpid.txt");
                $sql = "delete from $g_webdb.bills where id=$id";
                $rst2   = DoQuery($sql);
			}
			else
			{
				// 校验内容
				global $g_bid_array;
				global $g_prefix_array;
				if( $info['bid']==$g_bid_array[$appid] && $product_id==$info['product_id'] && $transid==$info['transaction_id'] )
				{
					// receipt is valid, now do something with $info
					echo "Good receipt: $product_id $transid\n";
					$purchase_date	= strtotime($info['purchase_date']);
					if( $purchase_date<=time()-24*3600*30 )
					{
                        echo "too old\n";
						// 太久远的了
						$sql	= "insert into $g_webdb.billslog (id,account_id,client_time,server_time,bill,donetime,status,transaction_id,appid) values($id,$account_id,$client_time,$server_time,compress('$bill'),unix_timestamp(),-1000,$transid,$appid)";
						DoQuery($sql);
					}
					else
					{
						// 判断一下transid是否已经存在
						$sql	= "select count(1) from $g_webdb.billslog where transaction_id=$transid";
						$rst	= DoQuery($sql);
						if( $rst )
						{
							if( $row=mysql_fetch_array($rst) )
							{
								if( $row[0]>0 ) {
                                    echo "already have\n";
									// 说明之前已经有了
									$sql	= "insert into $g_webdb.billslog (id,account_id,client_time,server_time,bill,donetime,status,transaction_id,appid) values($id,$account_id,$client_time,$server_time,compress('$bill'),unix_timestamp(),1,$transid,$appid)";
									DoQuery($sql);
								}
								else {
									// 之前没有，则先给用户发钱
									wlog("$id $account_id $client_time $server_time $bill $appid\n", "/tmp/IAP_done-$grpid.txt");
									// 获取钻石个数
									$l		= strlen($g_prefix_array[$appid]);
									$num	= substr($product_id, $l);
									$sql	= "insert into $g_webdb.pay91_procqueue values(substr(concat('IAP-',UUID()),36), $account_id, 0, $num)";
									DoQuery($sql);
									// 然后记录日志
									$bvrs	= $info['bvrs'];
									$sql	= "insert into $g_webdb.billslog (id,account_id,client_time,server_time,bill,donetime,status,val,purchase_date,bvrs,transaction_id,appid) values($id,$account_id,$client_time,$server_time,compress('$bill'),unix_timestamp(),0,$num,$purchase_date,'$bvrs',$transid,$appid)";
									DoQuery($sql);
								}
							}	
						}
					}
					// 看看时间，如果超过一个月了就不要插入了
					// 判断log中是否已经有了
				}
				else
				{
					// 记录到log中
					echo "hacker receipt:" . $info['bid'] . ";". $product_id . ";" . $info['product_id'] . ";"
					 . $transid . ";" . $info['transaction_id'] . "\n";
					$sql	= "insert into $g_webdb.billslog (id,account_id,client_time,server_time,bill,donetime,status,appid) values($id,$account_id,$client_time,$server_time,compress('$bill'),unix_timestamp(),-8,$appid)";
					DoQuery($sql);
                    $v1=$info['bid'];
                    $v2=$info['product_id'];
                    $v3=$info['transaction_id'];
                    wlog("$id $account_id $client_time $server_time $bill bid:$v1, product_id:$v2, trans_id:$v3, $appid\n", "/tmp/IAP_fail-$grpid.txt");
				}
				// 删除原来的请求串
				$sql	= "delete from $g_webdb.bills where id=$id";
				DoQuery($sql);
			}
		}
	}
}

// 每隔1秒来一次
while(1)
{
	if( file_exists("/tmp/stop_iap_check-$grpid.txt") )
	{
		echo "IAP check is stopped now ^____^\n";
		break;
	}
	sleep(1);
	doCheck();
}

// 删除pid文件
unlink($pidfile);

?>
