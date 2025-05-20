SET GLOBAL log_bin_trust_function_creators=ON;

-- ----------------------------
-- Procedure structure for `account_create`
-- ----------------------------
DROP PROCEDURE IF EXISTS `account_create`;
DELIMITER ;;
CREATE PROCEDURE `account_create`(IN `_name` varchar(32),IN `_pass` varchar(256),IN `_use_random_name` int unsigned,IN `_device_id` varchar(64),IN `_device_type` int unsigned,IN `_appid` int unsigned,IN `_group_id` int unsigned)
BEGIN
	DECLARE	_account_id	BIGINT UNSIGNED DEFAULT 0;
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT				HANDLER FOR SQLEXCEPTION BEGIN SET _result = -100; SELECT _result; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '40001' BEGIN SET _result = -101; SELECT _result; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '22003' BEGIN SET _result = -102; SELECT _result; END;
	
label_body:BEGIN
	SET @_gift_appid = 0;
	IF _device_id!='' THEN
		IF EXISTS (SELECT 1 FROM banned_device WHERE device_id=_device_id) THEN
			SET _result = -10; # 设备被封禁
			LEAVE label_body;
		END IF;
	END IF;

	IF _use_random_name=1 THEN
		-- 生成64位ID
		SET _account_id = gc_global_id();
		-- 分解出时间因子和递增部分
		CALL gc_unglobal_id(_account_id,0);

		SET @vBaseTick = '2012-05-22 06:00:00';
		SET @vCurTick = NOW();
		SET @vDiffDay = DATEDIFF(@vCurTick,@vBaseTick);
		-- SET @vDiffSec=TIME_TO_SEC(@vCurTick);
		SET @vDiffMin=FLOOR(TIME_TO_SEC(@vCurTick)/60)-FLOOR(TIME_TO_SEC(@vBaseTick)/60);
		SET @vDiffMin=IF(@vDiffMin<0, 1440+@vDiffMin, @vDiffMin);
		-- SET _name = CONCAT('g', @vTime-UNIX_TIMESTAMP(@vBaseTick), @vIDIn1S);
		SET _name = CONCAT('g', @vDiffMin, @vDiffDay, @vIDIn1S);

		SET @_binded = 0;
	ELSE
		IF EXISTS (SELECT 1 FROM common_accounts WHERE name=_name) THEN
			SET _result = -1; # 账号名重复
			LEAVE label_body;
		END IF;
		SET _account_id = gc_global_id();
		SET @_binded = 1;
	END IF;
	
	# 以前在该大区有这个device_id的则创建失败
	IF EXISTS (SELECT 1 FROM common_accounts WHERE device_id=_device_id AND group_id=_group_id) THEN
		#UPDATE common_accounts SET device_id=NULL WHERE device_id=_device_id AND group_id=_group_id;
		SET _result = -3;
		LEAVE label_body;
	END IF;
	IF _device_id='' THEN
		SET _device_id=NULL;
	END IF;
	INSERT INTO common_accounts (account_id,name,pass,last_login_time,device_id,device_type,binded,create_time,appid,group_id)
	VALUES (_account_id,_name,_pass,UNIX_TIMESTAMP(),_device_id,_device_type,@_binded,UNIX_TIMESTAMP(),_appid,_group_id);
	IF ROW_COUNT()=0 THEN
		SET _result = -2; # 账号插入失败
		LEAVE label_body;
	END IF;

	SET @_gift_appid=0;
	# 创建账号时传来了设备ID,则检查该设备是否使用过
	IF _device_id IS NOT NULL THEN
		IF NOT EXISTS (SELECT 1 FROM gifts WHERE device_id=_device_id AND appid=_appid) THEN
			INSERT INTO gifts (device_id,appid,account_id) VALUES (_device_id,_appid,_account_id);
			SET @_gift_appid=_appid;
		END IF;
	END IF;
END label_body;
	SELECT _result,_account_id,_name,@_gift_appid;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `account_login`
-- ----------------------------
DROP PROCEDURE IF EXISTS `account_login`;
DELIMITER ;;
CREATE PROCEDURE `account_login`(IN `_account` varchar(32),IN `_password` varchar(256),IN `_device_id` varchar(64),IN `_check_pass` int,IN `_device_type` int unsigned,IN `_group_id` int unsigned,OUT `_result` int)
BEGIN
	DECLARE EXIT		HANDLER FOR SQLEXCEPTION BEGIN SET _result=-100; SELECT _result; END;
	DECLARE EXIT		HANDLER FOR SQLSTATE '02000' BEGIN SET _result=-101; SELECT _result; END;

	SET _result = 0;
	SET @_account_id = 0;
	SET @_pass = '';
	SET @_binded = 0;
	SET @_device_id = '';
	SET @_gift_appid = 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_accounts WHERE name=_account AND (@_banned:=banned)>=0 AND (@_group_id:=group_id)>=0 AND (@_account_id:=account_id) AND LENGTH(@_pass:=pass)>=0 AND (@_binded:=binded)>=0 AND LENGTH(@_device_id:=IFNULL(device_id,''))>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	IF @_group_id!=_group_id THEN
		SET _result = -12;
		LEAVE label_body;
	END IF;

	IF @_banned=1 THEN
		SET _result = -10;
		LEAVE label_body;
	END IF;

	IF EXISTS (SELECT 1 FROM banned_device WHERE device_id=@_device_id) THEN
		SET _result = -11;
		LEAVE label_body;
	END IF;
	
	IF EXISTS (SELECT 1 FROM banned_device WHERE device_id=_device_id) THEN
		SET _result = -11;
		LEAVE label_body;
	END IF;

	IF _check_pass=1 THEN
		IF (@_pass != _password) THEN
			SET _result = -2;
			LEAVE label_body;
		END IF;
	END IF;
	UPDATE common_accounts SET last_login_time=UNIX_TIMESTAMP(),device_type=_device_type WHERE account_id=@_account_id;
	IF NOT EXISTS (SELECT 1 FROM gifts WHERE account_id=@_account_id AND (@_gift_appid:=appid)>=0) THEN
		SET @_gift_appid = 0;
	END IF;
END label_body;
	SELECT _result,@_account_id,@_binded,@_device_id,@_pass,@_gift_appid;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `account_logout`
-- ----------------------------
DROP PROCEDURE IF EXISTS `account_logout`;
DELIMITER ;;
CREATE PROCEDURE `account_logout`(IN `_account_id` bigint unsigned)
BEGIN
	UPDATE common_accounts SET last_logout_time=UNIX_TIMESTAMP() WHERE account_id=_account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_notification`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_notification`;
DELIMITER ;;
CREATE PROCEDURE `add_notification`(IN `_group_id` int unsigned,IN `_time` int unsigned,IN `_title` blob,IN `_content` blob,OUT `_result` int )
BEGIN
	DECLARE	_id	INT UNSIGNED DEFAULT 0;

	DECLARE EXIT				HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result = -2; SELECT _result; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result = -3; SELECT _result; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result = -4; SELECT _result; END;

	SET _result = -1;
	START TRANSACTION;

	IF NOT EXISTS(SELECT id from notification_id) THEN
		INSERT INTO notification_id VALUES(1);
	ELSE
		UPDATE notification_id SET id=id+1;
	END IF;
	
	SELECT id INTO _id FROM notification_id;

	INSERT INTO notification VALUES(_id,_group_id,_time,_title,_content);

	SET _result = 0;

	SELECT _result,_id,_group_id,_time;

	COMMIT;
END
;;
DELIMITER ;


-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 初始化DB --
drop procedure if exists `gc_init_db`;
DELIMITER ;;
create procedure gc_init_db(in bySvrGrpID tinyint unsigned, in byDBSID tinyint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_init_db: begin

if exists (select 1 from global_id where (svrgrp_id>>54)!=bySvrGrpID or (dbs_id>>49)!=byDBSID) then
	set nRst=-2;
	leave label_init_db;
end if;

-- 大区号   2^54=18014398509481984
-- 服务器号 2^49=562949953421312
if not exists (select 1 from global_id) then
	insert into global_id (svrgrp_id,dbs_id,asc_id) values (18014398509481984*bySvrGrpID,562949953421312*byDBSID,0);
end if;

set nRst=0;
end label_init_db;
select nRst;
end
;;
DELIMITER ;

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 生成全局ID --
drop function if exists `gc_global_id`;
DELIMITER ;;
create function gc_global_id() returns bigint
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vTmpID=0;
set @vID=0;
set @vIDIn1S=0;
update global_id set asc_id=(@vTmpID:=asc_id+1),last_gen_time=unix_timestamp();


-- 18 1~18   递增部分
-- 31 19~49  时间因子
-- 5  50~54  服务器号
-- 9  55~63  大区号
-- 1  64     保留

-- 时间因子  2^18=262144
select svrgrp_id+dbs_id+unix_timestamp()*262144+@vTmpID,asc_id_in1s into @vID,@vIDIn1S from global_id;
return @vID;
end
;;
DELIMITER ;

-- 分解全局ID --
drop procedure if exists `gc_unglobal_id`;
DELIMITER ;;
create procedure gc_unglobal_id(in nID bigint unsigned, in nEcho int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vSvrGrpID=nID<<1>>55;
set @vSvrID=nID<<10>>59;
set @vTime=nID<<15>>33;
set @vIncIdx=nID<<46>>46;

-- 是否需要回显
if (nEcho=1) then
	select @vSvrGrpID,@vSvrID,@vTime,@vIncIdx;
end if;

end
;;
DELIMITER ;

drop trigger if exists `gc_ascidin1s`;
DELIMITER ;;
create trigger gc_ascidin1s before update on global_id
for each row
begin

-- if (new.last_gen_time!=old.last_gen_time) then
-- if (to_days(from_unixtime(new.last_gen_time))!=to_days(from_unixtime(old.last_gen_time)) or 
--	hour(from_unixtime(new.last_gen_time))!=hour(from_unixtime(old.last_gen_time))) then
if (to_days(from_unixtime(new.last_gen_time))!=to_days(from_unixtime(old.last_gen_time)) or 
	hour(from_unixtime(new.last_gen_time))!=hour(from_unixtime(old.last_gen_time)) or 
	minute(from_unixtime(new.last_gen_time))!=minute(from_unixtime(old.last_gen_time))) then
	set new.asc_id_in1s=1;
else
	set new.asc_id_in1s=new.asc_id_in1s+1;
end if;

end;
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `account_login_with_device_id`
-- ----------------------------
DROP PROCEDURE IF EXISTS `account_login_with_device_id`;
DELIMITER ;;
CREATE PROCEDURE `account_login_with_device_id`(IN `_device_id` varchar(64),IN `_device_type` int unsigned,IN `_group_id` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_account_id = 0;
	SET @_binded = 0;
	SET @_device_id = '';
	SET @_pass = '';
	SET @_gift_appid = 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_accounts WHERE device_id=_device_id AND group_id=_group_id AND (@_banned:=banned)>=0 AND (@_account_id:=account_id)>0 AND (@_binded:=binded)>=0 AND LENGTH(@_device_id:=IFNULL(device_id,''))>=0 AND LENGTH(@_account_name:=name)>=0 AND LENGTH(@_pass:=pass)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	IF @_banned=1 THEN
		SET _result = -10;
		LEAVE label_body;
	END IF;

	IF EXISTS (SELECT 1 FROM banned_device WHERE device_id=@_device_id) THEN
		SET _result = -11;
		LEAVE label_body;
	END IF;

	UPDATE common_accounts SET last_login_time=UNIX_TIMESTAMP(),device_type=_device_type WHERE account_id=@_account_id;
	IF NOT EXISTS (SELECT 1 FROM gifts WHERE account_id=@_account_id AND (@_gift_appid:=appid)>=0) THEN
		SET @_gift_appid = 0;
	END IF;
END label_body;
	SELECT _result,@_account_id,@_binded,@_device_id,@_account_name,@_pass,@_gift_appid;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `register`
-- ----------------------------
DROP PROCEDURE IF EXISTS `register`;
DELIMITER ;;
CREATE PROCEDURE `register`(IN `_account_id` bigint unsigned,IN `_name` varchar(32),IN `_pass` varchar(256),IN `_old_pass` varchar(256))
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _is_register INT DEFAULT 0;
	
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_accounts WHERE account_id=_account_id AND (@_binded:=binded)>=0 AND LENGTH(@_old_pass:=pass)>=0) THEN
		SET _result = -1;
	END IF;

	IF @_binded=1 THEN
		# 修改密码
		IF @_old_pass=_old_pass THEN
			UPDATE common_accounts SET pass=_pass WHERE account_id=_account_id;
		ELSE
			SET _result = -10; # 老密码不对
			LEAVE label_body;
		END IF;
	ELSE
		# 注册
		IF EXISTS (SELECT 1 FROM common_accounts WHERE name=_name) THEN
			SET _result = -20; # 该名字已经被注册
			LEAVE label_body;
		END IF;
		UPDATE common_accounts SET name=_name,pass=_pass,binded=1 WHERE account_id=_account_id;
		IF ROW_COUNT()=0 THEN
			SET _result = -21; # 注册失败
			LEAVE label_body;
		END IF;
		SET _is_register = 1;
	END IF;
END label_body;
	SELECT _result,_is_register;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `bind_device`
-- ----------------------------
DROP PROCEDURE IF EXISTS `bind_device`;
DELIMITER ;;
CREATE PROCEDURE `bind_device`(IN `_account_id` bigint unsigned,IN `_device_id` varchar(64),IN `_group_id` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	IF EXISTS (SELECT 1 FROM common_accounts WHERE account_id=_account_id AND device_id IS NOT NULL and LENGTH(@_device_id:=device_id)>=0) THEN
		# 已经绑定设备的
		IF @_device_id=_device_id THEN
			# 绑定的是同一个设备
			LEAVE label_body;
		END IF;
	END IF;

	IF EXISTS (SELECT 1 FROM common_accounts WHERE device_id=_device_id AND group_id=_group_id) THEN
		# 已经存在在该大区绑定了该设备的账号
		#UPDATE common_accounts SET device_id=NULL WHERE device_id=_device_id AND group_id=_group_id;
		SET _result = -2;
		LEAVE label_body;
	END IF;
	UPDATE common_accounts SET device_id=_device_id WHERE account_id=_account_id;

	IF ROW_COUNT()=0 THEN
		SET _result = -1; # 绑定失败
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `ban_account`
-- ----------------------------
DROP PROCEDURE IF EXISTS `ban_account`;
DELIMITER ;;
CREATE PROCEDURE `ban_account`(IN `_account_id` bigint unsigned,IN `_ban_device` int unsigned)
BEGIN
	SET @_result = 0;
	SET @_device_id = '';
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_accounts WHERE account_id=_account_id) THEN
		SET @_result = -1;
		LEAVE label_body;
	END IF;
	UPDATE common_accounts SET banned=1 WHERE account_id=_account_id;

	IF _ban_device=1 THEN
		SELECT device_id INTO @_device_id FROM common_accounts WHERE account_id=_account_id;
		IF @_device_id IS NOT NULL THEN
			INSERT IGNORE INTO banned_device VALUES (@_device_id);
		END IF;
	END IF;
END label_body;
	SELECT @_result,IFNULL(@_device_id,'');
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_adcolony`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_adcolony`;
DELIMITER ;;
CREATE PROCEDURE `deal_adcolony`(IN `_transaction_id` bigint unsigned)
BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN SET @_result=-100; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN SET @_result=-101; END;

	IF EXISTS (SELECT 1 FROM adcolony_transactions WHERE transaction_id=_transaction_id) THEN
		SELECT time,name,account_id,amount INTO @_time,@_name,@_account_id,@_amount FROM adcolony_transactions WHERE transaction_id=_transaction_id;
		DELETE FROM adcolony_transactions WHERE transaction_id=_transaction_id;
		INSERT INTO adcolony_logs (transaction_id,time,name,account_id,amount) VALUES (_transaction_id,@_time,@_name,@_account_id,@_amount);
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_adcolony_transaction`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_adcolony_transaction`;
DELIMITER ;;
CREATE PROCEDURE `get_adcolony_transaction`()
BEGIN
	SELECT t.transaction_id,t.time,t.name,t.account_id,t.amount,a.group_id FROM adcolony_transactions t JOIN common_accounts a ON t.account_id=a.account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `global_id`
-- ----------------------------
DROP FUNCTION IF EXISTS `global_id`;
DELIMITER ;;
CREATE FUNCTION `global_id`() RETURNS bigint(20) unsigned
BEGIN
	DECLARE _id BIGINT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN END;

	SET @_tmp_asc_id=0;
	SET @_tmp_asc_id_in1s=0;
	UPDATE global_id SET asc_id=(@_tmp_asc_id:=IF(asc_id>=1048575,0,asc_id+1))
						,asc_id_in1s=(@_tmp_asc_id_in1s:=IF(last_gen_time!=UNIX_TIMESTAMP(),1,asc_id_in1s+1))
						,last_gen_time=UNIX_TIMESTAMP();

	# 20 1~20   递增部分
	# 31 21~51  时间因子
	# 12 52~63  大区号
	# 1  64     保留	2^63=9223372036854775808

	# 时间因子  2^20=1048576
	SELECT 9223372036854775808+svrgrp_id+UNIX_TIMESTAMP()*1048576+@_tmp_asc_id INTO _id FROM global_id;
	RETURN _id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `unglobal_id`
-- ----------------------------
DROP PROCEDURE IF EXISTS `unglobal_id`;
DELIMITER ;;
CREATE PROCEDURE `unglobal_id`(IN `_id` bigint unsigned,IN `_echo` int unsigned,OUT `_group_id` int unsigned,OUT `_time` int unsigned,OUT `_asc_id` int unsigned)
BEGIN
	DECLARE EXIT            HANDLER FOR SQLEXCEPTION        BEGIN END;
	DECLARE EXIT            HANDLER FOR SQLSTATE '22003'    BEGIN END;

	SET _group_id=_id<<1>>52;
	SET _time=_id<<13>>33;
	SET _asc_id=_id<<44>>44;

	IF(_echo=1) THEN
		SELECT _group_id,_time,_asc_id;
	END IF;
END
;;
DELIMITER ;
