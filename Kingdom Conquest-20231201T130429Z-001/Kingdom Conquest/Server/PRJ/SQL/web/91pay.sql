SET GLOBAL log_bin_trust_function_creators=ON;

-- 游戏服务器调用这个生成guid发给客户端
drop function if exists gen91PaySerial;
delimiter //
create function gen91PaySerial(_accountid bigint unsigned) returns char(36)
begin
	declare str char(36) default  UUID();	-- 这个理论上是不会重复的，如果重复了，就返回错误吧。
	insert into pay91_prepare values(str, _accountid, unix_timestamp());
	return str;
end
//
delimiter ;

-- php页面调用这个来存放结果
drop function if exists buy91done;
delimiter //
create function buy91done(_serial char(36), _goodsid int, _goodscount int, _ConsumeStreamId char(36), _accountidtobe bigint unsigned, _accountnametobe char(32), _ip char(16)) returns int
begin
	declare _accountid bigint unsigned default NULL;
	declare _createtime int unsigned;
	-- 判断序列号是否存在（不存在应该记录到错误日志表中）
	select accountid,createTime into _accountid,_createtime from pay91_prepare where serial = _serial;
	-- 不存在
	if isnull(_accountid) then
		-- 判断一下是否在done中
		if not exists (select 1 from pay91_done where serial = _serial) then
			-- 保存到fail表中
			insert into pay91_fail values(_serial, _accountidtobe, _accountnametobe, _createtime, unix_timestamp(), -1, inet_aton(_ip));
			return	-1;
		end if;
		-- 已经成功了的
		return 2;
	end if;
	-- 加入done表（如果有dup，只能是在上次存储过程中失败了，就直接忽略吧）
	insert into pay91_done values(_serial, _accountid, _accountnametobe, _createtime, unix_timestamp(), _goodsid, _goodscount, _ConsumeStreamId, 0, inet_aton(_ip)) on duplicate key update trycount=trycount+1;
	-- 加入queue（如果有dup，只能是在上次存储过程中失败了，就直接忽略吧）
	insert into pay91_procqueue values(_serial, _accountid, _goodsid, _goodscount);
	-- 从prepare中删除
	delete from pay91_prepare where serial = _serial;
	return 1;
end
//
delimiter ;

-- 购买过程：
-- 客户端点开购买界面，决定购买，向服务器请求serial
-- 服务器调用gen91PaySerial，生成serial，返回给客户端
-- 客户端利用serial进行91的购买
-- 91购买成功转向我们的php购买结果页面pay.php
-- pay.php判断成功把购买记录写入pay91_procqueue中
-- 服务器程序每隔2秒检查pay91_procqueue，发现有记录就反给客户端（同时记得要给返奖哦！！！！）
