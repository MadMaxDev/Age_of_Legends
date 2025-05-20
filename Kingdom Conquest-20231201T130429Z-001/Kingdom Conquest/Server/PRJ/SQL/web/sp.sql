SET GLOBAL log_bin_trust_function_creators=ON;
-- ----------------------------
-- Procedure structure for `accept_alliance_member`
-- ----------------------------
DROP PROCEDURE IF EXISTS `accept_alliance_member`;
DELIMITER ;;
CREATE PROCEDURE `accept_alliance_member`(IN `_account_id` bigint unsigned,IN `_alliance_id` bigint unsigned,IN `_new_member_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _member_num INT UNSIGNED DEFAULT 0;
	DECLARE _development_cost INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_development_cost; END;

	
label_body:BEGIN
	# 是否存在该事件
	IF NOT EXISTS (SELECT 1 FROM alliance_join_events WHERE account_id=_new_member_id AND alliance_id=_alliance_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 是否有权限(是否是该盟的管理者)
	IF NOT EXISTS (SELECT 1 FROM alliance_members m JOIN excel_alliance_position_right e ON m.position=e.excel_id WHERE m.account_id=_account_id AND m.alliance_id=_alliance_id AND e.approve_member=1) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 成员上限是否够
	SELECT count(*) INTO _member_num FROM alliance_members WHERE alliance_id=_alliance_id;
	# 是否还有空余位置
	IF NOT EXISTS (SELECT 1 FROM alliance_buildings b JOIN excel_alliance_congress c ON b.level=c.level WHERE alliance_id=_alliance_id AND excel_id=1 AND c.member_num>_member_num) THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	
	START TRANSACTION;
	# 设置联盟ID
	UPDATE common_characters SET alliance_id=_alliance_id WHERE account_id=_new_member_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 插入联盟表
	INSERT INTO alliance_members VALUES (_alliance_id, _new_member_id, 4, 0, 0); # 刚进入都是普通成员
	IF ROW_COUNT()=0 THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;
	DELETE FROM alliance_join_events WHERE account_id=_new_member_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -6;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 增加联盟的发展度
	SELECT development INTO _development_cost FROM excel_alliance_development WHERE action_id=1;	#action_id=1为加入联盟
	UPDATE alliances SET development=development+_development_cost,total_development=total_development+_development_cost WHERE alliance_id=_alliance_id;
	
	-- 修改当前奖杯排行信息
	call game_update_cup(_new_member_id,@vRst);
END label_body;
	SELECT _result,_development_cost;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `account_login`
-- ----------------------------
DROP PROCEDURE IF EXISTS `account_login`;
DELIMITER ;;
CREATE PROCEDURE `account_login`(IN `_account_id` bigint unsigned,IN `_IP` int unsigned,IN `_binded` tinyint unsigned,OUT `_result` int)
BEGIN
	DECLARE _time INT UNSIGNED DEFAULT 0;
	DECLARE _name VARCHAR(32);
	DECLARE _population_production INT UNSIGNED DEFAULT 0;
	DECLARE _population_capacity INT UNSIGNED DEFAULT 0;
	DECLARE _population INT UNSIGNED DEFAULT 0;
	DECLARE _population_event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _last_logout_time INT UNSIGNED DEFAULT 0;
	DECLARE _produce_time INT UNSIGNED DEFAULT 0;
	
	# 目前为联盟跑商,抽奖,联盟抽奖
	DECLARE _lottery_date DATE DEFAULT '0000-00-00';
	DECLARE _lottery_num INT UNSIGNED DEFAULT 0;
	DECLARE _trade_date DATE DEFAULT '0000-00-00';
	DECLARE _trade_num INT UNSIGNED DEFAULT 0;
	DECLARE _alliance_lottery_date DATE DEFAULT '0000-00-00';
	DECLARE _alliance_lottery_num INT UNSIGNED DEFAULT 0;
	DECLARE _cur_date DATE DEFAULT '0000-00-00';

	DECLARE EXIT		HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result; END;
	DECLARE EXIT		HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result; END;
	DECLARE EXIT		HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result; END;
	DECLARE EXIT		HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result; END;

	SET _result	= 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND (@_banned:=banned)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	IF @_banned=1 THEN
		SET _result = -10;
		LEAVE label_body;
	END IF;

	SELECT name,population,last_logout_time,draw_lottery_date,draw_lottery_num,trade_date,trade_num,alliance_lottery_date,alliance_lottery_num 
	INTO _name,_population,_last_logout_time,_lottery_date,_lottery_num,_trade_date,_trade_num,_alliance_lottery_date,_alliance_lottery_num 
	FROM common_characters WHERE account_id=_account_id;

	START TRANSACTION;
	SET _time		= UNIX_TIMESTAMP();	
	IF EXISTS (SELECT account_id FROM online_characters WHERE account_id=_account_id) THEN
		UPDATE online_characters SET name=_name,login_time=_time WHERE account_id=_account_id;
	ELSE
		INSERT INTO online_characters VALUES(_account_id, _name, _time);
	END IF;
	
	IF NOT EXISTS (SELECT event_id FROM once_perperson_time_events WHERE account_id=_account_id AND type=2) THEN
		SELECT produce_time INTO _produce_time FROM excel_house_production LIMIT 1;
		CALL get_population_production_n_capacity(_account_id, _result, _population_production, _population_capacity);
		SET _population_production = (_time-_last_logout_time)/_produce_time*_population_production;
		IF _population+_population_production > _population_capacity THEN
			IF _population_capacity <= _population THEN
				SET _population_production = 0;
			ELSE 
				SET _population_production = _population_capacity-_population;
			END IF;
		END IF;
	END IF;
	
	SET _cur_date = CURDATE();
	# 抽奖
	IF _lottery_date!=_cur_date THEN
		SELECT free_draw_num INTO _lottery_num FROM excel_lottery_cfg;
	END IF;
	# 联盟跑商
	IF _trade_date!=_cur_date THEN
		IF EXISTS (SELECT 1 FROM buildings WHERE account_id=_account_id AND excel_id=5 AND (@_level:=level)) THEN
			SELECT trade_num INTO _trade_num FROM excel_embassy_cfg WHERE level<=@_level ORDER BY level DESC LIMIT 1;
		END IF;
	END IF;
	# 联盟抽奖
	IF _alliance_lottery_date!=_cur_date THEN
		SELECT alliance_lottery_num INTO _alliance_lottery_num FROM excel_lottery_cfg LIMIT 1;
	END IF;
	
	UPDATE common_characters SET last_login_time=_time,last_logout_time=_time,population=population+_population_production,ip=_IP,binded=_binded, 
	draw_lottery_date=_cur_date,draw_lottery_num=_lottery_num,trade_date=_cur_date,trade_num=_trade_num,alliance_lottery_date=_cur_date,alliance_lottery_num=_alliance_lottery_num 
	WHERE account_id=_account_id;
	
	# 删除老邮件
	SET @_delete_time=_time-3*24*3600; # 删除三天之前的邮件
	DELETE FROM private_mails WHERE account_id=_account_id AND time<@_delete_time AND type NOT IN (1,2) AND flag!=3;
	
	# 修改联盟副本次数
	UPDATE alli_instance_player SET day_times_free=0,last_time=UNIX_TIMESTAMP() WHERE account_id=_account_id AND DATE(FROM_UNIXTIME(last_time))!=DATE(NOW());
END label_body;
	SELECT _result;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `ban_char`
-- ----------------------------
DROP PROCEDURE IF EXISTS `ban_char`;
DELIMITER ;;
CREATE PROCEDURE `ban_char`(IN `_account_id` bigint unsigned)
BEGIN
	SET @_result = 0;
	UPDATE common_characters SET banned=1 WHERE account_id=_account_id;
	IF ROW_COUNT()=0 THEN
		SET @_result = -1;
	END IF;
	SELECT @_result;
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
	DECLARE _time INT UNSIGNED DEFAULT 0;
	DECLARE _population_production INT UNSIGNED DEFAULT 0;
	DECLARE _population_capacity INT UNSIGNED DEFAULT 0;
	DECLARE _population INT UNSIGNED DEFAULT 0;
	DECLARE _population_event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _result INT DEFAULT 0;

	SET _time=UNIX_TIMESTAMP();

	# 当前人口,登陆时间
	SELECT population,last_login_time INTO _population,@_last_login_time FROM common_characters WHERE account_id=_account_id;
		
	# 下线的时候把人口生产事件删除
	IF EXISTS (SELECT event_id FROM once_perperson_time_events WHERE account_id=_account_id AND type=2) THEN
		# 事件ID
		SELECT event_id INTO _population_event_id FROM once_perperson_time_events WHERE account_id=_account_id AND type=2;
		# 获取当前时间事件可以生产的人口
		CALL get_population_production_n_capacity(_account_id, _result, _population_production, _population_capacity);
		SELECT (_time-begin_time)/(end_time-begin_time)*_population_production INTO _population_production FROM time_events WHERE event_id=_population_event_id;
		IF _population+_population_production > _population_capacity THEN
			IF _population >= _population_capacity THEN
				SET _population_production	= 0;
			ELSE
				SET _population_production	= _population_capacity-_population;
			END IF;
		END IF;
		DELETE FROM time_events WHERE event_id=_population_event_id;
		DELETE FROM once_perperson_time_events WHERE event_id=_population_event_id;
	END IF;

	SET @_new_online_time	= _time-@_last_login_time;
	UPDATE common_characters SET last_logout_time=_time,population=population+_population_production
	,total_online_time=total_online_time+@_new_online_time
	WHERE account_id=_account_id;
	DELETE FROM online_characters WHERE account_id=_account_id;
	CALL add_online_time(_time, @_new_online_time);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_online_time`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_online_time`;
DELIMITER ;;
CREATE PROCEDURE `add_online_time`(IN `_time_now` int unsigned,IN `_online_time` int unsigned)
BEGIN
	SET @_date	= DATE(FROM_UNIXTIME(_time_now));
	IF NOT EXISTS (SELECT 1 FROM char_online_stat WHERE stat_date=@_date) THEN
		INSERT INTO char_online_stat VALUES (@_date, 0, 0);
	END IF;

	UPDATE char_online_stat SET online_time=online_time+_online_time WHERE stat_date=@_date;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_once_perperson_time_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_once_perperson_time_event`;
DELIMITER ;;
CREATE PROCEDURE `add_once_perperson_time_event`(IN `_account_id` bigint unsigned,IN `_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _time_interval INT UNSIGNED DEFAULT 0;
	DECLARE _time_now INT UNSIGNED DEFAULT 0;
	DECLARE _char_level BIGINT UNSIGNED DEFAULT 0;
	
	DECLARE EXIT				HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_account_id,_event_id,_type; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_account_id,_event_id,_type; END;
	
label_body:BEGIN

	# 看看是否已经存在该类型的事件了
	IF EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=_account_id AND type=_type)
	THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	# 获取时间事件时间周期
	CASE
#		# 许愿树
#		WHEN _type=1 THEN
#			# 是否有许愿树
#			IF NOT EXISTS (SELECT 1 FROM buildings WHERE account_id=_account_id AND excel_id=12) THEN
#				SET _result = -10; # 圣诞树不存在
#				LEAVE label_body;
#			END IF;
#			IF EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND type=2) THEN
#				SET _result = -11; # 圣诞树成熟需要先摘取
#				LEAVE label_body;
#			END IF;
#			# 君主等级
#			SELECT level INTO _char_level FROM common_characters WHERE account_id=_account_id;
#			# 成熟时间
#			SELECT time INTO _time_interval FROM excel_christmas_tree WHERE char_level<=_char_level ORDER BY char_level DESC LIMIT 1;
		# 居民生产
		WHEN _type=2 THEN
			# produce_time以秒为基本单位
			SELECT produce_time INTO _time_interval FROM excel_house_production LIMIT 1;
		ELSE
			SET _result = -2;
			LEAVE label_body;
	END CASE;

	# 插入事件事件
	START TRANSACTION;
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	INSERT INTO time_events VALUES(_event_id, _time_now, _time_now+_time_interval, 2, 0);
	IF ROW_COUNT() = 0
	THEN
		SET _result = -3;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO once_perperson_time_events VALUES (_event_id, _account_id, _type);
	IF ROW_COUNT() = 0
	THEN
		SET _result = -4;
		ROLLBACK;
		LEAVE label_body;
	END IF;

END label_body;

	SELECT _result,_account_id,_event_id,_type;
	COMMIT;

END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_production_time_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_production_time_event`;
DELIMITER ;;
CREATE PROCEDURE `add_production_time_event`(IN `_account_id` bigint unsigned,IN `_auto_id` int unsigned,IN `_production_choice` int unsigned,IN `_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _production INT UNSIGNED DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _time_now INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_production,_event_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_production,_event_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_production,_event_id; END;

	# 这些不能通过declare,动态sql使用@
	SET @_base_production = 0;
	SET @_upgrade_rate = 0;
	SET @_produce_time = 0;
	SET @_produce_rate = 0;

label_body:BEGIN
	# 检测是否已经事件是否存在
	IF EXISTS (SELECT 1 FROM production_time_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=_type) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 检测成熟事件是否已经存在
	IF EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=_type) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 根据选项获取产量及生产时间(type:1,黄金生产)
	CASE
		WHEN _type=1 THEN
			# 判断是否有金矿
			IF NOT EXISTS (SELECT level FROM buildings WHERE account_id=_account_id AND auto_id=_auto_id AND excel_id=11) THEN
				SET _result = -3;
				LEAVE label_body;
			END IF;
			# 获取等级
			SELECT level INTO _level FROM buildings WHERE account_id=_account_id AND auto_id=_auto_id;
			IF _production_choice > 3 THEN
				SET _result = -4;
				LEAVE label_body;
			END IF;
			# 动态sql
			SET @query = CONCAT('SELECT produce_time_', _production_choice, ',production_', _production_choice, ' INTO @_produce_time,@_production FROM excel_goldore_production WHERE level=', _level);
			PREPARE smt FROM @query;
			EXECUTE smt;
			DEALLOCATE PREPARE smt;

			SET _production = @_production;
		ELSE
			SET _result = -5;
			LEAVE label_body;
	END CASE;
	
	START TRANSACTION;
	# 插入时间事件
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	INSERT INTO time_events VALUES (_event_id, _time_now, _time_now+@_produce_time, 3, 0);
	IF ROW_COUNT() = 0 THEN
		SET _result = -6;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO production_time_events VALUES (_event_id, _account_id, _auto_id, _production, _type);
	IF ROW_COUNT() = 0 THEN
		SET _result = -7;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	
	SELECT _result,_production,_event_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `build_alliance_building`
-- ----------------------------
DROP PROCEDURE IF EXISTS `build_alliance_building`;
DELIMITER ;;
CREATE PROCEDURE `build_alliance_building`(IN `_account_id` bigint unsigned,IN `_excel_id` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _time_now INT UNSIGNED DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _position INT UNSIGNED DEFAULT 0;
	DECLARE _total_development INT UNSIGNED DEFAULT 0;
	DECLARE _development_cost INT UNSIGNED DEFAULT 0;
	DECLARE _time_cost INT UNSIGNED DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;
	DECLARE _max_level INT UNSIGNED DEFAULT 0;
	DECLARE _cur_max_level INT UNSIGNED DEFAULT 0;
	DECLARE _type INT UNSIGNED DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_development_cost,_time_cost,_alliance_id,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_development_cost,_time_cost,_alliance_id,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_development_cost,_time_cost,_alliance_id,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_development_cost,_time_cost,_alliance_id,_type; END;

label_body:BEGIN
	# 是否有联盟
	IF NOT EXISTS (SELECT 1 FROM alliance_members WHERE account_id=_account_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 获取联盟ID和联盟职位
	SELECT alliance_id,position INTO _alliance_id,_position FROM alliance_members WHERE account_id=_account_id;
	# 是否有建设权限
	IF (SELECT build FROM excel_alliance_position_right WHERE excel_id=_position) = 0 THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 是否已经存在该事件了
	IF (SELECT 1 FROM alliance_building_time_events WHERE alliance_id=_alliance_id AND excel_id=_excel_id) THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	# 该建筑当前等级
	IF EXISTS (SELECT 1 FROM alliance_buildings WHERE alliance_id=_alliance_id AND excel_id=_excel_id) THEN
		SELECT level INTO _level FROM alliance_buildings WHERE alliance_id=_alliance_id AND excel_id=_excel_id;
		SET _type = 2; # 升级
	ELSE
		SET _level = 0;
		SET _type = 1; # 建造
	END IF;
	# 联盟当前总发展度
	SELECT total_development INTO _total_development FROM alliances WHERE alliance_id=_alliance_id;
	# 当前等级上限
	CASE
	WHEN _excel_id=1 THEN
		SELECT level INTO _cur_max_level FROM excel_alliance_congress WHERE development <= _total_development ORDER BY development DESC LIMIT 1;
	ELSE
		IF EXISTS (SELECT 1 FROM alliance_buildings WHERE alliance_id=_alliance_id AND excel_id=1) THEN
			SELECT level INTO _cur_max_level FROM alliance_buildings WHERE alliance_id=_alliance_id AND excel_id=1;
		ELSE
			SET _result = -4;	# 议会不存在
			LEAVE label_body;
		END IF;
	END CASE;
	SELECT max(level) INTO _max_level FROM excel_alliance_building_list WHERE excel_id=_excel_id;
	IF _max_level<_cur_max_level THEN
		SET _cur_max_level=_max_level;
	END IF;
	IF _level >= _cur_max_level THEN
		SET _result = -5;	# 当前等级已经到达上限
		LEAVE label_body;
	END IF;
	# 时间消耗、发展度消耗
	SELECT build_time,build_development INTO _time_cost,_development_cost FROM excel_alliance_building_list WHERE excel_id=_excel_id AND level=_level+1;
	
	START TRANSACTION;
	# 扣除发展度
	UPDATE alliances SET development=development-_development_cost WHERE alliance_id=_alliance_id AND development>=_development_cost;
	IF ROW_COUNT()=0 THEN
		SET _result = -6;	# 发展度不够
		LEAVE label_body;
	END IF;
	# 插入时间事件
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	INSERT INTO time_events VALUES (_event_id, _time_now, _time_now+_time_cost, 5, 0);	# 5为联盟建筑事件
	IF ROW_COUNT()=0 THEN
		SET _result = -7;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO alliance_building_time_events VALUES (_event_id, _alliance_id, _excel_id, _type);
	IF ROW_COUNT()=0 THEN
		SET _result = -8;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_development_cost,_time_cost,_alliance_id,_type;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `build_building`
-- ----------------------------
DROP PROCEDURE IF EXISTS `build_building`;
DELIMITER ;;
CREATE PROCEDURE `build_building`(IN `_account_id` bigint unsigned,IN `_excel_id` int unsigned,IN `_auto_id` int unsigned)
BEGIN
	DECLARE _result INT	DEFAULT 0;
	DECLARE _cur_num INT UNSIGNED DEFAULT 0;
	DECLARE _cur_build_num INT UNSIGNED DEFAULT 0;
	DECLARE _max_num INT UNSIGNED DEFAULT 0;
	DECLARE _time_now	INT UNSIGNED DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	
	# 对应excel_building_num_limit表中的记录
	DECLARE _cur_char_level INT UNSIGNED DEFAULT 0;
	DECLARE _cur_char_exp INT UNSIGNED DEFAULT 0;
	DECLARE _cur_max_num INT UNSIGNED DEFAULT 0;
	
	# 建造数据
	DECLARE _build_time INT UNSIGNED DEFAULT 0;
	DECLARE _build_gold INT UNSIGNED DEFAULT 0;
	DECLARE _build_population INT UNSIGNED DEFAULT 0;
	DECLARE _exp INT UNSIGNED DEFAULT 0;
	DECLARE _development INT UNSIGNED DEFAULT 0;
	
	# 一些角色数据
	DECLARE _char_level	INT	UNSIGNED DEFAULT 0;
	DECLARE _char_gold INT UNSIGNED DEFAULT 0;
	DECLARE _char_population INT UNSIGNED DEFAULT 0;
	DECLARE _char_build_num INT UNSIGNED DEFAULT 0;
	DECLARE _char_added_build_num INT UNSIGNED DEFAULT 0;
	# 声明异常处理
	DECLARE EXIT				HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_exp,_development,_cur_char_exp,_cur_char_level,_build_gold,_build_population; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_exp,_development,_cur_char_exp,_cur_char_level,_build_gold,_build_population; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_exp,_development,_cur_char_exp,_cur_char_level,_build_gold,_build_population; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_exp,_development,_cur_char_exp,_cur_char_level,_build_gold,_build_population; END;
	
label_body : BEGIN
	
	# 该auto_id已经被占据了
	IF EXISTS (SELECT auto_id FROM buildings WHERE account_id=_account_id AND auto_id=_auto_id) THEN
		SET	_result	= -1;
		LEAVE label_body;
	END IF;
	IF EXISTS (SELECT auto_id FROM building_time_events WHERE account_id=_account_id AND auto_id=_auto_id) THEN
		SET	_result	= -1; # 这个位置有建造
		LEAVE label_body;
	END IF;
	# 获取该建筑的当前数量
	SELECT count(*) INTO _cur_num FROM buildings WHERE account_id=_account_id AND excel_id=_excel_id;
	# 建筑的当前数量应该包括在建的数量
	SELECT _cur_num+count(*) INTO _cur_num FROM building_time_events WHERE account_id=_account_id AND excel_id=_excel_id AND type=1;
	
	# 获取君主等级,君主等级会限制建造数量上限
	SELECT level,gold,population,build_num,added_build_num,vip INTO _char_level,_char_gold,_char_population,_char_build_num,_char_added_build_num,@_vip_level FROM common_characters WHERE account_id=_account_id;
	# 获取该建筑的最大数量,建造时间,建造金钱,建造人口
	SELECT max_num,build_time,build_gold,build_population,exp,development INTO _max_num,_build_time,_build_gold,_build_population,_exp,_development FROM excel_building_list WHERE excel_id=_excel_id AND level=1;
	# 判断auto_id是否合法
	IF NOT EXISTS (SELECT excel_id FROM excel_building_list WHERE excel_id=_excel_id AND level=1 AND _auto_id>=auto_id_begin AND _auto_id<=auto_id_end) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 判断金钱是否足够
	IF _char_gold < _build_gold THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	# 判断人口是否足够
	IF _char_population < _build_population THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
	# 判断是否有足够的建造队列
	SELECT added_build_num INTO @_vip_added_build_num FROM excel_vip_right WHERE level=@_vip_level;
	SELECT count(*) INTO _cur_build_num FROM building_time_events WHERE account_id=_account_id AND type IN (1, 2);
	SET @_item_added_build_num = 0;
	IF EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=_account_id AND type=3) THEN
		SELECT val_int INTO @_item_added_build_num FROM excel_item_list WHERE excel_id=2012;
	END IF;
	IF _cur_build_num >= (_char_build_num + _char_added_build_num + @_vip_added_build_num + @_item_added_build_num) THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;
	# 民居、军营、金矿需要excel_building_num_limit来进一步看数量上限
	IF _excel_id IN (9, 10, 11) THEN
		SELECT max(char_level) INTO _cur_char_level FROM excel_building_num_limit WHERE char_level<=_char_level;
		CASE
			WHEN _excel_id=9 THEN
				SELECT house_max_num INTO _cur_max_num FROM excel_building_num_limit WHERE char_level=_cur_char_level;
			WHEN _excel_id=10 THEN
				SELECT barrack_max_num INTO _cur_max_num FROM excel_building_num_limit WHERE char_level=_cur_char_level;
			WHEN _excel_id=11 THEN
				SELECT goldore_max_num INTO _cur_max_num FROM excel_building_num_limit WHERE char_level=_cur_char_level;
		END CASE;
		IF _cur_max_num < _max_num THEN
			SET _max_num = _cur_max_num;
		END IF;
	END IF;
	# 判断建筑数量是否达到上限
	IF _cur_num >= _max_num THEN
		SET _result = -6;
		LEAVE label_body;
	END IF;
	START TRANSACTION;

	# 生成建造时间事件(这里也可能出错,比如已经有了该建筑对应的时间事件了)
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	INSERT INTO time_events VALUES (_event_id, _time_now, _time_now+_build_time, 1, 0);	#事件类型(1:建筑)
	IF ROW_COUNT() = 0
	THEN
		SET _result = -7;
		LEAVE label_body;
	END IF;
	INSERT INTO building_time_events VALUES (_event_id, _account_id, _auto_id, _excel_id, 1); #建筑事件类型(1:建造,2:升级,3:生产)
	IF ROW_COUNT() = 0
	THEN
		ROLLBACK;
		SET _result = -8;
		LEAVE label_body;
	END IF;

	# 金矿需要附加数据
	IF _excel_id=11 THEN
		INSERT INTO buildings_ext_data (account_id,auto_id,excel_id) VALUES (_account_id,_auto_id,_excel_id);
	END IF;

	# 扣除金钱和人口
	UPDATE common_characters SET gold=gold-_build_gold,population=population-_build_population,development=development+_development WHERE account_id=_account_id AND gold>=_build_gold AND population>=_build_population;
	IF ROW_COUNT() = 0
	THEN
		ROLLBACK;
		SET _result = -9;
		LEAVE label_body;
	END IF;
	CALL add_char_exp(_account_id, _exp, _cur_char_exp, _cur_char_level);
END label_body;
	SELECT _result,_exp,_development,_cur_char_exp,_cur_char_level,_build_gold,_build_population;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `cancel_join_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `cancel_join_alliance`;
DELIMITER ;;
CREATE PROCEDURE `cancel_join_alliance`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN SELECT _result,_alliance_id; END;

	SELECT alliance_id INTO _alliance_id FROM alliance_join_events WHERE account_id=_account_id;
	DELETE FROM alliance_join_events WHERE account_id=_account_id;
	SELECT _result,_alliance_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `create_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `create_alliance`;
DELIMITER ;;
CREATE PROCEDURE `create_alliance`(IN `_account_id` bigint unsigned,IN `_alliance_name` varchar(32))
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_gold,_alliance_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_gold,_alliance_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_gold,_alliance_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_gold,_alliance_id; END;

label_body:BEGIN
	# 是否已经有联盟了
	IF EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND alliance_id<>0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	# 联盟是否重名
	IF EXISTS (SELECT 1 FROM alliances WHERE name=_alliance_name) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;

	# 联盟创建所需要的金钱额度
	SELECT gold INTO _gold FROM excel_alliance_create_cost;

	START TRANSACTION;
	# 扣除黄金
	UPDATE common_characters SET gold=gold-_gold WHERE account_id=_account_id AND gold>=_gold;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;	# 黄金不够
		LEAVE label_body;
	END IF;
	# 创建联盟
	SET _alliance_id = game_global_id();
	INSERT INTO alliances (alliance_id,account_id,name,development,total_development,introduction) VALUES (_alliance_id,_account_id,_alliance_name,0,0,'');
	IF ROW_COUNT()=0 THEN
		SET _result = -4;	# 生成失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 成员表中将创建者设置为创始人
	INSERT INTO alliance_members (alliance_id,account_id,position,development,total_development) VALUES (_alliance_id,_account_id,1,0,0);
	IF ROW_COUNT()=0 THEN
		SET _result = -5;	# 设置权限出错
		ROLLBACK;
		LEAVE label_body;
	END IF;
	UPDATE common_characters SET alliance_id=_alliance_id WHERE account_id=_account_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -6;	# 设置联盟ID
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 插入议会建筑
	INSERT INTO alliance_buildings VALUES (_alliance_id,1,1);

	# 联盟邮件ID
	INSERT INTO alliance_mail_id VALUES (_alliance_id,0);
	# 联盟日志ID
	INSERT INTO alliance_log_id VALUES (_alliance_id,0);
	
	-- 修改当前奖杯排行信息
	call game_update_cup(_account_id,@vRst);
END label_body;
	SELECT _result,_gold,_alliance_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `create_char`
-- ----------------------------
DROP PROCEDURE IF EXISTS `create_char`;
DELIMITER ;;
CREATE PROCEDURE `create_char`(IN `_account_id` bigint unsigned,IN `_name` varchar(32),IN `_sex` int unsigned,IN `_head_id` int unsigned,IN `_pos_x` int unsigned,IN `_pos_y` int unsigned,IN `_binded` tinyint unsigned,IN `_gift_appid` int unsigned)
BEGIN
	DECLARE	_result	INT DEFAULT 0;
	DECLARE _build_num INT UNSIGNED DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _crystal INT UNSIGNED DEFAULT 0;
	DECLARE _diamond INT UNSIGNED DEFAULT 0;
	DECLARE _population INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;
	DECLARE _excel_id INT UNSIGNED DEFAULT 0;
	DECLARE _auto_id INT UNSIGNED DEFAULT 0;
	DECLARE _excel_tech_id INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT excel_id,auto_id_begin FROM excel_building_list WHERE auto_build=1 AND level=1;
	DECLARE _cursor_tech CURSOR FOR SELECT excel_id FROM excel_soldier WHERE level=1;

	DECLARE EXIT				HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result = -100; SELECT _result; END;
	DECLARE CONTINUE		HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	DECLARE EXIT			 	HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result; END;
	DECLARE EXIT 				HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result; END;
	
label_body:BEGIN
	START TRANSACTION;
	SET @_time_now=UNIX_TIMESTAMP();
	IF EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id) THEN
		SET _result = -1;
		LEAVE label_body;
	ELSE
		IF EXISTS (SELECT 1 FROM common_characters WHERE name=_name) THEN
			SET _result = -2;
			LEAVE label_body;
		ELSE
			# 是否这个位置有角色了,这个是严重错误
			IF EXISTS (SELECT 1 FROM common_characters WHERE pos_x=_pos_x AND pos_y=_pos_y) THEN
				SET _result = -8;
				LEAVE label_body;
			END IF;
			# 根据模板创建角色
			SELECT build_num,gold,crystal,diamond,population INTO _build_num,_gold,_crystal,_diamond,_population FROM excel_character_template;
			select val_int into @vProtectTime from excel_cmndef where id=3;
			# 是否需要赠送宝石
			SET @_gift_diamond=0;
			IF EXISTS (SELECT 1 FROM excel_app_gift WHERE appid=_gift_appid AND (@_gift_end_date:=UNIX_TIMESTAMP(end_date)) AND (@_diamond:=diamond)) THEN
				IF @_time_now<=@_gift_end_date THEN
					SET @_gift_diamond=@_diamond;
				END IF;
			END IF;

			# 抽奖，联盟抽奖，联盟跑商
			SELECT free_draw_num INTO @_lottery_num FROM excel_lottery_cfg;
			SELECT alliance_lottery_num INTO @_alliance_lottery_num FROM excel_lottery_cfg LIMIT 1;
			SELECT trade_num INTO @_trade_num FROM excel_embassy_cfg WHERE level<=1 ORDER BY level DESC LIMIT 1;
			SET @_cur_date = CURDATE();
			
			INSERT INTO common_characters (account_id,name,sex,head_id,pos_x,pos_y,
					last_login_time,last_logout_time,level,build_num,gold,crystal,diamond,population,protect_time,
					draw_lottery_date,draw_lottery_num,trade_date,trade_num,alliance_lottery_date,alliance_lottery_num,
					create_time,binded,drug) 
					VALUES (_account_id,_name,_sex,_head_id,_pos_x,_pos_y,
					@_time_now,@_time_now,1,_build_num,_gold,_crystal,_diamond+@_gift_diamond,_population,@_time_now+@vProtectTime,
					@_cur_date,@_lottery_num,@_cur_date,@_trade_num,@_cur_date,@_alliance_lottery_num,
					@_time_now,_binded,20);
			IF ROW_COUNT()=0 THEN
				SET _result = -3;
				LEAVE label_body;
			END IF;
		END IF;
	END IF;
	IF EXISTS (SELECT 1 FROM taverns WHERE account_id=_account_id) THEN
		SET _result = -4;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO taverns (account_id,level) VALUES (_account_id,1);
	IF ROW_COUNT()=0 THEN
		SET _result = -5;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO mail_id (account_id,mail_id,today_send_num) VALUES (_account_id,0,0);
	IF ROW_COUNT()=0 THEN
		SET _result = -6;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO citydefenses (account_id) VALUES (_account_id);
	IF ROW_COUNT()=0 THEN
		SET _result = -7;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 初始建筑
	SET _done = 0;
	OPEN _cursor;
		REPEAT
			FETCH _cursor INTO _excel_id,_auto_id;
			IF NOT _done THEN
				INSERT INTO buildings (account_id,excel_id,auto_id,level) VALUES (_account_id, _excel_id, _auto_id, 1);
				# 许愿树
				IF _excel_id=12 THEN
					IF NOT EXISTS (SELECT 1 FROM excel_christmas_tree WHERE (@_ripe_watering_num:=ripe_watering_num)>=0 LIMIT 1) THEN
						SET @_ripe_watering_num = 0;
					END IF;
					INSERT INTO buildings_ext_data (account_id,auto_id,excel_id,data_0) VALUES (_account_id,_auto_id,_excel_id,@_ripe_watering_num-1);
				END IF;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;

	# 初始科技
	SET _done = 0;
	OPEN _cursor_tech;
		REPEAT
			FETCH _cursor_tech INTO _excel_tech_id;
			IF NOT _done THEN
				INSERT INTO technologys VALUES (_account_id,_excel_tech_id,1);
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor_tech;

	INSERT INTO private_log_id (account_id,log_id) VALUES (_account_id,0);
END label_body;
	SELECT _result;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_alliance_building_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_alliance_building_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_alliance_building_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _excel_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _type BIGINT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-100; SELECT _result,_alliance_id,_excel_id,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-101; SELECT _result,_alliance_id,_excel_id,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-102; SELECT _result,_alliance_id,_excel_id,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-103; SELECT _result,_alliance_id,_excel_id,_type; END;

label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;
	
	SELECT alliance_id,excel_id,type INTO _alliance_id,_excel_id,_type FROM alliance_building_time_events WHERE event_id=_event_id;
	
	START TRANSACTION;
	CASE
		WHEN _type = 1 THEN
			INSERT INTO alliance_buildings VALUES (_alliance_id, _excel_id, 1);
		WHEN _type = 2 THEN
			UPDATE alliance_buildings SET level=level+1 WHERE alliance_id=_alliance_id AND excel_id=_excel_id;
		ELSE
			SET _result=-1;			# type类型不对
			LEAVE label_body;
	END CASE;
	IF ROW_COUNT()=0 THEN
		SET _result = -2;			# 失败
		LEAVE label_body;
	END IF;
	
	# 删除时间事件
	DELETE FROM time_events WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;			# 失败
		LEAVE label_body;
	END IF;
	DELETE FROM alliance_building_time_events WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -4;			# 失败
		LEAVE label_body;
	END IF;

END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result=_release_te_lock(_event_id);
	END IF;
	SELECT _result,_alliance_id,_excel_id,_type;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_building_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_building_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_building_event`(IN `_event_id` bigint)
BEGIN
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _auto_id INT UNSIGNED DEFAULT 0;
	DECLARE _excel_id INT UNSIGNED DEFAULT 0;
	DECLARE _type INT UNSIGNED DEFAULT 0;
	DECLARE _result INT DEFAULT 0;

	# 声明异常处理
	DECLARE EXIT				HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-100; SELECT _result,_account_id,_auto_id,_excel_id,_type; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-101; SELECT _result,_account_id,_auto_id,_excel_id,_type; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-102; SELECT _result,_account_id,_auto_id,_excel_id,_type; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-103; SELECT _result,_account_id,_auto_id,_excel_id,_type; END;

label_body : BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;
	
	# 事件数据
	SELECT account_id,auto_id,excel_id,type INTO _account_id,_auto_id,_excel_id,_type FROM building_time_events WHERE event_id=_event_id;

	START TRANSACTION;
	CASE 
		WHEN _type=1 THEN	#建造
			INSERT INTO buildings VALUES (_account_id,_auto_id,_excel_id,1);
		WHEN _type=2 THEN #升级
			IF _excel_id=5 THEN
				# 大使馆升级需要重新计算跑商次数
				SET @_delta_trade_num=0;
				SELECT level INTO @_cur_level FROM buildings WHERE account_id=_account_id AND excel_id=5;
				SELECT trade_num INTO @_trade_num_pre FROM excel_embassy_cfg WHERE level=@_cur_level;
				SELECT trade_num INTO @_trade_num_after FROM excel_embassy_cfg WHERE level=@_cur_level+1;
				SET @_delta_trade_num=@_trade_num_after-@_trade_num_pre;
				UPDATE common_characters SET trade_num=trade_num+@_delta_trade_num WHERE account_id=_account_id;
			END IF;
			UPDATE buildings SET level=level+1 WHERE account_id=_account_id AND auto_id=_auto_id;
		#WHEN _type=3 THEN #生产完成,将结果插到数据库,等待收获
		#ELSE #记录日志
	END CASE;

	# 删除时间事件
	DELETE FROM time_events WHERE event_id=_event_id;
	DELETE FROM building_time_events WHERE event_id=_event_id;

END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result=_release_te_lock(_event_id);
	END IF;
	SELECT _result,_account_id,_auto_id,_excel_id,_type; 
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_once_perperson_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_once_perperson_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_once_perperson_event`(IN `_event_id` bigint)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _type INT UNSIGNED DEFAULT 0;

	# 居民生产相关
	DECLARE _production INT UNSIGNED DEFAULT 0;
	DECLARE _capacity INT UNSIGNED DEFAULT 0;
	DECLARE _population INT UNSIGNED DEFAULT 0;

	DECLARE EXIT			HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-100; SELECT _result,_account_id,_type,_production; END;
	DECLARE EXIT			HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-101; SELECT _result,_account_id,_type,_production; END;
	DECLARE EXIT			HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-102; SELECT _result,_account_id,_type,_production; END;
	DECLARE EXIT			HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-103; SELECT _result,_account_id,_type,_production; END;

label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;

	SELECT account_id,type INTO _account_id,_type FROM once_perperson_time_events WHERE event_id=_event_id;
	SELECT population INTO _population FROM common_characters WHERE account_id=_account_id;

	START TRANSACTION;

	CASE
		# 许愿树
		WHEN _type = 1 THEN
			INSERT INTO production_events (account_id,auto_id,type,production) VALUES (_account_id,400,2,0);
			# 清理浇水
			DELETE FROM relation_events WHERE account_id=_account_id AND type=1;
		WHEN _type = 2 THEN
			# 获取人口产量
			CALL get_population_production_n_capacity(_account_id, _result, _production, _capacity);
			IF _result = 0 THEN
				IF _population+_production > _capacity THEN
					IF _population>=_capacity THEN
						SET _production = 0;
					ELSE
						SET _production = _capacity-_population;
					END IF;
				END IF;
				UPDATE common_characters SET population=population+_production WHERE account_id=_account_id;
			ELSE
				SET _result = -1;
				LEAVE label_body;
			END IF;
		WHEN _type = 3 THEN
			SET _result = 0;
		ELSE
			LEAVE label_body;
	END CASE;
	
	# 删除时间事件
	DELETE FROM time_events WHERE event_id=_event_id;
	DELETE FROM once_perperson_time_events WHERE event_id=_event_id;
	
	IF _type = 3 THEN
		SET _production = get_build_num(_account_id);
	END IF;
END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result=_release_te_lock(_event_id);
	END IF;
	# 由上层根据_result,_type进行判断,是否需要注册下一次事件
	SELECT _result,_account_id,_type,_production;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_production_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_production_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_production_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _auto_id INT UNSIGNED DEFAULT 0;
	DECLARE _production INT UNSIGNED DEFAULT 0;
	DECLARE _type INT UNSIGNED DEFAULT 0;
	
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-100; SELECT _result,_account_id,_auto_id,_production,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-101; SELECT _result,_account_id,_auto_id,_production,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-102; SELECT _result,_account_id,_auto_id,_production,_type; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-103; SELECT _result,_account_id,_auto_id,_production,_type; END;

	# 根据类型取出产量，将成熟结果插入production_event
label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;

	IF NOT EXISTS (SELECT 1 FROM production_time_events WHERE event_id=_event_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	SELECT account_id,auto_id,production,type INTO _account_id,_auto_id,_production,_type FROM production_time_events WHERE event_id=_event_id;

	START TRANSACTION;
	CASE
		WHEN _type=1 THEN
			INSERT INTO production_events VALUES (_account_id, _auto_id, _production, _type);
			IF ROW_COUNT() = 0 THEN
				SET _result = -2;
				LEAVE label_body;
			END IF;
			# 这里需要插入保护事件
			SELECT e.goldore_protection_interval INTO @_protect_time FROM excel_vip_right e JOIN common_characters c ON e.level=c.vip WHERE c.account_id=_account_id;
			IF @_protect_time!=0 THEN
				SET @_event_id = game_global_id();
				SET @_time_now = UNIX_TIMESTAMP();
				INSERT INTO production_time_events (event_id,account_id,auto_id,production,type) VALUES (@_event_id,_account_id,_auto_id,0,3);
				IF ROW_COUNT() = 0 THEN
					SET _result = -2;
					ROLLBACK;
					LEAVE label_body;
				END IF;
				INSERT INTO time_events (event_id,begin_time,end_time,type,locked) VALUES (@_event_id,@_time_now,@_time_now+@_protect_time,3,0);
				IF ROW_COUNT() = 0 THEN
					SET _result = -2;
					ROLLBACK;
					LEAVE label_body;
				END IF;
			END IF;
		WHEN _type=3 THEN
			SET _result = 0; #什么都不用干,只要删除事件就行
		ELSE
			INSERT INTO production_events VALUES (_account_id, _auto_id, _production, _type);
			IF ROW_COUNT() = 0 THEN
				SET _result = -2;
				LEAVE label_body;
			END IF;
	END CASE;
	
	DELETE FROM time_events WHERE event_id=_event_id;
	IF ROW_COUNT() = 0 THEN
		SET _result = -3;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	DELETE FROM production_time_events WHERE event_id=_event_id;
	IF ROW_COUNT() = 0 THEN
		SET _result = -4;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result=_release_te_lock(_event_id);
	END IF;
	SELECT _result,_account_id,_auto_id,_production,_type;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `dismiss_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `dismiss_alliance`;
DELIMITER ;;
CREATE PROCEDURE `dismiss_alliance`(IN `_account_id` bigint)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _position INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;
	DECLARE _account_id_tmp BIGINT UNSIGNED DEFAULT 0;
	
	DECLARE _cursor CURSOR FOR SELECT account_id FROM alliance_members WHERE alliance_id=_alliance_id;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_alliance_id; END;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_alliance_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_alliance_id; END;

	label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alliance_members WHERE account_id=_account_id) THEN
		SET _result = -1; # 没有联盟
		LEAVE label_body;
	END IF;

	SELECT alliance_id,position INTO _alliance_id,_position FROM alliance_members WHERE account_id=_account_id;
	IF _position<>1 THEN
		SET _result = -2;	# 只有盟主才能解除
		LEAVE label_body;
	END IF;

	START TRANSACTION;
	# 删除联盟数据
	DELETE FROM alliances WHERE alliance_id=_alliance_id;
	IF ROW_COUNT()=0 THEN
		SET _result=-3;
		LEAVE label_body;
	END IF;

	# 删除联盟事件
	DELETE FROM alliance_join_events WHERE alliance_id=_alliance_id;
	# 删除联盟建筑时间事件
	DELETE FROM time_events WHERE event_id IN (SELECT event_id FROM alliance_building_time_events WHERE alliance_id=_alliance_id);
	DELETE FROM alliance_building_time_events WHERE alliance_id=_alliance_id;
	# 删除联盟建筑
	DELETE FROM alliance_buildings WHERE alliance_id=_alliance_id;
	# 删除联盟事件邮件，联盟跑商
	SET _done=0;
	OPEN _cursor;
		REPEAT
			FETCH _cursor INTO _account_id_tmp;
			IF NOT _done THEN
				DELETE FROM private_mails WHERE account_id=_account_id_tmp AND type=8;
				IF EXISTS (SELECT 1 FROM trade_time_events WHERE account_id=_account_id_tmp AND (@_event_id:=event_id)>=0) THEN
					DELETE FROM trade_time_events WHERE event_id=@_event_id;
					DELETE FROM time_events WHERE event_id=@_event_id;
				END IF;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;
	# 清空联盟成员
	DELETE FROM alliance_members WHERE alliance_id=_alliance_id;
	IF ROW_COUNT()=0 THEN
		SET _result=-4;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 清空联盟ID
	UPDATE common_characters SET alliance_id=0 WHERE alliance_id=_alliance_id;
	IF ROW_COUNT()=0 THEN
		SET _result=-5;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	END label_body;
	SELECT _result,_alliance_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `exit_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `exit_alliance`;
DELIMITER ;;
CREATE PROCEDURE `exit_alliance`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _development_cost INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_alliance_id,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_alliance_id,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_alliance_id,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_alliance_id,_development_cost; END;

label_body:BEGIN
	SELECT alliance_id INTO _alliance_id FROM common_characters WHERE account_id=_account_id;
	IF _alliance_id=0 THEN
		SET _result = -1;	# 没有联盟
		LEAVE label_body;
	END IF;

	# 盟主不能退出联盟,需要使用解散联盟功能
	IF EXISTS (SELECT 1 FROM alliance_members WHERE account_id=_account_id AND position=1) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	
	START TRANSACTION;
	# 从联盟成员表中删除
	DELETE FROM alliance_members WHERE alliance_id=_alliance_id AND account_id=_account_id;
	# 将联盟ID设置为0
	UPDATE common_characters SET alliance_id=0 WHERE account_id=_account_id;
	# 扣除发展度
	SELECT development INTO _development_cost FROM excel_alliance_development WHERE action_id=2;	# action_id=2,退出联盟
	IF EXISTS (SELECT 1 FROM alliances WHERE alliance_id=_alliance_id AND development>=_development_cost) THEN
		UPDATE alliances SET development=development-_development_cost,total_development=total_development-_development_cost WHERE alliance_id=_alliance_id;
	ELSE
		UPDATE alliances SET development=0,total_development=total_development-_development_cost WHERE alliance_id=_alliance_id;
	END IF;
	# 删除联盟事件邮件，联盟跑商
	DELETE FROM private_mails WHERE account_id=_account_id AND type=8;
	IF EXISTS (SELECT 1 FROM trade_time_events WHERE account_id=_account_id AND (@_event_id:=event_id)>=0) THEN
		DELETE FROM trade_time_events WHERE event_id=@_event_id;
		DELETE FROM time_events WHERE event_id=@_event_id;
	END IF;

	-- 修改当前奖杯排行信息
	call game_update_cup(_account_id,@vRst);
END label_body;
	SELECT _result,_alliance_id,_development_cost;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `expel_alliance_member`
-- ----------------------------
DROP PROCEDURE IF EXISTS `expel_alliance_member`;
DELIMITER ;;
CREATE PROCEDURE `expel_alliance_member`(IN `_manager_id` bigint unsigned,IN `_alliance_id` bigint unsigned,IN `_member_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _manager_position INT UNSIGNED DEFAULT 0;
	DECLARE _member_position INT UNSIGNED DEFAULT 0;
	DECLARE _development_cost INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_development_cost; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_development_cost; END;

label_body:BEGIN
	# 必须在一个联盟里 
	# 可以用一句SQL进行判断
	# SELECT 1 FROM common_characters c1 JOIN common_characters c2 ON c1.alliance_id=c2.alliance_id WHERE c1.account_id=_manager_id AND c2.account_id=_member_id AND c1.alliance_id=_alliance_id;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_manager_id AND alliance_id=_alliance_id) THEN
		SET _result = -1;	# 管理者不在该联盟
		LEAVE label_body;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_member_id AND alliance_id=_alliance_id) THEN
		SET _result = -2; # 被开除者不在该联盟
		LEAVE label_body;
	END IF;
	# 只能开除职位比自己低的
	# 可以用一句SQL进行判断
	# SELECT 1 FROM alliance_members m1 JOIN alliance_members m2 WHERE m1.account_id=1 AND m2.account_id=2 AND m1.position<m2.position;
	SELECT position INTO _manager_position FROM alliance_members WHERE account_id=_manager_id;
	SELECT position INTO _member_position FROM alliance_members WHERE account_id=_member_id;
	IF _manager_position >= _member_position THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;

	START TRANSACTION;
	# 从成员表中删除
	DELETE FROM alliance_members WHERE account_id=_member_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
	# 修改联盟ID
	UPDATE common_characters SET alliance_id=0 WHERE account_id=_member_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -5;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 以后开除还需要扣除联盟发展度
	SELECT development INTO _development_cost FROM excel_alliance_development WHERE action_id=3;	# action_id=3,开除出联盟
	IF EXISTS (SELECT 1 FROM alliances WHERE alliance_id=_alliance_id AND development>=_development_cost) THEN
		UPDATE alliances SET development=development-_development_cost,total_development=total_development-_development_cost WHERE alliance_id=_alliance_id;
	ELSE
		UPDATE alliances SET development=0,total_development=total_development-_development_cost WHERE alliance_id=_alliance_id;
	END IF;
	# 删除联盟事件邮件，联盟跑商
	DELETE FROM private_mails WHERE account_id=_member_id AND type=8;
	IF EXISTS (SELECT 1 FROM trade_time_events WHERE account_id=_member_id AND (@_event_id:=event_id)>=0) THEN
		DELETE FROM trade_time_events WHERE event_id=@_event_id;
		DELETE FROM time_events WHERE event_id=@_event_id;
	END IF;
END label_body;
	SELECT _result,_development_cost;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `fetch_production`
-- ----------------------------
DROP PROCEDURE IF EXISTS `fetch_production`;
DELIMITER ;;
CREATE PROCEDURE `fetch_production`(IN `_account_id` bigint unsigned,IN `_auto_id` int unsigned,IN `_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _production INT UNSIGNED DEFAULT 0;
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_production; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_production; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_production; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_production; END;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=_type) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	SELECT production INTO _production FROM production_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=_type;
	
	START TRANSACTION;
	CASE
	WHEN _type=1 THEN
		UPDATE common_characters SET gold=gold+_production WHERE account_id=_account_id;
		DELETE FROM relation_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=2;
		# 成熟保护事件
		IF EXISTS (SELECT 1 FROM production_time_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=3 AND (@_event_id:=event_id)>=0) THEN
			DELETE FROM production_time_events WHERE event_id=@_event_id;
			DELETE FROM time_events WHERE event_id=@_event_id;
		END IF;
		UPDATE buildings_ext_data SET data_0=0 WHERE account_id=_account_id AND auto_id=_auto_id;
	ELSE
		SET _result = -2;
		LEAVE label_body;
	END CASE;
	DELETE FROM production_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=_type;
	IF ROW_COUNT() = 0 THEN
		SET _result = -3;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	
END label_body;
	SELECT _result,_production;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_population_production_n_capacity`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_population_production_n_capacity`;
DELIMITER ;;
CREATE PROCEDURE `get_population_production_n_capacity`(IN `_account_id` bigint unsigned,OUT `_result` int,OUT `_production` int unsigned,OUT `_capacity` int unsigned)
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;

	
	DECLARE _cur_production INT UNSIGNED DEFAULT 0;
	DECLARE _cur_capacity INT UNSIGNED DEFAULT 0;
	
--	DECLARE _cursor CURSOR FOR SELECT level FROM buildings WHERE account_id=_account_id AND excel_id=9;

	DECLARE EXIT	HANDLER FOR SQLEXCEPTION BEGIN SET _result=-100; END;
	DECLARE CONTINUE	HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	# 切记此处赋初值,否则全玩完
	SET _result = 0;
	SET _production = 0;
	SET _capacity = 0;

label_body:BEGIN
--	IF NOT EXISTS (SELECT 1 FROM buildings WHERE account_id=_account_id AND excel_id=9 LIMIT 1) THEN
--		LEAVE label_body;
--	END IF;

--	SET _production = 0;
--	SET _capacity = 0;

--	SET _done = 0;
--	OPEN _cursor;
--		REPEAT
--			FETCH _cursor INTO _level;
--			IF NOT _done THEN
--				SELECT production,capacity INTO _cur_production,_cur_capacity FROM excel_house_production WHERE level=_level;
--				IF _cur_production IS NULL THEN
--					SET _cur_production	= 0;
--					SET _cur_capacity	= 0;
--				END IF;
--				SET _production = _production + _cur_production;
--				SET _capacity = _capacity + _cur_capacity;
--			END IF;
--		UNTIL _done END REPEAT;
--	CLOSE _cursor;

    select ifnull(sum(T2.production),0),ifnull(sum(T2.capacity),0) into _production,_capacity from buildings T1 left join excel_house_production T2 on T2.level=T1.level where T1.account_id=_account_id and T1.excel_id=9;

END label_body;
	SET _result = 0;

END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_te_before_now`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_te_before_now`;
DELIMITER ;;
CREATE PROCEDURE `get_te_before_now`()
BEGIN
	IF _get_lock(3)!=0 THEN
		SELECT event_id,type FROM time_events WHERE end_time<=UNIX_TIMESTAMP() AND locked=0;
		SET @_result = _release_lock(3);
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `join_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `join_alliance`;
DELIMITER ;;
CREATE PROCEDURE `join_alliance`(IN `_account_id` bigint unsigned,IN `_alliance_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	# 等级够不
	#set @vLevel=0;
	#select val_int into @vLevel from excel_cmndef where id=15;
	#if not exists (select 1 from common_characters where account_id=_account_id and level>=@vLevel) then
	#	set _result = -10;
	#	leave label_body;
	#end if;
	# 该联盟是否存在
	IF NOT EXISTS (SELECT 1 FROM alliances WHERE alliance_id=_alliance_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 是否已经有联盟了
	IF EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND alliance_id<>0) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 是否已经申请过了
	IF EXISTS (SELECT 1 FROM alliance_join_events WHERE account_id=_account_id) THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	# 成员上限是否够
	SELECT count(*) INTO @_member_num FROM alliance_members WHERE alliance_id=_alliance_id;
	# 是否还有空余位置
	IF NOT EXISTS (SELECT 1 FROM alliance_buildings b JOIN excel_alliance_congress c ON b.level=c.level WHERE alliance_id=_alliance_id AND excel_id=1 AND c.member_num>@_member_num) THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;
	INSERT INTO alliance_join_events VALUES (_account_id, _alliance_id);
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `set_alliance_position`
-- ----------------------------
DROP PROCEDURE IF EXISTS `set_alliance_position`;
DELIMITER ;;
CREATE PROCEDURE `set_alliance_position`(IN `_manager_id` bigint unsigned,IN `_alliance_id` bigint unsigned,IN `_member_id` bigint unsigned,IN `_position` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE _manager_position INT UNSIGNED DEFAULT 0;
	DECLARE _member_position INT UNSIGNED DEFAULT 0;
	DECLARE _max_num INT UNSIGNED DEFAULT 0;
	DECLARE _cur_num INT UNSIGNED DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result; END;

label_body:BEGIN
	# 必须在一个联盟里 
	# 可以用一句SQL进行判断
	# SELECT 1 FROM common_characters c1 JOIN common_characters c2 ON c1.alliance_id=c2.alliance_id WHERE c1.account_id=_manager_id AND c2.account_id=_member_id AND c1.alliance_id=_alliance_id;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_manager_id AND alliance_id=_alliance_id) THEN
		SET _result = -1;	# 管理者不在该联盟
		LEAVE label_body;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_member_id AND alliance_id=_alliance_id) THEN
		SET _result = -2; # 被任命者不在该联盟
		LEAVE label_body;
	END IF;
	# 只能任命比自己职位低的
	# 可以用一句SQL进行判断
	# SELECT 1 FROM alliance_members m1 JOIN alliance_members m2 WHERE m1.account_id=1 AND m2.account_id=2 AND m1.position<m2.position;
	SELECT position INTO _manager_position FROM alliance_members WHERE account_id=_manager_id;
	SELECT position INTO _member_position FROM alliance_members WHERE account_id=_member_id;
	IF _manager_position >= _member_position THEN
		SET _result = -3;	# 职位没人家高
		LEAVE label_body;
	END IF;
	IF _manager_position >= _position THEN
		SET _result = -4; #没权限任命这个职位
		LEAVE label_body;
	END IF;
	IF _member_position = _position THEN
		SET _result = -5; #没有职位变动
		LEAVE label_body;
	END IF;
	# 这个职位是否还有名额
	SELECT level INTO _level FROM alliance_buildings WHERE alliance_id=_alliance_id AND excel_id=1;
	SELECT COUNT(*) INTO _cur_num FROM alliance_members WHERE alliance_id=_alliance_id AND position=_position;
	CASE
		WHEN _position=2 THEN
			SELECT vice_leader_num INTO _max_num FROM excel_alliance_congress WHERE level=_level;
		WHEN _position=3 THEN
			SELECT manager_num INTO _max_num FROM excel_alliance_congress WHERE level=_level;
		WHEN _position=4 THEN
			SELECT member_num INTO _max_num FROM excel_alliance_congress WHERE level=_level;
		ELSE
			SET _result = -6;	# 没用这种职位
			LEAVE label_body;
	END CASE;
	IF _cur_num >= _max_num THEN
			SET _result = -7; # 该职位名额已满
			LEAVE label_body;
	END IF;

	START TRANSACTION;
	UPDATE alliance_members SET position=_position WHERE alliance_id=_alliance_id AND account_id=_member_id;
END label_body;
	SELECT _result;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `upgrade_building`
-- ----------------------------
DROP PROCEDURE IF EXISTS `upgrade_building`;
DELIMITER ;;
CREATE PROCEDURE `upgrade_building`(IN `_account_id` bigint unsigned,IN `_auto_id` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _cur_level INT UNSIGNED DEFAULT 0;
	DECLARE _cur_build_num INT UNSIGNED DEFAULT 0;
	DECLARE _excel_id INT UNSIGNED DEFAULT 0;
	DECLARE _max_level INT UNSIGNED DEFAULT 0;
	DECLARE _cur_max_level INT UNSIGNED DEFAULT 0;
	DECLARE _cur_development INT UNSIGNED DEFAULT 0;
	DECLARE _time_now	INT UNSIGNED DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	
	# 升级数据
	DECLARE _upgrade_time INT UNSIGNED DEFAULT 0;
	DECLARE _upgrade_gold INT UNSIGNED DEFAULT 0;
	DECLARE _upgrade_population INT UNSIGNED DEFAULT 0;
	DECLARE _upgrade_exp INT UNSIGNED DEFAULT 0;
	DECLARE _upgrade_development INT UNSIGNED DEFAULT 0;
	
	# 角色数据
	DECLARE _char_development INT UNSIGNED DEFAULT 0;
	DECLARE _char_gold INT UNSIGNED DEFAULT 0;
	DECLARE _char_population INT UNSIGNED DEFAULT 0;
	DECLARE _char_build_num INT UNSIGNED DEFAULT 0;
	DECLARE _char_added_build_num INT UNSIGNED DEFAULT 0;
	DECLARE _char_level INT UNSIGNED DEFAULT 0;
	DECLARE _char_exp INT UNSIGNED DEFAULT 0;
	
	# 声明异常处理
	DECLARE EXIT				HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_excel_id,_upgrade_exp,_upgrade_development,_char_exp,_char_level,_upgrade_gold,_upgrade_population; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_excel_id,_upgrade_exp,_upgrade_development,_char_exp,_char_level,_upgrade_gold,_upgrade_population; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_excel_id,_upgrade_exp,_upgrade_development,_char_exp,_char_level,_upgrade_gold,_upgrade_population; END;
	DECLARE EXIT				HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_excel_id,_upgrade_exp,_upgrade_development,_char_exp,_char_level,_upgrade_gold,_upgrade_population; END;
	
label_body : BEGIN
	
	# 该auto_id是否存在建筑
	IF NOT EXISTS (SELECT auto_id FROM buildings WHERE account_id=_account_id AND auto_id=_auto_id) THEN
		SET _result=-1;
		LEAVE label_body;
	END IF;
	
	# 获取该建筑的当前等级
	SELECT level,excel_id INTO _cur_level,_excel_id FROM buildings WHERE account_id=_account_id AND auto_id=_auto_id;
	
	# 获取该建筑的最大等级
	SELECT MAX(level) INTO _max_level FROM excel_building_list WHERE excel_id=_excel_id;
	
	# 获取角色数据
	SELECT development,gold,population,build_num,added_build_num,vip INTO _char_development,_char_gold,_char_population,_char_build_num,_char_added_build_num,@_vip_level FROM common_characters WHERE account_id=_account_id;
	
	# 判断是否有足够的建造队列
	SELECT added_build_num INTO @_vip_added_build_num FROM excel_vip_right WHERE level=@_vip_level;
	SELECT count(*) INTO _cur_build_num FROM building_time_events WHERE account_id=_account_id AND type IN (1, 2);
	SET @_item_added_build_num = 0;
	IF EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=_account_id AND type=3) THEN
		SELECT val_int INTO @_item_added_build_num FROM excel_item_list WHERE excel_id=2012;
	END IF;
	IF _cur_build_num >= (_char_build_num + _char_added_build_num + @_vip_added_build_num + @_item_added_build_num) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	
	# 城堡的等级受发展度限制,其他建筑等级受城堡限制(城堡excel_id=1)
	IF _excel_id=1 THEN
		SELECT MAX(development) INTO _cur_development FROM excel_building_level_limit WHERE development<=_char_development;
		SELECT castle_max_level INTO _cur_max_level FROM excel_building_level_limit WHERE development=_cur_development;
	ELSE
		SELECT level INTO _cur_max_level FROM buildings WHERE account_id=_account_id AND excel_id=1;
	END IF;
	IF _max_level > _cur_max_level THEN
		SET _max_level = _cur_max_level;
	END IF;
	
	# 等级是否达到当前上限
	IF _cur_level >= _max_level THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	SELECT build_time,build_gold,build_population,exp,development INTO _upgrade_time,_upgrade_gold,_upgrade_population,_upgrade_exp,_upgrade_development FROM excel_building_list WHERE excel_id=_excel_id AND level=_cur_level+1;
	
	# 金钱是否足够
	IF _upgrade_gold > _char_gold THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
	
	# 人口是否足够
	IF _upgrade_population > _char_population THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;
	START TRANSACTION;
	
	# 生成建造时间事件(这里也可能出错,比如已经有了该建筑对应的时间事件了)
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	INSERT INTO time_events VALUES (_event_id, _time_now, _time_now+_upgrade_time, 1, 0);	#事件类型(1:建筑)
	IF ROW_COUNT() = 0 
	THEN
		SET _result = -6;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO building_time_events VALUES (_event_id, _account_id, _auto_id, _excel_id, 2); 
	IF ROW_COUNT() = 0 
	THEN
		ROLLBACK;
		SET _result = -7;
		LEAVE label_body;
	END IF;
	
	# 扣除金钱和人口
	UPDATE common_characters SET gold=gold-_upgrade_gold,population=population-_upgrade_population,development=development+_upgrade_development WHERE account_id=_account_id AND gold>=_upgrade_gold AND population>=_upgrade_population;
	IF ROW_COUNT() = 0 
	THEN
		ROLLBACK;
		SET _result = -8;
		LEAVE label_body;
	END IF;
	CALL add_char_exp(_account_id,_upgrade_exp,_char_exp,_char_level);
END label_body;
	
	SELECT _result,_excel_id,_upgrade_exp,_upgrade_development,_char_exp,_char_level,_upgrade_gold,_upgrade_population;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `accelerate`
-- ----------------------------
DROP PROCEDURE IF EXISTS `accelerate`;
DELIMITER ;;
CREATE PROCEDURE `accelerate`(IN `_account_id` bigint unsigned,IN `_auto_id` int unsigned,IN `_excel_id` int unsigned,IN `_type` int unsigned,IN `_subtype` int unsigned,IN `_time` int unsigned,IN `_money_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _time_base FLOAT UNSIGNED DEFAULT 0;
	DECLARE _diamond INT UNSIGNED DEFAULT 0;
	DECLARE _crystal INT UNSIGNED DEFAULT 0;
	DECLARE _begin_time INT UNSIGNED DEFAULT 0;
	DECLARE _end_time INT UNSIGNED DEFAULT 0;
	DECLARE _money_num INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_time,_money_num,_excel_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_time,_money_num,_excel_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_time,_money_num,_excel_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_time,_money_num,_excel_id; END;

label_body:BEGIN
	IF _time=0 THEN
		SET _time = 0;
		SET _diamond = 0;
		LEAVE label_body;
	END IF;

	IF _money_type NOT IN (1,2) THEN
		SET _result = -11;
		LEAVE label_body;
	END IF;

	SELECT time,diamond,crystal INTO _time_base,_diamond,_crystal FROM excel_accelerate_cost;

	CASE
		WHEN _type=1 THEN
			SELECT event_id,excel_id INTO _event_id,_excel_id FROM building_time_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=_subtype;
		WHEN _type=3 THEN
			SELECT event_id INTO _event_id FROM production_time_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=_subtype;
		WHEN _type=4 THEN
			SELECT event_id INTO _event_id FROM research_time_events WHERE account_id=_account_id AND excel_id=_excel_id;
		ELSE
			SET _result = -1;
			LEAVE label_body;
	END CASE;

	# 获取时间
	SET _begin_time=UNIX_TIMESTAMP();
	SELECT end_time INTO _end_time FROM time_events WHERE event_id=_event_id;
	IF (_end_time <= _begin_time) THEN
		SET _time = 0;
		SET _diamond = 0;
		LEAVE label_body;
	ELSE
		IF (_end_time-_begin_time) < _time THEN
			SET _time = _end_time-_begin_time;
		END IF;
	END IF;

	START TRANSACTION;
	CASE
		WHEN _money_type=1 THEN
			SET _money_num = CEILING(_time/_time_base)*_diamond;
			CALL _calc_accelerate_cost(_money_num);
			IF _money_num = 0 THEN
				SET _result = -12;
				LEAVE label_body;
			END IF;
			# 修改钻石余额
			UPDATE common_characters SET diamond=diamond-_money_num WHERE account_id=_account_id AND diamond>=_money_num;
			IF ROW_COUNT()=0 THEN
				SET _result = -2;
				LEAVE label_body;
			END IF;
		WHEN _money_type=2 THEN
			SET _money_num = CEILING(_time/_time_base)*_crystal;
			CALL _calc_accelerate_cost(_money_num);
			IF _money_num = 0 THEN
				SET _result = -13;
				LEAVE label_body;
			END IF;
			# 修改水晶余额
			UPDATE common_characters SET crystal=crystal-_money_num WHERE account_id=_account_id AND crystal>=_money_num;
			IF ROW_COUNT()=0 THEN
				SET _result = -10;
				LEAVE label_body;
			END IF;
	END CASE;
	# 修改时间事件
	UPDATE time_events SET end_time=end_time-_time WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	
	SET @_acce_time	= _time;
	
	# 需要返回的是剩余多少时间
	IF _time>(_end_time-_begin_time) THEN
		SET _time=0;
	ELSE
		SET _time=_end_time-_begin_time-_time;
	END IF;
END label_body;
	SELECT _result,_time,_money_num,_excel_id,@_acce_time;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_research_time_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_research_time_event`;
DELIMITER ;;
CREATE PROCEDURE `add_research_time_event`(IN `_account_id` bigint unsigned,IN `_excel_id` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;
	DECLARE _institute_level INT UNSIGNED DEFAULT 0;
	DECLARE _max_level INT UNSIGNED DEFAULT 0;
	DECLARE _time_cost INT UNSIGNED DEFAULT 0;
	DECLARE _gold_cost INT UNSIGNED DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _time_now INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_level,_time_cost,_gold_cost,_event_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_level,_time_cost,_gold_cost,_event_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_level,_time_cost,_gold_cost,_event_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_level,_time_cost,_gold_cost,_event_id; END;

label_body:BEGIN
	# 是否有该种科技
	IF NOT EXISTS (SELECT 1 FROM excel_soldier WHERE excel_id=_excel_id LIMIT 1) THEN
		SET _result = -1; #不存在该种科技
		LEAVE label_body;
	END IF;
	# 是否已经有科技了
	# IFNULL没意义,_level还是NULL
	# SELECT IFNULL(level,0) INTO _level FROM technologys WHERE account_id=_account_id AND excel_id=_excel_id;
	IF EXISTS (SELECT level FROM technologys WHERE account_id=_account_id AND excel_id=_excel_id) THEN
		SELECT level INTO _level FROM technologys WHERE account_id=_account_id AND excel_id=_excel_id;
	END IF;
	SET _level = _level+1;
	# 当前是否科技已经到达上限
	IF NOT EXISTS (SELECT 1 FROM excel_technology WHERE level=_level) THEN
		SET _result = -2; #科技已经到达上限
		LEAVE label_body;
	END IF;
	# 研究院等级是否足够
	IF NOT EXISTS (SELECT 1 FROM buildings WHERE account_id=_account_id AND excel_id=2) THEN
		SET _result = -3; #未建造研究院
		LEAVE label_body;
	ELSE
		SELECT level INTO _institute_level FROM buildings WHERE account_id=_account_id AND excel_id=2;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM excel_technology WHERE institute_level<=_institute_level AND level=_level) THEN
		SET _result = -4; #研究院等级不够
		LEAVE label_body;
	END IF;
	# 是否已经有科技在研究了
	IF EXISTS (SELECT 1 FROM research_time_events WHERE account_id=_account_id) THEN
		SET _result = -5; #有科技正在研究中
		LEAVE label_body;
	END IF;
	# 金钱和时间花销
	SELECT gold,time*3600 INTO _gold_cost,_time_cost FROM excel_technology WHERE level=_level;
	
	START TRANSACTION;
	UPDATE common_characters SET gold=gold-_gold_cost WHERE account_id=_account_id AND gold>=_gold_cost;
	IF ROW_COUNT()=0 THEN
		SET _result = -6; #余额不够
		LEAVE label_body;
	END IF;
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	INSERT INTO time_events VALUES (_event_id,_time_now,_time_now+_time_cost,4,0);	#4为研究科技
	IF ROW_COUNT()=0 THEN
		SET _result = -7; 
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO research_time_events VALUES (_event_id, _account_id, _excel_id);
	IF ROW_COUNT()=0 THEN
		SET _result = -8; 
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_level,_time_cost,_gold_cost,_event_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_research_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_research_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_research_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _excel_id INT UNSIGNED DEFAULT 0;
	DECLARE	_level INT UNSIGNED DEFAULT 0;
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-100; SELECT _result,_account_id,_excel_id,_level; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-101; SELECT _result,_account_id,_excel_id,_level; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-102; SELECT _result,_account_id,_excel_id,_level; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-103; SELECT _result,_account_id,_excel_id,_level; END;
label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;

	SELECT account_id,excel_id INTO _account_id,_excel_id FROM research_time_events WHERE event_id=_event_id;
	START TRANSACTION;
	IF EXISTS (SELECT 1 FROM technologys WHERE account_id=_account_id AND excel_id=_excel_id) THEN
		UPDATE technologys SET level=level+1 WHERE account_id=_account_id AND excel_id=_excel_id;
		SELECT level INTO _level FROM technologys WHERE account_id=_account_id AND excel_id=_excel_id;
	ELSE
		INSERT INTO technologys VALUES (_account_id, _excel_id, 1);
		SET _level=1;
	END IF;
	IF ROW_COUNT()=0 THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	DELETE FROM time_events WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -2;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	DELETE FROM research_time_events WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result=_release_te_lock(_event_id); 
	END IF;
	SELECT _result,_account_id,_excel_id,_level;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `conscript_soldier`
-- ----------------------------
DROP PROCEDURE IF EXISTS `conscript_soldier`;
DELIMITER ;;
CREATE PROCEDURE `conscript_soldier`(IN `_account_id` bigint unsigned,IN `_excel_id` int unsigned,IN `_level` int unsigned,IN `_num` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _capacity INT UNSIGNED DEFAULT 0;
	DECLARE _cur_num INT UNSIGNED DEFAULT 0;
	DECLARE _cur_num_with_hero INT UNSIGNED DEFAULT 0;	# 英雄当前的带兵量
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_gold; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_gold; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_gold; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_gold; END;

label_body:BEGIN
	# 是否存在该兵种
	IF NOT EXISTS (SELECT 1 FROM excel_soldier WHERE excel_id=_excel_id LIMIT 1) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 科技是否足够
	IF NOT EXISTS (SELECT 1 FROM technologys WHERE account_id=_account_id AND excel_id=_excel_id AND level>=_level) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;

	# 居民是否足够
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND population>=_num) THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;

	# 金钱是否足够
	SELECT gold*_num INTO _gold FROM excel_soldier WHERE excel_id=_excel_id AND level=_level;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND gold>=_gold) THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;

	# 获取当前士兵数量
	SELECT IFNULL(sum(num),0) INTO _cur_num FROM soldiers WHERE account_id=_account_id;
	SELECT IFNULL(sum(army_num),0) INTO _cur_num_with_hero FROM hire_heros WHERE account_id=_account_id;
	# 兵营容量是否足够
	CALL get_barrack_capacity(_account_id, _result, _capacity);
	IF _result<>0 THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;
	IF (_num+_cur_num+_cur_num_with_hero) > _capacity THEN
		SET _result = -6;
		LEAVE label_body;
	END IF;

	START TRANSACTION;
	# 扣钱、人口
	UPDATE common_characters SET gold=gold-_gold,population=population-_num WHERE account_id=_account_id AND gold>=_gold AND population>=_num;
	IF ROW_COUNT()=0 THEN
		SET _result = -7;
		LEAVE label_body;
	END IF;
	# 插入兵
	IF EXISTS (SELECT 1 FROM soldiers WHERE account_id=_account_id AND level=_level AND excel_id=_excel_id) THEN
		UPDATE soldiers SET num=num+_num WHERE account_id=_account_id AND level=_level AND excel_id=_excel_id;
	ELSE
		INSERT INTO soldiers VALUES (_account_id, _excel_id, _level, _num);
	END IF;
	IF ROW_COUNT()=0 THEN
		SET _result = -8;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_gold;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_barrack_capacity`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_barrack_capacity`;
DELIMITER ;;
CREATE PROCEDURE `get_barrack_capacity`(IN `_account_id` bigint unsigned,OUT `_result` int,OUT `_capacity` int unsigned)
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _cur_capacity INT UNSIGNED DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT level FROM buildings WHERE account_id=_account_id AND excel_id=10;

	DECLARE EXIT	HANDLER FOR SQLEXCEPTION BEGIN SET _result=-100; END;
	DECLARE CONTINUE	HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	
	# 先赋值,不然_capacity是NULL
	SET _capacity = 0;

	SET _done = 0;
	OPEN _cursor;
		REPEAT
			FETCH _cursor INTO _level;
			IF NOT _done THEN
				SELECT capacity INTO _cur_capacity FROM excel_barrack_production WHERE level=_level;
				IF _cur_capacity IS NULL THEN
					SET _cur_capacity=0;
				END IF;
				SET _capacity = _capacity + _cur_capacity;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;

	SET _result = 0;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `upgrade_soldier`
-- ----------------------------
DROP PROCEDURE IF EXISTS `upgrade_soldier`;
DELIMITER ;;
CREATE PROCEDURE `upgrade_soldier`(IN `_account_id` bigint unsigned,IN `_excel_id` int unsigned,IN `_from_level` int unsigned,IN `_to_level` int unsigned,IN `_num` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _cur_num INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_gold; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_gold; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_gold; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_gold; END;

label_body:BEGIN
	IF _from_level >= _to_level THEN
		LEAVE label_body;
	END IF;

	# 科技是否足够
	IF NOT EXISTS (SELECT 1 FROM technologys WHERE account_id=_account_id AND excel_id=_excel_id AND level>=_to_level) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	# 原等级兵种是否足够
	IF NOT EXISTS (SELECT 1 FROM soldiers WHERE account_id=_account_id AND excel_id=_excel_id AND level=_from_level AND num>=_num) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;

	# 金钱
	SELECT gold*_num INTO _gold FROM excel_soldier WHERE excel_id=_excel_id AND level=_to_level;
	SELECT _gold-gold*_num INTO _gold FROM excel_soldier WHERE excel_id=_excel_id AND level=_from_level;

	START TRANSACTION;
	# 扣钱
	UPDATE common_characters SET gold=gold-_gold WHERE account_id=_account_id AND gold>=_gold;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	# 删除原等级兵
	IF EXISTS (SELECT 1 FROM soldiers WHERE account_id=_account_id AND level=_from_level AND excel_id=_excel_id AND num>_num) THEN
		UPDATE soldiers SET num=num-_num WHERE account_id=_account_id AND level=_from_level AND excel_id=_excel_id AND num>_num;
	ELSE
		DELETE FROM soldiers WHERE account_id=_account_id AND level=_from_level AND excel_id=_excel_id AND num=_num;
	END IF;
	IF ROW_COUNT()=0 THEN
		SET _result = -2;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 插入兵
	IF EXISTS (SELECT 1 FROM soldiers WHERE account_id=_account_id AND level=_to_level AND excel_id=_excel_id) THEN
		UPDATE soldiers SET num=num+_num WHERE account_id=_account_id AND level=_to_level AND excel_id=_excel_id;
	ELSE
		INSERT INTO soldiers VALUES (_account_id, _excel_id, _to_level, _num);
	END IF;
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_gold;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_private_mail`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_private_mail`;
DELIMITER ;;
CREATE PROCEDURE `add_private_mail`(IN `_sender_id` bigint unsigned,IN `_receiver_id` bigint unsigned,IN `_type` tinyint unsigned,IN `_flag` tinyint unsigned,IN `_send_limit` int unsigned,IN `_text` blob,IN `_ext_data` blob,IN `_echo` int unsigned,OUT `_result` int, OUT `_mail_id` int unsigned)
BEGIN
	DECLARE _max_num INT UNSIGNED DEFAULT 0;
	DECLARE _max_send_num INT UNSIGNED DEFAULT 0;
	DECLARE _cur_num INT UNSIGNED DEFAULT 0;

	SET _result=0;
	SET _mail_id=0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_receiver_id) THEN
		SET _result = -1; # 接受者不存在
		LEAVE label_body;
	END IF;
	
	IF _send_limit=1 THEN
		# 发送者是否继续发送邮件
		UPDATE mail_id SET today_send_num=0,last_send_day=CURDATE() WHERE account_id=_sender_id AND last_send_day!=CURDATE();
		SELECT e.send_mail_num INTO _max_send_num FROM excel_vip_right e JOIN common_characters c ON e.level=c.vip WHERE c.account_id=_sender_id;
		IF EXISTS (SELECT 1 FROM mail_id WHERE account_id=_sender_id AND today_send_num>=_max_send_num) THEN
			SET _result = -2;
			LEAVE label_body;
		END IF;
		
		# 接受者是否可以接收邮件
		SELECT e.mail_num INTO _max_num FROM excel_vip_right e JOIN common_characters c ON e.level=c.vip WHERE c.account_id=_receiver_id;
		CASE
		WHEN _type=1 THEN
			SELECT count(*) INTO _cur_num FROM private_mails WHERE account_id=_receiver_id AND type=1;
			IF _cur_num>=_max_num THEN
				SET _result = -3;	# 接受者邮箱已满
			LEAVE label_body;
			END IF;
		ELSE
			SET _result = 0;
		END CASE;
	END IF;
	SET _mail_id=new_mail_id(_receiver_id);
	IF _mail_id=0 THEN
		SET _result = -4;	# msg_id不存在
		LEAVE label_body;
	END IF;
	
	INSERT INTO private_mails (account_id,sender_id,type,flag,readed,text,mail_id,time,ext_data) VALUES (_receiver_id,_sender_id,_type,_flag,0,COMPRESS(_text),_mail_id,UNIX_TIMESTAMP(),COMPRESS(_ext_data));
	IF _send_limit=1 THEN
		UPDATE mail_id SET today_send_num=today_send_num+1 WHERE account_id=_sender_id;
	END IF;
END label_body;
	IF _echo=1 THEN
		SELECT _result,_mail_id;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `new_mail_id`
-- ----------------------------
DROP FUNCTION IF EXISTS `new_mail_id`;
DELIMITER ;;
CREATE FUNCTION `new_mail_id`(`_account_id` bigint unsigned) RETURNS int(10) unsigned
BEGIN
	DECLARE _new_mail_id INT UNSIGNED DEFAULT 0;
	UPDATE mail_id SET mail_id=mail_id+1 WHERE account_id=_account_id;
	SELECT mail_id INTO _new_mail_id FROM mail_id WHERE account_id=_account_id;
	RETURN _new_mail_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `abdicate_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `abdicate_alliance`;
DELIMITER ;;
CREATE PROCEDURE `abdicate_alliance`(IN `_leader_id` bigint unsigned,IN `_member_name` varchar(32))
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT DEFAULT 0;
	DECLARE _member_id BIGINT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_alliance_id,_member_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_alliance_id,_member_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_alliance_id,_member_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_alliance_id,_member_id; END;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alliances WHERE account_id=_leader_id) THEN
		SET _result = -1; 
		LEAVE label_body;
	ELSE
		SELECT alliance_id INTO _alliance_id FROM alliances WHERE account_id=_leader_id;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE name=_member_name AND alliance_id=_alliance_id) THEN
		SET _result = -2; 
		LEAVE label_body;
	END IF;
	SELECT account_id INTO _member_id FROM common_characters WHERE name=_member_name;
	START TRANSACTION;
	UPDATE alliance_members SET position=4 WHERE account_id=_leader_id AND alliance_id=_alliance_id;
	UPDATE alliance_members SET position=1 WHERE account_id=_member_id AND alliance_id=_alliance_id;
	UPDATE alliances SET account_id=_member_id WHERE alliance_id=_alliance_id;
END label_body;
	SELECT _result,_alliance_id,_member_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `refuse_join_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `refuse_join_alliance`;
DELIMITER ;;
CREATE PROCEDURE `refuse_join_alliance`(IN `_manager_id` bigint unsigned,IN `_new_member_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT DEFAULT 0;
	
label_body:BEGIN
	SELECT alliance_id INTO _alliance_id FROM common_characters WHERE account_id=_manager_id;
	IF _alliance_id=0 THEN
		SET _result = -1;	# 管理者没有加入联盟
		LEAVE label_body;
	END IF;

	IF NOT EXISTS (SELECT 1 FROM alliance_join_events WHERE alliance_id=_alliance_id AND account_id=_new_member_id) THEN
		SET _result = -2; # 没有该申请事件
		LEAVE label_body;
	END IF;

	# 是否有权限拒绝申请
	IF NOT EXISTS (SELECT 1 FROM alliance_members m JOIN excel_alliance_position_right e ON m.position=e.excel_id WHERE m.account_id=_manager_id AND e.approve_member=1) THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;

	DELETE FROM alliance_join_events WHERE account_id=_new_member_id AND alliance_id=_alliance_id;
END label_body;
	SELECT _result,_alliance_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `apply_friend`
-- ----------------------------
DROP PROCEDURE IF EXISTS `apply_friend`;
DELIMITER ;;
CREATE PROCEDURE `apply_friend`(IN `_account_id` bigint unsigned,IN `_peer_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	# 自己向自己申请
	IF _account_id=_peer_account_id THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	# 已经是好友
	IF EXISTS (SELECT 1 FROM personal_relations WHERE account_id=_account_id AND peer_account_id=_peer_account_id AND relation_type=1) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 已经有申请事件了
	IF EXISTS (SELECT 1 FROM friend_apply_events WHERE account_id=_account_id AND peer_account_id=_peer_account_id) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	
	SELECT v.friend_num INTO @_max_friend_num FROM common_characters c JOIN excel_vip_right v ON c.vip=v.level WHERE c.account_id=_account_id;
	SELECT COUNT(*) INTO @_cur_friend_num FROM personal_relations WHERE account_id=_account_id AND relation_type=1;
	IF @_cur_friend_num >= @_max_friend_num THEN
		SET _result = -4; # 自己的好友数量超过上限
		LEAVE label_body;
	END IF;

	SELECT v.friend_num INTO @_max_friend_num FROM common_characters c JOIN excel_vip_right v ON c.vip=v.level WHERE c.account_id=_peer_account_id;
	SELECT COUNT(*) INTO @_cur_friend_num FROM personal_relations WHERE account_id=_peer_account_id AND relation_type=1;
	IF @_cur_friend_num >= @_max_friend_num THEN
		SET _result = -5; # 对方的好友数量超过上限
		LEAVE label_body;
	END IF;

	# 添加申请事件
	INSERT INTO friend_apply_events VALUES (_account_id, _peer_account_id);
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `approve_friend`
-- ----------------------------
DROP PROCEDURE IF EXISTS `approve_friend`;
DELIMITER ;;
CREATE PROCEDURE `approve_friend`(IN `_account_id` bigint unsigned,IN `_peer_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _friend_num INT UNSIGNED DEFAULT 0;
	DECLARE _peer_friend_num INT UNSIGNED DEFAULT 0;
	
label_body:BEGIN
	# 不能同意自己为好友
	IF _account_id=_peer_account_id THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;

	IF NOT EXISTS (SELECT 1 FROM friend_apply_events WHERE account_id=_peer_account_id AND peer_account_id=_account_id) THEN
		SET _result = -1;	# 申请好友事件不存在
		LEAVE label_body;
	END IF;

	# 自己好友是否已达上限
	SELECT COUNT(*) INTO _friend_num FROM personal_relations WHERE account_id=_account_id AND relation_type=1;
	IF NOT EXISTS (SELECT 1 FROM common_characters c JOIN excel_vip_right e ON c.vip=e.level WHERE c.account_id=_account_id AND e.friend_num>_friend_num) THEN
		SET _result = -2; # 自己好友已达上限
		LEAVE label_body;
	END IF;
	# 对方好友是否已达上限
	SELECT COUNT(*) INTO _peer_friend_num FROM personal_relations WHERE account_id=_peer_account_id AND relation_type=1;
	IF NOT EXISTS (SELECT 1 FROM common_characters c JOIN excel_vip_right e ON c.vip=e.level WHERE c.account_id=_peer_account_id AND e.friend_num>_peer_friend_num) THEN
		SET _result = -3; # 对方好友已达上限
		LEAVE label_body;
	END IF;

	DELETE FROM friend_apply_events WHERE (account_id=_peer_account_id AND peer_account_id=_account_id) OR (account_id=_account_id AND peer_account_id=_peer_account_id);
	INSERT INTO personal_relations (account_id,peer_account_id,relation_type) VALUES (_account_id,_peer_account_id,1);
	INSERT INTO personal_relations (account_id,peer_account_id,relation_type) VALUES (_peer_account_id,_account_id,1);
END label_body;
	SELECT _result,_friend_num,_peer_friend_num;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `cancel_friend_apply`
-- ----------------------------
DROP PROCEDURE IF EXISTS `cancel_friend_apply`;
DELIMITER ;;
CREATE PROCEDURE `cancel_friend_apply`(IN `_account_id` bigint unsigned,IN `_peer_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DELETE FROM friend_apply_events WHERE account_id=_account_id AND peer_account_id=_peer_account_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -1;	# 没有申请事件
	END IF;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `delete_friend`
-- ----------------------------
DROP PROCEDURE IF EXISTS `delete_friend`;
DELIMITER ;;
CREATE PROCEDURE `delete_friend`(IN `_account_id` bigint unsigned,IN `_peer_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DELETE FROM personal_relations WHERE ((account_id=_account_id AND peer_account_id=_peer_account_id) OR (account_id=_peer_account_id AND peer_account_id=_account_id)) AND relation_type=1;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `refuse_friend_apply`
-- ----------------------------
DROP PROCEDURE IF EXISTS `refuse_friend_apply`;
DELIMITER ;;
CREATE PROCEDURE `refuse_friend_apply`(IN `_account_id` bigint unsigned,IN `_peer_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DELETE FROM friend_apply_events WHERE account_id=_peer_account_id AND peer_account_id=_account_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -1;	# 没有申请事件
	END IF;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_char_exp`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_char_exp`;
DELIMITER ;;
CREATE PROCEDURE `add_char_exp`(IN `_account_id` bigint unsigned,IN `_exp` int unsigned,OUT `_rst_exp` int unsigned,OUT `_rst_level` int unsigned)
BEGIN
	# 当前经验条长度
	DECLARE _cur_exp_max INT UNSIGNED DEFAULT 0;
	DECLARE _level_max INT UNSIGNED DEFAULT 0;
	# 当前的等级和经验
	DECLARE _cur_exp INT UNSIGNED DEFAULT 0;
	DECLARE _cur_level INT UNSIGNED DEFAULT 0;

	DECLARE _done INT DEFAULT 0;

label_body:BEGIN
	SELECT level,exp INTO _cur_level,_cur_exp FROM common_characters WHERE account_id=_account_id;
	SELECT MAX(level) INTO _level_max FROM excel_char_level_exp;
	IF _cur_level>=_level_max THEN # 已达等级上限
			SET _cur_level			= _level_max;
			SET _cur_exp				= 0;
			LEAVE label_body;
	END IF;

	SELECT exp INTO _cur_exp_max FROM excel_char_level_exp WHERE level=_cur_level+1;

	IF (_cur_exp+_exp)>=_cur_exp_max THEN	# 需要升级
		SET _cur_level			= _cur_level + 1;
		# 计算升级完后剩余经验值
		IF (_cur_exp>_cur_exp_max) THEN
			SET _exp					= _exp + (_cur_exp-_cur_exp_max);
		ELSE
			SET _exp					= _exp - (_cur_exp_max-_cur_exp);
		END IF;

		REPEAT	# 连续升级
			IF _cur_level>=_level_max THEN # 已达等级上限
				SET _cur_level			= _level_max;
				SET _cur_exp				= 0;
				LEAVE label_body;
			END IF;
			SELECT exp INTO _cur_exp_max FROM excel_char_level_exp WHERE level=_cur_level+1;
			IF _exp>=_cur_exp_max THEN
				SET _cur_level 	= _cur_level+1;
				SET _exp				= _exp - _cur_exp_max;
			ELSE
				LEAVE label_body;
			END IF;
		UNTIL _done END REPEAT;
	ELSE
		SET _exp				=	_cur_exp+_exp;
	END IF;
END label_body;
	IF _cur_level>=_level_max THEN # 已达等级上限
		SET _cur_level	= _level_max;
		SET _exp				= 0;
	END IF;
	UPDATE common_characters SET exp=_exp,level=_cur_level WHERE account_id=_account_id;
	SET _rst_exp			= _exp;
	SET _rst_level		= _cur_level;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `borrow_soldier`
-- ----------------------------
DROP FUNCTION IF EXISTS `borrow_soldier`;
DELIMITER ;;
CREATE FUNCTION `borrow_soldier`(`_account_id` bigint unsigned,`_excel_id` int unsigned,`_level` int unsigned,`_num` int unsigned) RETURNS int(11)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _real_num INT UNSIGNED DEFAULT 0;

	if (_num=0) then
		return 0;
	end if;

	SELECT IFNULL(num,0) INTO _real_num FROM soldiers WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level;
	IF _real_num < _num THEN
		SET _result = -1;
	ELSE
		IF _real_num > _num THEN
			UPDATE soldiers SET num=num-_num WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level AND num>_num;
			IF ROW_COUNT()=0 THEN
				SET _result=-2;
			END IF;
		ELSE
			DELETE FROM soldiers WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level AND num=_num;
			IF ROW_COUNT()=0 THEN
				SET _result=-3;
			END IF;
		END IF;
	END IF;

	RETURN _result;
END
;;
DELIMITER ;


-- ----------------------------
-- Function structure for `revert_soldier`
-- ----------------------------
DROP FUNCTION IF EXISTS `revert_soldier`;
DELIMITER ;;
CREATE FUNCTION `revert_soldier`(`_account_id` bigint unsigned,`_excel_id` int unsigned,`_level` int unsigned,`_num` int unsigned) RETURNS int(11)
BEGIN
	DECLARE _result INT DEFAULT 0;
	if (_num=0) then
		return 0;
	end if;
	
--	IF EXISTS (SELECT 1 FROM soldiers WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level) THEN
--		UPDATE soldiers SET num=num+_num WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level;
--	ELSE
--		INSERT INTO soldiers (account_id,excel_id,level,num) VALUES (_account_id,_excel_id,_level,_num);
--	END IF;

	insert into soldiers (account_id,excel_id,level,num) values (_account_id,_excel_id,_level,_num) on duplicate key update num=num+_num;

	IF ROW_COUNT()=0 THEN
		SET _result=-1;
	END IF;
	RETURN _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `new_alliance_log_id`
-- ----------------------------
DROP FUNCTION IF EXISTS `new_alliance_log_id`;
DELIMITER ;;
CREATE FUNCTION `new_alliance_log_id`(`_alliance_id` bigint unsigned) RETURNS int(10) unsigned
BEGIN
	DECLARE _new_log_id INT UNSIGNED DEFAULT 0;
	UPDATE alliance_log_id SET log_id=log_id+1 WHERE alliance_id=_alliance_id;
	SELECT log_id INTO _new_log_id FROM alliance_log_id WHERE alliance_id=_alliance_id;
	RETURN _new_log_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `new_alliance_mail_id`
-- ----------------------------
DROP FUNCTION IF EXISTS `new_alliance_mail_id`;
DELIMITER ;;
CREATE FUNCTION `new_alliance_mail_id`(`_alliance_id` bigint unsigned) RETURNS int(10) unsigned
BEGIN
	DECLARE _new_mail_id INT UNSIGNED DEFAULT 0;
	UPDATE alliance_mail_id SET mail_id=mail_id+1 WHERE alliance_id=_alliance_id;
	SELECT mail_id INTO _new_mail_id FROM alliance_mail_id WHERE alliance_id=_alliance_id;
	RETURN _new_mail_id;
END
;;
DELIMITER ;


-- ----------------------------
-- Function structure for `new_private_log_id`
-- ----------------------------
DROP FUNCTION IF EXISTS `new_private_log_id`;
DELIMITER ;;
CREATE FUNCTION `new_private_log_id`(`_account_id` bigint unsigned) RETURNS int(10) unsigned
BEGIN
	DECLARE _new_log_id INT UNSIGNED DEFAULT 0;
	UPDATE private_log_id SET log_id=log_id+1 WHERE account_id=_account_id;
	SELECT log_id INTO _new_log_id FROM private_log_id WHERE account_id=_account_id;
	RETURN _new_log_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_alliance_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_alliance_log`;
DELIMITER ;;
CREATE PROCEDURE `add_alliance_log`(IN `_alliance_id` bigint unsigned,IN `_text` varchar(256))
BEGIN
	DECLARE _log_id INT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;

	SET _log_id = new_alliance_log_id(_alliance_id);
	SET _time = UNIX_TIMESTAMP();
	INSERT INTO alliance_logs (alliance_id,log_id,text,time) VALUES (_alliance_id,_log_id,_text,_time);

	SELECT _log_id,_time;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_alliance_mail`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_alliance_mail`;
DELIMITER ;;
CREATE PROCEDURE `add_alliance_mail`(IN `_account_id` bigint unsigned,IN `_text` varchar(1024))
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _mail_id INT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;
	
label_body:BEGIN
		SELECT alliance_id INTO _alliance_id FROM common_characters WHERE account_id=_account_id;
		IF _alliance_id=0 THEN
			SET _result = -1;	# 没有加入联盟
			LEAVE label_body;
		END IF;
	
		SET _mail_id = new_alliance_mail_id(_alliance_id);
		SET _time = UNIX_TIMESTAMP();
		INSERT INTO alliance_mails (alliance_id,mail_id,account_id,text,time) VALUES (_alliance_id,_mail_id,_account_id,_text,_time);
END label_body;

	SELECT _result,_alliance_id,_mail_id,_time;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_private_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_private_log`;
DELIMITER ;;
CREATE PROCEDURE `add_private_log`(IN `_account_id` bigint unsigned,IN `_text` varchar(256))
BEGIN
	DECLARE _log_id INT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;

	SET _log_id = new_private_log_id(_account_id);
	SET _time = UNIX_TIMESTAMP();

	INSERT INTO private_logs (account_id,log_id,text,time) VALUES (_account_id,_log_id,_text,_time);

	SELECT _log_id,_time;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `buy_item`
-- ----------------------------
DROP PROCEDURE IF EXISTS `buy_item`;
DELIMITER ;;
CREATE PROCEDURE `buy_item`(IN `_account_id` bigint unsigned,IN `_excel_id` int unsigned,IN `_num` int unsigned,IN `_money_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _money_num INT UNSIGNED DEFAULT 0;
	DECLARE _can_crystal_buy INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_money_num,@vItem; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_money_num,@vItem; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_money_num,@vItem; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_money_num,@vItem; END;

label_body:BEGIN
	IF _num=0 THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	
	IF NOT EXISTS (SELECT 1 FROM excel_store_item WHERE excel_id=_excel_id AND can_crystal_buy!=2) THEN
		SET _result =-2; # 物品不能卖或者不存在
		LEAVE label_body;
	END IF;

	CASE
		WHEN _money_type=1 THEN
			# 总价
			SELECT diamond*_num INTO _money_num FROM excel_store_item WHERE excel_id=_excel_id;
		WHEN _money_type=2 THEN
			# 总价
			SELECT crystal*_num,can_crystal_buy INTO _money_num,_can_crystal_buy FROM excel_store_item WHERE excel_id=_excel_id;
			IF _can_crystal_buy=0 THEN
				SET _result =-3; # 该物品不可以用水晶购买
				LEAVE label_body;
			END IF;
		ELSE
			SET _result = -10; # 不支持的金钱类型
			LEAVE label_body;
	END CASE;


	START TRANSACTION;
	
	CASE
		WHEN _money_type=1 THEN
			# 扣除金钱
			UPDATE common_characters SET diamond=diamond-_money_num WHERE account_id=_account_id AND diamond>=_money_num;
			IF ROW_COUNT()=0 THEN
				SET _result = -4;	# 余额不足
				LEAVE label_body;
			END IF;
		WHEN _money_type=2 THEN
			# 扣除金钱
			UPDATE common_characters SET crystal=crystal-_money_num WHERE account_id=_account_id AND crystal>=_money_num;
			IF ROW_COUNT()=0 THEN
				SET _result = -4;	# 余额不足
				LEAVE label_body;
			END IF;
		ELSE
			SET _result = -10;
			LEAVE label_body;
	END CASE;
	
	# 添加道具
	CALL game_add_item(_account_id,0,_excel_id,_num,0,_result);
	IF _result<=0 THEN
		SET _result = -5;	# 添加道具失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
	SET _result = 0;
END label_body;
	SELECT _result,_money_num,@vItem;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `sell_item`
-- ----------------------------
DROP PROCEDURE IF EXISTS `sell_item`;
DELIMITER ;;
CREATE PROCEDURE `sell_item`(IN `_account_id` bigint unsigned,IN `_item_id` bigint unsigned,IN `_num` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _excel_id INT UNSIGNED DEFAULT 0;
	DECLARE _cur_num INT UNSIGNED DEFAULT 0;
	
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_gold,_excel_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_gold,_excel_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_gold,_excel_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_gold,_excel_id; END;

label_body:BEGIN
	IF _num=0 THEN
		LEAVE label_body;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM items WHERE account_id=_account_id AND item_id=_item_id) THEN
		SET _result = -1; # 道具不存在
		LEAVE label_body;
	END IF;

	SELECT excel_id,num INTO _excel_id,_cur_num FROM items WHERE item_id=_item_id;
	IF _cur_num<_num THEN
		SET _result = -2; # 数量超过
		LEAVE label_body;
	END IF;

	SELECT price*_num INTO _gold FROM excel_item_list WHERE excel_id=_excel_id;
	
	START TRANSACTION;
	CALL game_del_item(_account_id, _item_id, _num, 0, _result);
	IF _result<=0 THEN
		SET _result = -3;	# 道具扣除失败
		LEAVE label_body;
	END IF;
	SET _result = 0;
	set @vCup=0;
	IF _gold>0 THEN
		UPDATE common_characters SET gold=gold+_gold WHERE account_id=_account_id and (@vCup:=cup)>=0;
	END IF;
END label_body;
	SELECT _result,_gold,_excel_id,@vCup;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `_get_percent_value`
-- ----------------------------
DROP PROCEDURE IF EXISTS `_get_percent_value`;
DELIMITER ;;
CREATE PROCEDURE `_get_percent_value`(IN `_src_string` varchar(1024),IN `_used_percent` float,IN `_rand` float,OUT `_value` int,OUT `_percent` float,OUT `_total_percent` float,OUT `_result` int)
BEGIN
	# 备注
	# _src_string:源串,概率*数值
	# _used_percent:已经用了的概率
	# _rand:随机值
	# _value:满足概率的值
	# _percent:满足概率值对应的概率
	# _total_percent:当前使用的概率总值
	# _result:是否成功

	DECLARE _count INT DEFAULT 0;
	DECLARE _total_char_count INT DEFAULT 0;
	DECLARE _cur_percent_value FLOAT UNSIGNED DEFAULT 0;

label_body:BEGIN
	SET _cur_percent_value = _used_percent;
	SET _total_char_count = _get_char_count(_src_string, '*');
	
	SET _value = 0;
	SET _percent = 0;
	SET _total_percent = _used_percent;
	SET _result = -1; # 默认为失败
	WHILE _count<_total_char_count DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_src_string,'*',_count+1),'*',-1) INTO @_tmp_percent;
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_src_string,'*',_count+2),'*',-1) INTO @_tmp_value;
		SET _count=_count+2;
		SET _total_percent = _total_percent + @_tmp_percent;
		SET _cur_percent_value = _cur_percent_value + @_tmp_percent;
		IF _rand < _cur_percent_value THEN
			SET _value = @_tmp_value;
			SET _percent = @_tmp_percent;
			SET _result = 0;
			LEAVE label_body;
		END IF;
	END WHILE;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_get_char_count`
-- ----------------------------
DROP FUNCTION IF EXISTS `_get_char_count`;
DELIMITER ;;
CREATE FUNCTION `_get_char_count`(`_src_string` blob,`_char` char) RETURNS int(11)
BEGIN
	DECLARE _length INT DEFAULT 0;
	DECLARE _count INT DEFAULT 0;
	DECLARE _pos INT DEFAULT 0;

	SET _length = LENGTH(_src_string);
	WHILE _pos <= _length DO
		IF (STRCMP(SUBSTRING(_src_string, _pos, 1), _char) = 0) THEN
			SET _count = _count + 1;
		END IF;
		SET _pos = _pos + 1;
	END WHILE;
	RETURN _count;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `fetch_christmas_tree`
-- ----------------------------
DROP PROCEDURE IF EXISTS `fetch_christmas_tree`;
DELIMITER ;;
CREATE PROCEDURE `fetch_christmas_tree`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _str_buf varchar(1024) DEFAULT '';
	DECLARE _item_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _excel_id INT UNSIGNED DEFAULT 0;
	DECLARE _crystal INT UNSIGNED DEFAULT 0;
	DECLARE _result INT DEFAULT 0;
	DECLARE _percent FLOAT DEFAULT 0;
	DECLARE _total_percent FLOAT DEFAULT 0;
	DECLARE _rand FLOAT DEFAULT 0;
	DECLARE _char_level INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_item_id,_excel_id,_crystal,_percent,_total_percent,_rand; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_item_id,_excel_id,_crystal,_percent,_total_percent,_rand; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_item_id,_excel_id,_crystal,_percent,_total_percent,_rand; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_item_id,_excel_id,_crystal,_percent,_total_percent,_rand; END;

label_body:BEGIN
	# 是否存在圣诞树成熟事件
	IF NOT EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND type=2) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	
	# 角色等级
	SELECT level INTO _char_level FROM common_characters WHERE account_id=_account_id;

	# 随机数
	SET _rand = RAND();

	# 获得水晶奖励列表
	SELECT award_crystal_string INTO _str_buf FROM excel_christmas_tree WHERE char_level<=_char_level ORDER BY char_level DESC LIMIT 1;
	CALL _get_percent_value(_str_buf, 0, _rand, _crystal, _percent, _total_percent, _result);

	START TRANSACTION;
	IF _result=0 THEN
		UPDATE common_characters SET crystal=crystal+_crystal WHERE account_id=_account_id;
		LEAVE label_body;
	END IF;

	# 获得道具列表
	SELECT award_item_string INTO _str_buf FROM excel_christmas_tree WHERE char_level<=_char_level ORDER BY char_level DESC LIMIT 1;
	CALL _get_percent_value(_str_buf, _total_percent, _rand, _excel_id, _percent, _total_percent, _result);
	IF _result=0 THEN
		SET @vItemID = 0;
		CALL game_add_item(_account_id,0,_excel_id,1,0,_result);
		IF _result<=0 THEN
			SET _result=-3; # 添加道具失败
			LEAVE label_body;
		END IF;
		# 获取道具ID
		SELECT @vItemID INTO _item_id;
		SET _result = 0;
		LEAVE label_body;
	END IF;

	# 都没成功,发送水晶
	SELECT last_award_crystal INTO _crystal FROM excel_christmas_tree WHERE char_level<=_char_level ORDER BY char_level DESC LIMIT 1;
	UPDATE common_characters SET crystal=crystal+_crystal WHERE account_id=_account_id;
	
	SET _result = 0;
END label_body;
	IF _result=0 THEN
		# 清理成熟事件
		DELETE FROM production_events WHERE account_id=_account_id AND type=2;
	END IF;

	SELECT _result,_item_id,_excel_id,_crystal,_percent,_total_percent,_rand;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `watering_christmas_tree`
-- ----------------------------
DROP PROCEDURE IF EXISTS `watering_christmas_tree`;
DELIMITER ;;
CREATE PROCEDURE `watering_christmas_tree`(IN `_account_id` bigint unsigned,IN `_watering_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _ripe_watering_num INT UNSIGNED DEFAULT 0;
	DECLARE _self_watering_interval INT UNSIGNED DEFAULT 0;
	DECLARE _ripe_time_interval INT UNSIGNED DEFAULT 0;
	DECLARE _char_level INT UNSIGNED DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _ready_ripe INT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_event_id,_time,_ready_ripe,_ripe_time_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_event_id,_time,_ready_ripe,_ripe_time_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_event_id,_time,_ready_ripe,_ripe_time_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_event_id,_time,_ready_ripe,_ripe_time_interval; END;
	
label_body:BEGIN
	# 圣诞树已经成熟,不需要浇水
	IF EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND type=2) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 圣诞树成熟时间事件,不能浇水
	IF EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=_account_id AND type=1) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	
	# 浇水的次数是否已经满
	SELECT level INTO _char_level FROM common_characters WHERE account_id=_account_id AND (@_vip:=vip)>=0;
	SELECT ripe_watering_num,self_watering_interval,ripe_time_interval INTO _ripe_watering_num,_self_watering_interval,_ripe_time_interval FROM excel_christmas_tree WHERE char_level<=_char_level ORDER BY char_level DESC LIMIT 1;

	IF _account_id=_watering_account_id THEN
		# 自己浇水有时间限制
		IF EXISTS (SELECT 1 FROM buildings_ext_data WHERE account_id=_account_id AND auto_id=400 AND data_1>UNIX_TIMESTAMP()) THEN
			SET _result = -3; # 自己还不能浇水,CD中
			LEAVE label_body;
		END IF;
	ELSE
		# 是否已经浇过水了
		IF EXISTS (SELECT 1 FROM relation_events WHERE account_id=_account_id AND p_account_id=_watering_account_id AND type=1) THEN
			SET _result = -4;
			LEAVE label_body;
		END IF;
	END IF;

	SET _time = UNIX_TIMESTAMP();

	START TRANSACTION;
	IF _account_id=_watering_account_id THEN
		# 增加浇水的次数并修改时间
		UPDATE buildings_ext_data SET data_0=data_0+1,data_1=_time+_self_watering_interval WHERE account_id=_account_id AND auto_id=400 AND data_0<_ripe_watering_num;
		IF ROW_COUNT()=0 THEN
			SET _ready_ripe = 1;
			LEAVE label_body;
		END IF;
	ELSE
		# 添加浇水事件
		INSERT INTO relation_events (account_id,p_account_id,type,auto_id) VALUES (_account_id,_watering_account_id,1,400);
		# 增加浇水次数
		UPDATE buildings_ext_data SET data_0=data_0+1 WHERE account_id=_account_id AND auto_id=400 AND data_0<_ripe_watering_num;
		IF ROW_COUNT()=0 THEN
			SET _ready_ripe = 1;
			LEAVE label_body;
		END IF;
	END IF;
	
	IF EXISTS (SELECT 1 FROM buildings_ext_data WHERE account_id=_account_id AND auto_id=400 AND data_0=_ripe_watering_num) THEN
		SET _ready_ripe = 1;
	END IF;
	
	IF _ready_ripe=1 THEN
		SELECT christmas_tree_ripe_interval INTO @_vip_sub_time FROM excel_vip_right WHERE level=@_vip;
		# 添加成熟时间事件
		SET _event_id = game_global_id();
		INSERT INTO once_perperson_time_events (event_id,account_id,type) VALUES (_event_id,_account_id,1);
		IF ROW_COUNT()=0 THEN
			ROLLBACK;
			LEAVE label_body;
		END IF;
		INSERT INTO time_events (event_id,begin_time,end_time,type,locked) VALUES (_event_id,_time,_time+_ripe_time_interval-@_vip_sub_time,2,0);
		# 清理附加数据
		UPDATE buildings_ext_data SET data_0=0,data_1=0 WHERE account_id=_account_id AND auto_id=400;
	END IF;
END label_body;
	SELECT _result,_event_id,_time,_ready_ripe,_ripe_time_interval;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `steal_gold`
-- ----------------------------
DROP PROCEDURE IF EXISTS `steal_gold`;
DELIMITER ;;
CREATE PROCEDURE `steal_gold`(IN `_account_id` bigint unsigned,IN `_thief_account_id` bigint unsigned,IN `_auto_id` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _can_steal INT UNSIGNED DEFAULT 0;
	DECLARE _steal_gold INT UNSIGNED DEFAULT 0;
	DECLARE _steal_percent FLOAT UNSIGNED DEFAULT 0;
	DECLARE _steal_num_max INT UNSIGNED DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100;	SELECT _result,_steal_gold,_steal_percent,_steal_num_max; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101;	SELECT _result,_steal_gold,_steal_percent,_steal_num_max; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102;	SELECT _result,_steal_gold,_steal_percent,_steal_num_max; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103;	SELECT _result,_steal_gold,_steal_percent,_steal_num_max; END;

label_body:BEGIN
	# 自己不能偷自己
	IF _account_id=_thief_account_id THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 还没成熟
	IF NOT EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=1) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 等级
	SELECT level INTO _level FROM buildings WHERE account_id=_account_id AND auto_id=_auto_id;
	
	# 是否在保护期
	IF EXISTS (SELECT 1 FROM production_time_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=3) THEN
		SET _result = -20;
		LEAVE label_body;
	END IF;
	
	SELECT can_steal,steal_percent,steal_num INTO _can_steal,_steal_percent,_steal_num_max FROM excel_goldore_production WHERE level=_level;
	# 不能偷
	IF _can_steal=0 THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	# 偷窃次数是否已满
	IF NOT EXISTS (SELECT 1 FROM buildings_ext_data WHERE account_id=_account_id AND auto_id=_auto_id AND data_0<_steal_num_max) THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
	# 是否已经偷过了
	IF EXISTS (SELECT 1 FROM relation_events WHERE account_id=_account_id AND p_account_id=_thief_account_id AND auto_id=_auto_id AND type=2) THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;
	# 计算偷窃数量
	SELECT CAST(production*_steal_percent AS UNSIGNED) INTO _steal_gold FROM production_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=1;

	START TRANSACTION;
	# 扣除黄金
	UPDATE production_events SET production=production-_steal_gold WHERE account_id=_account_id AND auto_id=_auto_id AND type=1 AND production>=_steal_gold;
	IF ROW_COUNT()=0 THEN
		SET _result = -10;
		LEAVE label_body;
	END IF;
	# 增加黄金
	UPDATE common_characters SET gold=gold+_steal_gold WHERE account_id=_thief_account_id;
	# 增加偷窃次数
	UPDATE buildings_ext_data SET data_0=data_0+1 WHERE account_id=_account_id AND auto_id=_auto_id;
	# 增加偷窃事件
	INSERT INTO relation_events (account_id,p_account_id,type,auto_id) VALUES (_account_id,_thief_account_id,2,_auto_id);
END label_body;
	SELECT _result,_steal_gold,_steal_percent,_steal_num_max;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_relation_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_relation_log`;
DELIMITER ;;
CREATE PROCEDURE `add_relation_log`(IN `_account_id` bigint,IN `_p_account_id` bigint,IN `_type` int,IN `_text` varchar(256))
BEGIN
	DECLARE _log_id INT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;
	SET _log_id = new_private_log_id(_account_id);
	SET _time = UNIX_TIMESTAMP();
	INSERT INTO relation_logs (account_id,log_id,type,p_account_id,text,time) VALUES (_account_id,_log_id,_type,_p_account_id,_text,_time);
	SELECT _log_id,_time;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_hero_exp`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_hero_exp`;
DELIMITER ;;
CREATE PROCEDURE `add_hero_exp`(IN `_account_id` bigint unsigned,IN `_hero_id` bigint unsigned,IN `_exp` int unsigned,OUT `_rst` int)
BEGIN
	DECLARE _cur_exp INT UNSIGNED DEFAULT 0;
	DECLARE _cur_level INT UNSIGNED DEFAULT 0;
	DECLARE _cur_max_exp INT UNSIGNED DEFAULT 0;
	DECLARE _max_level INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;

	SET _rst=0;

	label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM hire_heros WHERE hero_id=_hero_id AND account_id=_account_id) THEN
		LEAVE label_body;
	END IF;
	SELECT level,exp INTO _cur_level,_cur_exp FROM hire_heros WHERE hero_id=_hero_id;
	
	SELECT MAX(level) INTO _max_level FROM excel_hero_level_exp;
	SELECT IF(_max_level>level,level,_max_level) INTO _max_level FROM common_characters WHERE account_id=_account_id;
	IF _cur_level>=_max_level THEN
		LEAVE label_body;
	END IF;
	SELECT exp INTO _cur_max_exp FROM excel_hero_level_exp WHERE level=_cur_level+1;
	
	IF (_cur_exp+_exp)<_cur_max_exp THEN
		# 不需要升级
		SET _cur_exp=_cur_exp+_exp;
	ELSE
		# 升级了
		SET _cur_exp=_cur_exp+_exp-_cur_max_exp;
		SET _cur_level=_cur_level+1;
		CALL game_levelup_hero(_account_id,_hero_id,0,_rst);
		IF _rst=0 THEN
			REPEAT
				# 当期等级是否已达上限
				IF _cur_level>=_max_level THEN
					LEAVE label_body;
				END IF;

				# 需要看看是否连续升级
				SELECT exp INTO _cur_max_exp FROM excel_hero_level_exp WHERE level=_cur_level+1;
				IF (_cur_exp<_cur_max_exp) THEN
					SET _done=1;
				ELSE
					SET _cur_exp=_cur_exp-_cur_max_exp;
					SET _cur_level=_cur_level+1;
					CALL game_levelup_hero(_account_id,_hero_id,0,_rst);
					IF _rst!=0 THEN
						SET _rst=-1;	# 升级失败
						LEAVE label_body;
					END IF;
				END IF;
			UNTIL _done END REPEAT;
		ELSE
			SET _rst=-1;	# 升级失败
		END IF;
	END IF;

	END label_body;
	IF _rst=0 THEN
		IF _cur_level>=_max_level THEN
			SET _cur_exp	= 0;
		END IF;
		UPDATE hire_heros SET exp=_cur_exp WHERE hero_id=_hero_id;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_training_time_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_training_time_event`;
DELIMITER ;;
CREATE PROCEDURE `add_training_time_event`(IN `_account_id` bigint unsigned,IN `_hero_ids` varchar(1024),IN `_time_unit_num` int  unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;
	DECLARE _training_hall_level INT UNSIGNED DEFAULT 0;
	DECLARE _max_hero_training_level INT UNSIGNED DEFAULT 0;
	DECLARE _max_hero_training_time INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _hero_level INT UNSIGNED DEFAULT 0;
	DECLARE _hero_status INT UNSIGNED DEFAULT 0;
	DECLARE _training_exp INT UNSIGNED DEFAULT 0;
	DECLARE _training_gold INT UNSIGNED DEFAULT 0;
	DECLARE _exp INT UNSIGNED DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _time_itv INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT hero_id,level,status,training_exp,training_gold FROM tmp_training_info;
	DECLARE _cursor_te CURSOR FOR SELECT hero_id,exp FROM tmp_training_info;
	
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_gold; END;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_gold; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_gold; END;

	# 创建临时表
	DROP TEMPORARY TABLE IF EXISTS `tmp_training_info`;
	CREATE TEMPORARY TABLE `tmp_training_info`(
		hero_id BIGINT UNSIGNED NOT NULL,
		level INT UNSIGNED NOT NULL,
		status INT UNSIGNED NOT NULL,
		training_exp INT UNSIGNED NOT NULL,
		training_gold INT UNSIGNED NOT NULL,
		exp INT UNSIGNED NOT NULL,
		PRIMARY KEY(`hero_id`)
	);

	SET @query	= CONCAT('INSERT INTO tmp_training_info SELECT h.hero_id,h.level,h.status,e.training_exp,e.training_gold,0 FROM hire_heros h JOIN excel_hero_level_exp e ON h.level=e.level WHERE h.account_id=', _account_id,' AND h.hero_id IN (', _hero_ids, ')');
	PREPARE smt FROM @query;
	EXECUTE smt;
	DEALLOCATE PREPARE smt;

	# 可训练的最大武将等级
	SELECT e.hero_level_max,e.training_max_time INTO _max_hero_training_level,_max_hero_training_time FROM buildings b JOIN excel_training_hall e ON b.level=e.level WHERE b.account_id=_account_id AND b.excel_id=4;

	label_body:BEGIN
	SET _done=0;
	OPEN _cursor;
		REPEAT
			FETCH _cursor INTO _hero_id,_hero_level,_hero_status,_training_exp,_training_gold;
			IF NOT _done THEN
				IF _hero_status!=0 THEN
					SET _result=-1;	# 将领状态不对,不能修炼
					LEAVE label_body;
				END IF;
				IF _hero_level>_max_hero_training_level THEN
					SET _result=-2;	# 修炼馆等级太低,不能修炼
					LEAVE label_body;
				END IF;
				IF _time_unit_num > _max_hero_training_time THEN
					SET _result=-3;	# 修炼时间太长,不能修炼
					LEAVE label_body;
				END IF;
				# 增加金钱
				SET _gold=_gold+_training_gold*_time_unit_num;
				# 修改获取的经验
				UPDATE tmp_training_info SET exp=_training_exp*_time_unit_num WHERE hero_id=_hero_id;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;
	
	START TRANSACTION;
	# 修改金钱
	UPDATE common_characters SET gold=gold-_gold WHERE account_id=_account_id AND gold>=_gold;
	IF ROW_COUNT()=0 THEN
		SET _result = -10;
		LEAVE label_body;
	END IF;
	# 插入时间事件
	SET _time = UNIX_TIMESTAMP();
	SET _time_itv = _time_unit_num*3600;

	SET _done=0;
	OPEN _cursor_te;
		REPEAT
			FETCH _cursor_te INTO _hero_id,_exp;
			IF NOT _done THEN
				SET _event_id = game_global_id();
				INSERT INTO time_events (event_id,begin_time,end_time,type,locked) VALUES (_event_id,_time,_time+_time_itv,6,0);
				IF ROW_COUNT()=0 THEN
					SET _result = -11;
					ROLLBACK;
					LEAVE label_body;
				END IF;
				INSERT INTO hero_training_time_events (event_id,account_id,hero_id,exp) VALUES (_event_id,_account_id,_hero_id,_exp);
				IF ROW_COUNT()=0 THEN
					SET _result = -12;
					ROLLBACK;
					LEAVE label_body;
				END IF;
				# 修改状态
				UPDATE hire_heros SET status=1000 WHERE hero_id=_hero_id AND status=0;
				IF ROW_COUNT()=0 THEN
					SET _result = -13;
					ROLLBACK;
					LEAVE label_body;
				END IF;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor_te;

	END label_body;
	SELECT _result,_gold;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_training_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_training_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_training_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _exp INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-100; SELECT _result,_account_id,_hero_id,_exp; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-101; SELECT _result,_account_id,_hero_id,_exp; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-102; SELECT _result,_account_id,_hero_id,_exp; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET @_result=_release_te_lock(_event_id); SET _result=-103; SELECT _result,_account_id,_hero_id,_exp; END;
	
label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;

	SELECT account_id,hero_id,exp INTO _account_id,_hero_id,_exp FROM hero_training_time_events WHERE event_id=_event_id;
	
	START TRANSACTION;
	CALL add_hero_exp(_account_id,_hero_id,_exp,_result);
	IF _result!=0 THEN
		SET _result = -1;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	UPDATE hire_heros SET status=0 WHERE hero_id=_hero_id AND status=1000;
	IF ROW_COUNT()=0 THEN
		SET _result = -2;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	DELETE FROM time_events WHERE event_id=_event_id;
	DELETE FROM hero_training_time_events WHERE event_id=_event_id;
END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result=_release_te_lock(_event_id);
	END IF;
	SELECT _result,_account_id,_hero_id,_exp;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `exit_training`
-- ----------------------------
DROP PROCEDURE IF EXISTS `exit_training`;
DELIMITER ;;
CREATE PROCEDURE `exit_training`(IN `_account_id` bigint unsigned,IN `_hero_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _exp INT UNSIGNED DEFAULT 0;
	DECLARE _training_percent FLOAT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_training_percent,_exp; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_training_percent,_exp; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_training_percent,_exp; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_training_percent,_exp; END;

	label_body:BEGIN
	SELECT event_id,exp INTO _event_id,_exp FROM hero_training_time_events WHERE hero_id=_hero_id AND account_id=_account_id;
	SELECT (UNIX_TIMESTAMP()-begin_time)/(end_time-begin_time) INTO _training_percent FROM time_events WHERE event_id=_event_id;
	IF _training_percent<0 THEN
		SET _training_percent=0;
	ELSE
		IF _training_percent>1 THEN
			SET _training_percent=1;
		END IF;
	END IF;
	SET _exp=_training_percent*_exp;

	START TRANSACTION;
	# 增加经验
	CALL add_hero_exp(_account_id, _hero_id,_exp,_result);
	IF _result!=0 THEN
		SET _result=-1;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 删除时间事件
	DELETE FROM time_events WHERE event_id=_event_id;
	DELETE FROM hero_training_time_events WHERE event_id=_event_id;
	# 恢复状态
	UPDATE hire_heros SET status=0 WHERE hero_id=_hero_id AND status=1000;
	IF ROW_COUNT()=0 THEN
		SET _result=-2;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	END label_body;
	SELECT _result,_training_percent,_exp;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `contribute_alliance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `contribute_alliance`;
DELIMITER ;;
CREATE PROCEDURE `contribute_alliance`(IN `_account_id` bigint unsigned,IN `_gold` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _gold_cost_unit INT UNSIGNED DEFAULT 0;
	DECLARE _contribute_num INT UNSIGNED DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_contribute_num,_gold,_alliance_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_contribute_num,_gold,_alliance_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_contribute_num,_gold,_alliance_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_contribute_num,_gold,_alliance_id; END;

	SET @_congress_level = 0;

label_body:BEGIN
	SELECT gold INTO _gold_cost_unit FROM excel_alliance_contribute_rate;
	SET _contribute_num = FLOOR(_gold/_gold_cost_unit);
	SET _gold=_contribute_num*_gold_cost_unit;
	
	IF _contribute_num=0 THEN
		SET _result=-1;	#不能兑换0贡献度
		LEAVE label_body;
	END IF;

	SELECT alliance_id INTO _alliance_id FROM common_characters WHERE account_id=_account_id;
	IF _alliance_id=0 THEN
		SET _result=-2;	#联盟ID为0
		LEAVE label_body;
	END IF;

	START TRANSACTION;
	UPDATE common_characters SET gold=gold-_gold WHERE account_id=_account_id AND gold>=_gold;
	IF ROW_COUNT()=0 THEN
		SET _result=-3;	#黄金不够
		LEAVE label_body;
	END IF;
	UPDATE alliance_members SET development=development+_contribute_num,total_development=total_development+_contribute_num WHERE account_id=_account_id;
	IF ROW_COUNT()=0 THEN
		SET _result=-4; #个人贡献度添加失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 计算联盟议会当前等级
	SELECT total_development INTO @_total_development FROM alliances WHERE alliance_id=_alliance_id;
	SELECT MAX(level) INTO @_congress_level FROM excel_alliance_congress WHERE development<=(@_total_development+_contribute_num);
	UPDATE alliance_buildings SET level=@_congress_level WHERE alliance_id=_alliance_id AND excel_id=1;
	UPDATE alliances SET development=development+_contribute_num,total_development=total_development+_contribute_num WHERE alliance_id=_alliance_id;
		IF ROW_COUNT()=0 THEN
		SET _result=-5; #联盟贡献度添加失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_contribute_num,_gold,_alliance_id,@_congress_level;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `convert_diamond_gold`
-- ----------------------------
DROP PROCEDURE IF EXISTS `convert_diamond_gold`;
DELIMITER ;;
CREATE PROCEDURE `convert_diamond_gold`(IN `_account_id` bigint unsigned,IN `_diamond` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _convert_rate INT UNSIGNED DEFAULT 0;
	
label_body:BEGIN
	IF _diamond=0 THEN
		SET _result=-1;	#兑换的钻石为0
		LEAVE label_body;
	END IF;
	SELECT gold INTO _convert_rate FROM excel_store_conversion_rate WHERE money_type=1;
	SET _gold=_convert_rate*_diamond;
	UPDATE common_characters SET diamond=diamond-_diamond,gold=gold+_gold WHERE account_id=_account_id AND diamond>=_diamond;
	IF ROW_COUNT()=0 THEN
		SET _result=-2; #钻石不够
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_gold;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `join_alliance_name`
-- ----------------------------
DROP PROCEDURE IF EXISTS `join_alliance_name`;
DELIMITER ;;
CREATE PROCEDURE `join_alliance_name`(IN `_account_id` bigint unsigned,IN `_alliance_name` varchar(32))
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_alliance_id = 0;

label_body:BEGIN
	# 该联盟是否存在
	IF NOT EXISTS (SELECT 1 FROM alliances WHERE name=_alliance_name AND (@_alliance_id:=alliance_id)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 是否已经有联盟了
	IF EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND alliance_id<>0) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 是否已经申请过了
	IF EXISTS (SELECT 1 FROM alliance_join_events WHERE account_id=_account_id) THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	# 成员上限是否够
	SELECT count(*) INTO @_member_num FROM alliance_members WHERE alliance_id=@_alliance_id;
	# 是否还有空余位置
	IF NOT EXISTS (SELECT 1 FROM alliance_buildings b JOIN excel_alliance_congress c ON b.level=c.level WHERE alliance_id=@_alliance_id AND excel_id=1 AND c.member_num>@_member_num) THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;	INSERT INTO alliance_join_events VALUES (_account_id, @_alliance_id);
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,@_alliance_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `set_alliance_introduction`
-- ----------------------------
DROP PROCEDURE IF EXISTS `set_alliance_introduction`;
DELIMITER ;;
CREATE PROCEDURE `set_alliance_introduction`(IN `_account_id` bigint unsigned,IN `_introduction` blob)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _position INT UNSIGNED DEFAULT 0;
	DECLARE _can_set INT UNSIGNED DEFAULT 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alliance_members WHERE account_id=_account_id) THEN
		SET _result = -1; # 不是联盟成员
		LEAVE label_body;
	END IF;
	SELECT alliance_id,position INTO _alliance_id,_position FROM alliance_members WHERE account_id=_account_id;
	SELECT can_change_introduction INTO _can_set FROM excel_alliance_position_right WHERE excel_id=_position;
	IF _can_set=0 THEN
		SET _result = -2; # 权限不够
		LEAVE label_body;
	END IF;
	UPDATE alliances SET introduction=_introduction WHERE alliance_id=_alliance_id;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_other_christmas_tree_info`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_other_christmas_tree_info`;
DELIMITER ;;
CREATE PROCEDURE `get_other_christmas_tree_info`(IN `_account_id` bigint unsigned,IN `_peer_account_id` bigint unsigned)
BEGIN
	DECLARE _can_water INT UNSIGNED DEFAULT 0;
	DECLARE _ripe INT UNSIGNED DEFAULT 0;
	DECLARE _begin_time INT UNSIGNED DEFAULT 0;
	DECLARE _end_time INT UNSIGNED DEFAULT 0;
	DECLARE _watered_num INT UNSIGNED DEFAULT 0;

	# 圣诞树已经成熟,不需要浇水
	IF EXISTS (SELECT 1 FROM production_events WHERE account_id=_peer_account_id AND type=2) THEN
		SET _ripe = 1;
	ELSE
		# 圣诞树成熟时间事件,不能浇水
		IF EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=_peer_account_id AND type=1) THEN
			SELECT t.begin_time,t.end_time INTO _begin_time,_end_time FROM once_perperson_time_events o JOIN time_events t ON o.event_id=t.event_id WHERE o.account_id=_peer_account_id AND o.type=1;
		ELSE
			# 获取已经浇水次数
			SELECT data_0 INTO _watered_num FROM buildings_ext_data WHERE account_id=_peer_account_id AND auto_id=400;
			# 是否已经浇过水了
			IF NOT EXISTS (SELECT 1 FROM relation_events WHERE account_id=_peer_account_id AND p_account_id=_account_id AND type=1) THEN
				SET _can_water = 1;
			END IF;
		END IF;
	END IF;

	SELECT _can_water,_ripe,_begin_time,IF(_begin_time>0,_end_time-UNIX_TIMESTAMP(),0),_watered_num;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `check_draw_lottery`
-- ----------------------------
DROP PROCEDURE IF EXISTS `check_draw_lottery`;
DELIMITER ;;
CREATE PROCEDURE `check_draw_lottery`(IN `_account_id` bigint unsigned,IN `_money_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _price INT UNSIGNED DEFAULT 0;

label_body:BEGIN
	CASE
		# money_type:0->none,1->diamond,2->crystal,3->gold,4->alliance_contribute
		WHEN _money_type=0 THEN
			UPDATE common_characters SET draw_lottery_num=draw_lottery_num-1 WHERE account_id=_account_id AND draw_lottery_num>0;
			IF ROW_COUNT() = 0 THEN
				SET _result = -1; # 免费抽奖次数已经用完
				LEAVE label_body;
			END IF;
		WHEN _money_type=1 THEN
			SELECT paid_draw_price INTO _price FROM excel_lottery_cfg;
			UPDATE common_characters SET diamond=diamond-_price WHERE account_id=_account_id AND diamond>=_price;
			IF ROW_COUNT() = 0 THEN
				SET _result = -2; # 钻石不够
				LEAVE label_body;
			END IF;
		WHEN _money_type=2 THEN
			SELECT paid_draw_price INTO _price FROM excel_lottery_cfg;
			UPDATE common_characters SET crystal=crystal-_price WHERE account_id=_account_id AND crystal>=_price;
			IF ROW_COUNT() = 0 THEN
				SET _result = -3; # 水晶不够
				LEAVE label_body;
			END IF;
		WHEN _money_type=4 THEN
			IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND (@_lottery_num:=alliance_lottery_num)>=0 AND alliance_id>0) THEN
				SET _result = -5; # 没有联盟
				LEAVE label_body;
			END IF;
			IF @_lottery_num=0 THEN
				SET _result = -6; # 抽奖次数为0
				LEAVE label_body;
			END IF;
			SELECT paid_draw_alliance_contribute INTO _price FROM excel_lottery_cfg;
			UPDATE alliance_members SET development=development-_price WHERE account_id=_account_id AND development>=_price;
			IF ROW_COUNT() = 0 THEN
				SET _result = -4; # 联盟贡献度不够
				LEAVE label_body;
			END IF;
			UPDATE common_characters SET alliance_lottery_num=alliance_lottery_num-1 WHERE account_id=_account_id AND alliance_lottery_num>0;
		ELSE
			SET _result = -10;	# 抽奖类型不对，支持三种,0->免费抽奖,1->钻石抽奖,2->水晶抽奖
			LEAVE label_body;
	END CASE;
END label_body;
	SELECT _result,_price;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `fetch_lottery`
-- ----------------------------
DROP PROCEDURE IF EXISTS `fetch_lottery`;
DELIMITER ;;
CREATE PROCEDURE `fetch_lottery`(IN `_account_id` bigint unsigned,IN `_type` int unsigned,IN `_data` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @vItem = 0;
	SET @_item_name = '';
	
label_body:BEGIN
	# 发送奖励
	CASE
		WHEN _type=1 THEN			# 道具(lottery_type_item) 
			CALL game_add_item(_account_id, 0, _data, 1, 0, _result);
			IF _result < 1 THEN
				SET _result = -1;
				LEAVE label_body;
			END IF;
			SELECT IFNULL(name,'') INTO @_item_name FROM excel_item_list WHERE excel_id=_data;
			SET _result = 0;
		WHEN _type=2 THEN			# 水晶(lottery_type_crystal)
			UPDATE common_characters SET crystal=crystal+_data WHERE account_id=_account_id;
		WHEN _type=3 THEN			# 黄金(lottery_type_gold)
			UPDATE common_characters SET gold=gold+_data WHERE account_id=_account_id;
		ELSE
			SET _result = -2;
	END CASE;

	# 记录日志及是否需要广播
	SELECT top_log_value,notify_world_value INTO @_top_log_value,@_notify_world_value FROM excel_lottery_cfg;
	SET @_value = 0;
	CASE _type
		WHEN 1 THEN
			IF NOT EXISTS (SELECT 1 FROM excel_store_item WHERE excel_id=_data AND (@_value:=diamond)>=0) THEN
				SELECT theroy_diamond INTO @_value FROM excel_item_list WHERE excel_id=_data;
			END IF;
		WHEN 2 THEN
			SET @_value = _data;
		WHEN 3 THEN
			SELECT FLOOR(_data/gold) INTO @_value FROM excel_store_conversion_rate WHERE money_type=1;
		ELSE
			SET @_value = 0;
	END CASE;
	
	IF @_value>=@_top_log_value THEN
		SET @_time_now	= UNIX_TIMESTAMP();
		INSERT INTO top_lottery_reward_log (account_id,type,data,value,time) VALUES (_account_id,_type,_data,@_value,@_time_now);
	END IF;

	SELECT MAX(id) INTO @_lottery_id FROM unfetched_lottery WHERE account_id=_account_id AND type=_type AND data=_data;
	DELETE FROM unfetched_lottery WHERE id=@_lottery_id AND account_id=_account_id;

END label_body;
	# 执行结果,道具名字,道具价值,是否记录入顶级奖励日志,是否广播给世界,item_id
	SELECT _result,@_item_name,@_value,@_value>=@_top_log_value,@_value>=@_notify_world_value,@vItem;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_christmas_tree_info`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_christmas_tree_info`;
DELIMITER ;;
CREATE PROCEDURE `get_christmas_tree_info`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _can_water INT UNSIGNED DEFAULT 0;
	DECLARE _ripe INT UNSIGNED DEFAULT 0;
	DECLARE _begin_time INT UNSIGNED DEFAULT 0;
	DECLARE _end_time INT UNSIGNED DEFAULT 0;
	DECLARE _watered_num INT UNSIGNED DEFAULT 0;
	DECLARE _can_watered_time INT UNSIGNED DEFAULT 0;
	DECLARE _watering_interval INT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;

	# 圣诞树已经成熟,不需要浇水
	IF EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND type=2) THEN
		SET _ripe = 1;
	ELSE
		# 圣诞树成熟时间事件,不能浇水
		IF EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=_account_id AND type=1) THEN
			SELECT t.begin_time,t.end_time INTO _begin_time,_end_time FROM once_perperson_time_events o JOIN time_events t ON o.event_id=t.event_id WHERE o.account_id=_account_id AND o.type=1;
		ELSE
			# 获取已经浇水次数
			SELECT data_0,data_1 INTO _watered_num,_can_watered_time FROM buildings_ext_data WHERE account_id=_account_id AND auto_id=400;
			SET _time=UNIX_TIMESTAMP();
			IF _time>=_can_watered_time THEN
				SET _can_water = 1;
				SET _time=0;	#浇水倒计时 
			ELSE
				SET _can_water = 0;
				SET _time=_can_watered_time-_time; #浇水倒计时
			END IF;
		END IF;
	END IF;

	SELECT _can_water,_ripe,_begin_time,IF(_begin_time>0,_end_time-UNIX_TIMESTAMP(),0),_watered_num,_time;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_other_goldore_info`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_other_goldore_info`;
DELIMITER ;;
CREATE PROCEDURE `get_other_goldore_info`(IN `_account_id` bigint unsigned,IN `_my_account_id` bigint unsigned)
BEGIN
	DECLARE _auto_id INT UNSIGNED DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;
	DECLARE _count_down_time INT UNSIGNED DEFAULT 0;
	DECLARE _state INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;
	DECLARE _time_now INT UNSIGNED DEFAULT 0;
	DECLARE _steal_num INT UNSIGNED DEFAULT 0;
	DECLARE _steal_num_max INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT auto_id,level FROM buildings WHERE account_id=_account_id AND excel_id=11;

	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	# 创建临时表
	DROP TEMPORARY TABLE IF EXISTS `tmp_other_goldore_info`;
	CREATE TEMPORARY TABLE `tmp_other_goldore_info`(
		auto_id INT UNSIGNED NOT NULL,
		level INT UNSIGNED NOT NULL,
		count_down_time INT UNSIGNED NOT NULL,
		state INT UNSIGNED NOT NULL,
		PRIMARY KEY(`auto_id`)
	);

	SET _done=0;
	SET _time_now=UNIX_TIMESTAMP();
	OPEN _cursor;
		REPEAT
			SET _count_down_time=0;
			SET _state=0;
			FETCH _cursor INTO _auto_id,_level;
			IF NOT _done THEN
				SET @_end_time=0;
				IF EXISTS (SELECT 1 FROM production_time_events e JOIN time_events t ON e.event_id=t.event_id WHERE e.account_id=_account_id AND e.auto_id=_auto_id AND e.type=1 AND (@_end_time:=t.end_time)) THEN
					IF _time_now>=@_end_time THEN
						SET _count_down_time=0;
						SET _state=2;
					ELSE
						SET _count_down_time=@_end_time-_time_now;
						SET _state=1;
					END IF;
				ELSE
					# 是否已经成熟
					IF EXISTS (SELECT 1 FROM production_events WHERE account_id=_account_id AND auto_id=_auto_id AND type=1) THEN
						# 是否偷过
						IF NOT EXISTS (SELECT 1 FROM relation_events WHERE account_id=_account_id AND p_account_id=_my_account_id AND auto_id=_auto_id AND type=2) THEN
							# 是否偷取次数已经达到上限
							SELECT steal_num INTO _steal_num_max FROM excel_goldore_production LIMIT 1;
							SELECT data_0 INTO _steal_num FROM buildings_ext_data WHERE account_id=_account_id AND auto_id=_auto_id;
							IF _steal_num<_steal_num_max THEN
								SET _state=2;
							END IF;
						END IF;
					END IF;
				END IF;
				INSERT INTO tmp_other_goldore_info (auto_id,level,count_down_time,state) VALUES (_auto_id,_level,_count_down_time,_state);
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;

	SELECT auto_id,level,count_down_time,state FROM tmp_other_goldore_info;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_others_goldore_smp_info`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_others_goldore_smp_info`;
DELIMITER ;;
CREATE PROCEDURE `get_others_goldore_smp_info`(IN `_account_id` bigint unsigned,IN `_account_ids` blob)
BEGIN
	DECLARE _count INT DEFAULT 0;
	DECLARE _total_char_count INT DEFAULT 0;
	DECLARE _can_water INT UNSIGNED DEFAULT 0;
	DECLARE _can_fetch_gold INT UNSIGNED DEFAULT 0;
	
	# 创建临时表
	DROP TEMPORARY TABLE IF EXISTS `tmp_others_goldore_smp_info`;
	CREATE TEMPORARY TABLE `tmp_others_goldore_smp_info`(
		account_id BIGINT UNSIGNED NOT NULL,
		can_water INT UNSIGNED NOT NULL,
		can_fetch_gold INT UNSIGNED NOT NULL,
		PRIMARY KEY(`account_id`)
	);

	# 偷窃最大数目
	SELECT steal_num INTO @_tmp_steal_max_num FROM excel_goldore_production LIMIT 1;
	
	SET _total_char_count = _get_char_count(_account_ids, '*');
	WHILE _count<=_total_char_count DO
		SET _can_water=0;
		SET _can_fetch_gold=0;
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_account_ids,'*',_count+1),'*',-1) INTO @_tmp_account_id;

		# 1.金矿是否可以偷取
		SELECT GROUP_CONCAT(auto_id) INTO @_tmp_auto_ids FROM production_events WHERE account_id=@_tmp_account_id AND type=1;
		IF ISNULL(@_tmp_auto_ids)=0 THEN
			SET @_tmp_count=0;
			SET @_tmp_total_count=_get_char_count(@_tmp_auto_ids, ',');
			SET @_done=0;
			# 看看是否存在有还可以偷取的金矿
			WHILE @_tmp_count<=@_tmp_total_count AND @_done=0 DO
				SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_tmp_auto_ids,',',@_tmp_count+1),',',-1) INTO @_tmp_auto_id;
				# 是否已经偷取过
				IF NOT EXISTS (SELECT 1 FROM relation_events WHERE account_id=@_tmp_account_id AND p_account_id=_account_id AND auto_id=@_tmp_auto_id AND type=2) THEN
					# 偷取次数是否已达上限
					SELECT data_0 INTO @_tmp_steal_num FROM buildings_ext_data WHERE account_id=@_tmp_account_id AND auto_id=@_tmp_auto_id;
					IF @_tmp_steal_num<@_tmp_steal_max_num THEN
						SET _can_fetch_gold=1;
						SET @_done=1;
					END IF;
				END IF;
				SET @_tmp_count=@_tmp_count+1;
			END WHILE;
		END IF;

		# 2.是否可以给许愿树浇水
		# 是否有成熟事件
		IF NOT EXISTS (SELECT 1 FROM production_events WHERE account_id=@_tmp_account_id AND auto_id=400 AND type=2) THEN
			# 是否有即将成熟倒计时
			IF NOT EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=@_tmp_account_id AND type=1) THEN
				# 是否已经浇过水
				IF NOT EXISTS (SELECT 1 FROM relation_events WHERE account_id=@_tmp_account_id AND p_account_id=_account_id AND type=1) THEN
					SET _can_water=1;
				END IF;
			END IF;
		END IF;
		
		INSERT INTO tmp_others_goldore_smp_info (account_id,can_water,can_fetch_gold) VALUES (@_tmp_account_id,_can_water,_can_fetch_gold);
		SET _count=_count+1;
	END WHILE;

	SELECT account_id,can_water,can_fetch_gold FROM tmp_others_goldore_smp_info;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alliance_trade`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alliance_trade`;
DELIMITER ;;
CREATE PROCEDURE `alliance_trade`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _alliance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _trade_num INT UNSIGNED DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _trade_time INT UNSIGNED DEFAULT 0;
	DECLARE _added_trade_rate FLOAT UNSIGNED DEFAULT 0;
	DECLARE _time_now INT UNSIGNED DEFAULT 0;
	DECLARE _event_id BIGINT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_event_id,_gold,_alliance_id,_trade_time,_trade_num; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,_event_id,_gold,_alliance_id,_trade_time,_trade_num; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,_event_id,_gold,_alliance_id,_trade_time,_trade_num; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_event_id,_gold,_alliance_id,_trade_time,_trade_num; END;

label_body:BEGIN
	# 1.联盟ID是否为0
	SELECT alliance_id,trade_num INTO _alliance_id,_trade_num FROM common_characters WHERE account_id=_account_id;
	IF _alliance_id=0 THEN
		SET _result = -1; # 联盟不存在
		LEAVE label_body;
	END IF;

	# 2.跑商次数是否用完
	IF _trade_num=0 THEN
		SET _result = -2; # 跑商次数用完
		LEAVE label_body;
	END IF;

	# 3.获取大使馆等级
	IF NOT EXISTS (SELECT 1 FROM buildings WHERE account_id=_account_id AND excel_id=5 AND (@_embassy_level:=level)) THEN
		SET _result = -3; # 大使馆不存在
		LEAVE label_body;
	END IF;

	# 4.获取联盟等级
	IF NOT EXISTS (SELECT 1 FROM alliance_buildings WHERE alliance_id=_alliance_id AND excel_id=1 AND (@_alliance_level:=level)) THEN
		SET _result = -4; # 议会不存在
		LEAVE label_body;
	END IF;

	# 5.上次跑商结束否
	IF EXISTS (SELECT 1 FROM trade_time_events WHERE account_id=_account_id) THEN
		SET _result = -5; # 上次跑商还未结束
		LEAVE label_body;
	END IF;

	# 6.获取单次跑商获取的金钱和所费时间
	SELECT gold,time INTO _gold,_trade_time FROM excel_embassy_cfg WHERE level<=@_embassy_level ORDER BY level DESC LIMIT 1;
	SELECT added_trade_rate INTO _added_trade_rate FROM excel_alliance_congress WHERE level<=@_alliance_level ORDER BY level DESC LIMIT 1;
	SET _gold=_gold*(1+_added_trade_rate);

	START TRANSACTION;
	UPDATE common_characters SET trade_num=trade_num-1 WHERE account_id=_account_id AND trade_num>0;
	IF ROW_COUNT()=0 THEN
		SET _result = -2; # 跑商次数用完
		LEAVE label_body;
	END IF;
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	INSERT INTO trade_time_events (event_id,account_id,gold) VALUES (_event_id,_account_id,_gold);
	IF ROW_COUNT()=0 THEN
		SET _result = -6; # 时间事件数据插入失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO time_events (event_id,begin_time,end_time,type,locked) VALUES (_event_id,_time_now,_time_now+_trade_time,7,0);
	IF ROW_COUNT()=0 THEN
		SET _result = -7; # 时间事件插入失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_event_id,_gold,_alliance_id,_trade_time,_trade_num;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_alliance_trade_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_alliance_trade_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_alliance_trade_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;
	SET @_account_id = 0;
	SET @_gold = 0;
	IF NOT EXISTS (SELECT 1 FROM trade_time_events WHERE event_id=_event_id AND (@_account_id:=account_id) AND (@_gold:=gold)) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	
	UPDATE common_characters SET gold=gold+@_gold WHERE account_id=@_account_id;
	DELETE FROM trade_time_events WHERE event_id=_event_id;
	DELETE FROM time_events WHERE event_id=_event_id;
END label_body;
	SELECT _result,@_account_id,@_gold;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_alliance_trade_info`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_alliance_trade_info`;
DELIMITER ;;
CREATE PROCEDURE `get_alliance_trade_info`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	
	DECLARE _excel_trade_num INT UNSIGNED DEFAULT 0;
	DECLARE _excel_trade_gold INT UNSIGNED DEFAULT 0;
	DECLARE _excel_trade_time INT UNSIGNED DEFAULT 0;
	DECLARE _excel_added_rate FLOAT UNSIGNED DEFAULT 0;

	DECLARE _time_now INT UNSIGNED DEFAULT 0;

label_body:BEGIN
	SET @_trade_num = 0;
	SET @_gold = 0;
	SET @_end_time = 0;

	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND (@_alliance_id:=alliance_id)!=0 AND (@_trade_num:=trade_num)>=0) THEN
		SET _result = -1; # 没有联盟
		LEAVE label_body;
	END IF;

	IF NOT EXISTS (SELECT 1 FROM buildings WHERE account_id=_account_id AND excel_id=5 AND (@_embassy_level:=level)) THEN
		SET _result = -2; # 没有大使馆
		LEAVE label_body;
	END IF;

	IF NOT EXISTS (SELECT 1 FROM alliance_buildings WHERE alliance_id=@_alliance_id AND (@_congress_level:=level)) THEN
		SET _result = -3; # 联盟议会不存在
		LEAVE label_body;
	END IF;

	SELECT added_trade_rate INTO _excel_added_rate FROM excel_alliance_congress WHERE level<=@_congress_level ORDER BY level DESC LIMIT 1;
	SELECT trade_num,gold*(1+_excel_added_rate),time INTO _excel_trade_num,_excel_trade_gold,_excel_trade_time FROM excel_embassy_cfg WHERE level<=@_embassy_level ORDER BY level DESC LIMIT 1;

	IF NOT EXISTS (SELECT 1 FROM trade_time_events e JOIN time_events t ON e.event_id=t.event_id WHERE e.account_id=_account_id AND (@_gold:=e.gold) AND (@_end_time:=t.end_time)) THEN
		LEAVE label_body; # 没有跑商时间事件
	END IF;
	
	SET _time_now = UNIX_TIMESTAMP();
END label_body;
	SELECT _result,@_gold,IF(_time_now>=@_end_time,0,@_end_time-_time_now),@_trade_num,_excel_trade_num,_excel_trade_gold,_excel_trade_time;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_enemy`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_enemy`;
DELIMITER ;;
CREATE PROCEDURE `add_enemy`(IN `_account_id` bigint unsigned,IN `_enemy_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	
label_body:BEGIN
	IF _account_id=_enemy_id THEN
		SET _result = -2; # 不能和自己成为敌人
		LEAVE label_body;
	END IF;
	
	#IF NOT EXISTS (SELECT 1 FROM personal_relations WHERE account_id=_account_id AND peer_account_id=_enemy_id AND relation_type=2) THEN
		INSERT IGNORE INTO personal_relations (account_id,peer_account_id,relation_type) VALUES (_account_id,_enemy_id,2);
	#END IF;

	#IF NOT EXISTS (SELECT 1 FROM personal_relations WHERE account_id=_enemy_id AND peer_account_id=_account_id AND relation_type=2) THEN
		INSERT IGNORE INTO personal_relations (account_id,peer_account_id,relation_type) VALUES (_enemy_id,_account_id,2);
	#END IF;

END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_alliance_development`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_alliance_development`;
DELIMITER ;;
CREATE PROCEDURE `add_alliance_development`(IN `_alliance_id` bigint unsigned,IN `_delta_development` int)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _abs_delta_dev INT UNSIGNED DEFAULT 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alliances WHERE alliance_id=_alliance_id AND (@_cur_development:=development)>=0 AND (@_cur_total_development:=total_development)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	IF _delta_development >= 0 THEN
		UPDATE alliances SET development=development+_delta_development,total_development=total_development+_delta_development WHERE alliance_id=_alliance_id;
	ELSE
		SET _abs_delta_dev = ABS(_delta_development);
		# @_cur_total_development>=@_cur_development
		IF @_cur_development>=_abs_delta_dev THEN
			UPDATE alliances SET development=development-_abs_delta_dev,total_development=total_development-_abs_delta_dev WHERE alliance_id=_alliance_id AND development>=_abs_delta_dev AND total_development>=_abs_delta_dev; 
		ELSE
			IF @_cur_total_development>=_abs_delta_dev THEN
				UPDATE alliances SET development=0,total_development=total_development-_abs_delta_dev WHERE alliance_id=_alliance_id AND total_development>=_abs_delta_dev; 
			ELSE 
				UPDATE alliances SET development=0,total_development=0 WHERE alliance_id=_alliance_id;
			END IF;
		END IF;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_rank_data`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_rank_data`;
DELIMITER ;;
CREATE PROCEDURE `get_rank_data`(IN `_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	CASE
		WHEN _type=1 THEN
			# 角色等级排行
			SELECT account_id,development FROM common_characters ORDER BY level DESC,development DESC;
		WHEN _type=2 THEN
			# 角色黄金排行
			SELECT account_id,gold FROM common_characters ORDER BY gold DESC,level DESC,development DESC;
		WHEN _type=3 THEN
			# 角色钻石排行
			SELECT account_id,diamond FROM common_characters ORDER BY diamond DESC,level DESC,development DESC;
		WHEN _type=4 THEN
			# 角色王者之路排行榜
			SELECT account_id,instance_wangzhe FROM common_characters ORDER BY instance_wangzhe DESC,level DESC,development DESC;
		ELSE
			# 错误的_type
			SET _result = -1;
	END CASE;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `recharge_diamond`
-- ----------------------------
DROP PROCEDURE IF EXISTS `recharge_diamond`;
DELIMITER ;;
CREATE PROCEDURE `recharge_diamond`(IN `_account_id` bigint unsigned,IN `_added_diamond` int unsigned,IN `_echo` int unsigned,OUT `_vip` int unsigned,OUT `_gift_diamond` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_total_diamond	= 0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND (@_total_diamond:=total_diamond)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	SELECT IFNULL(MAX(added_diamond),0),IFNULL(MAX(diamond),1) INTO _gift_diamond,@_base_diamond FROM excel_recharge_diamond WHERE diamond<=_added_diamond;
	SELECT MAX(level) INTO _vip FROM excel_vip_right WHERE total_diamond<=(@_total_diamond+_added_diamond);
	
	SET _gift_diamond	= _gift_diamond/@_base_diamond*_added_diamond;
	UPDATE common_characters SET vip=_vip,total_diamond=total_diamond+_added_diamond,diamond=diamond+_added_diamond+_gift_diamond WHERE account_id=_account_id;
END label_body;
	IF _echo=1 THEN
		SELECT _result,_vip,_gift_diamond;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `_calc_accelerate_cost`
-- ----------------------------
DROP PROCEDURE IF EXISTS `_calc_accelerate_cost`;
DELIMITER ;;
CREATE PROCEDURE `_calc_accelerate_cost`(INOUT `_money` int unsigned)
BEGIN
	DECLARE _pre_price INT UNSIGNED DEFAULT 0;
	DECLARE _cur_price INT UNSIGNED DEFAULT 0;
	DECLARE _pre_discount FLOAT UNSIGNED DEFAULT 0;
	DECLARE _cur_discount FLOAT UNSIGNED DEFAULT 0;
	DECLARE _money_dst FLOAT DEFAULT 0;
	DECLARE _money_src FLOAT DEFAULT 0;
	DECLARE _done INT UNSIGNED DEFAULT 0;
	DECLARE _is_begin INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT price,discount FROM excel_accelerate_cost_discount ORDER BY price ASC;

	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	SET _money_src = _money;

	SET _is_begin = 1;
	SET _done=0;
	OPEN _cursor;
		REPEAT
			FETCH _cursor INTO _cur_price,_cur_discount;
			IF NOT _done THEN
				# 最开始的区间是不打折的
				IF _is_begin = 1 THEN
					SET _is_begin = 0;
					IF _money_src > _cur_price THEN # 费用部分在打折区间内
						SET _money_dst = _cur_price;
						SET _money_src = _money_src - _cur_price;
					ELSE # 费用完全在不打折区间内
						SET _money_dst = _money_src;
						SET _money_src = 0;
						SET _done = 1;
					END IF;
				ELSE
					IF _money_src > 0 THEN
					# 打折区间计算(需要使用上次取道的折扣)
						SET @_money_field = _cur_price-_pre_price;
						IF _money_src > @_money_field THEN
							SET _money_dst = _money_dst + @_money_field*_pre_discount;
							SET _money_src = _money_src - @_money_field;
						ELSE
							SET _money_dst = _money_dst + _money_src*_pre_discount;
							SET _money_src = 0;
							SET _done = 1;
						END IF;
					END IF;
				END IF;
				SET _pre_price = _cur_price;
				SET _pre_discount = _cur_discount;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;

	IF _money_src >= 0 THEN
		SET _money_dst = _money_dst + _money_src*_pre_discount; #_pre_discount=_cur_discount
	END IF;

	IF FLOOR(_money_dst) > 0 THEN
		SET _money = CEILING(_money_dst);
	END IF;

END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_population`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_population`;
DELIMITER ;;
CREATE PROCEDURE `add_population`(IN `_account_id` bigint unsigned,IN `_added_population` int unsigned,OUT `_real_added_population` int unsigned)
BEGIN
	SET _real_added_population = 0;

	CALL get_population_production_n_capacity(_account_id, @_result, @_production, @_capacity);
	
	IF @_result=0 THEN
		SELECT population INTO @_population FROM common_characters WHERE account_id=_account_id;
		IF (@_population+_added_population) > @_capacity THEN
			SET _real_added_population = IF(@_capacity>@_population, @_capacity-@_population, 0);
		ELSE
			SET _real_added_population = _added_population;
		END IF;
		UPDATE common_characters SET population=population+_real_added_population WHERE account_id=_account_id;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `change_name`
-- ----------------------------
DROP PROCEDURE IF EXISTS `change_name`;
DELIMITER ;;
CREATE PROCEDURE `change_name`(IN `_account_id` bigint unsigned,IN `_name` varchar(32))
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _free_num INT UNSIGNED DEFAULT 0;
	DECLARE _diamond INT UNSIGNED DEFAULT 0;

	SET @_change_num = 0;
	SET @_old_name = '';
	SET @_cost_diamond = 0;

label_body:BEGIN
	IF EXISTS (SELECT 1 FROM common_characters WHERE name=_name) THEN
		SET _result = -1; # 已经有人取这个名字了
		LEAVE label_body;
	END IF;
	SELECT name,change_name_num INTO @_old_name,@_change_num FROM common_characters WHERE account_id=_account_id;
	SELECT free_num,diamond INTO _free_num,_diamond FROM excel_change_name WHERE excel_id=1;

	IF @_change_num >= _free_num THEN
		SET @_cost_diamond = _diamond;
	END IF;

	UPDATE common_characters SET name=_name,change_name_num=change_name_num+1,diamond=diamond-@_cost_diamond WHERE account_id=_account_id AND diamond>=@_cost_diamond;
	IF ROW_COUNT()=0 THEN
		SET _result = -2; # 金钱不够
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,@_old_name,@_cost_diamond;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `refresh_rank_list` (暂时废弃)
-- ----------------------------
DROP PROCEDURE IF EXISTS `refresh_rank_list`;
DELIMITER ;;
CREATE PROCEDURE `refresh_rank_list`(IN `_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DELETE FROM rank_list WHERE type = _type;
	SET @_rank = 0;
	CASE
		WHEN _type=1 THEN
			# 角色等级排行
			INSERT INTO rank_list SELECT _type,(@_rank:=@_rank+1),account_id,development FROM common_characters ORDER BY level DESC,development DESC;
		WHEN _type=2 THEN
			# 角色黄金排行
			INSERT INTO rank_list SELECT _type,(@_rank:=@_rank+1),account_id,gold FROM common_characters ORDER BY gold DESC;
		WHEN _type=3 THEN
			# 角色钻石排行
			INSERT INTO rank_list SELECT _type,(@_rank:=@_rank+1),account_id,diamond FROM common_characters ORDER BY diamond DESC;
		ELSE
			# 错误的_type
			SET _result = -1;
	END CASE;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `move_position`
-- ----------------------------
DROP PROCEDURE IF EXISTS `move_position`;
DELIMITER ;;
CREATE PROCEDURE `move_position`(IN `_account_id` bigint unsigned,IN `_pos_x` int unsigned,IN `_pos_y` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	IF EXISTS (SELECT 1 FROM common_characters WHERE pos_x=_pos_x AND pos_y=_pos_y) THEN
		SET _result = -1; # 目标位置已经有人了
		LEAVE label_body;
	END IF;

	# 消耗道具
	IF NOT EXISTS (SELECT 1 FROM items WHERE account_id=_account_id AND excel_id=2004 AND (@_item_id:=item_id)>=0 AND num>0) THEN
		SET _result = -2; # 没有迁城卷轴
		LEAVE label_body;
	END IF;
	CALL game_del_item(_account_id, @_item_id, 1, 0, _result);
	IF _result > 0 THEN
		SET _result = 0;
	ELSE
		SET _result = -3; # 扣除道具失败
		LEAVE label_body;
	END IF;

	# 迁移城市
	UPDATE common_characters SET pos_x=_pos_x,pos_y=_pos_y WHERE account_id=_account_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -4; # 更新坐标失败
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_build_num`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_build_num`;
DELIMITER ;;
CREATE PROCEDURE `add_build_num`(IN `_account_id` bigint unsigned, IN `_excel_id` int unsigned, IN `_echo` int unsigned,OUT `_result` int)
BEGIN
	SET _result = 0;

label_body:BEGIN
	# 获取时效
	SELECT time*3600 INTO @_effected_time FROM excel_item_list WHERE excel_id=_excel_id;

	# 添加时间事件
	SET @_time_now = UNIX_TIMESTAMP();
	SET @_event_id = game_global_id();
	START TRANSACTION;
	INSERT INTO time_events VALUES(@_event_id, @_time_now, @_time_now+@_effected_time, 2, 0);
	INSERT INTO once_perperson_time_events VALUES (@_event_id, _account_id, 3);
	IF ROW_COUNT()=0 THEN
		SET _result = -1;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	IF _echo!=0 THEN
		SELECT _result;
	END IF;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `write_signature`
-- ----------------------------
DROP PROCEDURE IF EXISTS `write_signature`;
DELIMITER ;;
CREATE PROCEDURE `write_signature`(IN `_account_id` bigint unsigned,IN `_signature` blob)
BEGIN
	UPDATE common_characters SET signature=_signature WHERE account_id=_account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `get_build_num`
-- ----------------------------
DROP FUNCTION IF EXISTS `get_build_num`;
DELIMITER ;;
CREATE FUNCTION `get_build_num`(`_account_id` bigint unsigned) RETURNS int(10) unsigned
BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND (@_char_build_num:=build_num+added_build_num)>=0 AND (@_vip:=vip)>=0) THEN
		RETURN 0;
	END IF;
	SELECT added_build_num INTO @_vip_build_num FROM excel_vip_right WHERE level=@_vip;
	SET @_item_build_num = 0;
	IF EXISTS (SELECT 1 FROM once_perperson_time_events WHERE account_id=_account_id AND type=3) THEN
		SELECT val_int INTO @_item_build_num FROM excel_item_list WHERE excel_id=2012;
	END IF;
	RETURN @_char_build_num + @_vip_build_num + @_item_build_num;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_one_91pay`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_one_91pay`;
DELIMITER ;;
CREATE PROCEDURE `deal_one_91pay`(IN `_serial` char(36))
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,@_account_id,@_GoodsCount,@_vip,@_gift_diamond; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,@_account_id,@_GoodsCount,@_vip,@_gift_diamond; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,@_account_id,@_GoodsCount,@_vip,@_gift_diamond; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,@_account_id,@_GoodsCount,@_vip,@_gift_diamond; END;

	SET @_account_id	= 0;
	SET @_GoodsCount	= 0;
	SET @_vip	= 0;
	SET @_gift_diamond = 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM pay91_procqueue WHERE serial=_serial AND (@_GoodsCount:=GoodsCount)>=0 AND (@_account_id:=accountid)) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	SET @_vip = 0;
	START TRANSACTION;
	CALL recharge_diamond(@_account_id, @_GoodsCount, 0, @_vip, @_gift_diamond);
	DELETE FROM pay91_procqueue WHERE serial=_serial;
END label_body;
	COMMIT;
	SELECT _result,@_account_id,@_GoodsCount,@_vip,@_gift_diamond;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_char_atb`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_char_atb`;
DELIMITER ;;
CREATE PROCEDURE `get_char_atb`(IN `_account_id` bigint unsigned)
BEGIN
	IF NOT EXISTS (SELECT 1 FROM once_perperson_time_events once_te JOIN time_events te ON once_te.event_id=te.event_id WHERE once_te.account_id=_account_id AND once_te.type=3 AND (@_add_build_end_time:=te.end_time)>=0) THEN
		SET @_add_build_end_time = 0;
	END IF;
	SELECT c.name,c.last_login_time,c.last_logout_time,c.exp,c.level,c.diamond,c.crystal,c.gold,c.vip,c.sex,c.head_id,c.pos_x,c.pos_y,c.development,c.population,c.build_num,c.added_build_num,c.draw_lottery_num,IFNULL(ar.alliance_id,0),IFNULL(a.name, ''),c.drug,c.trade_num,IFNULL(c.signature, ''),c.change_name_num,c.protect_time,@_add_build_end_time,get_build_num(c.account_id),c.notification_id,c.alliance_lottery_num,c.total_diamond,c.instance_wangzhe,c.instance_zhengzhan,c.cup,c.binded,c.vip_display FROM common_characters c LEFT JOIN alliance_members ar ON c.account_id=ar.account_id LEFT JOIN alliances a ON ar.alliance_id=a.alliance_id WHERE c.account_id=_account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `load_player_card`
-- ----------------------------
DROP PROCEDURE IF EXISTS `load_player_card`;
DELIMITER ;;
CREATE PROCEDURE `load_player_card`()
BEGIN
	SELECT c.account_id,c.name,c.level,c.vip,c.sex,c.head_id,c.pos_x,c.pos_y,IFNULL(ar.alliance_id,0),IFNULL(a.name, ''),c.development,IFNULL(c.signature, ''),c.protect_time,IFNULL(b.level,0) as city_level,c.instance_wangzhe,c.instance_zhengzhan,c.cup,c.vip_display FROM common_characters c LEFT JOIN buildings b ON c.account_id=b.account_id LEFT JOIN alliance_members ar ON c.account_id=ar.account_id LEFT JOIN alliances a ON ar.alliance_id=a.alliance_id WHERE b.excel_id=1;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `change_hero_name`
-- ----------------------------
DROP PROCEDURE IF EXISTS `change_hero_name`;
DELIMITER ;;
CREATE PROCEDURE `change_hero_name`(IN `_account_id` bigint unsigned,IN `_hero_id` bigint unsigned,IN `_new_name` varchar(32))
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _diamond INT UNSIGNED DEFAULT 0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM hire_heros WHERE account_id=_account_id AND hero_id=_hero_id) THEN
		SET _result = -1; # 英雄不存在
		LEAVE label_body;
	END IF;
	SELECT diamond INTO _diamond FROM excel_change_name WHERE excel_id=2;
	UPDATE common_characters SET diamond=diamond-_diamond WHERE account_id=_account_id AND diamond>=_diamond;
	IF ROW_COUNT()=0 THEN
		SET _result = -2; # 金钱不够
		LEAVE label_body;
	END IF;
	UPDATE hire_heros SET name=_new_name WHERE hero_id=_hero_id;
END label_body;
	SELECT _result,_diamond;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_top_lottery_reward_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_top_lottery_reward_log`;
DELIMITER ;;
CREATE PROCEDURE `add_top_lottery_reward_log`(IN `_account_id` bigint unsigned,IN `_type` int unsigned,IN `_data` int unsigned)
BEGIN
	SET @_value = 0;

	CASE _type
		WHEN 1 THEN
			IF NOT EXISTS (SELECT 1 FROM excel_store_item WHERE excel_id=_data AND (@_value:=diamond)>=0) THEN
				SELECT theroy_diamond INTO @_value FROM excel_item_list WHERE excel_id=_data;
			END IF;
		WHEN 2 THEN
			SET @_value = _data;
		WHEN 3 THEN
			SELECT FLOOR(_data/gold) INTO @_value FROM excel_store_conversion_rate WHERE money_type=1;
		ELSE
			SET @_value = 0;
	END CASE;
	SET @_time_now	= UNIX_TIMESTAMP();
	INSERT INTO top_lottery_reward_log (account_id,type,data,value,time) VALUES (_account_id,_type,_data,@_value,@_time_now);
	SELECT _account_id,_type,_data,@_value,@_time_now;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_gm_mail`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_gm_mail`;
DELIMITER ;;
CREATE PROCEDURE `add_gm_mail`(IN `_account_id` bigint unsigned,IN `_text` blob)
BEGIN
	INSERT INTO gm_mail (account_id,time,text,mail_id) VALUES (_account_id,UNIX_TIMESTAMP(),COMPRESS(_text), new_mail_id(_account_id));
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_player_card`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_player_card`;
DELIMITER ;;
CREATE PROCEDURE `get_player_card`(IN `_account_id` bigint unsigned)
BEGIN
	SELECT c.account_id,c.name,c.level,c.vip,c.sex,c.head_id,c.pos_x,c.pos_y,IFNULL(ar.alliance_id,0),IFNULL(a.name, ''),c.development,IFNULL(c.signature, ''),c.protect_time,IFNULL(b.level,0) as city_level,c.instance_wangzhe,c.instance_zhengzhan,c.cup,c.vip_display FROM common_characters c LEFT JOIN buildings b ON c.account_id=b.account_id LEFT JOIN alliance_members ar ON c.account_id=ar.account_id LEFT JOIN alliances a ON ar.alliance_id=a.alliance_id WHERE b.excel_id=1 AND c.account_id=_account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_excel_time_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_excel_time_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_excel_time_event`()
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE _excel_id INT UNSIGNED DEFAULT 0;
	DECLARE _begin_date DATETIME DEFAULT '0000-00-00 00:00:00';
	DECLARE _is_cycle INT UNSIGNED DEFAULT 0;
	DECLARE _cycle_time INT UNSIGNED DEFAULT 0;
	DECLARE _next_tick INT UNSIGNED DEFAULT 0;
	DECLARE _dealed INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;
	DECLARE _need_deal INT UNSIGNED DEFAULT 0;
	DECLARE _begin_time INT UNSIGNED DEFAULT 0;
	DECLARE _data INT UNSIGNED DEFAULT 0;

	DECLARE _cursor4excel CURSOR FOR SELECT excel_id,begin_date,is_cycle,cycle_time FROM excel_time_event;
	DECLARE _cursor4deal CURSOR FOR SELECT e.excel_id,e.is_cycle,e.cycle_time,UNIX_TIMESTAMP(e.begin_date),t.next_tick,t.dealed FROM excel_time_event e JOIN wait_deal_excel_time_event t ON e.excel_id=t.excel_id;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN SET @_release_lock=_release_lock(2); END;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN SET @_release_lock=_release_lock(2); END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN SET @_release_lock=_release_lock(2); END;

label_body:BEGIN
	IF _get_lock(2)=0 THEN
		LEAVE label_body;
	END IF;

	# 存储时间事件处理结果
	DROP TEMPORARY TABLE IF EXISTS `tmp_excel_timeevent_result`;
	CREATE TEMPORARY TABLE `tmp_excel_timeevent_result`(
		excel_id INT UNSIGNED NOT NULL,
		data INT UNSIGNED NOT NULL,
		PRIMARY KEY(`excel_id`)
	);

	# 是否有新的时间事件需要注册
	OPEN _cursor4excel;
		SET _done = 0;
		REPEAT
			FETCH _cursor4excel INTO _excel_id,_begin_date,_is_cycle,_cycle_time;
			IF NOT _done THEN
				IF NOT EXISTS (SELECT 1 FROM wait_deal_excel_time_event WHERE excel_id=_excel_id) THEN
					INSERT INTO wait_deal_excel_time_event (excel_id,next_tick,dealed) VALUES (_excel_id,UNIX_TIMESTAMP(_begin_date), 0);
				END IF;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor4excel;

	# 遍历处理到期时间事件
	SET @_time_now = UNIX_TIMESTAMP();
	OPEN _cursor4deal;
		SET _done = 0;
		REPEAT
			FETCH _cursor4deal INTO _excel_id,_is_cycle,_cycle_time,_begin_time,_next_tick,_dealed;
			IF NOT _done THEN
				# 本次是否需要处理该事件
				SET _need_deal = 0;
				IF _dealed=0 THEN # 非循环事件只有最开始dealed=0,循环事件的dealed永远为0
					IF _next_tick<=@_time_now  THEN
						SET _need_deal = 1;
					END IF;
				END IF;
				
				# 处理时间事件
				IF _need_deal = 1 THEN
					SET _data = 0;
					CASE
						WHEN _excel_id=1 THEN
							# CALL _deal_char_day_te();
							SET _result = 0;
						WHEN _excel_id=2 THEN
							# CALL _deal_mail_day_te();
							SET _result = 0;
						WHEN _excel_id=3 THEN
							CALL _deal_gen_world_famous_city(_data);
						WHEN _excel_id=4 THEN
							#CALL stat_char_online_time();
							SET _result = 0;
						ELSE
							SET _result = 0;
					END CASE;

					# 准备下次事件处理
					IF _is_cycle=1 THEN
						SET _next_tick = _cycle_time-(@_time_now-_begin_time)%_cycle_time+@_time_now;
						UPDATE wait_deal_excel_time_event SET next_tick=_next_tick WHERE excel_id=_excel_id;
					ELSE
						UPDATE wait_deal_excel_time_event SET dealed=1 WHERE excel_id=_excel_id;
					END IF;

					INSERT INTO tmp_excel_timeevent_result VALUES (_excel_id,_data);
				END IF;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor4deal;

	SET @_release_lock=_release_lock(2);
	SELECT excel_id,data FROM tmp_excel_timeevent_result;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `_deal_char_day_te`
-- ----------------------------
DROP PROCEDURE IF EXISTS `_deal_char_day_te`;
DELIMITER ;;
CREATE PROCEDURE `_deal_char_day_te`(IN `_account_id` bigint unsigned)
BEGIN
	# 目前为联盟跑商,抽奖,联盟抽奖
	DECLARE _lottery_date DATE DEFAULT '0000-00-00';
	DECLARE _lottery_num INT UNSIGNED DEFAULT 0;
	DECLARE _trade_date DATE DEFAULT '0000-00-00';
	DECLARE _trade_num INT UNSIGNED DEFAULT 0;
	DECLARE _alliance_lottery_date DATE DEFAULT '0000-00-00';
	DECLARE _alliance_lottery_num INT UNSIGNED DEFAULT 0;
	DECLARE _cur_date DATE DEFAULT '0000-00-00';

	SET _cur_date = CURDATE();
	SELECT draw_lottery_date,draw_lottery_num,trade_date,trade_num,alliance_lottery_date,alliance_lottery_num INTO _lottery_date,_lottery_num,_trade_date,_trade_num,_alliance_lottery_date,_alliance_lottery_num FROM common_characters WHERE account_id=_account_id;

	# 抽奖
	IF _lottery_date!=_cur_date THEN
		SELECT free_draw_num INTO _lottery_num FROM excel_lottery_cfg;
	END IF;

	# 联盟跑商
	IF _trade_date!=_cur_date THEN
		IF EXISTS (SELECT 1 FROM buildings WHERE account_id=_account_id AND excel_id=5 AND (@_level:=level)) THEN
			SELECT trade_num INTO _trade_num FROM excel_embassy_cfg WHERE level<=@_level ORDER BY level DESC LIMIT 1;
		END IF;
	END IF;

	# 联盟抽奖
	IF _alliance_lottery_date!=_cur_date THEN
		SELECT alliance_lottery_num INTO _alliance_lottery_num FROM excel_lottery_cfg LIMIT 1;
	END IF;

	UPDATE common_characters SET draw_lottery_date=_cur_date,draw_lottery_num=_lottery_num,trade_date=_cur_date,trade_num=_trade_num,alliance_lottery_date=_cur_date,alliance_lottery_num=_alliance_lottery_num,draw_lottery_date=_cur_date,draw_lottery_num=_lottery_num,trade_date=_cur_date,trade_num=_trade_num,alliance_lottery_date=_cur_date,alliance_lottery_num=_alliance_lottery_num WHERE account_id=_account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `change_alliance_name`
-- ----------------------------
DROP PROCEDURE IF EXISTS `change_alliance_name`;
DELIMITER ;;
CREATE PROCEDURE `change_alliance_name`(IN `_account_id` bigint unsigned,IN `_name` varchar(32))
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alliance_members WHERE account_id=_account_id AND (@_alliance_id:=alliance_id)>=0 AND (@_position:=position)>=0) THEN
		SET _result = -1; # 没有联盟
		LEAVE label_body;
	END IF;
	
	IF @_position!=1 THEN
		SET _result = -2; # 只有盟主能够改名
		LEAVE label_body;
	END IF;

	SELECT diamond INTO @_price FROM excel_change_name WHERE excel_id=3;
	UPDATE common_characters SET diamond=diamond-@_price WHERE account_id=_account_id AND diamond>=@_price;
	IF ROW_COUNT()=0 THEN
		SET _result = -3; # 钻石不够
		LEAVE label_body;
	END IF;

	UPDATE alliances SET name=_name WHERE alliance_id=@_alliance_id;
END label_body;
	SELECT _result,@_alliance_id,@_price;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `_deal_mail_day_te`
-- ----------------------------
DROP PROCEDURE IF EXISTS `_deal_mail_day_te`;
DELIMITER ;;
CREATE PROCEDURE `_deal_mail_day_te`()
BEGIN
	SET @_delete_time=UNIX_TIMESTAMP(CURDATE())-3*24*3600; # 删除三天之前的邮件
	DELETE FROM private_mails WHERE time<@_delete_time AND type NOT IN (1,2) AND flag!=3;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `fetch_mail_reward`
-- ----------------------------
DROP PROCEDURE IF EXISTS `fetch_mail_reward`;
DELIMITER ;;
CREATE PROCEDURE `fetch_mail_reward`(IN `_account_id` bigint unsigned,IN `_mail_id` int unsigned)
BEGIN
	# 附件格式为'类型*数量*附加数据,类型*数量*附加数据,'
	DECLARE _result INT DEFAULT 0;
	SET @_rewards='';

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM private_mails WHERE account_id=_account_id AND mail_id=_mail_id AND flag=3 AND LENGTH(@_str_reward:=UNCOMPRESS(ext_data))>0) THEN
		SET _result = -1; # 邮件不存在
		LEAVE label_body;
	END IF;

	SET @_count=_get_char_count(@_str_reward, ',');
	SET @_pos=1;
	WHILE @_pos<=@_count DO
		SET @_one_reward=SUBSTRING_INDEX(SUBSTRING_INDEX(@_str_reward,',',@_pos),',',-1);
		SET @_type=0;
		SET @_num=0;
		SET @_data=0;
		SET @_splitter_num = _get_char_count(@_one_reward, '*');
		CASE @_splitter_num
			WHEN 1 THEN
				CALL game_split_2(@_one_reward, '*', @_type, @_num);
			WHEN 2 THEN
				CALL game_split_3(@_one_reward, '*', @_type, @_num, @_data);
			ELSE
				SET _result=-2;
				LEAVE label_body;
		END CASE;

		# 派发奖励
		SET @vItem = 0;
		CASE @_type
			WHEN 1 THEN # 道具
				CALL game_add_item(_account_id, 0, @_data, @_num, 0, _result);
				IF _result<=0 THEN
					SET _result = -10; # 道具添加失败
				END IF;
				SELECT item_type INTO @_item_type FROM excel_item_list WHERE excel_id=@_data;
				# 类型*数量*excel_id*only_id数量,item_ids;
				IF @_item_type=1 THEN
					SET @_rewards=CONCAT(@_rewards,@_type,'*',1,'*',@_data,'*',@_num,@vItem,';'); # 每一个道具一个ID
				ELSE
					SET @_rewards=CONCAT(@_rewards,@_type,'*',@_num,'*',@_data,'*',1,@vItem,';'); # 多个道具共有一个ID
				END IF;
				SET _result = 0;
			WHEN 2 THEN # 钻石
				UPDATE common_characters SET diamond=diamond+@_num WHERE account_id=_account_id;
				IF ROW_COUNT()=0 THEN
					SET _result = -20; # 添加钻石失败
				END IF;
				SET @_rewards=CONCAT(@_rewards,@_type,'*',@_num,';');
			WHEN 3 THEN # 水晶
				UPDATE common_characters SET crystal=crystal+@_num WHERE account_id=_account_id;
				IF ROW_COUNT()=0 THEN
					SET _result = -30; # 添加水晶失败
				END IF;
				SET @_rewards=CONCAT(@_rewards,@_type,'*',@_num,';');
			WHEN 4 THEN # 黄金
				UPDATE common_characters SET gold=gold+@_num WHERE account_id=_account_id;
				IF ROW_COUNT()=0 THEN
					SET _result = -40; # 添加黄金失败
				END IF;
				SET @_rewards=CONCAT(@_rewards,@_type,'*',@_num,';');
			ELSE
				SET _result = -99; # 奖励类型不对
		END CASE;

		SET @_pos=@_pos+1;
	END WHILE;

	# 删除邮件
	DELETE FROM private_mails WHERE account_id=_account_id AND mail_id=_mail_id;
END label_body;
	SELECT _result,@_rewards; # 格式:type*num[*excel_id*only_id数量,item_id0,item_id1...];type*num[*excel_id*only_id数量,item_id0,item_id1...];...
END
;;
DELIMITER ;

-- ------------------------------------------
-- call this procedure at 23:58:00 every day
-- ------------------------------------------
DROP PROCEDURE IF EXISTS `calc_stat_login_today`;
DELIMITER ;;
CREATE PROCEDURE `calc_stat_login_today`()
begin
declare _today int unsigned;
declare _cday int unsigned;

declare _c1 int unsigned default 0;
declare _c2 int unsigned default 0;
declare _c3 int unsigned default 0;
declare _c4 int unsigned default 0;
declare _cr1 int unsigned default 0;
declare _cr3 int unsigned default 0;
declare _day1 int unsigned default 0;
declare _day2 int unsigned default 0;
declare _day3 int unsigned default 0;
declare _tt int unsigned default 0;

SET @_cur_date=CURDATE();

set _today = UNIX_TIMESTAMP(@_cur_date);
set _day1 = _today-86400;
set _day2 = _today-86400*2;
set _day3 = _today-86400*3;

select count(1) into _tt from common_characters;
select count(1) into _c1 from common_characters where create_time> _today;
select count(1) into _c2 from common_characters where last_login_time> _today;
set _cday  = _today-7*86400;
select count(1) into _c3 from common_characters where last_login_time>_today and create_time<_cday;
set _cday  = _today-86400*30;
select count(1) into _c4 from common_characters where last_login_time>_today and create_time<_cday;
select count(1) into _cr1 from common_characters where last_login_time>_today and create_time<_today and create_time>_day1;
select count(1) into _cr3 from common_characters where last_login_time>_today and create_time<_day2 and create_time>_day3;
insert into stat_login values(_today,_c1,_c2,_c3,_c4,_cr1,_cr3,_tt) on duplicate key update todaynew=_c1,todaylogin=_c2,player7=_c3,player30=_c4,total=_tt,remain1=_cr1,remain3=_cr3;

INSERT INTO char_online_stat (stat_date,online_num,online_time) VALUES (@_cur_date,_c2,0) ON DUPLICATE KEY UPDATE online_num=_c2;
end
;;
delimiter ;

-- ----------------------------
-- Procedure structure for `store_bill`
-- ----------------------------
DROP PROCEDURE IF EXISTS `store_bill`;
DELIMITER ;;
CREATE PROCEDURE `store_bill`(IN `_account_id` bigint unsigned,IN `_client_time` int unsigned,IN `_server_time` int unsigned,IN `_appid` int unsigned,IN `_bill` blob)
BEGIN
	INSERT INTO bills (id, account_id, client_time, server_time, bill, appid) VALUES (_get_bill_id(), _account_id, _client_time, _server_time, COMPRESS(_bill), _appid);
	IF ROW_COUNT() = 0 THEN
		SELECT -1;
	ELSE
		SELECT 0;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `get_bill_id`
-- ----------------------------
DROP FUNCTION IF EXISTS `get_bill_id`;
DELIMITER ;;
CREATE FUNCTION `get_bill_id`() RETURNS int(11)
BEGIN
	SET @_new_id=0;
	IF NOT EXISTS (SELECT 1 FROM bill_id WHERE (@_new_id:=id)>0) THEN
		INSERT INTO bill_id (id) VALUES (1);
		SET @_new_id=1;
	ELSE
		UPDATE bill_id SET id=id+1;
		SET @_new_id=@_new_id+1;
	END IF;

	RETURN @_new_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `_deal_gen_world_famous_city`
-- ----------------------------
DROP PROCEDURE IF EXISTS `_deal_gen_world_famous_city`;
DELIMITER ;;
CREATE PROCEDURE `_deal_gen_world_famous_city`(OUT `_result` int unsigned)
BEGIN
	DECLARE _times INT UNSIGNED DEFAULT 0;
label_body:BEGIN
	SELECT val_int INTO @_num FROM excel_cmndef WHERE id=13;
	SELECT t.ext_data,t.next_tick,e.cycle_time INTO @_cur_num,@_last_tick,@_cycle_time FROM wait_deal_excel_time_event t JOIN excel_time_event e ON t.excel_id=e.excel_id WHERE t.excel_id=3;
	SET _times = (UNIX_TIMESTAMP()-@_last_tick)/@_cycle_time + 1;
	IF @_cur_num+_times>@_num THEN
		UPDATE wait_deal_excel_time_event SET ext_data=1 WHERE excel_id=3;
		SET _result=1;
	ELSE
		UPDATE wait_deal_excel_time_event SET ext_data=ext_data+_times WHERE excel_id=3;
		SET _result=0;
	END IF;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `get_cup_activity_left_time`
-- ----------------------------
DROP FUNCTION IF EXISTS `get_cup_activity_left_time`;
DELIMITER ;;
CREATE FUNCTION `get_cup_activity_left_time`() RETURNS int(11)
BEGIN
	DECLARE _left_time INT UNSIGNED DEFAULT 0;

	SELECT val_int INTO @_num FROM excel_cmndef WHERE id=13;
	SELECT t.next_tick,t.ext_data,e.cycle_time INTO @_next_tick,@_cur_num,@_cycle_time FROM wait_deal_excel_time_event t JOIN excel_time_event e ON t.excel_id=e.excel_id WHERE t.excel_id=3;
	SET @_time_now = UNIX_TIMESTAMP();
	
	SET @_end_time = @_next_tick + (@_num-@_cur_num)*@_cycle_time;
	SET _left_time = IF(@_time_now>=@_end_time, 0, @_end_time-@_time_now);

	RETURN _left_time;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_unfetched_lottery_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_unfetched_lottery_log`;
DELIMITER ;;
CREATE PROCEDURE `add_unfetched_lottery_log`(IN `_account_id` bigint unsigned,IN `_type` tinyint unsigned,IN `_data` int unsigned,IN `_time` int unsigned)
BEGIN
	SET @_id = new_mail_id(_account_id);
	INSERT INTO unfetched_lottery (id,account_id,type,data,time) VALUES (@_id,_account_id,_type,_data,_time);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `stat_char_online_time`
-- ----------------------------
DROP PROCEDURE IF EXISTS `stat_char_online_time`;
DELIMITER ;;
CREATE PROCEDURE `stat_char_online_time`()
BEGIN
	SET @_date			= CURDATE();
	SET @_time_now	= UNIX_TIMESTAMP();

	# 统计时长
	SELECT COUNT(*),IFNULL(SUM(today_online_time),0) INTO @_today_online_num,@_today_online_time FROM common_characters WHERE last_logout_time<@_time_now;
	# 清理已经统计过的时长
	UPDATE common_characters SET today_online_time=0 WHERE last_logout_time<@_time_now;
	INSERT INTO char_online_stat (stat_date,online_num,online_time) VALUES (@_date,@_today_online_num,@_today_online_time);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `game_gen_name_en`
-- ----------------------------
DROP PROCEDURE IF EXISTS `game_gen_name_en`;
DELIMITER ;;
CREATE PROCEDURE `game_gen_name_en`(in _male int, out _result int)
BEGIN
	set @vFix=0;
	if (_male=1) then
		-- 男生
		set @vFix=1;
	elseif (_male=0) then
		-- 女生
		set @vFix=2;
	else
		set @vFix=floor(1+(rand()*2));
	end if;
	SELECT lastname INTO @vName FROM excel_hero_name_en ORDER BY RAND() LIMIT 1;
	SET _result = 0;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_get_lock`
-- ----------------------------
DROP FUNCTION IF EXISTS `_get_lock`;
DELIMITER ;;
CREATE FUNCTION `_get_lock`(`_type` int unsigned) RETURNS int(10) unsigned
BEGIN
	IF NOT EXISTS (SELECT 1 FROM locks WHERE type=_type) THEN
		INSERT INTO locks (type,locked) VALUES (_type,0);
	END IF;

	UPDATE locks SET locked=1 WHERE type=_type AND locked=0;
	RETURN ROW_COUNT();
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_release_lock`
-- ----------------------------
DROP FUNCTION IF EXISTS `_release_lock`;
DELIMITER ;;
CREATE FUNCTION `_release_lock`(`_type` int unsigned) RETURNS int(10) unsigned
BEGIN
	UPDATE locks SET locked=0 WHERE type=_type AND locked=1;
	RETURN ROW_COUNT();
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_adcolony`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_adcolony`;
DELIMITER ;;
CREATE PROCEDURE `deal_adcolony`()
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _transaction_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _time INT UNSIGNED DEFAULT 0;
	DECLARE _name VARCHAR(32) DEFAULT '';
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _amount INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT transaction_id,time,name,account_id,amount FROM adcolony_transactions;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN SET @_release_lock=_release_lock(1); END;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

label_body:BEGIN
	IF _get_lock(1)=0 THEN
		LEAVE label_body;
	END IF;

	DROP TEMPORARY TABLE IF EXISTS `tmp_adcolony`;
	CREATE TEMPORARY TABLE `tmp_adcolony`(
		account_id BIGINT UNSIGNED NOT NULL,
		money_type INT UNSIGNED NOT NULL,
		num INT UNSIGNED NOT NULL
	);

	SET _done=0;
	OPEN _cursor;
		repeat_cursor:
		REPEAT
			FETCH _cursor INTO _transaction_id,_time,_name,_account_id,_amount;
			IF NOT _done THEN
				IF EXISTS (SELECT 1 FROM adcolony_logs WHERE transaction_id=_transaction_id) THEN
					DELETE FROM adcolony_transactions WHERE transaction_id=_transaction_id;
					ITERATE repeat_cursor;
				END IF;
				UPDATE common_characters SET diamond=diamond+_amount WHERE account_id=_account_id;
				INSERT INTO tmp_adcolony VALUES (_account_id, 1, _amount);

				INSERT INTO adcolony_logs (transaction_id,time,name,account_id,amount) VALUES (_transaction_id,_time,_name,_account_id,_amount);
				DELETE FROM adcolony_transactions WHERE transaction_id=_transaction_id;
			END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;

	SET @_release_lock=_release_lock(1);
	SELECT account_id,money_type,num FROM tmp_adcolony;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `dismiss_soldier`
-- ----------------------------
DROP PROCEDURE IF EXISTS `dismiss_soldier`;
DELIMITER ;;
CREATE PROCEDURE `dismiss_soldier`(IN `_account_id` bigint unsigned,IN `_excel_id` int unsigned,IN `_level` int unsigned,IN `_num` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM soldiers WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level AND (@_num:=num)>=_num) THEN
		SET _result = -1; # 兵种数量不够
		LEAVE label_body;
	END IF;

	IF @_num=_num THEN
		DELETE FROM soldiers WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level AND num=_num;
	ELSE
		UPDATE soldiers SET num=num-_num WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level AND num>_num;
	END IF;
	IF ROW_COUNT()=0 THEN
		SET _result = -2;	# 回收士兵失败
		LEAVE label_body;
	END IF;

	UPDATE common_characters SET population=population+_num WHERE account_id=_account_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -3; # 人口增加失败
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_get_bill_id`
-- ----------------------------
DROP FUNCTION IF EXISTS `_get_bill_id`;
DELIMITER ;;
CREATE FUNCTION `_get_bill_id`() RETURNS int(10) unsigned
BEGIN
	IF NOT EXISTS (SELECT 1 FROM bill_id) THEN
		INSERT INTO bill_id (id) VALUES (0);
	END IF;

	UPDATE bill_id SET id=(@_id:=id+1);

	RETURN @_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alliance_donate_soldier`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alliance_donate_soldier`;
DELIMITER ;;
CREATE PROCEDURE `alliance_donate_soldier`(IN `_account_id` bigint unsigned,IN `_obj_id` bigint unsigned,IN `_excel_id` int unsigned,IN `_level` int unsigned,IN `_num` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,@_event_id,@_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,@_event_id,@_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,@_event_id,@_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,@_event_id,@_interval; END;

	SET @_event_id = 0;
	SET @_interval = 0;
label_body:BEGIN
	# 一次只能派遣一对士兵
	IF EXISTS (SELECT 1 FROM donate_soldier_time_events WHERE account_id=_account_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	SET @_alliance_id = 0;
	SET @_level = 0;
	SELECT alliance_id,level,pos_x,pos_y,vip INTO @_alliance_id,@_level,@_pos_x,@_pos_y,@_vip FROM common_characters WHERE account_id=_account_id;
	# 没有联盟不行
	IF @_alliance_id=0 THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 对方必须和自己同一个联盟
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_obj_id AND alliance_id=@_alliance_id AND (@_obj_x:=pos_x)>=0 AND (@_obj_y:=pos_y)>=0) THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;

	# 等级是否够
	SELECT val_int INTO @_need_level FROM excel_cmndef WHERE id=21;
	IF (@_level<@_need_level) THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;

	# 是否该兵种足够
	IF NOT EXISTS (SELECT 1 FROM soldiers WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level AND num>=_num) THEN
		SET _result = -5;
		LEAVE label_body;
	END IF;
	
	# 自己今天是否还能赠送士兵
	IF NOT EXISTS (SELECT 1 FROM char_donate_soldier WHERE account_id=_account_id AND (@_donated_num:=donated_num)>=0 AND (@_date:=date)) THEN
		SET @_donated_num = 0;
		SET @_date = CURDATE();
		INSERT INTO char_donate_soldier (account_id,donated_num,date) VALUES (_account_id,@_donated_num,@_date);
	END IF;
	SET @_max_donated_num = 0;
	SET @_acce_rate = 0;
	SET @_cur_date = CURDATE();
	SELECT e.day_max_num,e.accelerate_rate INTO @_max_num,@_acce_rate FROM excel_alliance_donate_soldier e JOIN alliance_buildings a ON e.level=a.level WHERE a.alliance_id=@_alliance_id AND a.excel_id=1;
	IF @_date=@_cur_date THEN
		IF (_num+@_donated_num)>@_max_num THEN
			SET _result = -6;
			LEAVE label_body;
		END IF;
	ELSE
		IF _num>@_max_num THEN
			SET _result = -6;
		END IF;
	END IF;

	# 对方兵营+将领+被送谴兵数量超过上限
	SELECT IFNULL(sum(num),0) INTO @_obj_num FROM soldiers WHERE account_id=_obj_id;
	SELECT IFNULL(sum(army_num),0) INTO @_obj_num_with_hero FROM hire_heros WHERE account_id=_obj_id;
	SELECT IFNULL(sum(num),0) INTO @_obj_in_march_num FROM donate_soldier_time_events WHERE obj_id=_obj_id;
	CALL get_barrack_capacity(_obj_id,@_result,@_cap);
	SET @_cap=@_cap*2;
	IF (@_obj_num+@_obj_num_with_hero+@_obj_in_march_num)>=@_cap THEN
		SET _result = -7;
		LEAVE label_body;
	END IF;

	# 赠送士兵,增加时间事件
	START TRANSACTION;
	# 扣除士兵
	UPDATE soldiers SET num=num-_num WHERE account_id=_account_id AND excel_id=_excel_id AND level=_level AND num>=_num;
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -5;
		LEAVE label_body;
	END IF;
	# 插入时间事件
	SET @_event_id = game_global_id();
	INSERT INTO donate_soldier_time_events (event_id,account_id,obj_id,excel_id,level,num,type) VALUES (@_event_id,_account_id,_obj_id,_excel_id,_level,_num,1);
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -10;
		LEAVE label_body;
	END IF;
	SET @_time_now=UNIX_TIMESTAMP();
	select army_accelerate into @_vip_acce_rate from excel_vip_right where level=@_vip;
	SET @_interval=game_calc_movetime(@_pos_x,@_pos_y,@_obj_x,@_obj_y);
	SET @_interval=CEIL(@_interval*@_acce_rate*@_vip_acce_rate);
	INSERT INTO time_events (event_id,begin_time,end_time,type,locked) VALUES (@_event_id,@_time_now,@_time_now+@_interval,8,0);
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -11;
		LEAVE label_body;
	END IF;
	# 增加今日赠送兵数量
	IF @_date=@_cur_date THEN
		UPDATE char_donate_soldier SET donated_num=donated_num+_num WHERE account_id=_account_id;
	ELSE
		UPDATE char_donate_soldier SET donated_num=_num,date=@_cur_date WHERE account_id=_account_id;
	END IF;
END label_body;
	SELECT _result,@_event_id,@_interval;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alliance_recall_soldier`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alliance_recall_soldier`;
DELIMITER ;;
CREATE PROCEDURE `alliance_recall_soldier`(IN `_account_id` bigint unsigned,IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,@_event_id,@_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,@_event_id,@_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,@_event_id,@_interval; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,@_event_id,@_interval; END;

	SET @_event_id=0,@_interval=0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM donate_soldier_time_events WHERE event_id=_event_id AND (@_account_id:=account_id)>=0 
									AND (@_obj_id:=obj_id)>=0 AND (@_excel_id:=excel_id)>=0 AND (@_level:=level)>=0 AND (@_num:=num)>=0 AND (@_type:=type)>=0)
	THEN
		SET _result = -1; # 事件不存在
		LEAVE label_body;
	END IF;
	SET @_time_now = UNIX_TIMESTAMP();
	IF NOT EXISTS (SELECT 1 FROM time_events WHERE event_id=_event_id AND (@_begin_time:=begin_time)>=0 AND (@_end_time:=end_time)>=0) THEN
		SET _result = -2; # 事件不存在
		LEAVE label_body;
	END IF;
	# 最后5s无法召回
	IF (@_time_now+5)>=@_end_time THEN
		SET _result = -3; # 不能召回
		LEAVE label_body;
	END IF;

	START TRANSACTION;
	# 删除原来的事件
	DELETE FROM donate_soldier_time_events WHERE event_id=_event_id;
	DELETE FROM time_events WHERE event_id=_event_id;
	UPDATE char_donate_soldier SET donated_num=IF(donated_num>@_num,donated_num-@_num,0) WHERE account_id=_account_id AND date=CURDATE();
	# 插入新事件
	SET @_event_id = game_global_id();
	INSERT INTO donate_soldier_time_events (event_id,account_id,obj_id,excel_id,level,num,type) VALUES (@_event_id,@_account_id,@_obj_id,@_excel_id,@_level,@_num,2);
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -4;
		LEAVE label_body;
	END IF;
	SET @_interval = @_time_now-@_begin_time;
	INSERT INTO time_events (event_id,begin_time,end_time,type,locked) VALUES (@_event_id,@_time_now,@_time_now+@_interval,8,0);
END label_body;
	SELECT _result,@_event_id,@_interval,@_obj_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `accelerate_donate_soldier`
-- ----------------------------
DROP PROCEDURE IF EXISTS `accelerate_donate_soldier`;
DELIMITER ;;
CREATE PROCEDURE `accelerate_donate_soldier`(IN `_account_id` bigint unsigned,IN `_event_id` bigint unsigned,IN `_time` int unsigned,IN `_money_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_money = 0;
label_body:BEGIN
	SELECT IF(t.end_time>UNIX_TIMESTAMP(),t.end_time-UNIX_TIMESTAMP(),0),d.obj_id INTO @_interval,@_obj_id FROM donate_soldier_time_events d JOIN time_events t ON d.event_id=t.event_id WHERE d.event_id=_event_id AND d.account_id=_account_id;
	IF FOUND_ROWS()=0 THEN
		SET _result = -1; # 事件不存在
		LEAVE label_body;
	END IF;
	IF @_interval=0 THEN
		LEAVE label_body;
	END IF;
	IF _time>@_interval THEN
		SET _time = @_interval;
	END IF;
	SELECT val_int INTO @_cost_base FROM excel_cmndef WHERE id=2;
	SET @_money = CEIL(_time/@_cost_base);
	
	# 消耗货币
	CASE _money_type
		WHEN 1 THEN
			UPDATE common_characters SET diamond=diamond-@_money WHERE account_id=_account_id AND diamond>=@_money;
			IF ROW_COUNT()=0 THEN
				SET _result = -2; # 钻石不够
				LEAVE label_body;
			END IF;
		WHEN 2 THEN
			UPDATE common_characters SET crystal=crystal-@_money WHERE account_id=_account_id AND crystal>=@_money;
			IF ROW_COUNT()=0 THEN
				SET _result = -3; # 水晶不够
				LEAVE label_body;
			END IF;
		ELSE
			SET _result = -10;
			LEAVE label_body;
	END CASE;

	# 减速
	UPDATE time_events SET end_time=end_time-_time WHERE event_id=_event_id;
END label_body;
	SELECT _result,@_money,_time,@_obj_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_donate_soldier_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_donate_soldier_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_donate_soldier_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	SET @_account_id=0,@_obj_id=0,@_excel_id=0,@_level=0,@_num=0,@_type=0,@_mail_id=0;
label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;

	SELECT account_id,obj_id,excel_id,level,num,type INTO @_account_id,@_obj_id,@_excel_id,@_level,@_num,@_type FROM donate_soldier_time_events WHERE event_id=_event_id;
	# 没有该事件数据
	IF FOUND_ROWS()=0 THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	
	CASE @_type
	WHEN 1 THEN # 派遣
		# 增加士兵
		IF EXISTS (SELECT 1 FROM soldiers WHERE account_id=@_obj_id AND excel_id=@_excel_id AND level=@_level) THEN
			UPDATE soldiers SET num=num+@_num WHERE account_id=@_obj_id AND excel_id=@_excel_id AND level=@_level;
		ELSE
			INSERT INTO soldiers (account_id,excel_id,level,num) VALUES (@_obj_id,@_excel_id,@_level,@_num);
		END IF;
		
		# 发送邮件进行通知
		SELECT content INTO @_text FROM excel_text WHERE excel_id=466;
		SELECT name INTO @_name FROM common_characters WHERE account_id=@_account_id;
		SELECT name INTO @_soldier_name FROM excel_soldier WHERE excel_id=@_excel_id AND level=@_level;
		SET @_text = REPLACE(@_text, '%s1', @_name);
		SET @_text = REPLACE(@_text, '%s2', @_level);
		SET @_text = REPLACE(@_text, '%s3', @_soldier_name);
		SET @_text = REPLACE(@_text, '%s4', @_num);
		CALL add_private_mail(0,@_obj_id,4,1,0,@_text,NULL,0,@_result,@_mail_id);
	WHEN 2 THEN # 召回
		# 增加士兵
		IF EXISTS (SELECT 1 FROM soldiers WHERE account_id=@_account_id AND excel_id=@_excel_id AND level=@_level) THEN
			UPDATE soldiers SET num=num+@_num WHERE account_id=@_account_id AND excel_id=@_excel_id AND level=@_level;
		ELSE
			INSERT INTO soldiers (account_id,excel_id,level,num) VALUES (@_account_id,@_excel_id,@_level,@_num);
		END IF;
	ELSE
		SET _result = -10;
		LEAVE label_body;
	END CASE;
	
	# 删除时间事件
	DELETE FROM donate_soldier_time_events WHERE event_id=_event_id;
	DELETE FROM time_events WHERE event_id=_event_id;
END label_body;
	SELECT _result,@_account_id,@_obj_id,@_excel_id,@_level,@_num,@_type,@_mail_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_donate_soldier_queue`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_donate_soldier_queue`;
DELIMITER ;;
CREATE PROCEDURE `get_donate_soldier_queue`(IN `_account_id` bigint unsigned)
BEGIN
	SET @_time_now = UNIX_TIMESTAMP();
	# 1为派遣,2为被派遣
	(SELECT 1,d.obj_id,d.excel_id,d.level,d.num,d.type,t.event_id,IF(t.end_time>@_time_now,t.end_time-@_time_now,0),c.name FROM donate_soldier_time_events d JOIN time_events t ON d.event_id=t.event_id JOIN common_characters c ON d.obj_id=c.account_id WHERE d.account_id=_account_id)
	UNION
	(SELECT 2,d.account_id,d.excel_id,d.level,d.num,d.type,t.event_id,IF(t.end_time>@_time_now,t.end_time-@_time_now,0),c.name FROM donate_soldier_time_events d JOIN time_events t ON d.event_id=t.event_id JOIN common_characters c ON d.account_id=c.account_id WHERE d.obj_id=_account_id);
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_left_donate_soldier_num`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_left_donate_soldier_num`;
DELIMITER ;;
CREATE PROCEDURE `get_left_donate_soldier_num`(IN `_account_id` bigint unsigned)
BEGIN
	SET @_result=0,@_left_num=0,@_max_num=0;
label_body:BEGIN
	# 等级不够,或者没有联盟
	SELECT val_int INTO @_need_level FROM excel_cmndef WHERE id=21;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND level>=@_need_level AND (@_alliance_id:=alliance_id)>0) THEN
		LEAVE label_body;
	END IF;

	# 获取已赠送数量
	SELECT donated_num,date INTO @_donated_num,@_date FROM char_donate_soldier WHERE account_id=_account_id;
	IF FOUND_ROWS()=0 OR @_date!=CURDATE() THEN
		SET @_donated_num=0;
	END IF;
	
	# 获取可赠送数量
	SELECT day_max_num INTO @_max_num FROM excel_alliance_donate_soldier e JOIN alliance_buildings a ON e.level=a.level WHERE a.alliance_id=@_alliance_id AND a.excel_id=1;
	SET @_left_num=IF(@_max_num>@_donated_num,@_max_num-@_donated_num,0);
END label_body;
	SELECT @_result,@_left_num,@_max_num;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_gold_deal`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_gold_deal`;
DELIMITER ;;
CREATE PROCEDURE `add_gold_deal`(IN `_account_id` bigint unsigned,IN `_num` bigint unsigned,IN `_price` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,@_transaction_id,_num; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,@_transaction_id,_num; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,@_transaction_id,_num; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,@_transaction_id,_num; END;

	SET @_transaction_id=0;
label_body:BEGIN
	# 一次只能进行一笔交易
	IF EXISTS (SELECT 1 FROM gold_market WHERE account_id=_account_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	# 等级是否足够
	SELECT val_int INTO @_min_level FROM excel_cmndef WHERE id=22;
	SELECT gold,level INTO @_gold,@_level FROM common_characters WHERE account_id=_account_id;
	IF @_level<@_min_level THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;

	# 一次交易的上限
	SELECT val_int INTO @_unit FROM excel_cmndef WHERE id=23; # 交易单位
	SELECT MAX(gold) INTO @_max_gold FROM excel_gold_market_level WHERE level<=@_level;
	SET _num = FLOOR(_num/@_unit)*@_unit;
	SET @_total_price = _num/@_unit*_price;

	IF _num=0 THEN
		SET _result = -10;	# 不能买卖0黄金
		LEAVE label_body;
	END IF;
	
	IF _num>@_max_gold THEN
		SET _result = -11;	# 交易的黄金超过当前等级
		LEAVE label_body;
	END IF;

	# 黄金是否够
	IF @_gold<_num THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;

	START TRANSACTION;
	# 黄金是否足够
	UPDATE common_characters SET gold=gold-_num WHERE account_id=_account_id AND gold>=_num;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;

	# 添加交易
	SET @_transaction_id = game_global_id();
	INSERT INTO gold_market (transaction_id,account_id,num,price,total_price,time) VALUES (@_transaction_id,_account_id,_num,_price,@_total_price,UNIX_TIMESTAMP());
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -4;
		LEAVE label_body;
	END IF;
	
	# 添加交易日志
	CALL add_gold_market_log(@_transaction_id,_account_id,0,_num,@_total_price,1);
END label_body;
	SELECT _result,@_transaction_id,_num;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `buy_gold_in_market`
-- ----------------------------
DROP PROCEDURE IF EXISTS `buy_gold_in_market`;
DELIMITER ;;
CREATE PROCEDURE `buy_gold_in_market`(IN `_account_id` bigint unsigned,IN `_transaction_id` bigint unsigned,IN `_money_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,@_seller_id,@_num,@_total_price,@_buyer_mail_id,@_seller_mail_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '02000' BEGIN ROLLBACK; SET _result=-101; SELECT _result,@_seller_id,@_num,@_total_price,@_buyer_mail_id,@_seller_mail_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN ROLLBACK; SET _result=-102; SELECT _result,@_seller_id,@_num,@_total_price,@_buyer_mail_id,@_seller_mail_id; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,@_seller_id,@_num,@_total_price,@_buyer_mail_id,@_seller_mail_id; END;
	
	SET @_seller_id=0,@_num=0,@_total_price=0,@_buyer_mail_id=0,@_seller_mail_id=0;
label_body:BEGIN
	# 等级是否足够
	SELECT val_int INTO @_min_level FROM excel_cmndef WHERE id=22;
	SELECT level INTO @_level FROM common_characters WHERE account_id=_account_id;
	IF @_level<@_min_level THEN
		SET _result = -3;
		LEAVE label_body;
	END IF;
	
	# 交易是否存在
	IF NOT EXISTS (SELECT 1 FROM gold_market WHERE transaction_id=_transaction_id AND (@_seller_id:=account_id)>=0 AND (@_num:=num)>=0 AND (@_total_price:=total_price)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	# 不能自买自卖
	IF _account_id= @_seller_id THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;

	SELECT crystal,diamond INTO @_crystal,@_diamond FROM common_characters WHERE account_id=_account_id;
	CASE _money_type
		WHEN 1 THEN
			IF @_diamond<@_total_price THEN
				SET _result = -10;	# 钻石不够
				LEAVE label_body;
			END IF;
		#WHEN 2 THEN
		#	IF @_crystal<@_total_price THEN
		#		SET _result = -11;	# 水晶不够
		#		LEAVE label_body;
		#	END IF;
		ELSE
			SET _result = -20;		# 货币类型不对
			LEAVE label_body;
	END CASE;

	START TRANSACTION;
	# 删除交易
	DELETE FROM gold_market WHERE transaction_id=_transaction_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
	# 买方扣除水晶/钻石,增加黄金
	CASE _money_type
		WHEN 1 THEN
			UPDATE common_characters SET diamond=diamond-@_total_price,gold=gold+@_num WHERE account_id=_account_id AND diamond>=@_total_price;
		WHEN 2 THEN
			UPDATE common_characters SET crystal=crystal-@_total_price,gold=gold+@_num WHERE account_id=_account_id AND crystal>=@_total_price;
		ELSE
			ROLLBACK;
			SET _result = -20;			# 货币类型不对
	END CASE;
	IF ROW_COUNT()=0 THEN
		# 扣款失败
		SET _result = -5;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 卖方增加钻石
	UPDATE common_characters SET diamond=diamond+@_total_price WHERE account_id=@_seller_id;
	IF ROW_COUNT()=0 THEN
		# 给卖方增加水晶失败
		SET _result = -8;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 邮件通知
	SELECT content INTO @_text FROM excel_text WHERE excel_id=467;
	SET @_text=REPLACE(@_text, '%s1', @_num);
	SET @_text=REPLACE(@_text, '%s2', @_total_price);
	CALL add_private_mail(0,_account_id,4,1,0,@_text,NULL,0,@_result,@_buyer_mail_id);
	IF @_result!=0 THEN
		# 发送买方邮件失败
		SET _result = -6;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 卖方
	SELECT content INTO @_text FROM excel_text WHERE excel_id=468;
	SET @_text=REPLACE(@_text, '%s1', @_num);
	SET @_text=REPLACE(@_text, '%s2', @_total_price);
	CALL add_private_mail(0,@_seller_id,4,1,0,@_text,NULL,0,@_result,@_seller_mail_id);
	IF @_result!=0 THEN
		# 发送卖方邮件失败
		SET _result = -7;
		ROLLBACK;
		LEAVE label_body;
	END IF;

	# 添加交易日志
	CALL add_gold_market_log(_transaction_id,@_seller_id,_account_id,@_num,@_total_price,2);
END label_body;
	SELECT _result,@_seller_id,@_num,@_total_price,@_buyer_mail_id,@_seller_mail_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `add_gold_market_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `add_gold_market_log`;
DELIMITER ;;
CREATE PROCEDURE `add_gold_market_log`(IN `_transaction_id` bigint unsigned,IN `_seller_id` bigint unsigned,IN `_buyer_id` bigint unsigned,IN `_gold` int unsigned,IN `_total_price` int unsigned,IN `_action` int unsigned)
BEGIN
	# action:1发布交易;2完成交易;3撤销交易;4系统撤销交易
	INSERT INTO gold_market_log VALUES (_transaction_id,_seller_id,_buyer_id,_gold,_total_price,_action,UNIX_TIMESTAMP());
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_gold_market_te`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_gold_market_te`;
DELIMITER ;;
CREATE PROCEDURE `deal_gold_market_te`()
BEGIN
	DECLARE _transaction_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _gold INT UNSIGNED DEFAULT 0;
	DECLARE _total_price INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;
	
	DECLARE _cursor CURSOR FOR SELECT transaction_id,account_id,num,total_price FROM gold_market WHERE time<UNIX_TIMESTAMP()-24*3600;

	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	
label_body:BEGIN	
	IF _get_lock(4)=0 THEN
		SET @_result = -150;
		LEAVE label_body;
	END IF;
	
	# 存储时间事件处理结果
	DROP TEMPORARY TABLE IF EXISTS `tmp_market_cancel_deal`;
	CREATE TEMPORARY TABLE `tmp_market_cancel_deal`(
		account_id BIGINT UNSIGNED NOT NULL,
		mail_id INT UNSIGNED NOT NULL,
		gold INT UNSIGNED NOT NULL
	);

	SET _done=0;
	OPEN _cursor;
	repeat_cursor:
	REPEAT
		FETCH _cursor INTO _transaction_id,_account_id,_gold,_total_price;
		IF NOT _done THEN
			# 删除交易
			DELETE FROM gold_market WHERE transaction_id=_transaction_id;
			IF ROW_COUNT()=0 THEN
				ITERATE repeat_cursor;
			END IF;
			# 增加黄金
			UPDATE common_characters SET gold=gold+_gold WHERE account_id=_account_id;
			IF ROW_COUNT()=0 THEN
				ITERATE repeat_cursor;
			END IF;
			# 发送邮件
			SELECT content INTO @_text FROM excel_text WHERE excel_id=469;
			SET @_text = REPLACE(@_text, '%s1', _gold);
			CALL add_private_mail(0,_account_id,4,1,0,@_text,NULL,0,@_result,@_mail_id);
			# 添加日志
			CALL add_gold_market_log(_transaction_id,_account_id,0,_gold,_total_price,4);
			# 插入邮件列表
			INSERT INTO tmp_market_cancel_deal VALUES (_account_id,@_mail_id,_gold);
		END IF;
	UNTIL _done END REPEAT;
	CLOSE _cursor;

END label_body;
	SET @_result = _release_lock(4);
	SELECT account_id,mail_id,_gold FROM tmp_market_cancel_deal;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `cancel_gold_deal`
-- ----------------------------
DROP PROCEDURE IF EXISTS `cancel_gold_deal`;
DELIMITER ;;
CREATE PROCEDURE `cancel_gold_deal`(IN `_account_id` bigint unsigned,IN `_transaction_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_mail_id=0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM gold_market WHERE account_id=_account_id AND transaction_id=_transaction_id AND (@_gold:=num)>=0 AND (@_total_price:=total_price)>=0 AND (@_time:=time)>=0) THEN
		SET _result = -1;	# 不存在
		LEAVE label_body;
	END IF;

	# 1分钟内不能取消交易
	IF (@_time+60)>=UNIX_TIMESTAMP() THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;

	# 删除交易
	DELETE FROM gold_market WHERE transaction_id=_transaction_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -1; # 不存在
		LEAVE label_body;
	END IF;
	# 增加黄金
	UPDATE common_characters SET gold=gold+@_gold WHERE account_id=_account_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -3; # 增加黄金失败
		LEAVE label_body;
	END IF;
	# 发送邮件
	SELECT content INTO @_text FROM excel_text WHERE excel_id=470;
	SET @_text = REPLACE(@_text, '%s1', @_gold);
	CALL add_private_mail(0,_account_id,4,1,0,@_text,NULL,0,@_result,@_mail_id);
	# 记录日志
	CALL add_gold_market_log(_transaction_id,_account_id,0,@_gold,@_total_price,3);
END label_body;
	SELECT _result,@_mail_id,@_gold;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_gold_deal_num`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_gold_deal_num`;
DELIMITER ;;
CREATE PROCEDURE `get_gold_deal_num`(IN `_account_id` bigint unsigned)
BEGIN
	SELECT COUNT(*) FROM gold_market;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_gold_deal`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_gold_deal`;
DELIMITER ;;
CREATE PROCEDURE `get_gold_deal`(IN `_account_id` bigint unsigned,IN `_page` int unsigned,IN `_page_size` int unsigned)
BEGIN
	# 动态sql(LIMIT的参数不能使用变量)
	SET @_query = CONCAT('SELECT transaction_id,num,price,total_price,account_id FROM gold_market ORDER BY price ASC,num ASC LIMIT ', _page*_page_size, ',', _page_size);
	PREPARE smt FROM @_query;
	EXECUTE smt;
	DEALLOCATE PREPARE smt;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_self_gold_deal`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_self_gold_deal`;
DELIMITER ;;
CREATE PROCEDURE `get_self_gold_deal`(IN `_account_id` bigint unsigned)
BEGIN
	SELECT transaction_id,num,price,total_price,account_id FROM gold_market WHERE account_id=_account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `check_protect_char`
-- ----------------------------
DROP PROCEDURE IF EXISTS `check_protect_char`;
DELIMITER ;;
CREATE PROCEDURE `check_protect_char`(IN `_account_id` bigint unsigned)
BEGIN
label_body:BEGIN
	IF EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND cup>0) THEN
		LEAVE label_body;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM char_attack_protect WHERE account_id=_account_id AND (@_last_time:=time)>=0) THEN
		INSERT INTO char_attack_protect (account_id,time) VALUES (_account_id,UNIX_TIMESTAMP());
		LEAVE label_body;
	END IF;
	SET @_time_now=UNIX_TIMESTAMP();
	SELECT val_int INTO @_reset_time FROM excel_cmndef WHERE id=32;
	IF (@_last_time+@_reset_time)<@_time_now THEN
		# 需要重置
		UPDATE char_attack_protect SET time=@_time_now WHERE account_id=_account_id;
		LEAVE label_body;
	END IF;
	SELECT val_int INTO @_protect_time FROM excel_cmndef WHERE id=31;
	IF EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND protect_time<=@_time_now) THEN
		UPDATE common_characters SET protect_time=@_time_now+@_protect_time WHERE account_id=_account_id;
	END IF;
	# 删除
	DELETE FROM char_attack_protect WHERE account_id=_account_id;
	CALL system_other_army_back(_account_id);
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `system_other_army_back`
-- ----------------------------
DROP PROCEDURE IF EXISTS `system_other_army_back`;
DELIMITER ;;
CREATE PROCEDURE `system_other_army_back`(in _account_id bigint unsigned)
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _begin_time INT UNSIGNED DEFAULT 0;
	DECLARE _combat_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _cursor CURSOR FOR SELECT combat_id,begin_time FROM combats WHERE obj_id=_account_id AND combat_type=2 AND status=0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN END;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '22003' BEGIN END;

	SET @_result=-1;
label_body: BEGIN
	SET _done=0;
	OPEN _cursor;
	REPEAT
		FETCH _cursor INTO _combat_id,_begin_time;
		IF NOT _done THEN
			# 删除原来的事件
			SET @_teid=0;
			IF NOT EXISTS (SELECT 1 FROM te_combat WHERE combat_id=_combat_id AND (@_teid:=te_id)>0) THEN
				SET @_result=-3;
				LEAVE label_body;
			END IF;
			CALL game_del_te_combat(@_teid,@_result);
			# 获取返回时间
			SET @_back_time=UNIX_TIMESTAMP()-_begin_time;
			UPDATE combats SET begin_time=unix_timestamp(),use_time=@_back_time,status=1,combat_rst=0 where combat_id=_combat_id;
			IF (ROW_COUNT()=0) THEN
				SET @_result=-4;
				LEAVE label_body;
			END IF;
			# 战斗返回超时 102
			CALL game_add_te_combat(_combat_id,@_back_time,102,0,@_result);
		END IF;
	UNTIL _done END REPEAT;
	CLOSE _cursor;

	SET @_result = 0;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_get_te_lock`
-- ----------------------------
DROP FUNCTION IF EXISTS `_get_te_lock`;
DELIMITER ;;
CREATE FUNCTION `_get_te_lock`(`_event_id` bigint unsigned) RETURNS int(11)
BEGIN
	UPDATE time_events SET locked=1 WHERE event_id=_event_id AND locked=0;
	RETURN ROW_COUNT();
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_release_te_lock`
-- ----------------------------
DROP FUNCTION IF EXISTS `_release_te_lock`;
DELIMITER ;;
CREATE FUNCTION `_release_te_lock`(`_event_id` bigint unsigned) RETURNS int(11)
BEGIN
	UPDATE time_events SET locked=0 WHERE event_id=_event_id AND locked=1;
	RETURN ROW_COUNT();
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_world_res`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_world_res`;
DELIMITER ;;
CREATE PROCEDURE `get_world_res`()
BEGIN
	SELECT id,type,level,pos_x,pos_y,army_deploy,army_data,gold,pop,crystal,can_build,floor2,terrain_type,prof FROM world_res;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `gen_world_res`
-- ----------------------------
DROP PROCEDURE IF EXISTS `gen_world_res`;
DELIMITER ;;
CREATE PROCEDURE `gen_world_res`(IN `_id` bigint unsigned,IN `_type` int unsigned,IN `_level` int unsigned,IN `_pos_x` int unsigned,IN `_pos_y` int unsigned,IN `_army_deploy` varchar(32),IN `_army_data` varchar(32),IN `_gold` int unsigned,IN `_pop` int unsigned,IN `_crystal` int unsigned,IN `_can_build` int unsigned,IN `_floor2` int unsigned,IN `_src_terrain_type` int unsigned,IN `_prof` int unsigned)
BEGIN
	SET @_result = 0;
	INSERT INTO world_res (id,type,level,pos_x,pos_y,army_deploy,army_data,combat_id,gold,pop,crystal,can_build,floor2,terrain_type,prof) 
	VALUES
	(_id,_type,_level,_pos_x,_pos_y,_army_deploy,_army_data,'',_gold,_pop,_crystal,_can_build,_floor2,_src_terrain_type,_prof);
	IF ROW_COUNT()=0 THEN
		SET @_result = -1;
	END IF;
	SELECT @_result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `calc_world_res_army`
-- ----------------------------
DROP PROCEDURE IF EXISTS `calc_world_res_army`;
DELIMITER ;;
CREATE PROCEDURE `calc_world_res_army`(IN `_res_id` bigint unsigned,OUT `_result` int)
BEGIN
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM world_res WHERE id=_res_id AND LENGTH(@_army_data:=army_data)>=0 AND LENGTH(@_army_deploy:=army_deploy)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	
	SET @_army_t1=0,@_army_t2=0,@_army_t3=0,@_army_t4=0,@_army_t5=0;
	SET @_army_level=0,@_army_num=0;
	CALL game_split_5(@_army_deploy,'*',@_army_t1,@_army_t2,@_army_t3,@_army_t4,@_army_t5);
	CALL game_split_2(@_army_data,'*',@_army_level,@_army_num);

	SET @_army1='',@_army2='',@_army3='',@_army4='',@_army5='';
	# 每一道布局:slot_idx*hero_id*hero_name*army_type*army_level*army_num*attack*defense*health*model*level*health_state
	SET @vAttackArmy=0,@vDefenseArmy=0,@vHealthArmy=0;
	CALL game_calc_army(0,@_army_t1,@_army_level,@_army_num,@_result);
	SET @_army1=CONCAT('1*0**',@_army_t1,'*',@_army_level,'*',@_army_num,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*0*',@_army_level,'*0');
	
	SET @vAttackArmy=0,@vDefenseArmy=0,@vHealthArmy=0;
	CALL game_calc_army(0,@_army_t2,@_army_level,@_army_num,@_result);
	SET @_army2=CONCAT('2*0**',@_army_t2,'*',@_army_level,'*',@_army_num,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*0*',@_army_level,'*0');
	
	SET @vAttackArmy=0,@vDefenseArmy=0,@vHealthArmy=0;
	CALL game_calc_army(0,@_army_t3,@_army_level,@_army_num,@_result);
	SET @_army3=CONCAT('3*0**',@_army_t3,'*',@_army_level,'*',@_army_num,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*0*',@_army_level,'*0');
	
	SET @vAttackArmy=0,@vDefenseArmy=0,@vHealthArmy=0;
	CALL game_calc_army(0,@_army_t4,@_army_level,@_army_num,@_result);
	SET @_army4=CONCAT('4*0**',@_army_t4,'*',@_army_level,'*',@_army_num,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*0*',@_army_level,'*0');
	
	SET @vAttackArmy=0,@vDefenseArmy=0,@vHealthArmy=0;
	CALL game_calc_army(0,@_army_t5,@_army_level,@_army_num,@_result);
	SET @_army5=CONCAT('5*0**',@_army_t5,'*',@_army_level,'*',@_army_num,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*0*',@_army_level,'*0');

	SET _result=0;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `world_res_let_other_back`
-- ----------------------------
DROP PROCEDURE IF EXISTS `world_res_let_other_back`;
DELIMITER ;;
CREATE PROCEDURE `world_res_let_other_back`(IN `_account_id` bigint unsigned,IN `_world_res_id` bigint unsigned,OUT `_result` int)
BEGIN
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM world_res WHERE id=_world_res_id AND LENGTH(@_ids:=combat_id)>=0) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	SET @_total_char_count = _get_char_count(@_ids, ',');
	SET @_count = 1;
	WHILE (@_count<@_total_char_count) DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_ids,',',@_count+1),',',-1) INTO @_account_id;
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_ids,',',@_count+2),',',-1) INTO @_combat_id;
		IF (_account_id!=@_account_id) THEN
			CALL call_army_back(@_account_id,@_combat_id,@_result);
		END IF;
		SET @_count = @_count+2;
	END WHILE;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `call_army_back`
-- ----------------------------
DROP PROCEDURE IF EXISTS `call_army_back`;
DELIMITER ;;
CREATE PROCEDURE call_army_back(IN _account_id bigint unsigned, IN _combat_id bigint unsigned, OUT _result int)
BEGIN

DECLARE EXIT            handler FOR SQLEXCEPTION        BEGIN END;
DECLARE EXIT            handler FOR SQLSTATE '22003'    BEGIN END;

SET _result=-1;
label_army_back: BEGIN

SET @_combat_type=0;
SET @_misc_data='';
SET @_begin_time=0;
IF NOT EXISTS (SELECT 1 FROM combats WHERE combat_id=_combat_id AND account_id=_account_id AND status=0 AND (@_combat_type:=combat_type)>=0 AND (@_misc_data:=misc_data) IS NOT NULL AND (@_begin_time:=begin_time)>=0) THEN
	SET _result=-2;
	LEAVE label_army_back;
END IF;
SET @_teid=0;
IF NOT EXISTS (SELECT 1 FROM te_combat WHERE combat_id=_combat_id AND (@_teid:=te_id)>0) THEN
	SET _result=-3;
	leave label_army_back;
END IF;

-- 
CALL game_del_te_combat(@_teid,@_res);

-- 获取返回时间
SET @_back_time=unix_timestamp()-@_begin_time;

UPDATE combats SET begin_time=unix_timestamp(),use_time=@_back_time,status=2,combat_rst=0 WHERE combat_id=_combat_id;
IF (row_count()=0) THEN
	SET _result=-4;
	leave label_army_back;
END IF;

-- 战斗返回超时 102
CALL game_add_te_combat(_combat_id,@_back_time,102,0,@_res);

SET _result=0;
END label_army_back;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_max_hacker`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_max_hacker`;
DELIMITER ;;
CREATE PROCEDURE `get_max_hacker`()
BEGIN
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _crystal INT UNSIGNED DEFAULT 0;
	DECLARE _done INT DEFAULT 0;
	DECLARE _cursor CURSOR FOR select sum(total_price),seller_id from gold_market_log where action=2 group by seller_id order by sum(total_price) desc;

	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	DROP TEMPORARY TABLE IF EXISTS `tmp_max_hacker`;
	CREATE TEMPORARY TABLE `tmp_max_hacker`(
		account_id BIGINT UNSIGNED NOT NULL,
		crystal INT UNSIGNED NOT NULL,
		name VARCHAR(32)
	);

	SET _done=0;
	OPEN _cursor;
	REPEAT
		FETCH _cursor INTO _crystal,_account_id;
		IF NOT _done THEN
			SELECT name INTO @_name FROM common_characters WHERE account_id=_account_id;
			INSERT INTO tmp_max_hacker VALUES (_account_id,_crystal,@_name);
		END IF;
		UNTIL _done END REPEAT;
	CLOSE _cursor;

	SELECT * FROM tmp_max_hacker ORDER BY _crystal DESC;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_small_char`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_small_char`;
DELIMITER ;;
CREATE PROCEDURE `deal_small_char`()
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _level INT UNSIGNED DEFAULT 0;
	DECLARE _crystal INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT DISTINCT(buyer_id) FROM gold_market_log WHERE action=2 AND total_price/(gold/10000)>20;
	
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	DROP TEMPORARY TABLE IF EXISTS `tmp_small_char`;
	CREATE TEMPORARY TABLE `tmp_small_char`(
		account_id BIGINT UNSIGNED NOT NULL,
		level INT UNSIGNED NOT NULL,
		crystal INT UNSIGNED NOT NULL,
		banned INT UNSIGNED NOT NULL
	);

	SET _done=0;
	OPEN _cursor;
	REPEAT
	FETCH _cursor INTO _account_id;
	IF NOT _done THEN
		SELECT level,crystal,banned INTO _level,_crystal,@_banned FROM common_characters WHERE account_id=_account_id;
		INSERT INTO tmp_small_char (account_id,level,crystal,banned) VALUES (_account_id,_level,_crystal,@_banned);
		UPDATE common_characters SET banned=1 WHERE account_id=_account_id AND binded=0 AND level<=20;
		
		
		
	END IF;
	UNTIL _done END REPEAT;
	CLOSE _cursor;

	SELECT * FROM tmp_small_char ORDER BY level DESC;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `kick_client_all`
-- ----------------------------
DROP PROCEDURE IF EXISTS `kick_client_all`;
DELIMITER ;;
CREATE PROCEDURE `kick_client_all`(IN `_version_appid` blob)
BEGIN
	IF NOT EXISTS (SELECT 1 FROM t_kick_client_all) THEN
		INSERT INTO t_kick_client_all VALUES (_version_appid);
	ELSE
		UPDATE t_kick_client_all SET version_appid=_version_appid;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_create`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_create`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_create`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_instance_id=0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND (@_alli_id:=alliance_id)>0) THEN
		SET _result = -1; # 没有联盟不能创建
		LEAVE label_body;
	END IF;

	CALL alli_instance_check_player(_account_id,0,_result);
	IF _result!=0 THEN
		SET _result = -1000+_result; # 创建条件不满足
		LEAVE label_body;
	END IF;

	START TRANSACTION;
	SET @_instance_id=game_global_id();
	INSERT INTO alli_instance (instance_id,creator_id,type,status,level,player_num,create_time,start_time,auto_combat,auto_supply,stop_level,retry_times,loot,alliance_id,combat_result,combat_log)
		VALUES (@_instance_id,_account_id,102,0,0,1,UNIX_TIMESTAMP(),0,0,0,0,0,'',@_alli_id,0,'');
	IF ROW_COUNT()=0 THEN
		SET _result = -2; # 插入失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 设置自己副本ID
	UPDATE alli_instance_player SET instance_id=@_instance_id,last_time=UNIX_TIMESTAMP() WHERE account_id=_account_id AND instance_id=0;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,@_instance_id;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_join`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_join`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_join`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _max_player_num INT UNSIGNED DEFAULT 0;
	
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND status=0 AND (@_alli_id:=alliance_id)>=0) THEN
		SET _result = -1; # 副本不存在
		LEAVE label_body;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND alliance_id=@_alli_id) THEN
		SET _result = -2; # 不是这个联盟的
		LEAVE label_body;
	END IF;
	CALL alli_instance_check_player(_account_id,_instance_id,_result);
	IF _result!=0 THEN
		SET _result = -1000+_result;
		LEAVE label_body;
	END IF;

	SELECT player_num INTO _max_player_num FROM excel_instance_list WHERE excel_id=102;
	START TRANSACTION;
	# @_max_player_num在alli_instance_check_player中赋值
	UPDATE alli_instance SET player_num=player_num+1 WHERE instance_id=_instance_id AND player_num<_max_player_num;
	IF ROW_COUNT()=0 THEN
		SET _result = -3; # 人数已满
		ROLLBACK;
		LEAVE label_body;
	END IF;
	UPDATE alli_instance_player SET instance_id=_instance_id,last_time=UNIX_TIMESTAMP() WHERE account_id=_account_id AND instance_id=0;
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_exit`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_exit`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_exit`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned,IN `_need_transaction` int unsigned,IN `_echo` int,OUT `_result` int)
BEGIN
	DECLARE _new_leader_id BIGINT DEFAULT 0;
	DECLARE _hero_ids BLOB;
	
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN SET _result=-100; ROLLBACK; IF(_echo=1)THEN SELECT _result; END IF; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001'BEGIN SET _result=-103; ROLLBACK; IF(_echo=1)THEN SELECT _result; END IF; END;

	SET _result = 0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alli_instance_player WHERE account_id=_account_id AND instance_id=_instance_id) THEN
		SET _result = -1; # 副本不存在
		LEAVE label_body;
	END IF;
	IF EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND status=2) THEN
		SET _result = -2; # 副本战斗中
		LEAVE label_body;
	END IF;

	SELECT type INTO @_combat_type FROM alli_instance WHERE instance_id=_instance_id;
	SELECT hero_ids INTO _hero_ids FROM alli_instance_player WHERE account_id=_account_id AND instance_id=_instance_id;
	IF _need_transaction=1 THEN
		START TRANSACTION;
	END IF;
	# 将领解锁
	IF _hero_ids!='' THEN
		SET _hero_ids=CONCAT(',', _hero_ids);
	END IF;
	CALL alli_instance_unlock_hero(_hero_ids,@_combat_type,_result);
	IF _result!=0 THEN
		SET _result = -3; # 解锁失败
		ROLLBACK;
		LEAVE label_body;
	END IF;

	# 修改本身状态
	UPDATE alli_instance_player SET instance_id=0,status=0,hero_ids='' WHERE account_id=_account_id AND instance_id=_instance_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -4; # 玩家副本状态修改失败
		ROLLBACK;
		LEAVE label_body;
	END IF;

	# 修改副本人数
	UPDATE alli_instance SET player_num=player_num-1 WHERE instance_id=_instance_id AND player_num>0;
	IF ROW_COUNT()=0 THEN
		SET _result = -5; # 修改副本人数失败
		ROLLBACK;
		LEAVE label_body;
	END IF;

	# 人数为0销毁副本
	IF EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND player_num=0) THEN
		DELETE FROM alli_instance WHERE instance_id=_instance_id;
		IF ROW_COUNT()=0 THEN
			SET _result = -6; # 销毁副本失败
			ROLLBACK;
			LEAVE label_body;
		END IF;
		# 副本销毁,直接退出副本
		LEAVE label_body;
	END IF;

	# 是队长则转让队长
	IF EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND creator_id=_account_id) THEN
		# 转让队长
		SELECT account_id INTO _new_leader_id FROM alli_instance_player WHERE instance_id=_instance_id LIMIT 1;
		IF FOUND_ROWS()=0 THEN
			SET _result = -7; # 转让失败
			ROLLBACK;
			LEAVE label_body;
		END IF;
		UPDATE alli_instance SET creator_id=_new_leader_id WHERE instance_id=_instance_id;
		IF ROW_COUNT()=0 THEN
			SET _result = -7; # 转让失败
			ROLLBACK;
			LEAVE label_body;
		END IF;
	END IF;
END label_body;
	IF (_echo=1) THEN 
		SELECT _result;
	END IF;
	IF _need_transaction=1 THEN
		COMMIT;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_kick`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_kick`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_kick`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned,IN `_obj_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	# 队长才能踢人
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND creator_id=_account_id AND (@_status:=status)>=0) THEN
		SET _result = -1; # 不是队长
		LEAVE label_body;
	END IF;

	IF @_status=2 THEN
		SET _result = -2; # 战斗中不能踢人
		LEAVE label_body;
	END IF;
	
	# 就是自己退出队伍
	CALL alli_instance_exit(_obj_id,_instance_id,1,0,_result);
	IF _result!=0 THEN
		SET _result = -10;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_destroy`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_destroy`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_destroy`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned,IN `_need_transaction` int unsigned,IN `_force` int unsigned,IN `_echo` int unsigned,OUT `_result` int)
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _member_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _ids BLOB;
	DECLARE _combat_over_time INT UNSIGNED DEFAULT 0;
	
	DECLARE _cursor CURSOR FOR SELECT account_id FROM alli_instance_player WHERE instance_id=_instance_id;

	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	SET _result = 0;
	
	SET _ids='';
label_body:BEGIN
	IF (_force=0) THEN
		IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND creator_id=_account_id AND status!=2) THEN
			SET _result = -1; # 战斗中不能销毁副本,队长才能销毁副本
			LEAVE label_body;
		END IF;
	ELSE
		# 这里可以加副本系统销毁的判断条件
		IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND status!=2) THEN
			SET _result = -1; # 战斗中不能销毁副本
			LEAVE label_body;
		END IF;
		
		SELECT val_int INTO _combat_over_time FROM excel_cmndef WHERE id=27;
		IF EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND status=1 AND (start_time+_combat_over_time)>UNIX_TIMESTAMP()) THEN
			SET _result = -2; # 开始战斗后,在战斗超时内不会被销毁
			LEAVE label_body;
		END IF;
	END IF;

	SELECT IFNULL(GROUP_CONCAT(account_id),'') INTO _ids FROM alli_instance_player WHERE instance_id=_instance_id;

	# 是否由上层来保证事务
	IF _need_transaction=1 THEN
		START TRANSACTION;
	END IF;
	SET _done=0;
	OPEN _cursor;
	REPEAT
		FETCH _cursor INTO _member_id;
		IF NOT _done THEN
			# 由这一层来进行事务ROLLBACK和COMMIT
			CALL alli_instance_exit(_member_id,_instance_id,0,0,_result);
			IF _result!=0 THEN
				SET _result = -3; # 有角色退出失败
				ROLLBACK;
				SET _ids=_member_id;
				LEAVE label_body;
			END IF;
		END IF;
	UNTIL _done END REPEAT;
	CLOSE _cursor;

	# 最后一个玩家退出的时候副本会自动销毁
END label_body;
	IF _echo=1 THEN
		SELECT _result,_ids;
	END IF;
	IF _need_transaction=1 THEN
		COMMIT;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_config_hero`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_config_hero`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_config_hero`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned,IN `_hero1` bigint unsigned,IN `_hero2` bigint unsigned,IN `_hero3` bigint unsigned,IN `_hero4` bigint unsigned,IN `_hero5` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN SET _result=-100; ROLLBACK; SELECT _result; END;
label_body:BEGIN
	# 英雄必须是自己的
	IF EXISTS (SELECT 1 FROM hire_heros WHERE hero_id IN (_hero1,_hero2,_hero3,_hero4,_hero5) AND account_id!=_account_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 副本必须是空闲状态
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND status=0 AND (@_combat_type:=type)>=0) THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM alli_instance_player WHERE account_id=_account_id AND instance_id=_instance_id AND (@_hero_ids:=hero_ids) IS NOT NULL) THEN
		SET _result = -3; # 不在副本中
		LEAVE label_body;
	END IF;
	START TRANSACTION;
	# 先解锁
	IF LENGTH(@_hero_ids)>0 THEN
		SET @_hero_ids=CONCAT(',', @_hero_ids);
		CALL alli_instance_unlock_hero(@_hero_ids,@_combat_type,_result);
		IF _result!=0 THEN
			SET _result = -4; # 英雄解锁失败
			ROLLBACK;
			LEAVE label_body;
		END IF;
	END IF;
	# 再锁住
	SET @_hero_ids='';
	IF (_hero1!=0) THEN SET @_hero_ids=CONCAT(@_hero_ids,'*0*0*',_hero1); END IF;
	IF (_hero2!=0) THEN SET @_hero_ids=CONCAT(@_hero_ids,'*0*0*',_hero2); END IF;
	IF (_hero3!=0) THEN SET @_hero_ids=CONCAT(@_hero_ids,'*0*0*',_hero3); END IF;
	IF (_hero4!=0) THEN SET @_hero_ids=CONCAT(@_hero_ids,'*0*0*',_hero4); END IF;
	IF (_hero5!=0) THEN SET @_hero_ids=CONCAT(@_hero_ids,'*0*0*',_hero5); END IF;
	IF @_hero_ids!='' THEN
		SET @_hero_ids=CONCAT(',', @_hero_ids);
	END IF;
	CALL alli_instance_lock_hero(@_hero_ids,@_combat_type,_result);
	IF _result!=0 THEN
		SET _result = -5; # 英雄加锁失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 更新副本武将
	UPDATE alli_instance_player SET hero_ids=@_hero_ids WHERE account_id=_account_id AND instance_id=_instance_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -6; # 更新武将配置失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_start`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_start`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_start`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_ids='';
label_body:BEGIN
	# 队长才能开始
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id AND creator_id=_account_id AND status=0) THEN
		SET _result = -1; # 不是队长
		LEAVE label_body;
	END IF;
	SELECT GROUP_CONCAT(account_id) INTO @_ids FROM alli_instance_player WHERE instance_id=_instance_id AND status=0;
	IF LENGTH(@_ids)>0 THEN
		SET _result = -2; # 有人未准备就绪
		LEAVE label_body;
	END IF;
	SELECT GROUP_CONCAT(account_id) INTO @_ids FROM alli_instance_player WHERE instance_id=_instance_id AND status=1;
	IF LENGTH(@_ids)=0 THEN
		SET _result = -3; # 没人也不能开始
		LEAVE label_body;
	END IF;

	# 修改副本状态
	UPDATE alli_instance SET status=1,start_time=UNIX_TIMESTAMP(),level=1 WHERE instance_id=_instance_id AND status=0;
	IF ROW_COUNT()=0 THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
	# 修改玩家进入次数,副本状态
	UPDATE alli_instance_player SET day_times_free=day_times_free+1,status=2 WHERE instance_id=_instance_id;
END label_body;
	SELECT _result,@_ids;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_ready`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_ready`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_ready`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned,IN `_status` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alli_instance_player WHERE instance_id=_instance_id AND account_id=_account_id AND (@_status:=status)>=0 AND (@_hero_ids:=hero_ids) IS NOT NULL) THEN
		SET _result = -1;	# 副本不存在
		LEAVE label_body;
	END IF;

	IF _status!=0 AND (LENGTH(@_hero_ids)=0) THEN
		SET _result = -2;	# 准备就绪则必须配置将领
		LEAVE label_body;
	END IF;
	IF @_status=_status THEN
		LEAVE label_body;
	END IF;
	UPDATE alli_instance_player SET status=_status WHERE account_id=_account_id AND instance_id=_instance_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;	# 设置失败
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_list`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_list`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_list`(IN `_account_id` bigint unsigned)
BEGIN
	IF EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND (@_alli_id:=alliance_id)>0) THEN
		SELECT i.instance_id,i.player_num,i.creator_id,c.name,c.level,c.head_id FROM alli_instance i JOIN common_characters c ON i.creator_id=c.account_id WHERE i.status=0 AND i.alliance_id=@_alli_id ORDER BY i.create_time ASC;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_status`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_status`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_status`(IN `_account_id` bigint unsigned, IN `_combat_type` int unsigned)
BEGIN
	SET @_max_instance_level=0;
	
label_body:BEGIN
	CASE _combat_type
		WHEN 102 THEN
			SELECT e.max_instance_level INTO @_max_instance_level FROM excel_alliance_congress e JOIN alliance_buildings a ON e.level=a.level 
				JOIN common_characters c ON a.alliance_id=c.alliance_id
				WHERE a.excel_id=1 AND c.account_id=_account_id;
		ELSE
			LEAVE label_body; 
	END CASE;
	# 总次数,已使用次数,副本ID,玩家副本状态,副本状态,副本关卡,自动战斗,自动补给,重试次数,停止关卡,最大关卡,上一场战斗结果
	SELECT e.day_times_free
		,IFNULL(p.day_times_free,0),IFNULL(p.instance_id,0),IFNULL(p.status,0)
		,IFNULL(i.status,0),IFNULL(i.level,0),IFNULL(i.auto_combat,0),IFNULL(i.auto_supply,0)
		,IFNULL(i.retry_times,0),IFNULL(i.creator_id,0),IFNULL(i.stop_level,0),@_max_instance_level,IFNULL(i.combat_result,0) 
		FROM excel_instance_list e LEFT JOIN alli_instance_player p ON p.account_id=_account_id 
		LEFT JOIN alli_instance i ON i.instance_id=p.instance_id WHERE e.excel_id=_combat_type;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_char_data`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_char_data`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_char_data`(IN `_instance_id` bigint unsigned)
BEGIN
label_body:BEGIN
	SELECT i.status,c.account_id,c.name,c.level,c.head_id FROM alli_instance_player i JOIN common_characters c ON i.account_id=c.account_id 
		WHERE i.instance_id=_instance_id ORDER BY i.last_time ASC;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_hero_data`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_hero_data`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_hero_data`(IN `_instance_id` bigint unsigned)
BEGIN
	DECLARE _hero_ids BLOB;
	
label_body:BEGIN
	# 将领布局
	SELECT IFNULL(GROUP_CONCAT(hero_ids),'') INTO @_all_ids FROM alli_instance_player WHERE instance_id=_instance_id;
	IF @_all_ids='' THEN
		LEAVE label_body;
	END IF;

	# 临时表格用于存储将领数据
	DROP TEMPORARY TABLE IF EXISTS `tmp_hero_info`;
	CREATE TEMPORARY TABLE `tmp_hero_info`(
		hero_id BIGINT UNSIGNED NOT NULL,
		name VARCHAR(32) NOT NULL,
		account_id BIGINT UNSIGNED NOT NULL,
		level INT UNSIGNED NOT NULL,
		health INT UNSIGNED NOT NULL,
		prof INT UNSIGNED NOT NULL,
		army_type INT UNSIGNED NOT NULL,
		army_level INT UNSIGNED NOT NULL,
		army_num INT UNSIGNED NOT NULL,
		row INT UNSIGNED NOT NULL,
		col INT UNSIGNED NOT NULL,
		head_id INT UNSIGNED NOT NULL,
		PRIMARY KEY(`hero_id`)
	);

	SET @_all_ids = CONCAT(',',@_all_ids);
	SET @_i = 1;
	SET @_imax = _get_char_count(@_all_ids,',');
	WHILE @_i<=@_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_all_ids,',',@_i+1),',',-1) INTO @_one_ids;
		SET @_j = 1;
		SET @_jmax = _get_char_count(@_one_ids,'*');
		WHILE @_j<=@_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_one_ids,'*',@_j+1),'*',-1) INTO @_row;
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_one_ids,'*',@_j+2),'*',-1) INTO @_col;
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_one_ids,'*',@_j+3),'*',-1) INTO @_hero_id;
			SELECT account_id,level,healthstate,army_prof,army_type,army_level,army_num,name,model
				INTO @_account_id,@_level,@_health,@_prof,@_army_type,@_army_level,@_army_num,@_name,@_head_id
				FROM hire_heros WHERE hero_id=@_hero_id;
			IF FOUND_ROWS()!=0 THEN
				INSERT INTO tmp_hero_info (hero_id,account_id,level,health,prof,army_type,army_level,army_num,row,col,name,head_id) 
					VALUES(@_hero_id,@_account_id,@_level,@_health,@_prof,@_army_type,@_army_level,@_army_num,@_row,@_col,@_name,@_head_id);
			END IF;
			SET @_j = @_j+3;
		END WHILE;
		SET @_i = @_i+1;
	END WHILE;
	SELECT hero_id,name,account_id,level,health,prof,army_type,army_level,army_num,row,col,head_id FROM tmp_hero_info;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_loot`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_loot`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_loot`(IN `_instance_id` bigint unsigned)
BEGIN
	SELECT loot FROM alli_instance WHERE instance_id=_instance_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_start_combat`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_start_combat`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_start_combat`(IN `_account_id` bigint unsigned,IN `_obj_id` bigint unsigned,IN `_combat_type` int unsigned,IN `_auto_combat` int unsigned,IN `_auto_supply` int unsigned,IN `_stop_level` int unsigned,IN `_hero_deploy` blob,IN `_need_save_deploy` int unsigned,IN `_need_transaction` int unsigned,IN `_echo` int unsigned,OUT `_result` int)
BEGIN
	DECLARE _event_id,_instance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _use_time,_time_now INT UNSIGNED DEFAULT 0;
	DECLARE _my_rank,_obj_rank,_used_times,_max_times INT UNSIGNED DEFAULT 0;
	DECLARE _src_hero_deploy BLOB;
	
	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result = -100; IF (_echo=1) THEN SELECT _result,_event_id,_use_time; END IF; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result = -103; IF (_echo=1) THEN SELECT _result,_event_id,_use_time; END IF; END;


	SET _result=0;
	# 不需要计算出征队列、不需要将领加锁
label_body:BEGIN
	IF (LENGTH(_hero_deploy)=0) AND (_combat_type!=200) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	CASE _combat_type
	WHEN 102 THEN
		# 副本是否处于可开战状态,队长才能发起战斗
		IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_obj_id AND status=1 AND creator_id=_account_id) THEN
			SET _result = -3; 
			LEAVE label_body;
		END IF;
		
		# 校验是否这个副本的英雄
		IF _need_save_deploy=1 THEN
			SELECT GROUP_CONCAT(hero_ids) INTO _src_hero_deploy FROM alli_instance_player WHERE instance_id=_obj_id;
			IF _src_hero_deploy!='' THEN
				SET _src_hero_deploy=CONCAT(',', _src_hero_deploy);
			END IF;
			CALL _check_hero_deploy(_src_hero_deploy,_hero_deploy,_result);
			IF _result!=0 THEN
				SET _result = -30;
				LEAVE label_body;
			END IF;
		END IF;
	WHEN 200 THEN
		SELECT rank INTO _obj_rank FROM arena WHERE account_id=_obj_id;
		IF FOUND_ROWS()=0 THEN
			SET _result = -4; # 目标不存在
			LEAVE label_body;
		END IF;
		SELECT rank,used_times INTO _my_rank,_used_times FROM arena WHERE account_id=_account_id;
		IF FOUND_ROWS()=0 THEN
			SET _result = -14; # 必须先上传数据才能参加竞技场
			LEAVE label_body;
		END IF;
		IF (_obj_rank>=_my_rank) THEN
			SET _result = -15; # 不能挑战排名比自己低的
			LEAVE label_body;
		END IF;
		IF NOT EXISTS (SELECT 1 FROM excel_arena WHERE rank_diff>=(_my_rank-_obj_rank)) THEN
			SET _result = -16; # 不能挑战比自己排名高太多的
			LEAVE label_body;
		END IF;
		SELECT free_times INTO _max_times FROM excel_arena;
		IF (_used_times>=_max_times) THEN
			SET _result = -17; # 次数用光
			LEAVE label_body;
		END IF;
	ELSE
		SET _result = -20; # 战斗类型不对
		LEAVE label_body;
	END CASE;
	
	IF _need_transaction=1 THEN
		START TRANSACTION;
	END IF;
	
	CASE _combat_type
	WHEN 102 THEN
		# 将将领布局放入表格中
		IF _need_save_deploy=1 THEN
			CALL alli_instance_save_hero_deploy(_account_id,_hero_deploy,0,0,_result);
			IF _result!=0 THEN
				SET _result = -5;
				LEAVE label_body;
			END IF;
		END IF;

		# 自动战斗开关,自动补给开关,停止关卡
		UPDATE alli_instance SET auto_combat=_auto_combat,auto_supply=_auto_supply,stop_level=_stop_level,status=2 WHERE instance_id=_obj_id;
	WHEN 200 THEN
		UPDATE arena SET used_times=used_times+1 WHERE account_id=_account_id AND used_times<_max_times;
		IF ROW_COUNT()=0 THEN
			SET _result = -17; # 次数用光
			LEAVE label_body;
		END IF;
	ELSE
		SET _result = -20;
		LEAVE label_body;
	END CASE;
	
	# 添加时间事件
	SET _event_id = game_global_id();
	SET _time_now = UNIX_TIMESTAMP();
	SELECT go_time INTO _use_time FROM excel_combat WHERE excel_id=_combat_type;
	INSERT INTO alli_instance_combat_event (event_id,account_id,march_type,combat_type,obj_id,hero_deploy,combat_log,combat_result) 
		VALUES (_event_id,_account_id,1,_combat_type,_obj_id,_hero_deploy,'',0);
	IF ROW_COUNT()=0 THEN
		SET _result = -10; # 添加时间事件失败
		ROLLBACK;
		LEAVE label_body;
	END IF;
	INSERT INTO time_events (event_id,begin_time,end_time,type,locked) 
		VALUES (_event_id,_time_now,_time_now+_use_time,9,0);
END label_body;
	IF (_echo=1) THEN
		SELECT _result,_event_id,_use_time;
	END IF;
	IF _need_transaction=1 THEN
		COMMIT;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_save_hero_deploy`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_save_hero_deploy`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_save_hero_deploy`(IN `_account_id` bigint unsigned,IN `_hero_deploy` blob,IN `_need_check_hero_deploy` int unsigned,IN `_echo` int,OUT `_result` int)
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _i,_imax,_j,_jmax,_col,_row INT UNSIGNED DEFAULT 0;
	DECLARE _member_id,_instance_id,_hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _member_deploy,_one_ids,_src_hero_deploy BLOB DEFAULT '';

	DECLARE _cursor CURSOR FOR SELECT account_id,hero_deploy FROM tmp_hero_deploy;

	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	SET _result = 0;
label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE creator_id=_account_id) THEN
		SET _result = -1; # 不是队长
		LEAVE label_body;
	END IF;
	SELECT instance_id INTO _instance_id FROM alli_instance WHERE creator_id=_account_id;
	
	IF _need_check_hero_deploy=1 THEN
		SELECT GROUP_CONCAT(hero_ids) INTO _src_hero_deploy FROM alli_instance_player WHERE instance_id=_instance_id;
		IF _src_hero_deploy!='' THEN
			SET _src_hero_deploy=CONCAT(',', _src_hero_deploy);
		END IF;
		CALL _check_hero_deploy(_src_hero_deploy,_hero_deploy,_result);
		IF _result != 0 THEN
			SET _result = -3; # 校验英雄布局失败
			LEAVE label_body;
		END IF;
	END IF;

	DROP TEMPORARY TABLE IF EXISTS `tmp_hero_deploy`;
	CREATE TEMPORARY TABLE `tmp_hero_deploy`(
		account_id BIGINT UNSIGNED NOT NULL,
		hero_deploy BLOB NOT NULL
	);

	INSERT INTO tmp_hero_deploy (SELECT account_id,'' FROM alli_instance_player WHERE instance_id=_instance_id);
	IF ROW_COUNT()=0 THEN
		SET _result = -2; # 副本为空
		LEAVE label_body;
	END IF;

	# 分解配置
	SET _i = 1;
	SET _imax = _get_char_count(_hero_deploy,',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_hero_deploy,',',_i+1),',',-1) INTO _one_ids;
		SET _j = 1;
		SET _jmax = _get_char_count(_one_ids,'*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+1),'*',-1) INTO _row;
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+2),'*',-1) INTO _col;
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+3),'*',-1) INTO _hero_id;
			SELECT account_id INTO _member_id FROM hire_heros WHERE hero_id=_hero_id;
			IF FOUND_ROWS()=0 THEN
				SET _result = -10;
				LEAVE label_body;
			END IF;
			UPDATE tmp_hero_deploy SET hero_deploy=CONCAT(hero_deploy,'*',_row,'*',_col,'*',_hero_id) WHERE account_id=_member_id;
			# 配置的将领是否都属于该副本
			IF ROW_COUNT()=0 THEN
				SET _result = -11; 
				LEAVE label_body;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;

	# 更新英雄配置
	SET _done=0;
	OPEN _cursor;
	REPEAT
		FETCH _cursor INTO _member_id,_member_deploy;
		IF NOT _done THEN
			UPDATE alli_instance_player SET hero_ids=_member_deploy WHERE account_id=_member_id;
		END IF;
	UNTIL _done END REPEAT;
	CLOSE _cursor;
END label_body;
	IF _echo=1 THEN
		SELECT _result;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_check_player`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_check_player`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_check_player`(IN `_account_id` bigint unsigned,IN `_instance_id` bigint unsigned,OUT `_result` int)
BEGIN
	DECLARE _already_instance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _done_day_times_free,_last_time,_day_times_free,_min_level,_retry_times,_max_player_num,_player_num,_status INT UNSIGNED DEFAULT 0;
	DECLARE _year,_month,_day,_week,_week_day,_hour,_cost_item BLOB;
	
	# 验证是否能够创建或者加入副本
	SET _result = 0;
	SET _year='',_month='',_day='',_week_day='',_week='',_hour='',_cost_item='';
	
label_body:BEGIN
	SET _already_instance_id=0,_done_day_times_free=0,_last_time=0;
	IF NOT EXISTS (SELECT 1 FROM alli_instance_player WHERE account_id=_account_id) THEN
		INSERT IGNORE INTO alli_instance_player (account_id,instance_id,hero_ids,day_times_free,last_time,status) 
			VALUES (_account_id,0,'',0,0,0);
	END IF;
	# 修改联盟副本次数
	UPDATE alli_instance_player SET day_times_free=0,last_time=UNIX_TIMESTAMP() WHERE account_id=_account_id AND DATE(FROM_UNIXTIME(last_time))!=DATE(NOW());

	SELECT instance_id,day_times_free,last_time INTO _already_instance_id,_done_day_times_free,_last_time 
		FROM alli_instance_player WHERE account_id=_account_id;
	IF (_already_instance_id>0) THEN
		SET _result = -1;	# 有副本尚未退出
		LEAVE label_body;
	END IF;

	SELECT open_year,open_month,open_day,open_weekday,open_hour,day_times_free,player_num,pre_level,retry_times,cost_item 
		INTO _year,_month,_day,_week_day,_hour,_day_times_free,_max_player_num,_min_level,_retry_times,_cost_item
		FROM excel_instance_list WHERE excel_id=102;
	IF FOUND_ROWS()=0 THEN
		SET _result = -2; # 副本不存在
		LEAVE label_body;
	END IF;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND level>=_min_level) THEN
		SET _result = -3; # 等级太低
		LEAVE label_body;
	END IF;
	IF (game_check_tick(_year,_month,_day,_week,_hour) < 0) THEN
		SET _result = -4; # 不在开放时段
		LEAVE label_body;
	END IF;

	IF _done_day_times_free>=_day_times_free THEN
		SET _result = -5; # 次数达到上限
		LEAVE label_body;
	END IF;

	IF (_instance_id>0) THEN
		IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_instance_id) THEN
			SET _result = -10; # 副本不存在
			LEAVE label_body;
		END IF;
		SELECT player_num,status INTO _player_num,_status FROM alli_instance WHERE instance_id=_instance_id;
		IF (_player_num>=_max_player_num) THEN
			SET _result = -11; # 人数已满
			LEAVE label_body;
		END IF;
		IF (_status!=0) THEN
			SET _result = -12; # 副本已经开始
			LEAVE label_body;
		END IF;
	END IF;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_lock_hero`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_lock_hero`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_lock_hero`(IN `_hero_deploy` blob,IN `_combat_type` INT UNSIGNED,OUT `_result` int)
BEGIN
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _i,_imax,_j,_jmax INT UNSIGNED DEFAULT 0;
	DECLARE _one BLOB;
	
	SET _result = 0;

label_body:BEGIN

	SET _i = 1;
	SET _imax = _get_char_count(_hero_deploy, ',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_hero_deploy,',',_i+1),',',-1) INTO _one;
		SET _j = 1;
		SET _jmax = _get_char_count(_one, '*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one,'*',_j+3),'*',-1) INTO _hero_id;
			IF _hero_id!=0 THEN
				UPDATE hire_heros SET status=_combat_type WHERE hero_id=_hero_id AND status=0 AND army_num>0;
				IF ROW_COUNT()=0 THEN
					SET _result = -10; # 将领没锁住
					LEAVE label_body;
				END IF;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_unlock_hero`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_unlock_hero`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_unlock_hero`(IN `_hero_deploy` blob,IN `_combat_type` INT UNSIGNED,OUT `_result` int)
BEGIN
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _i,_imax,_j,_jmax INT UNSIGNED DEFAULT 0;
	DECLARE _one BLOB;
	
	SET _result = 0;

label_body:BEGIN

	SET _i = 1;
	SET _imax = _get_char_count(_hero_deploy, ',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_hero_deploy,',',_i+1),',',-1) INTO _one;
		SET _j = 1;
		SET _jmax = _get_char_count(_one, '*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one,'*',_j+3),'*',-1) INTO _hero_id;
			IF _hero_id!=0 THEN
				UPDATE hire_heros SET status=0 WHERE hero_id=_hero_id AND status=_combat_type;
				IF ROW_COUNT()=0 THEN
					SET _result = -10; # 将领解锁失败
					LEAVE label_body;
				END IF;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_attacker_combat_data`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_attacker_combat_data`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_attacker_combat_data`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _i,_j,_imax,_jmax INT UNSIGNED DEFAULT 0;
	DECLARE _row,_col,_prof,_army_type,_army_level,_army_num,_level,_combat_type INT UNSIGNED DEFAULT 0;
	DECLARE _account_id,_hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _char_name,_hero_name VARCHAR(32) DEFAULT '';
	DECLARE _hero_deploy,_one_ids,_combat_data BLOB;

label_body:BEGIN
	SET _combat_data = '';
	
	SELECT hero_deploy,combat_type,account_id INTO _hero_deploy,_combat_type,_account_id FROM alli_instance_combat_event WHERE event_id=_event_id;
	IF FOUND_ROWS()=0 THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;

	CASE _combat_type
		WHEN 102 THEN
			DROP TEMPORARY TABLE IF EXISTS `tmp_hero_info`;
			CREATE TEMPORARY TABLE `tmp_hero_info`(
				char_name VARCHAR(32) NOT NULL,
				hero_id BIGINT UNSIGNED NOT NULL,
				hero_name VARCHAR(32) NOT NULL,
				level INT UNSIGNED NOT NULL,
				prof INT UNSIGNED NOT NULL,
				army_type INT UNSIGNED NOT NULL,
				army_level INT UNSIGNED NOT NULL,
				army_num INT UNSIGNED NOT NULL,
				row INT UNSIGNED NOT NULL,
				col INT UNSIGNED NOT NULL,
				PRIMARY KEY(`hero_id`)
			);
			
			SET _i = 1;
			SET _imax = _get_char_count(_hero_deploy,',');
			WHILE _i<=_imax DO
				SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_hero_deploy,',',_i+1),',',-1) INTO _one_ids;
				SET _j = 1;
				SET _jmax = _get_char_count(_one_ids,'*');
				WHILE _j<=_jmax DO
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+1),'*',-1) INTO _row;
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+2),'*',-1) INTO _col;
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+3),'*',-1) INTO _hero_id;
					SELECT h.army_prof,h.army_type,h.army_level,h.army_num,h.name,h.level,c.name
						INTO _prof,_army_type,_army_level,_army_num,_hero_name,_level,_char_name
						FROM hire_heros h JOIN common_characters c ON h.account_id=c.account_id WHERE hero_id=_hero_id;
					IF FOUND_ROWS()!=0 THEN
						INSERT INTO tmp_hero_info (row,col,char_name,hero_id,hero_name,prof,army_type,army_level,army_num,level) 
							VALUES(_row,_col,_char_name,_hero_id,_hero_name,_prof,_army_type,_army_level,_army_num,_level);
					END IF;
					SET _j = _j+3;
				END WHILE;
				SET _i = _i+1;
			END WHILE;
			
			# ,*row*col*char_name*hero_id*hero_level*hero_name*prof*army_type*army_level*army_num
			SELECT IFNULL(GROUP_CONCAT(CONCAT('*',CONCAT_WS('*',row,col,char_name,hero_id,level,hero_name,prof,army_type,army_level,army_num))),'') INTO _combat_data FROM tmp_hero_info;
			IF _combat_data!='' THEN
				SET _combat_data=CONCAT(',', _combat_data);
			END IF;
		WHEN 200 THEN
			SELECT combat_data INTO _combat_data FROM arena WHERE account_id=_account_id;
		ELSE
			SET _result = -10;
	END CASE;
END label_body;
	SELECT _result,_combat_data;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_defender_combat_data`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_defender_combat_data`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_defender_combat_data`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _imax INT UNSIGNED DEFAULT 0;
	DECLARE _army_level,_army_num,_army_type,_prof,_row,_col INT UNSIGNED DEFAULT 0;
	DECLARE _combat_type INT UNSIGNED DEFAULT 0;
	DECLARE _obj_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _army_deploy,_army_data,_combat_data BLOB;

	SET _combat_data = '';
label_body:BEGIN
	SELECT combat_type,obj_id INTO _combat_type,_obj_id FROM alli_instance_combat_event WHERE event_id=_event_id;
	IF FOUND_ROWS()=0 THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	
	CASE _combat_type
		WHEN 102 THEN
			DROP TEMPORARY TABLE IF EXISTS `tmp_hero_info`;
			CREATE TEMPORARY TABLE `tmp_hero_info`(
				char_name VARCHAR(32) NOT NULL,
				hero_id BIGINT UNSIGNED NOT NULL,
				hero_name VARCHAR(32) NOT NULL,
				level INT UNSIGNED NOT NULL,
				prof INT UNSIGNED NOT NULL,
				army_type INT UNSIGNED NOT NULL,
				army_level INT UNSIGNED NOT NULL,
				army_num INT UNSIGNED NOT NULL,
				row INT UNSIGNED NOT NULL,
				col INT UNSIGNED NOT NULL
			);
		WHEN 200 THEN
			SET _result = 0;
		ELSE
			SET _result = -10;
			LEAVE label_body;
	END CASE;
	
	CASE _combat_type
		WHEN 102 THEN
			SELECT e.army_data,e.army_deploy INTO _army_data,_army_deploy FROM excel_alli_instance e JOIN alli_instance i ON e.level_id=i.level WHERE i.instance_id=_obj_id;
		
			CALL game_split_2(_army_data,'*',_army_level,_army_num);
			SET _col = 0;
			SET _imax = _get_char_count(_army_deploy,'*');
			WHILE _col<=_imax DO
				SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_army_deploy,'*',_col+1),'*',-1) INTO _army_type;
				SET _row=1;
				SELECT (attack*_army_num+defense*_army_num*0.9+life*_army_num*0.8)/3 INTO _prof FROM excel_soldier WHERE excel_id=_army_type AND level=_army_level;
				WHILE _row<=5 DO
					INSERT INTO tmp_hero_info (row,col,char_name,hero_id,level,hero_name,prof,army_type,army_level,army_num) 
								VALUES(_row,_col+1,'Guard',_row*10000+_col+1,0,'Guard',_prof,_army_type,_army_level,_army_num);
					SET _row=_row+1;
				END WHILE;
				SET _col = _col+1;
			END WHILE;
			
			# ,*row*col*char_name*hero_id*hero_level*hero_name*prof*army_type*army_level*army_num
			SELECT IFNULL(GROUP_CONCAT(CONCAT('*',CONCAT_WS('*',row,col,char_name,hero_id,level,hero_name,prof,army_type,army_level,army_num))),'') INTO _combat_data FROM tmp_hero_info;
			IF _combat_data!='' THEN
				SET _combat_data=CONCAT(',', _combat_data);
			END IF;
		WHEN 200 THEN
			SELECT combat_data INTO _combat_data FROM arena WHERE account_id=_obj_id; 
		ELSE
			SET _result = -10;
			LEAVE label_body;
	END CASE;
END label_body;
	SELECT _result,_combat_data;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_alli_instance_march_advance_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_alli_instance_march_advance_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_alli_instance_march_advance_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SET @_result=_release_te_lock(_event_id); SELECT _result; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SET @_result=_release_te_lock(_event_id); SELECT _result; END;

label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET  _result = -150;
		LEAVE label_body;
	END IF;
	
	SET @_attack_restore_percent=0,@_defense_restore_percent=0;
	IF NOT EXISTS (SELECT 1 FROM alli_instance_combat_event WHERE event_id=_event_id AND (@_account_id:=account_id)>=0 AND march_type=1 
									AND (@_obj_id:=obj_id)>=0 AND (@_combat_type:=combat_type)>=0) THEN
		SET _result = -1; # 时间事件不存在
		LEAVE label_body;
	END IF;
	
	CASE @_combat_type
		WHEN 102 THEN
			# 恢复概率
			SELECT val_int/100 INTO @_attack_restore_percent FROM excel_cmndef WHERE id=5;
			SET @_defense_restore_percent = 0;
		WHEN 200 THEN
			SET @_attack_restore_percent = 1;
			SET @_defense_restore_percent = 1;
		ELSE
			SET _result = -10;
			LEAVE label_body;
	END CASE;
	
	START TRANSACTION;
	# 添加战斗处理,等待战斗服务器计算
	INSERT INTO alli_instance_combat (event_id,account_id,obj_id,combat_type,attack_restore_percent,defense_restore_percent) 
								VALUES (_event_id,@_account_id,@_obj_id,@_combat_type,@_attack_restore_percent,@_defense_restore_percent);
	IF ROW_COUNT()=0 THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 删除时间事件
	DELETE FROM time_events WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -3;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	COMMIT;
	
END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result=_release_te_lock(_event_id);
	END IF;
	SELECT _result;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_alli_instance_combat_result`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_alli_instance_combat_result`;
DELIMITER ;;
CREATE PROCEDURE `deal_alli_instance_combat_result`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result,_done INT DEFAULT 0;
	DECLARE _account_id,_member_id BIGINT DEFAULT 0;
	DECLARE _need_reward INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT account_id,need_reward FROM tmp_char_reward;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SET @_result=_release_combat_lock(_event_id); SELECT _result,@_use_time,@_cur_level,@_combat_result,@_combat_type,_account_id,@_obj_id,@_need_refresh_char_atb,@_mail_ids,@_notify,@_hero_ids,@_account_ids; END;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SET @_result=_release_combat_lock(_event_id); SELECT _result,@_use_time,@_cur_level,@_combat_result,@_combat_type,_account_id,@_obj_id,@_need_refresh_char_atb,@_mail_ids,@_notify,@_hero_ids,@_account_ids; END;

	SET @_use_time=0,@_cur_level=0,@_combat_result=0,@_combat_type=0,@_account_id=0,@_obj_id=0,@_ext_data='',@_mail_ids='',@_notify='',@_hero_ids='';
	SET @_char_names='',@_need_notify=0,@_need_refresh_char_atb=0;
	SET @_mail_text='',@_noreward_mail_text='',@_account_ids='';

label_body:BEGIN
	IF _get_combat_lock(_event_id)=0 THEN
		SET _result = -1;	# 加锁失败
		LEAVE label_body;
	END IF;

	IF NOT EXISTS (SELECT 1 FROM alli_instance_combat_result WHERE event_id=_event_id 
					AND (@_combat_result:=result)>=0 AND (@_attack_data:=attack_data) IS NOT NULL 
					AND (@_defense_data:=defense_data) IS NOT NULL AND (@_ext_data:=UNCOMPRESS(ext_data)) IS NOT NULL
					AND (@_loot_list:=loot_list) IS NOT NULL) THEN
		SET _result = -2; # 获取数据失败
		LEAVE label_body;
	END IF;

	SELECT account_id,combat_type,obj_id INTO _account_id,@_combat_type,@_obj_id FROM alli_instance_combat_event WHERE event_id=_event_id;
	IF @_combat_type=102 THEN
		SELECT level INTO @_cur_level FROM alli_instance i JOIN alli_instance_combat_event e ON i.instance_id=e.obj_id WHERE e.event_id=_event_id;
	END IF;
	
	CASE @_combat_type
		WHEN 102 THEN
			# 用于判断是否需要给君主发送奖励
			DROP TEMPORARY TABLE IF EXISTS `tmp_char_reward`;
			CREATE TEMPORARY TABLE `tmp_char_reward`
			(
				account_id BIGINT UNSIGNED NOT NULL,
				need_reward INT UNSIGNED NOT NULL DEFAULT 0
			);
			INSERT INTO tmp_char_reward (SELECT account_id,0 FROM alli_instance_player WHERE instance_id=@_obj_id);
				
			IF @_combat_result=1 THEN
				# 邮件
				SET @_item_excel_id=0,@_item_num=0;
				IF (LENGTH(@_loot_list)!=0) THEN
					CALL game_split_2(@_loot_list,'*',@_item_excel_id,@_item_num);
					IF @_item_excel_id!=0 AND @_item_num!=0 THEN
						# 邮件内容
						SELECT content INTO @_mail_text FROM excel_text WHERE excel_id=473;
						SELECT name INTO @_item_name FROM excel_item_list WHERE excel_id=@_item_excel_id;
						SELECT title INTO @_instance_name FROM excel_instance_list WHERE excel_id=@_combat_type;
						SET @_instance_name = CONCAT(@_instance_name,' Lv ',@_cur_level);
						SET @_mail_text = REPLACE(@_mail_text,'%s1',@_instance_name);
						SET @_mail_text = CONCAT(@_mail_text,@_item_name,' x',@_item_num);
						SET @_mail_text = CONCAT('[00FF00]',@_mail_text,'[-]');
						# 无奖励邮件
						SELECT content INTO @_noreward_mail_text FROM excel_text WHERE excel_id=479;
						SET @_noreward_mail_text = REPLACE(@_noreward_mail_text,'%s1',@_instance_name);
						SET @_noreward_mail_text = CONCAT('[00FF00]',@_noreward_mail_text,'[-]');
						# 通知内容
						SET @_need_notify=1;
						IF @_need_notify=1 THEN
							SELECT content INTO @_notify FROM excel_text WHERE excel_id=475;
							SET @_notify = REPLACE(@_notify,'%s2',@_instance_name);
							SET @_notify = CONCAT(@_notify,@_item_name,' x',@_item_num);
						END IF;
					END IF;
				END IF;
				# account_id用于刷新简单信息
				SELECT IFNULL(GROUP_CONCAT(account_id),'') INTO @_account_ids FROM tmp_char_reward;
			ELSE
				# 拼凑邮件
				SELECT content INTO @_mail_text FROM excel_text WHERE excel_id=474;
				SELECT title INTO @_instance_name FROM excel_instance_list WHERE excel_id=@_combat_type;
				SET @_instance_name = CONCAT(@_instance_name,' Lv ',@_cur_level);
				SET @_mail_text = REPLACE(@_mail_text,'%s1',@_instance_name);
				SET @_mail_text = CONCAT('[FF0000]',@_mail_text,'[-]');
			END IF;
		WHEN 200 THEN
			SELECT content INTO @_mail_text FROM excel_text WHERE excel_id=477;
			SELECT name INTO @_char_name FROM common_characters WHERE account_id=_account_id;
			SET @_mail_text = REPLACE(@_mail_text,'%s1',@_char_name);
		ELSE
			SET _result = 0;
	END CASE;
	
	START TRANSACTION;
	IF @_combat_result=1 THEN # 胜利
		CASE @_combat_type
			WHEN 102 THEN
				# LP刷新君主信息
				SET @_need_refresh_char_atb=1;
				
				SELECT awa_exp,awa_gold,awa_exp_char INTO @_hero_exp,@_gold,@_char_exp FROM excel_alli_instance WHERE level_id=@_cur_level;
				
				# 更新掉落列表
				IF @_item_excel_id!=0 AND @_item_num!=0 THEN
					UPDATE alli_instance SET loot=CONCAT(loot,'*',@_item_excel_id,'*',@_item_num) WHERE instance_id=@_obj_id;
				END IF;
				
				# 将领士兵,经验
				SET @_i=1;
				SET @_imax=_get_char_count(@_attack_data,'*');
				WHILE @_i<=@_imax DO
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_attack_data,'*',@_i+1),'*',-1) INTO @_hero_id;
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_attack_data,'*',@_i+2),'*',-1) INTO @_army_dead_num;
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_attack_data,'*',@_i+3),'*',-1) INTO @_army_raw_num;
					IF @_hero_ids='' THEN
						SET @_hero_ids=@_hero_id;
					ELSE
						SET @_hero_ids=CONCAT(@_hero_ids,',',@_hero_id);
					END IF;
					UPDATE hire_heros SET army_num=IF(army_num>@_army_dead_num,army_num-@_army_dead_num,0) WHERE hero_id=@_hero_id AND (@_account_id:=account_id)>=0;
					# 计算战力
					IF ROW_COUNT()!=0 THEN
						CALL game_calc_heroall(@_hero_id,0,0,1,@_result);
					END IF;
					SET @_reward_hero=1;
					IF @_army_raw_num=0 THEN
						SET @_reward_hero = 0;
					ELSE
						UPDATE tmp_char_reward SET need_reward=1 WHERE account_id=@_account_id;
					END IF;
					IF (@_hero_exp!=0) AND (@_reward_hero=1) THEN
						CALL add_hero_exp(@_account_id,@_hero_id,@_hero_exp,@_result);
					END IF;
					SET @_i=@_i+3;
				END WHILE;

				# 君主经验,黄金,掉落
				OPEN _cursor;
				SET _done = 0;
				REPEAT
					FETCH _cursor INTO _member_id,_need_reward;
					IF NOT _done THEN
						# 发送装备
						IF @_item_excel_id!=0 AND @_item_num!=0 THEN
							IF _need_reward=1 THEN
								SET @_result = 0;
								CALL game_add_item(_member_id,0,@_item_excel_id,@_item_num,0,@_result);
								INSERT INTO instance_drop_log (account_id,instance_id,class,level,excel_id,num,time)VALUES 
								(_member_id,@_obj_id,102,@_cur_level,@_item_excel_id,@_item_num,UNIX_TIMESTAMP());
								
								# 邮件
								CALL add_private_mail(0,_member_id,4,4,0,@_mail_text,@_ext_data,0,@_result,@_mail_id);
								IF @_mail_ids='' THEN
									SET @_mail_ids = CONCAT(_member_id,',',@_mail_id);
								ELSE
									SET @_mail_ids = CONCAT(@_mail_ids, ',',_member_id,',',@_mail_id);
								END IF;
							ELSE
								# 通知玩家没有贡献不能拿奖励
								CALL add_private_mail(0,_member_id,4,4,0,@_noreward_mail_text,@_ext_data,0,@_result,@_mail_id);
								IF @_mail_ids='' THEN
									SET @_mail_ids = CONCAT(_member_id,',',@_mail_id);
								ELSE
									SET @_mail_ids = CONCAT(@_mail_ids, ',',_member_id,',',@_mail_id);
								END IF;
							END IF;
							IF (@_need_notify=1) AND (_need_reward=1) THEN
								SELECT name INTO @_char_name FROM common_characters WHERE account_id=_member_id;
								IF @_char_names='' THEN
									SET @_char_names=@_char_name;
								ELSE
									SET @_char_names=CONCAT(@_char_names,',',@_char_name);
								END IF;
							END IF;
						END IF;
						# 增加经验
						IF @_char_exp!=0 THEN
							CALL add_char_exp(_member_id,@_char_exp,@_result_exp,@_result_level);
						END IF;
						# 发送黄金
						IF @_gold!=0 THEN
							UPDATE common_characters SET gold=gold+@_gold WHERE account_id=_member_id;
						END IF;
					END IF;
				UNTIL _done END REPEAT;
				CLOSE _cursor;

				IF @_need_notify=1 THEN
					SET @_notify = REPLACE(@_notify,'%s1',@_char_names);
				END IF;
				# 修改副本状态
				UPDATE alli_instance SET combat_log=COMPRESS(@_ext_data),combat_result=@_combat_result WHERE instance_id=@_obj_id;
			WHEN 200 THEN
				# 修改排名并给被打败的人发送邮件
				SET @_my_rank=0,@_obj_rank=0;
				SELECT rank INTO @_my_rank FROM arena WHERE account_id=_account_id;
				SELECT rank INTO @_obj_rank FROM arena WHERE account_id=@_obj_id;
				IF (@_my_rank > @_obj_rank) THEN
					UPDATE arena SET rank=@_obj_rank WHERE account_id=_account_id;
					UPDATE arena SET rank=@_my_rank WHERE account_id=@_obj_id;
					SET @_mail_text = CONCAT(@_mail_text,@_my_rank);
				ELSE
					SET @_mail_text = CONCAT(@_mail_text,@_obj_rank);
				END IF;
				# SET @_mail_text = CONCAT('[FF0000]',@_mail_text,'[-]');
				CALL add_private_mail(_account_id,@_obj_id,4,1,0,@_mail_text,NULL,0,@_result,@_mail_id);
				SET @_mail_ids = CONCAT(@_obj_id,',',@_mail_id);
			ELSE
				ROLLBACK;
				SET _result = -10;
				LEAVE label_body;
		END CASE;
	ELSE # 失败
		CASE @_combat_type
			WHEN 102 THEN
				# 发送邮件
				OPEN _cursor;
				SET _done = 0;
				REPEAT
					FETCH _cursor INTO _member_id,_need_reward;
					IF NOT _done THEN
						CALL add_private_mail(0,_member_id,4,4,0,@_mail_text,@_ext_data,0,@_result,@_mail_id);
						IF @_mail_ids='' THEN
							SET @_mail_ids = CONCAT(_member_id,',',@_mail_id);
						ELSE
							SET @_mail_ids = CONCAT(@_mail_ids, ',',_member_id,',',@_mail_id);
						END IF;
					END IF;
				UNTIL _done END REPEAT;
				# 将领士兵损耗
				SET @_i=1;
				SET @_imax=_get_char_count(@_attack_data,'*');
				WHILE @_i<=@_imax DO
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_attack_data,'*',@_i+1),'*',-1) INTO @_hero_id;
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_attack_data,'*',@_i+2),'*',-1) INTO @_army_dead_num;
					SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_attack_data,'*',@_i+3),'*',-1) INTO @_army_raw_num;
					IF @_hero_ids = '' THEN
						SET @_hero_ids=@_hero_id;
					ELSE
						SET @_hero_ids=CONCAT(@_hero_ids,',',@_hero_id);
					END IF;
					UPDATE hire_heros SET army_num=IF(army_num>@_army_dead_num,army_num-@_army_dead_num,0) WHERE hero_id=@_hero_id;
					# 计算战力
					IF ROW_COUNT()!= 0 THEN
						CALL game_calc_heroall(@_hero_id,0,0,1,@_result);
					END IF;
					SET @_i=@_i+3;
				END WHILE;

				# 修改副本状态
				UPDATE alli_instance SET retry_times=retry_times+1,combat_log=COMPRESS(@_ext_data),combat_result=@_combat_result WHERE instance_id=@_obj_id;
			WHEN 200 THEN
				SET _result = 0;
			ELSE
				ROLLBACK;
				SET _result = -10;
				LEAVE label_body;
		END CASE;
	END IF;

	# 发起战斗返回,清理结果表
	SET @_time_now = UNIX_TIMESTAMP();
	SELECT back_time INTO @_use_time FROM excel_combat WHERE excel_id=102;
	DELETE FROM alli_instance_combat_result WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -30;
		LEAVE label_body;
	END IF;
	UPDATE alli_instance_combat_event SET march_type=2,combat_log=COMPRESS(@_ext_data),combat_result=@_combat_result WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -31;
		LEAVE label_body;
	END IF;
	INSERT INTO time_events (event_id,begin_time,end_time,type,locked) 
		VALUES (_event_id,@_time_now,@_time_now+@_use_time,10,0);
END label_body;
	IF _result!=0 THEN
		SET @_result = _release_combat_lock(_event_id); # 解锁
	END IF;
	SELECT _result,@_use_time,@_cur_level,@_combat_result,@_combat_type,_account_id,@_obj_id,@_need_refresh_char_atb,@_mail_ids,@_notify,@_hero_ids,@_account_ids;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_alli_instance_combat_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_alli_instance_combat_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_alli_instance_combat_event`(IN `_event_id` bigint unsigned,IN `_combat_result` tinyint unsigned,IN `_attack_data` blob,IN `_defense_data` blob,IN `_loot_list` blob,IN `_ext_data` blob)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-100; SELECT _result; END;
	
label_body:BEGIN
	START TRANSACTION;
	# 删除战斗事件
	DELETE FROM alli_instance_combat WHERE event_id=_event_id;
	IF ROW_COUNT()=0 THEN
		SET _result = -1;
		ROLLBACK;
		LEAVE label_body;
	END IF;
	# 添加新的结果
	INSERT INTO alli_instance_combat_result (event_id,result,attack_data,defense_data,loot_list,ext_data,locked) 
		VALUES (_event_id,_combat_result,_attack_data,_defense_data,_loot_list,COMPRESS(_ext_data),0);
	IF ROW_COUNT()=0 THEN
		SET _result = -2;
		ROLLBACK;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_alli_instance_march_back_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_alli_instance_march_back_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_alli_instance_march_back_event`(IN `_event_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SET @_result=_release_te_lock(_event_id); SELECT _result,@_instance_destroyed,@_combat_type,@_account_id,@_obj_id,@_instance_level,@_auto_supply,@_hero_deploy,@_account_ids; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SET @_result=_release_te_lock(_event_id); SELECT _result,@_instance_destroyed,@_combat_type,@_account_id,@_obj_id,@_instance_level,@_auto_supply,@_hero_deploy,@_account_ids; END;
	
	SET @_instance_destroyed=0,@_combat_type=0,@_obj_id=0,@_account_id=0;
	SET @_hero_deploy='',@_alliance_id=0,@_instance_level=0,@_auto_supply=0;
	SET @_account_ids=''; # 用于刷新草药
label_body:BEGIN
	IF _get_te_lock(_event_id)=0 THEN
		SET _result = -150;
		LEAVE label_body;
	END IF;
	SELECT combat_type,hero_deploy INTO @_combat_type,@_hero_deploy FROM alli_instance_combat_event WHERE event_id=_event_id;
	
	IF @_combat_type=102 THEN
		SELECT e.obj_id,e.combat_result,i.creator_id,i.level,i.auto_combat,i.auto_supply,i.stop_level,i.retry_times,i.alliance_id 
			INTO @_obj_id,@_combat_result,@_account_id,@_instance_level,@_auto_combat,@_auto_supply,@_stop_level,@_retry_times,@_alliance_id
			FROM alli_instance i JOIN alli_instance_combat_event e ON i.instance_id=e.obj_id WHERE e.event_id=_event_id; 
		IF FOUND_ROWS()=0 THEN
			SET _result = -1;
			LEAVE label_body;
		END IF;
		
		SET @_max_retry_times=0;
		SELECT retry_times INTO @_max_retry_times FROM excel_instance_list WHERE excel_id=102;
		
		# 最大次数
		SET @_max_instance_level = 0;
		SELECT e.max_instance_level INTO @_max_instance_level FROM excel_alliance_congress e JOIN alliance_buildings a ON e.level=a.level WHERE a.alliance_id=@_alliance_id AND excel_id=1;
		
		IF @_auto_supply=1 THEN
			SELECT IFNULL(GROUP_CONCAT(account_id),'') INTO @_account_ids FROM alli_instance_player WHERE instance_id=@_obj_id;
		END IF;
	END IF;
	
	START TRANSACTION;
	# 删除时间事件
	DELETE FROM time_events WHERE event_id=_event_id;
	DELETE FROM alli_instance_combat_event WHERE event_id=_event_id;
	
	CASE @_combat_type
		WHEN 102 THEN
			# 修改战斗状态
			IF @_combat_result=1 THEN
				UPDATE alli_instance SET status=1,level=level+1 WHERE instance_id=@_obj_id AND status=2;
			ELSE
				UPDATE alli_instance SET status=1,auto_combat=0 WHERE instance_id=@_obj_id AND status=2;
			END IF;
			
			# 是否需要销毁
			IF (@_instance_level>=@_max_instance_level AND @_combat_result=1) OR @_retry_times>=@_max_retry_times THEN
				CALL alli_instance_destroy(@_account_id,@_obj_id,0,0,0,@_result);
				IF @_result!=0 THEN
					SET _result = -10;
					ROLLBACK;
					LEAVE label_body;
				END IF;
				SET @_instance_destroyed=1;
				LEAVE label_body;
			END IF;
			# 是否需要自动补给
			IF @_auto_supply=1 THEN
				CALL alli_instance_auto_supply(@_account_id,@_obj_id,@_hero_deploy,0,0,@_result);
				IF @_result!=0 THEN
					SET _result = -12;
					ROLLBACK;
					LEAVE label_body;
				END IF;
			END IF;

			# 是否需要继续发起战斗(需要判断是否为停止关卡,上一关是否胜利)
			IF @_auto_combat=1 AND @_combat_result=1 THEN
				# 此时@_instance_level代表的是已经打过的关卡,因为上面已经level=level+1
				IF @_instance_level<@_stop_level THEN
					# 继续自动战斗
					CALL alli_instance_start_combat(@_account_id,@_obj_id,@_combat_type,@_auto_combat,@_auto_supply,@_stop_level,@_hero_deploy,0,0,0,@_result);
					IF @_result!=0 THEN
						SET _result = -13;
						ROLLBACK;
						LEAVE label_body;
					END IF;
				ELSE
					# 停止自动战斗
					UPDATE alli_instance SET auto_combat=0 WHERE instance_id=@_obj_id;
				END IF;
			END IF;
		WHEN 200 THEN
			SET _result = 0;
		ELSE
			ROLLBACK;
			SET _result = -30;
			LEAVE label_body;
	END CASE;
END label_body;
	IF _result!=0 AND _result!=-150 THEN
		SET @_result = _release_te_lock(_event_id);
	END IF;
	SELECT _result,@_instance_destroyed,@_combat_type,@_account_id,@_obj_id,@_instance_level,@_auto_supply,@_hero_deploy,@_account_ids;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_get_combat_lock`
-- ----------------------------
DROP FUNCTION IF EXISTS `_get_combat_lock`;
DELIMITER ;;
CREATE FUNCTION `_get_combat_lock`(`_event_id` bigint unsigned) RETURNS int(10) unsigned
BEGIN
	UPDATE alli_instance_combat_result SET locked=1 WHERE event_id=_event_id AND locked=0;
	RETURN ROW_COUNT();
END
;;
DELIMITER ;

-- ----------------------------
-- Function structure for `_release_combat_lock`
-- ----------------------------
DROP FUNCTION IF EXISTS `_release_combat_lock`;
DELIMITER ;;
CREATE FUNCTION `_release_combat_lock`(`_event_id` bigint unsigned) RETURNS int(10) unsigned
BEGIN
	UPDATE alli_instance_combat_result SET locked=0 WHERE event_id=_event_id AND locked=1;
	RETURN ROW_COUNT();
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_auto_supply`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_auto_supply`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_auto_supply`(IN `_account_id` bigint unsigned, IN `_instance_id` bigint unsigned,IN `_hero_deploy` blob,IN `_need_transaction` int unsigned,IN `_echo` int unsigned,OUT `_result` int)
BEGIN
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _i,_imax,_j,_jmax INT UNSIGNED DEFAULT 0;
	DECLARE _one BLOB;
	
	SET _result=0,@_auto_supply_result=0;

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE creator_id=_account_id AND instance_id=_instance_id) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	IF _need_transaction=1 THEN
		START TRANSACTION;
	END IF;
	SET _i = 1;
	SET _imax = _get_char_count(_hero_deploy, ',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_hero_deploy,',',_i+1),',',-1) INTO _one;
		SET _j = 1;
		SET _jmax = _get_char_count(_one, '*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one,'*',_j+3),'*',-1) INTO _hero_id;
			IF _hero_id!=0 THEN
				CALL game_auto_supply(0,_hero_id,0,@_auto_supply_result);
				IF @_auto_supply_result<0 THEN
					SET _result = -2;
					ROLLBACK;
					LEAVE label_body;
				END IF;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;
END label_body;
	IF _need_transaction=1 THEN
		COMMIT;
	END IF;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_gen_loot`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_gen_loot`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_gen_loot`(IN `_instance_id` bigint unsigned,IN `_type` int unsigned)
BEGIN
	DECLARE _i,_imax,_rand,_percent,total_percent,_excel_id,_num,_level INT UNSIGNED DEFAULT 0;
	DECLARE _loot_list BLOB;

	SET _loot_list='';
label_body:BEGIN
	CASE _type
		WHEN 102 THEN
			SELECT e.loot_table INTO _loot_list FROM excel_alli_instance e JOIN alli_instance i ON e.level_id=i.level WHERE i.instance_id=_instance_id;
		ELSE
			LEAVE label_body;
	END CASE;

	SET _rand = rand()*100;

	SET _i = 1;
	SET _imax = _get_char_count(_loot_list, '*');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_loot_list,'*',_i),'*',-1) INTO _excel_id;
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_loot_list,'*',_i+1),'*',-1) INTO _percent;
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_loot_list,'*',_i+2),'*',-1) INTO _num;
		SET _i=_i+3;
		SET total_percent=total_percent+_percent;
		IF _rand<=total_percent THEN
			LEAVE label_body;
		END IF;
	END WHILE;
END label_body;
	SELECT _excel_id,_num;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_combat_result`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_combat_result_event`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_combat_result_event`()
BEGIN
	SELECT event_id FROM alli_instance_combat_result WHERE locked=0;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_instance_id`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_instance_id`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_instance_id`(IN `_account_id` bigint unsigned)
BEGIN
	SELECT instance_id FROM alli_instance_player WHERE account_id=_account_id;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_all_instance`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_all_instance`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_all_instance`()
BEGIN
	SELECT p.instance_id,p.account_id FROM alli_instance i JOIN alli_instance_player p ON i.instance_id=p.instance_id WHERE i.type=102;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_combat_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_combat_log`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_combat_log`(IN `_id` bigint unsigned,IN `_combat_type` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	CASE _combat_type
		WHEN 102 THEN
			SELECT UNCOMPRESS(combat_log) FROM alli_instance WHERE instance_id=_id AND type=_combat_type;
		WHEN 200 THEN
			SELECT UNCOMPRESS(combat_log) FROM alli_instance_combat_event WHERE event_id=_id AND combat_type=_combat_type;
		ELSE
			SET _result = 0;
	END CASE;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_get_simple_combat_log`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_simple_combat_log`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_simple_combat_log`(IN `_instance_id` bigint unsigned,IN `_combat_type` int unsigned)
BEGIN
	# 剩余时间,战斗结果,副本当前关卡
	SELECT IF(UNIX_TIMESTAMP()<t.end_time,t.end_time-UNIX_TIMESTAMP(),0),e.combat_result,i.level FROM time_events t 
		JOIN alli_instance_combat_event e ON t.event_id=e.event_id 
		JOIN alli_instance i ON e.obj_id=i.instance_id 
		WHERE i.instance_id=_instance_id AND i.type=_combat_type;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_stop_combat`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_stop_combat`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_stop_combat`(IN `_account_id` bigint unsigned,IN `_combat_type` bigint unsigned,IN `_combat_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
label_body:BEGIN
	# 只有队长才能停止战斗
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE instance_id=_combat_id AND creator_id=_account_id AND type=_combat_type) THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	UPDATE alli_instance SET auto_combat=0 WHERE instance_id=_combat_id;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_supply`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_supply`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_supply`(IN `_account_id` bigint,IN `_combat_type` int,IN `_instance_id` bigint)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _i,_imax,_j,_jmax INT UNSIGNED DEFAULT 0;
	DECLARE _one BLOB;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,@_hero_ids,@_account_ids; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,@_hero_ids,@_account_ids; END;

	SET _result=0,@_hero_deploy='',@_hero_ids='',@_auto_supply_result=0;
	SET @_account_ids='';

label_body:BEGIN
	IF NOT EXISTS (SELECT 1 FROM alli_instance WHERE creator_id=_account_id AND instance_id=_instance_id AND type=_combat_type AND status=1) THEN
		SET _result = -1;	# 副本状态必须为出征,且必须是队长
		LEAVE label_body;
	END IF;

	SELECT IFNULL(GROUP_CONCAT(hero_ids),''),IFNULL(GROUP_CONCAT(account_id),'') INTO @_hero_deploy,@_account_ids FROM alli_instance_player WHERE instance_id=_instance_id;
	IF @_hero_deploy!='' THEN
		SET @_hero_deploy=CONCAT(',', @_hero_deploy);
	END IF;

	START TRANSACTION;
	SET _i = 1;
	SET _imax = _get_char_count(@_hero_deploy, ',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(@_hero_deploy,',',_i+1),',',-1) INTO _one;
		SET _j = 1;
		SET _jmax = _get_char_count(_one, '*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one,'*',_j+3),'*',-1) INTO _hero_id;
			IF _hero_id!=0 THEN
				CALL game_auto_supply(0,_hero_id,0,@_auto_supply_result);
				IF @_auto_supply_result<0 THEN
					SET _result = -2;
					ROLLBACK;
					LEAVE label_body;
				END IF;
				IF @_auto_supply_result>0 THEN
					IF @_hero_ids='' THEN
						SET @_hero_ids=_hero_id;
					ELSE
						SET @_hero_ids=CONCAT(@_hero_ids, ',', _hero_id);
					END IF;
				END IF;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;
END label_body;
	SELECT _result,@_hero_ids,@_account_ids;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `alli_instance_system_destroy`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_system_destroy`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_system_destroy`()
BEGIN
	DECLARE _instance_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _over_time INT UNSIGNED DEFAULT 0;
	DECLARE _result,_done INT DEFAULT 0;
	DECLARE _instance_ids BLOB;

	DECLARE _cursor CURSOR FOR SELECT instance_id FROM alli_instance WHERE (create_time+_over_time)<UNIX_TIMESTAMP();
	
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	SET _instance_ids = '';

	SELECT val_int INTO _over_time FROM excel_cmndef WHERE id=4;
	
	SET _done = 0;
	OPEN _cursor;
	REPEAT
		FETCH _cursor INTO _instance_id;
		IF NOT _done THEN
			CALL alli_instance_destroy(0,_instance_id,1,1,0,_result);
			IF _result=0 THEN
				SET _instance_ids=CONCAT(_instance_ids,',',_instance_id);
			END IF;
		END IF;
	UNTIL _done END REPEAT;
	CLOSE _cursor;

	SELECT _instance_ids;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `upload_arena_data`
-- ----------------------------
DROP PROCEDURE IF EXISTS `upload_arena_data`;
DELIMITER ;;
CREATE PROCEDURE `upload_arena_data`(IN `_account_id` bigint unsigned,IN `_hero_deploy` blob)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _i,_j,_imax,_jmax INT UNSIGNED DEFAULT 0;
	DECLARE _row,_col,_prof,_army_type,_army_level,_army_num,_level INT UNSIGNED DEFAULT 0;
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _char_name,_hero_name VARCHAR(32) DEFAULT '';
	DECLARE _combat_data,_one_ids BLOB;
	DECLARE _rank,_new,_min_level,_total_force,_cd INT UNSIGNED DEFAULT 0;

	SET _combat_data='';

label_body:BEGIN
	SELECT min_level,upload_cd INTO _min_level,_cd FROM excel_arena;
	IF NOT EXISTS (SELECT 1 FROM common_characters WHERE account_id=_account_id AND level>=_min_level) THEN
		SET _result = -1;		# 等级太低
		LEAVE label_body;
	END IF;
	
	IF EXISTS (SELECT 1 FROM arena WHERE account_id=_account_id AND (last_upload_time+_cd)>UNIX_TIMESTAMP()) THEN
		SET _result = -3;		# 上传CD中
		LEAVE label_body;
	END IF;

	DROP TEMPORARY TABLE IF EXISTS `tmp_hero_info`;
	CREATE TEMPORARY TABLE `tmp_hero_info`(
		char_name VARCHAR(32) NOT NULL,
		hero_id BIGINT UNSIGNED NOT NULL,
		hero_name VARCHAR(32) NOT NULL,
		level INT UNSIGNED NOT NULL,
		prof INT UNSIGNED NOT NULL,
		army_type INT UNSIGNED NOT NULL,
		army_level INT UNSIGNED NOT NULL,
		army_num INT UNSIGNED NOT NULL,
		row INT UNSIGNED NOT NULL,
		col INT UNSIGNED NOT NULL,
		PRIMARY KEY(`hero_id`)
	);

	SET _i = 1;
	SET _imax = _get_char_count(_hero_deploy,',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_hero_deploy,',',_i+1),',',-1) INTO _one_ids;
		SET _j = 1;
		SET _jmax = _get_char_count(_one_ids,'*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+1),'*',-1) INTO _row;
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+2),'*',-1) INTO _col;
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+3),'*',-1) INTO _hero_id;
			SELECT h.army_prof,h.army_type,h.army_level,h.army_num,h.name,h.level,c.name
				INTO _prof,_army_type,_army_level,_army_num,_hero_name,_level,_char_name
				FROM hire_heros h JOIN common_characters c ON h.account_id=c.account_id WHERE hero_id=_hero_id AND h.account_id=_account_id;
			IF FOUND_ROWS()!=0 THEN
				INSERT INTO tmp_hero_info (row,col,char_name,hero_id,hero_name,prof,army_type,army_level,army_num,level) 
					VALUES(_row,_col,_char_name,_hero_id,_hero_name,_prof,_army_type,_army_level,_army_num,_level);
				SET _total_force = _total_force+_prof;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;
	
	IF _total_force=0 THEN
		SET _result = -4;
		LEAVE label_body;
	END IF;
	
	# ,*row*col*char_name*hero_id*hero_level*hero_name*prof*army_type*army_level*army_num
	SELECT IFNULL(GROUP_CONCAT(CONCAT('*',CONCAT_WS('*',row,col,char_name,hero_id,level,hero_name,prof,army_type,army_level,army_num))),'') INTO _combat_data FROM tmp_hero_info;
	IF _combat_data!='' THEN
		SET _combat_data=CONCAT(',', _combat_data);
	END IF;

	IF NOT EXISTS (SELECT 1 FROM arena WHERE account_id=_account_id) THEN
		SET _new = 1;
		SELECT COUNT(*)+1 INTO _rank FROM arena;
		INSERT INTO arena (account_id,rank,combat_data,last_reset_time,last_upload_time,used_times,total_force,used_pay_times) 
				VALUES (_account_id,_rank,_combat_data,UNIX_TIMESTAMP(),UNIX_TIMESTAMP(),0,_total_force,0);
		IF ROW_COUNT()=0 THEN
			SET _result = -2; # 上传失败
			LEAVE label_body;
		END IF;
	ELSE
		UPDATE arena SET combat_data=_combat_data,last_upload_time=UNIX_TIMESTAMP(),total_force=_total_force WHERE account_id=_account_id;
	END IF;
END label_body;
	SELECT _result,_rank,_new;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_arena_challenge_list`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_arena_challenge_list`;
DELIMITER ;;
CREATE PROCEDURE `get_arena_challenge_list`(IN `_account_id` bigint unsigned,IN `_num` int unsigned)
BEGIN
	DECLARE _rank INT UNSIGNED DEFAULT 0;

	SET @_query = '';

	SELECT rank INTO _rank FROM arena WHERE account_id=_account_id;
	IF FOUND_ROWS()=0 THEN
		SET @_query = CONCAT('SELECT a.rank,a.account_id,a.total_force,c.name,c.level,c.head_id FROM arena a JOIN common_characters c ON a.account_id=c.account_id ORDER BY a.rank DESC LIMIT ',_num);
	ELSE
		IF _rank<=_num THEN
			SET _rank = _num+2;
		END IF;
		SET @_query = CONCAT('SELECT a.rank,a.account_id,a.total_force,c.name,c.level,c.head_id FROM arena a JOIN common_characters c ON a.account_id=c.account_id WHERE a.rank<',_rank,' AND a.account_id!=',_account_id,' ORDER BY a.rank DESC LIMIT ',_num);
	END IF;
	
	PREPARE smt FROM @_query;
	EXECUTE smt;
	DEALLOCATE PREPARE smt;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_arena_rank_list`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_arena_rank_list`;
DELIMITER ;;
CREATE PROCEDURE `get_arena_rank_list`(IN `_page` int unsigned,IN `_page_size` int unsigned)
BEGIN
	# 动态sql(LIMIT的参数不能使用变量)
	SET @_query = CONCAT('SELECT a.rank,a.account_id,a.total_force,c.name,c.level,c.head_id,IFNULL(g.name,\'\') FROM arena a JOIN common_characters c ON a.account_id=c.account_id LEFT JOIN alliances g ON c.alliance_id=g.alliance_id ORDER BY a.rank ASC LIMIT ', _page*_page_size, ',', _page_size);
	PREPARE smt FROM @_query;
	EXECUTE smt;
	DEALLOCATE PREPARE smt;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_arena_status`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_arena_status`;
DELIMITER ;;
CREATE PROCEDURE `get_arena_status`(IN `_account_id` bigint unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _rank INT UNSIGNED DEFAULT 0;

	SET @_cd_upload_left_time=0,@_left_times=0,@_pay_times_left=0,@_total_num=0;
label_body:BEGIN
	SELECT free_times,pay_times,upload_cd INTO @_max_times,@_pay_times,@_cd FROM excel_arena;
	IF NOT EXISTS (SELECT 1 FROM arena WHERE account_id=_account_id) THEN
		# 没上传数据
		SET @_left_times=@_max_times;
		SET @_pay_times_left=@_pay_times;
		LEAVE label_body;
	END IF;
	
	# 重置竞技场次数
	UPDATE arena SET used_times=0,used_pay_times=0,last_reset_time=UNIX_TIMESTAMP() WHERE account_id=_account_id AND DATE(FROM_UNIXTIME(last_reset_time))!=DATE(NOW());

	SELECT rank,used_times,last_upload_time,used_pay_times INTO _rank,@_used_times,@_upload_time,@_used_pay_times FROM arena WHERE account_id=_account_id;

	SET @_time_now = UNIX_TIMESTAMP();
	SET @_cd_upload_left_time = IF(@_time_now-@_upload_time>@_cd,0,@_upload_time+@_cd-@_time_now);
	SET @_left_times = IF(@_max_times>@_used_times,@_max_times-@_used_times,0);
	SET @_pay_times_left = IF(@_pay_times>@_used_pay_times,@_pay_times-@_used_pay_times,0);
END label_body;
	SELECT _result,@_cd_upload_left_time,@_left_times,@_pay_times_left,_rank;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `pay_for_arena`
-- ----------------------------
DROP PROCEDURE IF EXISTS `pay_for_arena`;
DELIMITER ;;
CREATE PROCEDURE `pay_for_arena`(IN `_account_id` bigint unsigned,IN `_money_type` int unsigned,IN `_num` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;
	DECLARE _used_pay_times,_used_free_times,_max_pay_times,_max_free_times,_price INT UNSIGNED DEFAULT 0;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_price; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_price; END;
	
label_body:BEGIN
	SELECT pay_price,pay_times,free_times INTO _price,_max_pay_times,_max_free_times FROM excel_arena;
	SELECT used_times,used_pay_times INTO _used_free_times,_used_pay_times FROM arena WHERE account_id=_account_id;
	# 是否还有免费次数
	IF _used_free_times<_max_free_times THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
	# 是否还有付费次数
	IF (_used_pay_times+_num)>_max_pay_times THEN
		SET _result = -2;
		LEAVE label_body;
	END IF;
	# 购买次数太多
	IF _used_free_times<_num THEN
		SET _result = -6;
		LEAVE label_body;
	END IF;
	
	SET _price = _price*_num;

	START TRANSACTION;
	CASE _money_type
	WHEN 1 THEN
		UPDATE common_characters SET diamond=diamond-_price WHERE account_id=_account_id AND diamond>=_price;
		IF ROW_COUNT()=0 THEN
			ROLLBACK;
			SET _result = -3; # 钻石不足
			LEAVE label_body;
		END IF;
	WHEN 2 THEN
		UPDATE common_characters SET crystal=crystal-_price WHERE account_id=_account_id AND crystal>=_price;
		IF ROW_COUNT()=0 THEN
			ROLLBACK;
			SET _result = -4; # 水晶不足
			LEAVE label_body;
		END IF;
	ELSE
		SET _result = -10; # 货币类型不对
		LEAVE label_body;
	END CASE;

	UPDATE arena SET used_pay_times=used_pay_times+_num,used_times=used_times-_num 
		WHERE account_id=_account_id AND used_times>=_num AND (used_pay_times+_num)<=_max_pay_times;
	IF ROW_COUNT()=0 THEN
		ROLLBACK;
		SET _result = -5;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result,_price;
	COMMIT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `deal_arena_reward_event`
-- ----------------------------
DROP PROCEDURE IF EXISTS `deal_arena_reward_event`;
DELIMITER ;;
CREATE PROCEDURE `deal_arena_reward_event`()
BEGIN
	DECLARE _done INT DEFAULT 0;
	DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _max_rank,_rank,_gold INT UNSIGNED DEFAULT 0;

	DECLARE _cursor CURSOR FOR SELECT account_id,rank FROM arena WHERE rank<=_max_rank;
	
	DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' BEGIN SET _done=1; END;

	SET @_mail_ids='';
label_body:BEGIN
	SELECT MAX(rank) INTO _max_rank FROM excel_arena_reward;
	SELECT content INTO @_content FROM excel_text WHERE excel_id=478;
	OPEN _cursor;
	SET _done = 0;
	REPEAT
		FETCH _cursor INTO _account_id,_rank;
		IF NOT _done THEN
			SELECT gold INTO _gold FROM excel_arena_reward WHERE rank=_rank;
			UPDATE common_characters SET gold=gold+_gold WHERE account_id=_account_id;

			SET @_mail_text = CONCAT(@_content,_gold);
			SET @_mail_text = REPLACE(@_mail_text, '%s1', _rank);
			CALL add_private_mail(0,_account_id,4,1,0,@_mail_text,NULL,0,@_result,@_mail_id);
			SET @_mail_ids = CONCAT(@_mail_ids,',',_account_id,',',@_mail_id);
		END IF;
	UNTIL _done END REPEAT;
	CLOSE _cursor;
END label_body;
	SELECT @_mail_ids;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `_check_hero_deploy`
-- ----------------------------
DROP PROCEDURE IF EXISTS `_check_hero_deploy`;
DELIMITER ;;
CREATE PROCEDURE `_check_hero_deploy`(IN `_src_hero_deploy` blob,IN `_dst_hero_deploy` blob,OUT `_result` int)
BEGIN
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _i,_imax,_j,_jmax INT UNSIGNED DEFAULT 0;
	DECLARE _one_ids,_src_ids,_dst_ids BLOB;

	SET _result = 0;

	DROP TEMPORARY TABLE IF EXISTS `tmp_src_hero_id`;
	CREATE TEMPORARY TABLE `tmp_src_hero_id`(
		hero_id BIGINT UNSIGNED NOT NULL,
		PRIMARY KEY(`hero_id`)
	);

	DROP TEMPORARY TABLE IF EXISTS `tmp_dst_hero_id`;
	CREATE TEMPORARY TABLE `tmp_dst_hero_id`(
		hero_id BIGINT UNSIGNED NOT NULL,
		PRIMARY KEY(`hero_id`)
	);

label_body:BEGIN
	SET _i = 1;
	SET _imax = _get_char_count(_src_hero_deploy,',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_src_hero_deploy,',',_i+1),',',-1) INTO _one_ids;
		SET _j = 1;
		SET _jmax = _get_char_count(_one_ids,'*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+3),'*',-1) INTO _hero_id;
			INSERT IGNORE INTO tmp_src_hero_id (hero_id) VALUES (_hero_id);
			IF ROW_COUNT()=0 THEN
				SET _result = -1; # src重复的英雄ID
				LEAVE label_body;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;

	SET _i = 1;
	SET _imax = _get_char_count(_dst_hero_deploy,',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_dst_hero_deploy,',',_i+1),',',-1) INTO _one_ids;
		SET _j = 1;
		SET _jmax = _get_char_count(_one_ids,'*');
		WHILE _j<=_jmax DO
			SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_one_ids,'*',_j+3),'*',-1) INTO _hero_id;
			INSERT IGNORE INTO tmp_dst_hero_id (hero_id) VALUES (_hero_id);
			IF ROW_COUNT()=0 THEN
				SET _result = -2; # dst重复的英雄ID
				LEAVE label_body;
			END IF;
			SET _j = _j+3;
		END WHILE;
		SET _i = _i+1;
	END WHILE;

	SELECT GROUP_CONCAT(hero_id) INTO _src_ids FROM tmp_src_hero_id ORDER BY hero_id ASC;
	IF _src_ids IS NULL THEN
		SET _src_ids='';
	END IF;
	SELECT GROUP_CONCAT(hero_id) INTO _dst_ids FROM tmp_dst_hero_id ORDER BY hero_id ASC;
	IF _dst_ids IS NULL THEN
		SET _dst_ids='';
	END IF;

	IF _src_ids!=_dst_ids THEN
		SET _result = -3;	# ID不相同
		LEAVE label_body;
	END IF;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `hero_supply`
-- ----------------------------
DROP PROCEDURE IF EXISTS `hero_supply`;
DELIMITER ;;
CREATE PROCEDURE `hero_supply`(IN `_account_id` bigint unsigned,IN `_combat_type` int unsigned,IN `_hero_ids` blob)
BEGIN
	DECLARE _result,_auto_supply_result INT DEFAULT 0;
	DECLARE _hero_id BIGINT UNSIGNED DEFAULT 0;
	DECLARE _i,_imax INT UNSIGNED DEFAULT 0;
	DECLARE _back_ids BLOB;

	DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SET _result=-100; SELECT _result,_back_ids; END;
	DECLARE EXIT HANDLER FOR SQLSTATE '40001' BEGIN ROLLBACK; SET _result=-103; SELECT _result,_back_ids; END;

	SET _back_ids = '';
label_body:BEGIN
	START TRANSACTION;
	SET _i=1;
	SET _imax=_get_char_count(_hero_ids,',');
	WHILE _i<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_hero_ids,',',_i+1),',',-1) INTO _hero_id;
		CALL game_auto_supply(0,_hero_id,0,_auto_supply_result);
			IF _auto_supply_result<0 THEN
				SET _result = -1; # 补给失败
				ROLLBACK;
				LEAVE label_body;
			END IF;
			IF _auto_supply_result>0 THEN
				IF _back_ids='' THEN
					SET _back_ids=_hero_id;
				ELSE
					SET _back_ids=CONCAT(_back_ids, ',', _hero_id);
				END IF;
			END IF;
		SET _i=_i+1;
	END WHILE;
END label_body;
	SELECT _result,_back_ids;
	COMMIT;
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

	SET @_tmp_asc_id = 0;
	UPDATE global_id SET asc_id=(@_tmp_asc_id:=IF(asc_id>=1048575,0,asc_id+1));

	# 20 1~20   递增部分
	# 31 21~51  时间因子
	# 12 52~63  大区号
	# 1  64     保留	2^63=9223372036854775808

	# 时间因子  2^20=1048576
	SELECT svrgrp_id+UNIX_TIMESTAMP()*1048576+@_tmp_asc_id INTO _id FROM global_id;
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

-- ----------------------------
-- Procedure structure for `alli_instance_get_defender_deploy`
-- ----------------------------
DROP PROCEDURE IF EXISTS `alli_instance_get_defender_deploy`;
DELIMITER ;;
CREATE PROCEDURE `alli_instance_get_defender_deploy`(IN `_excel_id` int unsigned,IN `_level` int unsigned)
BEGIN
	DECLARE _imax INT UNSIGNED DEFAULT 0;
	DECLARE _army_level,_army_num,_army_type,_prof,_row,_col INT UNSIGNED DEFAULT 0;
	DECLARE _army_deploy,_army_data BLOB;

label_body:BEGIN
	SELECT army_data,army_deploy INTO _army_data,_army_deploy FROM excel_alli_instance WHERE level_id=_level;
	IF FOUND_ROWS()=0 THEN
		LEAVE label_body;
	END IF;
	
	DROP TEMPORARY TABLE IF EXISTS `tmp_defender_info`;
	CREATE TEMPORARY TABLE `tmp_defender_info`(
		prof INT UNSIGNED NOT NULL,
		army_type INT UNSIGNED NOT NULL,
		row INT UNSIGNED NOT NULL,
		col INT UNSIGNED NOT NULL,
		PRIMARY KEY(`row`,`col`)
	);
	
	CALL game_split_2(_army_data,'*',_army_level,_army_num);
	SET _col = 0;
	SET _imax = _get_char_count(_army_deploy,'*');
	WHILE _col<=_imax DO
		SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(_army_deploy,'*',_col+1),'*',-1) INTO _army_type;
		SET _row=1;
		SELECT (attack*_army_num+defense*_army_num*0.9+life*_army_num*0.8)/3 INTO _prof FROM excel_soldier WHERE excel_id=_army_type AND level=_army_level;
		WHILE _row<=5 DO
			INSERT INTO tmp_defender_info (row,col,prof,army_type) 
						VALUES(_row,_col+1,_prof,_army_type);
			SET _row=_row+1;
		END WHILE;
		SET _col = _col+1;
	END WHILE;
	
	SELECT prof,army_type,row,col FROM tmp_defender_info;
END label_body;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `position_mark_add_record`
-- ----------------------------
DROP PROCEDURE IF EXISTS `position_mark_add_record`;
DELIMITER ;;
CREATE PROCEDURE `position_mark_add_record`(IN `_account_id` bigint unsigned,IN `_pos_x` int unsigned,IN `_pos_y` int unsigned,IN `_head_id` int unsigned,IN `_prompt` tinyblob)
BEGIN
	DECLARE _result INT DEFAULT 0;

	SET @_total_num = 0;
label_body:BEGIN
	SELECT COUNT(*) INTO @_total_num FROM position_mark WHERE account_id=_account_id;
	IF NOT EXISTS (SELECT 1 FROM excel_position_mark WHERE capacity>@_total_num) THEN
		SET _result = -1; # 收藏夹已达上限
		LEAVE label_body;
	END IF;

	INSERT INTO position_mark (account_id,pos_x,pos_y,head_id,time,prompt) VALUES (_account_id,_pos_x,_pos_y,_head_id,UNIX_TIMESTAMP(),_prompt)
				ON DUPLICATE KEY UPDATE prompt=_prompt;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `position_mark_chg_record`
-- ----------------------------
DROP PROCEDURE IF EXISTS `position_mark_chg_record`;
DELIMITER ;;
CREATE PROCEDURE `position_mark_chg_record`(IN `_account_id` bigint unsigned,IN `_pos_x` int unsigned,IN `_pos_y` int unsigned,IN `_head_id` int unsigned,IN `_prompt` tinyblob)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	UPDATE position_mark SET head_id=_head_id,prompt=_prompt WHERE account_id=_account_id AND pos_x=_pos_x AND pos_y=_pos_y;
	IF ROW_COUNT()=0 THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `position_mark_del_record`
-- ----------------------------
DROP PROCEDURE IF EXISTS `position_mark_del_record`;
DELIMITER ;;
CREATE PROCEDURE `position_mark_del_record`(IN `_account_id` bigint unsigned,IN `_pos_x` int unsigned,IN `_pos_y` int unsigned)
BEGIN
	DECLARE _result INT DEFAULT 0;

label_body:BEGIN
	DELETE FROM position_mark WHERE account_id=_account_id AND pos_x=_pos_x AND pos_y=_pos_y;
	IF ROW_COUNT()=0 THEN
		SET _result = -1;
		LEAVE label_body;
	END IF;
END label_body;
	SELECT _result;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `position_mark_get_record`
-- ----------------------------
DROP PROCEDURE IF EXISTS `position_mark_get_record`;
DELIMITER ;;
CREATE PROCEDURE `position_mark_get_record`(IN `_account_id` bigint unsigned)
BEGIN
	SELECT pos_x,pos_y,head_id,prompt FROM position_mark WHERE account_id=_account_id ORDER BY time ASC;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_send_mails`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_send_mails`;
DELIMITER ;;
CREATE PROCEDURE `get_send_mails`(IN `_account_id` bigint unsigned)
BEGIN
	SELECT c.name as name,FROM_UNIXTIME(m.time) as time,UNCOMPRESS(m.text) as text 
		FROM private_mails m JOIN common_characters c ON m.account_id=c.account_id 
		WHERE m.sender_id=_account_id AND type=1 ORDER BY m.time DESC;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `get_receive_mails`
-- ----------------------------
DROP PROCEDURE IF EXISTS `get_receive_mails`;
DELIMITER ;;
CREATE PROCEDURE `get_receive_mails`(IN `_account_id` bigint unsigned)
BEGIN
	SELECT c.name as name,FROM_UNIXTIME(m.time) as time,UNCOMPRESS(m.text) as text 
		FROM private_mails m JOIN common_characters c ON m.sender_id=c.account_id 
		WHERE m.account_id=_account_id AND type=1 ORDER BY m.time DESC;
END
;;
DELIMITER ;
