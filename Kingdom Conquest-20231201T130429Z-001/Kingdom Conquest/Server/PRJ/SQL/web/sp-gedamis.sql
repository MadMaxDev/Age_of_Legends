set global log_bin_trust_function_creators=on;
set global sql_mode='strict_trans_tables';
delimiter //
 
--  错误处理参考条目
--  1. sqlwarnings和not found之外的都属于sqlexception
--          declare exit            handler for sqlexception        begin end;
--  2. 死锁/ER_LOCK_DEADLOCK
--          declare exit            handler for sqlstate '40001'    begin set nRst=-99; select nRst; end;
--  3. 数值越界/ER_WARN_DATA_OUT_OF_RANGE
--          declare exit            handler for sqlstate '22003'    begin end;
--  4. FETCH无数据/ER_SP_FETCH_NO_DATA
--          declare continue        handler for sqlstate '02000'    begin set _nDone=1; end;


-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 初始化DB --
drop procedure if exists `game_init_db`;
create procedure game_init_db(in bySvrGrpID tinyint unsigned, in byDBSID tinyint unsigned, out nRst int)
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
	insert into global_id values (18014398509481984*bySvrGrpID,562949953421312*byDBSID,0);
end if;

set nRst=0;
end label_init_db;
select nRst;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 生成全局ID --
drop function if exists `game_global_id`;
create function game_global_id() returns bigint
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vTmpID=0;
set @vID=0;
update global_id set asc_id=(@vTmpID:=asc_id+1);


-- 18 1~18   递增部分
-- 31 19~49  时间因子
-- 5  50~54  服务器号
-- 9  55~63  大区号
-- 1  64     保留

-- 时间因子  2^18=262144
select svrgrp_id+dbs_id+unix_timestamp()*262144+@vTmpID into @vID from global_id;
return @vID;
end
//

-- 分解全局ID --
drop procedure if exists `game_unglobal_id`;
create procedure game_unglobal_id(in nID bigint unsigned)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vSvrGrpID=nID<<1>>55;
set @vSvrID=nID<<10>>59;
set @vTime=nID<<15>>33;
set @vIncIdx=nID<<46>>46;

select @vSvrGrpID,@vSvrID,@vTime,@vIncIdx;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 获取账号名
drop procedure if exists `game_acct_info`;
create procedure game_acct_info(in nSuffix int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

if (nSuffix=0) then
	select T2.account_id,T2.name,T1.hero_id,T1.name from hire_heros T1 left join web_ims.common_accounts T2 on T2.account_id=T1.account_id;
else
	select T2.account_id,T2.name,T1.hero_id,T1.name from hire_heros T1 left join web_ims_cur.common_accounts T2 on T2.account_id=T1.account_id;
end if;

end
//

-- 高级账号
drop procedure if exists `game_mb`;
create procedure game_mb(in szCharName char(32), in nSuffix int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;

set @vAccountID=0;
select account_id into @vAccountID from common_characters where name=szCharName;

replace into buildings values (@vAccountID,101,9,25);
replace into buildings values (@vAccountID,102,9,25);
replace into buildings values (@vAccountID,103,9,25);
replace into buildings values (@vAccountID,104,9,25);
replace into buildings values (@vAccountID,105,9,25);
replace into buildings values (@vAccountID,106,9,25);
replace into buildings values (@vAccountID,107,9,25);
replace into buildings values (@vAccountID,108,9,25);

replace into buildings values (@vAccountID,201,10,25);
replace into buildings values (@vAccountID,202,10,25);
replace into buildings values (@vAccountID,203,10,25);
replace into buildings values (@vAccountID,204,10,25);
replace into buildings values (@vAccountID,205,10,25);
replace into buildings values (@vAccountID,206,10,25);
replace into buildings values (@vAccountID,207,10,25);
replace into buildings values (@vAccountID,208,10,25);

replace into buildings values (@vAccountID,301,11,25);
replace into buildings values (@vAccountID,302,11,25);
replace into buildings values (@vAccountID,303,11,25);
replace into buildings values (@vAccountID,304,11,25);
replace into buildings values (@vAccountID,305,11,25);
replace into buildings values (@vAccountID,306,11,25);
replace into buildings values (@vAccountID,307,11,25);
replace into buildings values (@vAccountID,308,11,25);

replace into buildings values (@vAccountID,1,1,26);
replace into buildings values (@vAccountID,2,2,25);
replace into buildings values (@vAccountID,3,3,25);
replace into buildings values (@vAccountID,4,4,25);
replace into buildings values (@vAccountID,5,5,25);
replace into buildings values (@vAccountID,6,6,25);

replace into technologys values (@vAccountID,1,5);
replace into technologys values (@vAccountID,2,5);
replace into technologys values (@vAccountID,3,5);
replace into technologys values (@vAccountID,4,5);
replace into technologys values (@vAccountID,5,5);

update common_characters set gold=999999,level=45,population=10000,development=59000 where account_id=@vAccountID;

set nRst=0;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 酒馆刷新武将 --
-- nAccountID 账号ID
drop procedure if exists `game_tavern_refresh`;
create procedure game_tavern_refresh(in nAccountID bigint unsigned, in nNum int, in nUseItem int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_tavern_refresh: begin

-- 是否有酒馆
set @vLevel=0;
if not exists (select 1 from taverns where account_id=nAccountID and (@vLevel:=level)>=0) then
	set nRst=-2;
	select nRst;
	leave label_tavern_refresh;
end if;

-- 获取该等级的刷新数据
set @vInterval=0;
set @vProb='';
select ref_interval,ref_prob into @vInterval,@vProb from excel_tavern_refresh where level=@vLevel;

set @vUseItemGrow=0;
set @vUseItemTimes=0;

set @vRefreshTime=0;
set @vRes=0;
if (nUseItem=0) then
-- 获取上次刷新时间
if exists (select 1 from taverns where account_id=nAccountID and (@vRefreshTime:=refresh_time)>0 and (@vRes:=(cast(unix_timestamp() as signed) - cast((refresh_time+@vInterval*60) as signed)))<0) then
	set nRst=-3;
	select nRst,abs(@vRes);
	leave label_tavern_refresh;
end if;
else
-- 消耗道具
-- 是英雄榜吗
if (nUseItem!=2010) then
	set nRst=-4;
	leave label_tavern_refresh;
end if;
-- 道具存在吗
if not exists (select 1 from items where account_id=nAccountID and excel_id=nUseItem and num>=1 and (@vItemID:=item_id)>0) then
	set nRst=-4;
	leave label_tavern_refresh;
end if;
call game_del_item(nAccountID,@vItemID,1,0,@vRst);
if (@vRst!=1) then
	set nRst=-4;
	leave label_tavern_refresh;
end if;
-- 成长率
set @vUseItemGrow=floor(6+rand()*3);
-- 刷新次数
set @vUseItemTimes=1;
end if;

-- 第一次进入酒馆，刷新1个绿色武将
-- if (@vRefreshTime=0) then
--	set @vProb='2*100';
--	set nNum=1;
-- end if;

-- 距离下次刷新还有多久
set @vNextRefresh=@vInterval*60-mod(@vRes,@vInterval*60);

-- 删除旧武将
delete from refresh_heros where account_id=nAccountID;

-- 刷新nNum次
while nNum>0 do

set @vGrow=0;
set @vGrowMax=0;
select val_int into @vGrowMax from excel_cmndef where id=11;
-- 武将刷新圆桌
set @vClass=0;
-- 用道具刷新吗
if (@vUseItemTimes>0) then
	set @vGrow=@vUseItemGrow;
	set @vUseItemTimes=@vUseItemItems-1;
	set @vClass=floor(3+rand()*3);
	set @vGrowMax=floor(20+rand()*9);
	if (@vClass > 5) then
		set @vClass = 5;
	end if;
	if (@vGrowMax > 28) then
		SET @vGrowMax = 28;
	end if;
else
	select game_roll_table(@vProb) into @vClass;
end if;

-- 获取刷新规则
set @vAttack=0;
set @vDefense=0;
set @vHealth=0;
set @vSet=0;
select attack_min,defense_min,health_min,init_set into @vAttack,@vDefense,@vHealth,@vSet from excel_hero_class where class_id=@vClass;
-- 生成三围
set @vRes=@vSet-(@vAttack+@vDefense+@vHealth);
while @vRes>0 do
	set @vRoll=rand()*3;
	if (@vRoll<=1) then
		set @vAttack=@vAttack+1;
	elseif (@vRoll<=2) then
		set @vDefense=@vDefense+1;
	else
		set @vHealth=@vHealth+1;
	end if;
	set @vRes=@vRes-1;
end while;

-- 生成成长率
if (@vGrow=0) then
case
when @vClass=1 then
	set @vGrow=5.0;
when @vClass=2 then
	set @vGrow=7.0;
when @vClass=3 then
	set @vGrow=10.0;
when @vClass=4 then
	set @vGrow=15.0;
when @vClass=5 then
	set @vGrow=20.0;
when @vClass=6 then
	set @vGrow=25.0;
else
	set @vRst=0;
end case;
end if;

-- 写入刷新表
set @vHeroID=0;
select game_global_id() into @vHeroID;
set @vRst=0;
call game_refresh_hero(nAccountID,nNum,@vHeroID,@vAttack,@vDefense,@vHealth,@vGrow,@vGrowMax,@vRst);

set nNum=nNum-1;
end while;

-- 更新刷新时刻
update taverns set refresh_time=(unix_timestamp()-(@vInterval*60-@vNextRefresh)) where account_id=nAccountID;

set nRst=0;
if (nUseItem!=0) then
	set nRst=1;
end if;
end label_tavern_refresh;
select nRst,@vNextRefresh;
end
//


-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 刷新武将 --
-- nAccountID 账号ID
-- nIdx 武将插槽
drop procedure if exists `game_refresh_hero`;
create procedure game_refresh_hero(in nAccountID bigint unsigned, in nSlotID int, in nHeroID bigint unsigned, in nAttack int, in nDefense int, in nHealth int, in fGrow float, in fGrowMax float, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_refresh_hero: begin

if not exists (select 1 from taverns where account_id=nAccountID) then
	set nRst=-2;
	leave label_refresh_hero;
end if;

-- 随机生成名字
-- @vName在 game_gen_name 里已经填好了
set @vRst=0;
-- call game_gen_name(2,@vRst);
call game_gen_name_en(2,@vRst);
if (@vRst!=0) then
	set nRst=-3;
	leave label_refresh_hero;
end if;

-- 随机职业
set @vProfession=floor(1+(rand()*5));

-- 随机头像
set @vModel=0;
if (@vFix=1) then
	-- 男生 (1,14)
	set @vModel=floor(1+(rand()*14));
else
	-- 女生 (15,20)
	set @vModel=floor(15+(rand()*5));
end if;

-- 插入刷新表
-- 等级=1,统率=10
insert into refresh_heros (hero_id,account_id,name,level,profession,model,attack,defense,health,leader,grow,slot_idx,grow_max) values (nHeroID,nAccountID,@vName,1,@vProfession,@vModel,nAttack,nDefense,nHealth,10,fGrow,nSlotID,fGrowMax);
-- 更新酒馆索引
set @vQuery=concat('update taverns set hero_', nSlotID, '=', nHeroID, ' where account_id=', nAccountID);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set nRst=0;
end label_refresh_hero;
-- select nRst;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 招募武将 --
-- nAccountID 账号ID
-- nIdx 武将插槽
-- nHeroID 武将ID
drop procedure if exists `game_hire_hero`;
create procedure game_hire_hero(in nAccountID bigint unsigned, in nSlotID int, in nHeroID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_hire_hero: begin

if not exists (select 1 from taverns where account_id=nAccountID) then
	set nRst=-2;
	leave label_hire_hero;
end if;

-- 是否还可以再招募
set @vMaxNum=0;
select max(T2.num) into @vMaxNum from common_characters T1 left join excel_hero_num T2 on T2.level<=T1.level where T1.account_id=nAccountID;
set @vCurNum=0;
select count(1) into @vCurNum from hire_heros where account_id=nAccountID;
if (@vMaxNum<=@vCurNum) then
	set nRst=-10;
	leave label_hire_hero;
end if;

-- 更新酒馆索引
set @vQuery=concat('update taverns set hero_', nSlotID, '=0 where account_id=', nAccountID, ' and hero_', nSlotID, '=', nHeroID);
prepare smt from @vQuery;
execute smt;
if (row_count()=0) then
	set nRst=-3;
	deallocate prepare smt;
	leave label_hire_hero;
end if;
deallocate prepare smt;

-- 插入使用表
set @vName='';
set @vLevel=0;
set @vProfession=0;
set @vModel=0;
set @vAttack=0;
set @vDefense=0;
set @vHealth=0;
set @vLeader=0;
set @vGrow=0;
set @vGrowMax=0;
select name,level,profession,model,attack,defense,health,leader,grow,grow_max into @vName,@vLevel,@vProfession,@vModel,@vAttack,@vDefense,@vHealth,@vLeader,@vGrow,@vGrowMax from refresh_heros where hero_id=nHeroID;
insert into hire_heros (hero_id,account_id,name,level,profession,model,attack_0,attack,defense_0,defense,health_0,health,leader_0,leader,grow,healthstate,grow_max) values (nHeroID,nAccountID,@vName,@vLevel,@vProfession,@vModel,@vAttack,@vAttack,@vDefense,@vDefense,@vHealth,@vHealth,@vLeader,@vLeader,@vGrow,100,@vGrowMax);
-- 从刷新表删除
delete from refresh_heros where hero_id=nHeroID;

set nRst=0;
end label_hire_hero;
select nRst;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 解雇武将 --
-- nHeroID 武将ID
drop procedure if exists `game_fire_hero`;
create procedure game_fire_hero(in nAccountID bigint unsigned, in nHeroID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_fire_hero: begin

if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID) then
	set nRst=-2;
	leave label_fire_hero;
end if;

-- 不能有装备
if exists (select 1 from hero_items where hero_id=nHeroID) then
	set nRst=-3;
	leave label_fire_hero;
end if;
-- 不能有部队
if exists (select 1 from hire_heros where hero_id=nHeroID and army_num>0) then
	set nRst=-4;
	leave label_fire_hero;
end if;
-- 不能在城防配置里
if exists (select 1 from citydefenses where hero_1=nHeroID or hero_2=nHeroID or hero_3=nHeroID or hero_4=nHeroID or hero_5=nHeroID) then
	set nRst=-5;
	leave label_fire_hero;
end if;
-- 不能在副本配置里
if exists (select 1 from instance_player where account_id=nAccountID and hero_id like concat('%',nHeroID,'%')) then
	set nRst=-6;
	leave label_fire_hero;
end if;
-- 不能在世界金矿配置里
if exists (select 1 from world_goldmine where account_id=nAccountID and hero_1=nHeroID or hero_2=nHeroID or hero_3=nHeroID or hero_4=nHeroID or hero_5=nHeroID) then
	set nRst=-7;
	leave label_fire_hero;
end if;
-- 只能是空闲状态
if not exists (select 1 from hire_heros where hero_id=nHeroID and status=0) then
	set nRst=-8;
	leave label_fire_hero;
end if;

-- 从使用表删除
delete from hire_heros where hero_id=nHeroID and status=0;
if (row_count()=0) then
	set nRst=-10;
	leave label_fire_hero;
end if;

set nRst=0;
end label_fire_hero;
select nRst;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 增加成长率 --
-- nHeroID 武将ID
drop procedure if exists `game_add_grow`;
create procedure game_add_grow(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in fGrow float(32,2), in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_grow: begin

-- 武将是否存在
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID) then
	set nRst=-2;
	leave label_add_grow;
end if;

-- set @vMaxGrow=0;
-- select max(grow) into @vMaxGrow from excel_hero_grow;
-- 成长值
-- update hire_heros set grow=if((grow+fGrow)>@vMaxGrow,@vMaxGrow,(grow+fGrow)) where hero_id=nHeroID;
update hire_heros set grow=if((grow+fGrow)>grow_max,grow_max,(grow+fGrow)) where hero_id=nHeroID;
-- 
call game_calc_heroall(nHeroID,1,0,1,@vRst);

set nRst=0;
end label_add_grow;
if (nEcho=1) then
	select nRst;
end if;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 根据等级和成长率计算基础属性 --
drop procedure if exists `game_calc_herobase`;
create procedure game_calc_herobase(in nAttack int, in fAttackBonusPct float(32,2), in nDefense int, in fDefenseBonusPct float(32,2), in nHealth int, in fHealthBonusPct float(32,2), in nLeader int, in fLeaderBonusPct float(32,2), in fGrow float(32,2), in nLevel int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;

-- 成长率换算成百分比
set @vPct=0;
select percent into @vPct from excel_hero_grow where grow=fGrow;
-- 
set @vAttackBase=floor((nAttack+nAttack*@vPct*(nLevel-1))*(1+fAttackBonusPct));
set @vDefenseBase=floor((nDefense+nDefense*@vPct*(nLevel-1))*(1+fDefenseBonusPct));
set @vHealthBase=floor((nHealth+nHealth*@vPct*(nLevel-1))*(1+fHealthBonusPct));
set @vLeaderBase=floor((10*nLevel)*(1+fLeaderBonusPct));

set nRst=0;
if (nEcho=1) then
	select nRst,@vAttackBase,@vDefenseBase,@vHealthBase,@vLeaderBase;
end if;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 武将升级 --
-- nHeroID 武将ID
drop procedure if exists `game_levelup_hero`;
create procedure game_levelup_hero(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_levelup_hero: begin

-- 武将是否存在
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID) then
	set nRst=-2;
	leave label_levelup_hero;
end if;

-- 等级
update hire_heros set level=level+1 where hero_id=nHeroID;
-- 
call game_calc_heroall(nHeroID,1,0,1,@vRst);

set nRst=0;
end label_levelup_hero;
if (nEcho=1) then
	select nRst;
end if;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 武将配兵 --
-- nAccountID 账号ID
-- nHeroID 武将ID
-- nArmyType 士兵类型
-- nArmyLevel 士兵等级
-- nArmyNum 士兵数量
drop procedure if exists `game_config_hero`;
create procedure game_config_hero(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nArmyType int, in nArmyLevel int, in nArmyNum int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin ROLLBACK; end;
declare exit            handler for sqlstate '22003'    begin ROLLBACK; end;

set nRst=-1;
label_config_hero: begin

-- 武将是否存在
set @vArmyType=0;
set @vArmyLevel=0;
set @vArmyNum=0;
set @vLeaderBase=0;
set @vLeaderAdd=0;
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID and (@vArmyType:=army_type)>=0 and (@vArmyLevel:=army_level)>=0 and (@vArmyNum:=army_num)>=0 and (@vLeaderBase:=leader)>=0 and (@vLeaderAdd:=leader_add)>=0) then
	set nRst=-2;
	leave label_config_hero;
end if;
-- 现在空闲吗
if exists (select 1 from hire_heros where hero_id=nHeroID and status!=0) then
	set nRst=-6;
	leave label_config_hero;
end if;

START TRANSACTION;
-- 先清空原来的
if (revert_soldier(nAccountID,@vArmyType,@vArmyLevel,@vArmyNum)!=0) then
	set nRst=-3;
	leave label_config_hero;
end if;

-- 统率与带兵的关系是啥？
set @vArmyNum=@vLeaderBase+@vLeaderAdd;
-- 数量对吗
if (nArmyNum>@vArmyNum) then
	set nRst=-14;
	ROLLBACK;
	leave label_config_hero;
end if;

-- 再重新配置
if (borrow_soldier(nAccountID,nArmyType,nArmyLevel,nArmyNum)!=0) then
	set nRst=-4;
	ROLLBACK;
	leave label_config_hero;
end if;
-- 计算配兵后的总属性
set @vRst=0;
call game_calc_army(nHeroID,nArmyType,nArmyLevel,nArmyNum,@vRst);
update hire_heros set army_type=nArmyType,army_level=nArmyLevel,army_num=nArmyNum,army_attack=@vAttackArmy,army_defense=@vDefenseArmy,army_health=@vHealthArmy,army_prof=@vProf where hero_id=nHeroID;
set nRst=row_count();
	IF nRst=0 THEN
		ROLLBACK;
	END IF;

end label_config_hero;
if (nEcho=1) then
	select nRst,@vProf;
end if;
	COMMIT;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 更新武将数据 --
drop procedure if exists `game_update_hero`;
create procedure game_update_hero(in nCombatType int, in nHeroID bigint unsigned, in nArmyNum int, in nObjID bigint unsigned, in nSlotIdx int unsigned, in nProf int, in nProfS int, in nProfP int, in nSlotNum int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_update_hero: begin

if (nCombatType=20 and nHeroID=0) then
-- 
set @vTmp='';
set @vQuery=concat('select army_', nSlotIdx, ' into @vTmp from world_city where id=', nObjID);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

if (length(@vTmp)=0) then
	set nRst=-10;
	leave label_update_hero;
end if;

set @vArmyType=0;
set @vArmyLevel=0;
set @vArmyNum=0;
call game_split_3(@vTmp,'*',@vArmyType,@vArmyLevel,@vArmyNum);

set @vArmyNum=cast(@vArmyNum as signed) + nArmyNum;
set @vTmp=concat_ws('*',@vArmyType,@vArmyLevel,@vArmyNum);
set @vQuery=concat('update world_city set army_', nSlotIdx, '=@vTmp where id=', nObjID);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set nRst=row_count();

else
-- 
set @vLevel=0;
set @vExp=0;
set @vArmyType=0;
set @vArmyLevel=0;
set @vArmyNum=0;
set @vHealthState=0;
set @vArmyNumRecover=ABS(nArmyNum);
if not exists (select 1 from hire_heros where hero_id=nHeroID and (@vLevel:=level)>=0 and (@vExp:=exp)>=0 and (@vArmyType:=army_type)>=0 and (@vArmyLevel:=army_level)>=0 and (@vArmyNum:=army_num)>=0 and (@vHealthState:=healthstate)>=0) then
	set nRst=-2;
	leave label_update_hero;
end if;

-- 兵力
set @vRecoverPct=0;
select val_int into @vRecoverPct from excel_cmndef where id=5;
set @vRecoverPct=@vRecoverPct/100;
-- 健康度
set @vRecoverPctH=0;
select val_int into @vRecoverPctH from excel_cmndef where id=12;
set @vRecoverPctH=(1-@vRecoverPctH/100);

-- PVE战斗会有兵力恢复
set @vArmyNumRecover=0;
if (nCombatType!=2) then
	set @vArmyNumRecover=floor(abs(nArmyNum)*@vRecoverPct);
	set nArmyNum=nArmyNum+@vArmyNumRecover;
end if;

-- 计算受伤程度

-- 计算消掉战力
-- set @vProfC=if(nProfS>nProfP,nProfP,nProfS);
-- 基础受伤点数
-- set @vBaseHurt=100;
-- set @vPct=@vProfC/nProfS;
-- set @vPct=if(@vPct>1,1,@vPct);
-- set @vFinHurt=floor(@vBaseHurt*@vPct);
-- set @vFinHurt=floor(@vFinHurt*nProf/nProfS);
-- 
-- if (nCombatType!=2) then
-- set @vFinHurt=floor(@vFinHurt*@vRecoverPctH);
-- end if;

-- set @vHealthState=@vHealthState-@vFinHurt;
-- set @vHealthState=if(@vHealthState<0,0,@vHealthState);
set @vPct=abs(nArmyNum)/@vArmyNum;
case
when @vPct<=0.05 then
	set @vPct=1;
when @vPct<=0.12 then
	set @vPct=0.98;
when @vPct<=0.2 then
	set @vPct=0.95;
when @vPct<=0.3 then
	set @vPct=0.91;
when @vPct<=0.4 then
	set @vPct=0.87;
when @vPct<=0.6 then
	set @vPct=0.79;
when @vPct<=0.8 then
	set @vPct=0.73;
else
	set @vPct=0.7;
end case;
set @vHealthState=floor(@vHealthState*@vPct);

set @vArmyNum=cast(@vArmyNum as signed) + nArmyNum;

-- tst
-- 先更新 healthstate ，trigger会重新计算有关属性
update hire_heros set healthstate=@vHealthState where hero_id=nHeroID;

-- 计算配兵后的总属性
set @vRst=0;
call game_calc_army(nHeroID,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
update hire_heros set army_num=@vArmyNum,army_attack=@vAttackArmy,army_defense=@vDefenseArmy,army_health=@vHealthArmy,army_prof=@vProf where hero_id=nHeroID;
set nRst=row_count();

end if;

end label_update_hero;
if (nEcho=1) then
	select nRst,@vArmyNum,@vAttackArmy,@vDefenseArmy,@vHealthArmy,@vHealthState,@vProf,@vArmyNumRecover,@vLevel,@vExp;
end if;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 计算武将配兵后的部队数据 --
drop procedure if exists `game_calc_army`;
create procedure game_calc_army(in nHeroID bigint unsigned, in nArmyType int, in nArmyLevel int, in nArmyNum int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

-- 兵种提供的
set @vAttack_0=0;
set @vDefense_0=0;
set @vHealth_0=0;
select attack*nArmyNum,defense*nArmyNum,life*nArmyNum into @vAttack_0,@vDefense_0,@vHealth_0 from excel_soldier where excel_id=nArmyType and level=nArmyLevel;
-- 武将提供的
set @vAttack_1=0;
set @vDefense_1=0;
set @vHealth_1=0;
set @vHealthState=0;
-- 有装备加成
select attack+attack_add,defense+defense_add,health+health_add,healthstate into @vAttack_1,@vDefense_1,@vHealth_1,@vHealthState from hire_heros where hero_id=nHeroID;

if (nHeroID=0) then
	set @vPct=1;
else
	set @vPct=@vHealthState/100;
end if;

-- total = army + hero*0.02*army_num
set @vBonus=0.02;

set @vAttackArmy=floor((@vAttack_0+@vAttack_1*@vBonus*nArmyNum)*@vPct);
set @vDefenseArmy=floor((@vDefense_0+@vDefense_1*@vBonus*nArmyNum)*@vPct);
set @vHealthArmy=floor((@vHealth_0+@vHealth_1*@vBonus*nArmyNum)*@vPct);

-- 计算战力
set @vProf=ceiling((@vAttackArmy+@vDefenseArmy*0.9+@vHealthArmy*0.8)*1/3);

-- select @vAttackArmy,@vDefenseArmy,@vHealthArmy,@vProf;

end
//

-- 获取武将信息
drop procedure if exists `game_get_hero_hire`;
create procedure game_get_hero_hire(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_hero_hire: begin

select T1.hero_id,T1.name,T1.profession,T1.model,T1.attack,T1.defense,T1.health,T1.leader,T1.grow,T1.army_type,T1.army_level,T1.army_num,T1.status,T1.attack_add,T1.defense_add,T1.health_add,T1.leader_add,T1.level,T1.army_prof,T1.healthstate,T1.exp,T1.grow_max from hire_heros T1 where T1.account_id=nAccountID;

end label_get_hero_hire;
end
//



-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 开始战斗
-- nAccountID 账号ID
-- nCombatType 战斗类型
drop procedure if exists `game_start_combat`;
create procedure game_start_combat(in nAccountID bigint unsigned, in nObjID bigint unsigned, in nCombatType int, in nAutoCombat int, in nAutoSupply int, in nHero1 bigint unsigned, in nHero2 bigint unsigned, in nHero3 bigint unsigned, in nHero4 bigint unsigned, in nHero5 bigint unsigned, in nStopLevel int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_start_combat: begin

set @vInstanceID=0;
SET @vA2='',@vAtkAllianceID=0;
set @vAccountID='';
set @vGoTime=0;
set @vBackTime=0;
-- 不能对自己开战
if (nAccountID=nObjID) then
	set nRst=-3;
	leave label_start_combat;
end if;
-- 不能没有武将
if (nHero1=0 and nHero2=0 and nHero3=0 and nHero4=0 and nHero5=0) then
	set nRst=-13;
	leave label_start_combat;
end if;

set @vCombatID=0;
set @vClassID=0;
set @vObjID=nObjID;
set @vStatus=0;
case
when nCombatType=1 then
	-- 军事任务
	if not exists (select 1 from excel_combat_raid where raid_id=nObjID) then
		set nRst=-17;
		leave label_start_combat;
	end if;
	
	set @vInstance=0;
	-- 生成战斗ID
	select game_global_id() into @vCombatID;

when nCombatType=2 then
	-- 玩家对战
	set @vProtectTime=0;
	if not exists (select 1 from common_characters where account_id=nObjID and (@vProtectTime:=protect_time)>=0) then
		set nRst=-17;
		leave label_start_combat;
	end if;
	-- 是否在保护期内
	if (@vProtectTime>unix_timestamp()) then
		set nRst=-19;
		leave label_start_combat;
	end if;
	-- 出征队列是否满了
	set @vPVPQueue=0;
	select val_int into @vPVPQueue from excel_cmndef where id=7;
	-- vip特权
	set @vVIP=0;
	set @vVIPQueue=0;
	select vip into @vVIP from common_characters where account_id=nAccountID;
	if (@vVIP>0) then
		select pvp_queue into @vVIPQueue from excel_vip_right where level=@vVIP;
	end if;
	set @vPVPQueue=@vPVPQueue+@vVIPQueue;
	
	set @vCurQueue=0;
	select count(1) into @vCurQueue from combats where account_id=nAccountID and (combat_type=2 OR combat_type=20 OR combat_type=1001);
	if (@vCurQueue>=@vPVPQueue) then
		set nRst=-20;
		leave label_start_combat;
	end if;
	
	set @vInstance=0;
	-- 生成战斗ID
	select game_global_id() into @vCombatID;

when nCombatType=10 or nCombatType=11 then
	-- 世界金矿
	-- 目标在 rob_worldgoldmine 已经判断过了
	set @vInstance=0;
	-- 生成战斗ID
	select game_global_id() into @vCombatID;

when nCombatType=100 then
	-- 百战不殆
	set @vInstance=1;
	-- 是副本就使用副本ID
	-- nCombatType是副本的excel id
	select T1.instance_id,T2.class_id,T2.cur_level into @vCombatID,@vClassID,@vObjID from instance_player T1 inner join instance T2 on T2.instance_id=T1.instance_id where T1.account_id=nAccountID and T1.excel_id=nCombatType;
	-- 自动战斗开关
	call game_switch_autocombat(nAccountID,@vCombatID,nAutoCombat,0,@vRst);
	-- 自动补给关凯
	call game_switch_autosupply(nAccountID,@vCombatID,nAutoSupply,0,@vRst);
	-- 自动配置副本武将
	call game_config_instancehero(nAccountID,nCombatType,@vCombatID,nHero1,nHero2,nHero3,nHero4,nHero5,0,@vRst);
	-- 停止关卡
	if (nStopLevel>0) then
		update instance set stop_level=nStopLevel where instance_id=@vCombatID;
	end if;
	set @vInstanceID=@vCombatID;

when nCombatType=101 then
	-- 南征北战
	set @vInstance=1;
	select T1.instance_id,T2.class_id,T2.cur_level,T2.cur_status into @vCombatID,@vClassID,@vObjID,@vStatus from instance_player T1 inner join instance T2 on T2.instance_id=T1.instance_id where T1.account_id=nAccountID and T1.excel_id=nCombatType;
	-- 副本开始了吗
	if (@vStatus=0) then
		set nRst=-15;
		leave label_start_combat;
	end if;
	-- 队长才能发起战斗
	if not exists (select 1 from instance where instance_id=@vCombatID and creator_id=nAccountID) then
		set nRst=-14;
		leave label_start_combat;
	end if;
	-- 获取所有玩家ID
	select group_concat(account_id) into @vAccountID from instance_player where instance_id=@vCombatID and status=1;
	if (length(@vAccountID)=0) then
		set nRst=-16;
		leave label_start_combat;
	end if;
	-- 自动战斗开关
	call game_switch_autocombat(nAccountID,@vCombatID,nAutoCombat,0,@vRst);
	-- 自动补给关凯
	call game_switch_autosupply(nAccountID,@vCombatID,nAutoSupply,0,@vRst);
	-- 停止关卡
	if (nStopLevel>0) then
		update instance set stop_level=nStopLevel where instance_id=@vCombatID;
	end if;
	set @vInstanceID=@vCombatID;

when nCombatType=20 then
	-- 世界名城
	if not exists (select 1 from world_city where id=nObjID and res>0) then
		set nRst=-17;
		leave label_start_combat;
	end if;
	
	-- 出征队列是否满了
	set @vPVPQueue=0;
	select val_int into @vPVPQueue from excel_cmndef where id=7;
	-- vip特权
	set @vVIP=0;
	set @vVIPQueue=0;
	select vip into @vVIP from common_characters where account_id=nAccountID;
	if (@vVIP>0) then
		select pvp_queue into @vVIPQueue from excel_vip_right where level=@vVIP;
	end if;
	set @vPVPQueue=@vPVPQueue+@vVIPQueue;
	
	set @vCurQueue=0;
	select count(1) into @vCurQueue from combats where account_id=nAccountID and (combat_type=2 OR combat_type=20 OR combat_type=1001);
	if (@vCurQueue>=@vPVPQueue) then
		set nRst=-20;
		leave label_start_combat;
	end if;
	
	set @vInstance=0;
	-- 生成战斗ID
	select game_global_id() into @vCombatID;

WHEN nCombatType=1001 THEN
	-- 世界资源
	IF NOT EXISTS (SELECT 1 FROM world_res WHERE id=nObjID) THEN
		SET nRst=-17;
		LEAVE label_start_combat;
	END IF;
	
	-- 出征队列是否满了
	SET @vPVPQueue=0;
	SELECT val_int INTO @vPVPQueue FROM excel_cmndef WHERE id=7;
	-- vip特权
	SET @vVIP=0;
	SET @vVIPQueue=0;
	SELECT vip INTO @vVIP FROM common_characters WHERE account_id=nAccountID;
	IF (@vVIP>0) THEN
		SELECT pvp_queue INTO @vVIPQueue FROM excel_vip_right WHERE level=@vVIP;
	END IF;
	SET @vPVPQueue=@vPVPQueue+@vVIPQueue;
	
	SET @vCurQueue=0;
	SELECT COUNT(1) INTO @vCurQueue FROM combats WHERE account_id=nAccountID AND (combat_type=2 OR combat_type=20 OR combat_type=1001);
	IF (@vCurQueue>=@vPVPQueue) THEN
		SET nRst=-20;
		LEAVE label_start_combat;
	END IF;
	
	-- 不是副本战斗
	SET @vInstance=0;
	-- 生成战斗ID
	SELECT game_global_id() INTO @vCombatID;
else
	set nRst=-18;
	leave label_start_combat;

end case;

if (@vCombatID=0) then
	set nRst=-4;
	leave label_start_combat;
end if;

-- 是副本战斗吗
if (@vInstance=1) then
if exists (select 1 from combats where combat_id=@vCombatID) then
	-- combat_id存在就表示上一关还没打完
	set nRst=-10;
	leave label_start_combat;
end if;
end if;

-- 多人副本已经锁定过了
if (nCombatType!=101) then
set @vRst=0;
-- 依次锁定武将
-- <=0的都表示出错了
start transaction;
call game_lock_hero(nHero1,@vCombatID,nCombatType,1,@vRst);
if (@vRst<=0) then rollback; set nRst=-2; set @vCombatID=nHero1; leave label_start_combat; end if;
call game_lock_hero(nHero2,@vCombatID,nCombatType,2,@vRst);
if (@vRst<=0) then rollback; set nRst=-2; set @vCombatID=nHero2; leave label_start_combat; end if;
call game_lock_hero(nHero3,@vCombatID,nCombatType,3,@vRst);
if (@vRst<=0) then rollback; set nRst=-2; set @vCombatID=nHero3; leave label_start_combat; end if;
call game_lock_hero(nHero4,@vCombatID,nCombatType,4,@vRst);
if (@vRst<=0) then rollback; set nRst=-2; set @vCombatID=nHero4; leave label_start_combat; end if;
call game_lock_hero(nHero5,@vCombatID,nCombatType,5,@vRst);
if (@vRst<=0) then rollback; set nRst=-2; set @vCombatID=nHero5; leave label_start_combat; end if;
commit;
else
-- 改变一下 slot_idx
update combat_heros set slot_idx=1 where combat_id=@vCombatID and hero_id=nHero1;
update combat_heros set slot_idx=2 where combat_id=@vCombatID and hero_id=nHero2;
update combat_heros set slot_idx=3 where combat_id=@vCombatID and hero_id=nHero3;
update combat_heros set slot_idx=4 where combat_id=@vCombatID and hero_id=nHero4;
update combat_heros set slot_idx=5 where combat_id=@vCombatID and hero_id=nHero5;
-- 改变一下 join_time
set @vID=0;select account_id into @vID from hire_heros where hero_id=nHero1;
update instance_player set join_time=unix_timestamp()+1 where instance_id=@vCombatID and account_id=@vID;
set @vID=0;select account_id into @vID from hire_heros where hero_id=nHero2;
update instance_player set join_time=unix_timestamp()+2 where instance_id=@vCombatID and account_id=@vID;
set @vID=0;select account_id into @vID from hire_heros where hero_id=nHero3;
update instance_player set join_time=unix_timestamp()+3 where instance_id=@vCombatID and account_id=@vID;
set @vID=0;select account_id into @vID from hire_heros where hero_id=nHero4;
update instance_player set join_time=unix_timestamp()+4 where instance_id=@vCombatID and account_id=@vID;
set @vID=0;select account_id into @vID from hire_heros where hero_id=nHero5;
update instance_player set join_time=unix_timestamp()+5 where instance_id=@vCombatID and account_id=@vID;
end if;

-- 获取前往时间
set @vGoTime=0;
set @vBackTime=0;
-- 从表格读取
select go_time,back_time into @vGoTime,@vBackTime from excel_combat where excel_id=nCombatType;
if (nCombatType=2) then
	-- 根据玩家距离计算
	set @vX1=0;set @vX2=0;
	set @vY1=0;set @vY2=0;
	select pos_x,pos_y into @vX1,@vY1 from common_characters where account_id=nAccountID;
	select pos_x,pos_y into @vX2,@vY2 from common_characters where account_id=nObjID;
	set @vGoTime=game_calc_movetime(@vX1,@vY1,@vX2,@vY2);
	set @vBackTime=@vGoTime;

elseif (nCombatType=20) then
	-- 根据世界名城举例计算
	set @vX1=0;set @vX2=0;
	set @vY1=0;set @vY2=0;
	select pos_x,pos_y into @vX1,@vY1 from common_characters where account_id=nAccountID;
	select pos_x,pos_y into @vX2,@vY2 from world_city where id=nObjID;
	set @vGoTime=game_calc_movetime(@vX1,@vY1,@vX2,@vY2);
	set @vBackTime=@vGoTime;
ELSEIF (nCombatType=1001) THEN
	-- 根据世界资源距离计算
	SET @vX1=0;SET @vX2=0;
	SET @vY1=0;SET @vY2=0;
	SELECT pos_x,pos_y INTO @vX1,@vY1 FROM common_characters WHERE account_id=nAccountID;
	SELECT pos_x,pos_y INTO @vX2,@vY2 FROM world_res WHERE id=nObjID;
	SET @vGoTime=game_calc_movetime(@vX1,@vY1,@vX2,@vY2);
	SET @vBackTime=@vGoTime;
end if;

-- vip特权
set @vVIP=0;
select vip into @vVIP from common_characters where account_id=nAccountID;
set @vArmyAccelerate=1;
if (@vVIP>0) then
	select army_accelerate into @vArmyAccelerate from excel_vip_right where level=@vVIP;
end if;

if (nCombatType=2 or nCombatType=20 OR nCombatType=1001) then
	set @vGoTime=ceiling(@vGoTime*@vArmyAccelerate);
	set @vBackTime=@vGoTime;
end if;

-- 双方名字
set @vAttackName='';
set @vDefenseName='';
select name into @vAttackName from common_characters where account_id=nAccountID;
if (nCombatType=2) then
	select name into @vDefenseName from common_characters where account_id=@vObjID;
end if;

-- 插入战斗表
insert into combats (combat_id,combat_type,account_id,class_id,obj_id,begin_time,use_time,status,attack_data,defense_data,misc_data,attack_name,defense_name,loot_list) values (@vCombatID,nCombatType,nAccountID,@vClassID,@vObjID,unix_timestamp(),@vGoTime,0,'','','',@vAttackName,@vDefenseName,'');
set nRst=row_count();

if (nCombatType=2 or nCombatType=20 OR nCombatType=1001) then
	-- 记录往返时间
	update combats set misc_data=concat(@vGoTime,'*',@vBackTime) where combat_id=@vCombatID;
end if;

-- 战斗前往超时 101
call game_add_te_combat(@vCombatID,@vGoTime,101,0,@vRst);

-- 清除保护状态
if (nCombatType=2 or nCombatType=20) then
	update common_characters set protect_time=0 where account_id=nAccountID;
end if;

-- 追加世界名城队列
if (nCombatType=20) then
	update world_city set combat_id=concat(combat_id,',',nAccountID,',',@vCombatID) where id=nObjID;
ELSEIF (nCombatType=1001) THEN
	UPDATE world_res SET combat_id=CONCAT(combat_id,',',nAccountID,',',@vCombatID) WHERE id=nObjID;
end if;

-- 对目标联盟的通知
set @vAllianceID=0;
set @vA1='';
if (nCombatType=2) then
	select alliance_id into @vAllianceID from common_characters where account_id=nObjID;
	select content into @vA1 from excel_text where excel_id=460;
	set @vA1=replace(@vA1,'%s1',@vAttackName);
	set @vA1=replace(@vA1,'%s2',@vDefenseName);
	
	SET @vA2='',@vAtkAllianceID=0;
	SELECT alliance_id INTO @vAtkAllianceID FROM common_characters where account_id=nAccountID;
	SELECT content INTO @vA2 FROM excel_text WHERE excel_id=476;
	SET @vA2=REPLACE(@vA2,'%s1',@vAttackName);
	SET @vA2=REPLACE(@vA2,'%s2',@vDefenseName);
end if;

end label_start_combat;
if (nEcho=1) then
	if (nRst<=0) then
	if (nCombatType=100 or nCombatType=101) then
	call game_switch_autocombat(nAccountID,@vInstanceID,0,0,@vRst);
	end if;
	end if;
	select nRst,@vCombatID,@vGoTime,@vBackTime,@vAccountID,@vAllianceID,@vA1,@vAtkAllianceID,@vA2;
end if;
end
//
-- 计算两点之间的移动时间
drop function if exists `game_calc_movetime`;
create function game_calc_movetime(nPosX1 int, nPosY1 int, nPosX2 int, nPosY2 int) returns int
begin

declare exit			handler for sqlexception		begin end;
declare exit			handler for sqlstate '22003'	begin end;

set @vDiffX=abs(cast(nPosX1 as signed)-cast(nPosX2 as signed));
set @vDiffY=abs(cast(nPosY1 as signed)-cast(nPosY2 as signed));
set @vDiffPos=sqrt(pow(@vDiffX,2)+pow(@vDiffY,2));
SELECT go_time INTO @vBaseTime FROM excel_combat WHERE excel_id=2;
set @vBasePos=sqrt(pow(400,2)+pow(400,2));
set @vNeedTime=floor(@vDiffPos/@vBasePos*@vBaseTime);

return @vNeedTime;
end
//

-- 获取战斗信息
drop procedure if exists `game_get_combat`;
create procedure game_get_combat(in nAccountID bigint unsigned, in nCombatID bigint unsigned, in nGen int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_combat: begin

-- 要生成战斗数据吗
if (nGen=1) then
	set @vCombatType=0;
	set @vClassID=0;
	set @vObjID=0;
	if exists (select 1 from combats where combat_id=nCombatID and status=0 and (@vCombatType:=combat_type)>0 and (@vClassID:=class_id)>=0 and (@vObjID:=obj_id)>0) then
		-- 生成战斗数据快照
		call game_gen_combat(nCombatID,@vCombatType,@vClassID,@vObjID,@vRst);
		-- 更新
		update combats set status=1,attack_data=@vAttackSide,defense_data=@vDefenseSide where combat_id=nCombatID;
	end if;
end if;

set @vObjName='';
set @vObjHeadID=0;
if (nAccountID>0) then
	set @vAccountID=0;
	set @vObjID=0;
	select account_id,obj_id into @vAccountID,@vObjID from combats where combat_id=nCombatID;
	if (nAccountID=@vAccountID) then
		select name,head_id into @vObjName,@vObjHeadID from common_characters where account_id=@vObjID;
	else
		select name,head_id into @vObjName,@vObjHeadID from common_characters where account_id=@vAccountID;
	end if;
	-- 玩家主动获取战斗数据，所以这里要填好 nCombatType
	select if(account_id=nAccountID or combat_type=101,combat_type,combat_type+1),account_id,obj_id,@vObjName,@vObjHeadID,convert(attack_data using utf8),convert(defense_data using utf8) from combats where combat_id=nCombatID and status=1;
else
	-- 战斗时间事件回调，nCombatType 会在外部处理
	select combat_type,account_id,obj_id,@vObjName,@vObjHeadID,convert(attack_data using utf8),convert(defense_data using utf8) from combats where combat_id=nCombatID and status=1;
end if;

end label_get_combat;
end
//
-- 战斗返回状态
drop procedure if exists `game_back_combat`;
create procedure game_back_combat(in nCombatID bigint unsigned, in nCombatRst int, out nRst int)
begin

declare nDone			int default 0;
declare nSlotIdx		int default 0;
declare nID				bigint unsigned default 0;

DECLARE _pos_x,_pos_y INT UNSIGNED DEFAULT 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select account_id from instance_player where instance_id=nCombatID;
declare vCursor1		cursor for select slot_idx,hero_id from combat_heros where combat_id=nCombatID;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_back_combat: begin

set @vCombatType=0;
set @vAccountID=0;
set @vClassID=0;
set @vObjID=0;
set @vAttackData='';
set @vDefenseData='';
set @vMiscData='';
select combat_type,account_id,class_id,obj_id,attack_data,defense_data,misc_data into @vCombatType,@vAccountID,@vClassID,@vObjID,@vAttackData,@vDefenseData,@vMiscData from combats where combat_id=nCombatID;

-- 双方名字
set @vAttackName='';
set @vDefenseName='';
set @vAttackHeadID=0;
set @vDefenseHeadID=0;
set @vAttackAlliance=0;
set @vDefenseAlliance=0;
select name,head_id,alliance_id into @vAttackName,@vAttackHeadID,@vAttackAlliance from common_characters where account_id=@vAccountID;
select name,head_id,alliance_id into @vDefenseName,@vDefenseHeadID,@vDefenseAlliance from common_characters where account_id=@vObjID;

set @vMan=0;
set @vManReal=0;

-- 更新pvp
if (@vCombatType=2) then
	update common_characters set pvp_attack=pvp_attack+1 where account_id=@vAccountID;
	update common_characters set pvp_defense=pvp_defense+1 where account_id=@vObjID;
	
	SELECT pos_x,pos_y INTO _pos_x,_pos_y FROM common_characters WHERE account_id=@vAccountID;

-- 更新每日pvp次数
	update common_characters set pvp_attack_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),0,pvp_attack_win_day),
		pvp_defense_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),0,pvp_defense_win_day),
		pvp_done_tick=unix_timestamp() where account_id=@vAccountID;
	update common_characters set pvp_attack_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),0,pvp_attack_win_day),
		pvp_defense_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),0,pvp_defense_win_day),
		pvp_done_tick=unix_timestamp() where account_id=@vObjID;
end if;

	IF @vCombatType=11 OR @vCombatType=2 THEN
		SELECT pos_x,pos_y INTO _pos_x,_pos_y FROM common_characters WHERE account_id=@vAccountID;
	END IF;

-- 奖杯
set @vOne=0;

if (nCombatRst=1) then
-- 胜利了

case
when @vCombatType=1 then
	-- 军事任务
	-- 会掉落道具
	set @vLootList='';
	set @vCharExp=0;
	set @vHeroExp=0;
	select loot_list,char_exp,hero_exp into @vLootList,@vCharExp,@vHeroExp from excel_combat_raid where raid_id=@vObjID;

	if (length(@vLootList)>0) then
		set @vRst=0;
		call game_add_stritemnum(@vAccountID,@vLootList,1,0,@vRst);
		if (@vRst!=0) then set nRst=-5; leave label_back_combat; end if;
		
		-- 记录掉落 nItemID,nExcelID
		update combats set loot_list=@vMyAddItem where combat_id=nCombatID;
	end if;

	-- 君主经验
	call add_char_exp(@vAccountID,@vCharExp,@vRstExp,@vRstLevel);
	
	-- 武将经验
	set nDone=0;
	open vCursor1;
	repeat
	fetch vCursor1 into nSlotIdx,nID;
	if not nDone then
		call add_hero_exp(@vAccountID,nID,@vHeroExp,@vRst);
	end if;
	until nDone end repeat;
	close vCursor1;

when @vCombatType=2 then
	-- 玩家对战
	-- 会有金币和人口的掠夺
	set @vAttackProf=0;
	set @vDefenseProf=0;
	
	set @vArmy1='';set @vArmy2='';set @vArmy3='';set @vArmy4='';set @vArmy5='';
	call game_split_5(@vAttackData,',',@vArmy1,@vArmy2,@vArmy3,@vArmy4,@vArmy5);
	call game_calc_strprof(@vArmy1,'*',0,@vRst); set @vAttackProf=@vAttackProf+@vProf;
	call game_calc_strprof(@vArmy2,'*',0,@vRst); set @vAttackProf=@vAttackProf+@vProf;
	call game_calc_strprof(@vArmy3,'*',0,@vRst); set @vAttackProf=@vAttackProf+@vProf;
	call game_calc_strprof(@vArmy4,'*',0,@vRst); set @vAttackProf=@vAttackProf+@vProf;
	call game_calc_strprof(@vArmy5,'*',0,@vRst); set @vAttackProf=@vAttackProf+@vProf;

	set @vArmy1='';set @vArmy2='';set @vArmy3='';set @vArmy4='';set @vArmy5='';
	call game_split_5(@vDefenseData,',',@vArmy1,@vArmy2,@vArmy3,@vArmy4,@vArmy5);
	call game_calc_strprof(@vArmy1,'*',0,@vRst); set @vDefenseProf=@vDefenseProf+@vProf;
	call game_calc_strprof(@vArmy2,'*',0,@vRst); set @vDefenseProf=@vDefenseProf+@vProf;
	call game_calc_strprof(@vArmy3,'*',0,@vRst); set @vDefenseProf=@vDefenseProf+@vProf;
	call game_calc_strprof(@vArmy4,'*',0,@vRst); set @vDefenseProf=@vDefenseProf+@vProf;
	call game_calc_strprof(@vArmy5,'*',0,@vRst); set @vDefenseProf=@vDefenseProf+@vProf;

	-- 根据战力比例计算掠夺比例
	set @vPct=@vAttackProf/if(@vDefenseProf>0,@vDefenseProf,1);
	-- 奖杯的掠夺比例
	set @vPct1=0;
	case
	when @vPct<=1.2 then
		set @vPct=0.2;
		set @vPct1=10;
	when @vPct<=1.5 then
		set @vPct=0.24;
		set @vPct1=20;
	when @vPct<=2 then
		set @vPct=0.28;
		set @vPct1=30;
	when @vPct<=3 then
		set @vPct=0.3;
		set @vPct1=40;
	else
		set @vPct=0.35;
		set @vPct1=50;
	end case;

	set @vCup1=0;
	set @vCup2=0;
	set @vGold1=0;
	set @vGold2=0;
	set @vMan1=0;
	set @vMan2=0;
	update common_characters set gold=(@vGold2:=floor(gold*(1-@vPct))),population=(@vMan2:=floor(population*(1-@vPct))),cup=(@vCup2:=if(cup>@vPct1,cup-@vPct1,0)) where account_id=@vObjID and (@vGold1:=gold)>=0 and (@vMan1:=population)>=0 and (@vCup1:=cup)>=0;
	set @vGold=@vGold1-@vGold2;
	set @vMan=@vMan1-@vMan2;
	set @vCup=@vCup1-@vCup2;
	update common_characters set gold=gold+@vGold where account_id=@vAccountID;
	
	if (@vCup>0) then
	update common_characters set cup=cup+@vCup where account_id=@vAccountID;
	
	set @vCur=0;
	select ifnull(max(id),0) into @vCur from world_city_history;
	set @vItemID=0;
	select item_id into @vItemID from items where account_id=@vObjID and excel_id=3010;
	call game_del_item(@vObjID,@vItemID,@vCup,0,@vRst);
	call game_add_item(@vAccountID,0,3010,@vCup,0,@vRst);
	insert into world_city_log (id,acct_id,alliance_id,cup) values (@vCur,@vAccountID,@vAttackAlliance,@vCup) on duplicate key update alliance_id=@vAttackAlliance,cup=cup+@vCup;
	end if;

	set @vManReal=0;
	call add_population(@vAccountID,@vMan,@vManReal);
--	set @vMan=@vManReal;

	-- 更新pvp
	update common_characters set pvp_attack_win=pvp_attack_win+1,pvp_attack_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),1,pvp_attack_win_day+1),pvp_done_tick=unix_timestamp() where account_id=@vAccountID;
	-- 修改保护时间
	UPDATE common_characters SET protect_time=UNIX_TIMESTAMP() WHERE account_id=@vAccountID AND protect_time>UNIX_TIMESTAMP();
	-- 保护测试
	CALL check_protect_char(@vObjID);
when @vCombatType=10 or @vCombatType=11 then
	-- 世界金矿
	-- 占领并锁定
	call game_split_3(@vMiscData,'*',@vArea,@vClass,@vIdx);
	call game_set_worldgoldmine(@vArea,@vClass,@vIdx,@vAccountID,@vObjID,1,@vRst);
	if (@vRst=1) then
		-- 删除旧的时间事件
		call game_del_te_worldgoldmine_1(@vArea,@vClass,@vIdx,@vRst);

		-- 配置金矿武将
		set @vHero1=0;
		set @vHero2=0;
		set @vHero3=0;
		set @vHero4=0;
		set @vHero5=0;

		set nDone=0;
		open vCursor1;
		repeat
		fetch vCursor1 into nSlotIdx,nID;
		if not nDone then
			set @vQuery=concat('set @vHero', nSlotIdx, '=', nID);
			prepare smt from @vQuery;
			execute smt;
			deallocate prepare smt;
		end if;
		until nDone end repeat;
		close vCursor1;
	
		call game_config_worldgoldmine_hero(@vAccountID,@vHero1,@vHero2,@vHero3,@vHero4,@vHero5,0,@vRst);

		-- 世界金矿收获超时 105
		call game_get_worldgoldmine_cfg(@vArea,@vClass,0,@vRst);
		call game_add_te_worldgoldmine(@vArea,@vClass,@vIdx,@vAccountID,@vOutFreq,105,0,@vRst);
		-- 解锁
		update world_goldmine set xlock=0 where area=@vArea and class=@vClass and idx=@vIdx and account_id=@vAccountID and xlock=1;
	end if;

when @vCombatType=100 or @vCombatType=101 then
	-- 百战不殆/南征北战
	-- 更新重试次数
	select T1.retry_times,T2.stop_level into @vRetryTimes,@vStopLevel from excel_instance_list T1 left join instance T2 on T2.excel_id=T1.excel_id where T2.instance_id=nCombatID;
	update instance set retry_times=@vRetryTimes where instance_id=nCombatID;

	-- 更新关卡号
	case @vCombatType
	when 100 then
		set @vMaxLevel=100;
	when 101 then
		set @vMaxLevel=20;
	else
		set @vMaxLevel=0;
	end case;

	set @vCurLevel=0;
	update instance set cur_level=(@vCurLevel:=cur_level)+1 where instance_id=nCombatID and cur_level<=@vMaxLevel;

	-- 设置排行榜
	if (@vCombatType=100) then
		SET @_begin_level=(@vClassID-1)*100;
		update common_characters set instance_wangzhe=@vCurLevel+@_begin_level where account_id=@vAccountID and instance_wangzhe<@vCurLevel+@_begin_level;
	end if;

	-- 到停止关卡了吗
	if (@vStopLevel>0) then
		if (@vStopLevel<=@vCurLevel) then
			call game_switch_autocombat(@vAccountID,nCombatID,0,0,@vRst);
		end if;
	end if;

	-- 奖励
	set @vExpHero=0;
	set @vGold=0;
	set @vLootTable='';
	set @vLootNum=0;
	set @vExpChar=0;
	case @vCombatType
	when 100 then
		CASE @vClassID
		WHEN 1 THEN
			select awa_exp,awa_gold,loot_table,loot_num,awa_exp_char into @vExpHero,@vGold,@vLootTable,@vLootNum,@vExpChar from excel_baizhanbudai where level_id=@vObjID;
		WHEN 2 THEN
			select awa_exp,awa_gold,loot_table,loot_num,awa_exp_char into @vExpHero,@vGold,@vLootTable,@vLootNum,@vExpChar from excel_baizhanbudai_epic where level_id=@vObjID;
		ELSE
			select awa_exp,awa_gold,loot_table,loot_num,awa_exp_char into @vExpHero,@vGold,@vLootTable,@vLootNum,@vExpChar from excel_baizhanbudai where level_id=@vObjID;
		END CASE;
	when 101 then
		select awa_exp,awa_gold,loot_table,loot_num,awa_exp_char into @vExpHero,@vGold,@vLootTable,@vLootNum,@vExpChar from excel_nanzhengbeizhan where class_id=@vClassID and level_id=@vObjID;
	else
		set @vRst=0;
	end case;

	-- 生成掉落
	set @vStrItem='';
	while @vLootNum>0 do
		set @vItemID=0;
		select game_roll_table(@vLootTable) into @vItemID;
		SET @_excel_id=@vItemID;
		if (length(@vStrItem)=0) then
			set @vStrItem=concat(@vItemID,'*',@vNum);
		else
			set @vStrItem=concat(@vStrItem,'*',@vItemID,'*',@vNum);
		end if;
		set @vLootNum=@vLootNum-1;
	end while;

	set @vAcctNameStr='';
	-- 发给副本里每个玩家
	set nDone=0;
	open vCursor;
	repeat
	fetch vCursor into nID;
	if not nDone then
		-- 金币
		set @vAcctName='';
		update common_characters set gold=gold+@vGold where account_id=nID and (@vAcctName:=name) is not null;

		if (@vCombatType=101) then
			if (@vCurLevel=20) then
			update common_characters set instance_zhengzhan=@vClassID where account_id=nID and instance_zhengzhan<@vClassID;
			set @vQuery=concat('update common_characters set zhengzhan_',@vClassID,'=1 where account_id=',nID);
			prepare smt from @vQuery;
			execute smt;
			deallocate prepare smt;
			end if;
		end if;
		
		-- 君主经验
		call add_char_exp(nID,@vExpChar,@vRstExp,@vRstLevel);

		-- 武将经验
		call game_get_instanceheroid(nID,nCombatID,@vCombatType,0,@vRst);
		if (@vHero1>0) then call add_hero_exp(nID,@vHero1,@vExpHero,@vRst); end if;
		if (@vHero2>0) then call add_hero_exp(nID,@vHero2,@vExpHero,@vRst); end if;
		if (@vHero3>0) then call add_hero_exp(nID,@vHero3,@vExpHero,@vRst); end if;
		if (@vHero4>0) then call add_hero_exp(nID,@vHero4,@vExpHero,@vRst); end if;
		if (@vHero5>0) then call add_hero_exp(nID,@vHero5,@vExpHero,@vRst); end if;

		set @vMyAddItem='';
		-- 道具
		if (length(@vStrItem)>0) then
			set @vRst=0;
			call game_add_stritemnum(nID,@vStrItem,1,0,@vRst);
			if (@vRst!=0) then set nRst=-5; leave label_back_combat; end if;

			-- 记录掉落 nItemID,nExcelID
			update instance_player set loot_list=@vMyAddItem where account_id=nID and instance_id=nCombatID;

			-- 记录奖励日志
			IF @vCombatType=101 THEN
				INSERT INTO instance_drop_log (account_id,instance_id,class,level,excel_id,num,time) 
				VALUES (nID,nCombatID,@vClassID,@vCurLevel,@_excel_id,@vNum,UNIX_TIMESTAMP());
			END IF;
		end if;

		set @vAcctNameStr=concat(@vAcctNameStr,',',@vAcctName);
	end if;
	until nDone end repeat;
	close vCursor;

	-- 记录掉落
	if (length(@vStrItem)>0) then
		update instance set loot_list=if(length(loot_list)=0,@vStrItem,concat(loot_list,'*',@vStrItem)) where instance_id=nCombatID;
	end if;

	-- 获得道具的世界通告
	set @vBeginLevel=0;
	select val_int into @vBeginLevel from excel_cmndef where id=8;
	if (length(@vStrItem)>0) then
		if (@vCombatType=101 or @vCurLevel>=@vBeginLevel) then
		set @vAcctNameStr=substring(@vAcctNameStr,2,length(@vAcctNameStr)-1);
		set @vA1='';
		set @vA2='';
		select content into @vA1 from excel_text where excel_id=410;
		set @vAcctNameStr=concat(@vAcctNameStr,' ',@vA1,' ');
		if @vCombatType=100 then
			select concat(' ',content,' ') into @vA1 from excel_text where excel_id=402;
			select content into @vA2 from excel_text where excel_id=415+@vClassID;
		else
			select concat(' ',content,' ') into @vA1 from excel_text where excel_id=403;
			select content into @vA2 from excel_text where excel_id=420+@vClassID;
		end if;
		set @vA1=replace(@vA1,'%s2',@vCurLevel);
		set @vA1=replace(@vA1,'%s1',@vA2);
		set @vAcctNameStr=concat(@vAcctNameStr,@vA1);
		select content into @vA1 from excel_text where excel_id=411;
		set @vAcctNameStr=concat(@vAcctNameStr,@vA1);
		set @vAcctNameStr=concat(@vAcctNameStr,',');
		select concat(content,' ') into @vA1 from excel_text where excel_id=412;
		set @vAcctNameStr=concat(@vAcctNameStr,@vA1);
	--	call game_split_2(@vStrItem,'*',@vAItemID,@vNull);
		select substring_index(substring_index(@vStrItem,'*',1),'*',-1) into @vAItemID;
		select name into @vA1 from excel_item_list where excel_id=@vAItemID;
		set @vAcctNameStr=concat(@vAcctNameStr,@vA1);
		else
		set @vAcctNameStr='';
		end if;
	else
		set @vAcctNameStr='';
	end if;

when @vCombatType=20 then
	-- 世界名城
	set @vWorldCityStr='';
	call game_rob_cup(@vAccountID,@vObjID,0,@vRst);
	if (@vRst=0) then
	-- 记录掉落 nItemID,nExcelID
	set @vMyAddItem=concat_ws('*',@vItemID,3010,@vOne);
	update combats set loot_list=@vMyAddItem where combat_id=nCombatID;

	-- 获得奖杯的世界通告
	set @vWorldCityStr=concat(@vWorldCityStr,@vAttackName);
	set @vA1='';
	select content into @vA1 from excel_text where excel_id=410;
	set @vWorldCityStr=concat(@vWorldCityStr,' ',@vA1,' ');
	select content into @vA1 from excel_text where excel_id=451;
	set @vA2='';
	select content into @vA2 from excel_text where excel_id=451+@vClass;
	set @vA1=replace(@vA1,'%s1',@vA2);
	set @vA1=replace(@vA1,'%s2',@vPosX);
	set @vA1=replace(@vA1,'%s3',@vPosY);
	set @vWorldCityStr=concat(@vWorldCityStr,@vA1);
	select content into @vA1 from excel_text where excel_id=411;
	set @vWorldCityStr=concat(@vWorldCityStr,' ',@vA1);
	set @vWorldCityStr=concat(@vWorldCityStr,',');
	select content into @vA1 from excel_text where excel_id=412;
	set @vWorldCityStr=concat(@vWorldCityStr,@vA1,' ');
	select content into @vA1 from excel_text where excel_id=450;
	set @vA1=replace(@vA1,'%s1',@vOne);
	set @vWorldCityStr=concat(@vWorldCityStr,@vA1);
	end if;

	-- 修改保护时间
	UPDATE common_characters SET protect_time=UNIX_TIMESTAMP() WHERE account_id=@vAccountID AND protect_time>UNIX_TIMESTAMP();
WHEN @vCombatType=1001 THEN
	-- 删除资源(先不删除,在发送战斗邮件的时候再删除,发送邮件也放那里)
	-- DELETE FROM world_res WHERE id=@vObjID;
	-- 召回其他玩家的队伍
	CALL world_res_let_other_back(@vAccountID,@vObjID,@_result);
else
	-- 啥都不做。。
	set @vRst=0;
end case;

else
-- 失败了

set @vAutoCombat=0;
case
when @vCombatType=2 then
	-- 更新pvp
	update common_characters set pvp_defense_win=pvp_defense_win+1,pvp_defense_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),1,pvp_defense_win_day+1),pvp_done_tick=unix_timestamp() where account_id=@vObjID;
	-- 修改保护时间
	UPDATE common_characters SET protect_time=UNIX_TIMESTAMP() WHERE account_id=@vAccountID AND protect_time>UNIX_TIMESTAMP();
when @vCombatType=100 or @vCombatType=101 then
	-- 是副本
	-- 失败了就不能继续自动战斗了
	call game_switch_autocombat(@vAccountID,nCombatID,0,0,@vRst);

	-- 减少重试次数
	update instance set retry_times=retry_times-1 where instance_id=nCombatID;

when @vCombatType=20 then
	-- 世界名城
	if not exists (select 1 from te_worldcityrecover where id=@vObjID) then
	set @vRecover=0;
	select T1.recover into @vRecover from excel_worldcity_combat T1 left join world_city T2 on T1.id=T2.class where T2.id=@vObjID;
	-- 世界名城恢复超时 120
	call game_add_te_worldcityrecover(@vObjID,@vRecover,120,0,@vRst);
	end if;
	set @vWorldCityStr='';

else
	set @vRst=0;
end case;

set @vRst=0;
end if;

-- 获取返回时间
set @vBackTime=0;
if (@vCombatType=2 or @vCombatType=20 OR @vCombatType=1001) then
	call game_split_2(@vMiscData,'*',@vGoTime,@vBackTime);
else
	select back_time into @vBackTime from excel_combat where excel_id=@vCombatType;
end if;

update combats set begin_time=unix_timestamp(),use_time=@vBackTime,combat_rst=nCombatRst where combat_id=nCombatID and status=1;
if (row_count()=0) then
	set nRst=-2;
	leave label_back_combat;
end if;

-- 战斗返回超时 102
call game_add_te_combat(nCombatID,@vBackTime,102,0,@vRst);

-- 刷新世界名城战斗时刻
if (@vCombatType=20) then
	update world_city set last_combat_time=unix_timestamp() where id=@vObjID;
end if;

set nRst=0;
end label_back_combat;
-- 根据类型返回
case
when @vCombatType=1 then
	-- 军事任务
	call game_split_2(@vLootList,'*',@vLootID,@vLootNum);
	select nRst,@vCombatType,@vAttackName,@vDefenseName,@vObjID,_pos_x,_pos_y,@vLootID,@vLootNum;
when @vCombatType=2 then
	-- 玩家对战
	select nRst,@vCombatType,@vAttackName,@vDefenseName,@vObjID,_pos_x,_pos_y,@vAttackHeadID,@vDefenseHeadID,@vGold,@vMan,@vManReal,@vCup;
when @vCombatType=10 or @vCombatType=11 then
	-- 世界金矿
	select nRst,@vCombatType,@vAttackName,@vDefenseName,@vObjID,_pos_x,_pos_y,@vArea,@vClass,@vIdx;
when @vCombatType=100 or @vCombatType=101 then
	-- 副本
	select nRst,@vCombatType,@vAttackName,@vDefenseName,@vObjID,_pos_x,_pos_y,@vMyAddItem,@vAcctNameStr;
when @vCombatType=20 then
	-- 世界名城
	select nRst,@vCombatType,@vAttackName,@vDefenseName,@vObjID,_pos_x,_pos_y,@vOne,@vRes,@vWorldCityStr;
WHEN @vCombatType=1001 THEN
	-- 世界资源
	SELECT nRst,@vCombatType,@vAttackName,@vDefenseName,@vObjID,_pos_x,_pos_y,@vAccountID;
else
	-- 其他
	select nRst,@vCombatType,@vAttackName,@vDefenseName,@vObjID,_pos_x,_pos_y;
end case;
end
//
-- 根据combats内容计算战力
drop procedure if exists `game_calc_strprof`;
create procedure game_calc_strprof(in szSrc blob, in szTag char, in nEcho int, out nRst int)
begin

declare exit			handler for sqlexception		begin end;
declare exit			handler for sqlstate '22003'	begin end;

set nRst=-1;
label_calc_strprof: begin

set @vAttackArmy=0;
set @vDefenseArmy=0;
set @vHealthArmy=0;
set @vProf=0;
if (length(szSrc)=0) then
	set nRst=-2;
	leave label_calc_strprof;
end if;

select substring_index(substring_index(szSrc,szTag,6),szTag,-1) into @vAttackArmy;
select substring_index(substring_index(szSrc,szTag,7),szTag,-1) into @vDefenseArmy;
select substring_index(substring_index(szSrc,szTag,8),szTag,-1) into @vHealthArmy;

set @vProf=ceiling((@vAttackArmy+@vDefenseArmy*0.9+@vHealthArmy*0.8)/3);

set nRst=0;
end label_calc_strprof;
if (nEcho=1) then
	select nRst,@vAttackArmy,@vDefenseArmy,@vHealthArmy,@vProf;
end if;
end
//
-- 反转HEX字符串 --
drop function if exists `game_reverse_str`;
create function game_reverse_str(szInput char(16)) returns char(16)
begin

declare exit			handler for sqlexception		begin end;
declare exit			handler for sqlstate '22003'	begin end;

set @vOutput='';
set @vSize=length(szInput);

while @vSize>0 do
	set @vOutput=concat(@vOutput, substr(szInput,@vSize-1,2));
	set @vSize=@vSize-2;
end while;

return @vOutput;
end
//
-- 获取BLOB单位，传入数值类型和数组下标 --
drop procedure if exists `game_get_blob`;
create procedure game_get_blob(in szTable char(32), in szColumn char(32), in szInto char(64), in szWhere char(64), in bCompress bool, in nSize int, in nIdx int, out nRst int)
begin

declare exit			handler for sqlexception		begin end;
declare exit			handler for sqlstate '22003'	begin end;

set nRst=-1;

if (bCompress=true) then
	set @vUncompress='uncompress';
else
	set @vUncompress='';
end if;

if (szInto!='') then
	set @vInto='into ';
else
	set @vInto='';
end if;

set @vOffset=nSize*nIdx+1;
set @vQuery=concat('select game_reverse_str(substr(hex(', @vUncompress, '(', szColumn, ')),', @vOffset, ',', nSize, ')) ', @vInto, szInto, ' from ', szTable, ' ', szWhere);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set nRst=0;
end
//
-- 设置BLOB单位 --
drop procedure if exists `game_set_blob`;
create procedure game_set_blob(in szTable char(32), in szColumn char(32), in szWhere char(64), in bCompress bool, in nSize int, in nIdx int, in nVal bigint, out nRst int)
begin

declare exit			handler for sqlexception		begin end;
declare exit			handler for sqlstate '22003'	begin end;

set nRst=-1;

if (bCompress=true) then
	set @vCompress='compress';
	set @vUncompress='uncompress';
else
	set @vCompress='';
	set @vUncompress='';
end if;

set @vBlob='';
set @vQuery=concat('select hex(', @vUncompress, '(', szColumn, ')) into @vBlob from ', szTable, ' ', szWhere);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set @vVal=game_reverse_str(lpad(hex(nVal),nSize,'0'));
set @vOffset=nSize*nIdx+1;
set @vBlob=insert(@vBlob,@vOffset,nSize,@vVal);

set @vQuery=concat('update ', szTable, ' set ', szColumn, '=', @vCompress, '(unhex(''', @vBlob, ''')) ', szWhere);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set nRst=0;
end
//

-- 记录战斗日志
drop procedure if exists `game_combat_log`;
create procedure game_combat_log(in nCombatID bigint unsigned, in szAttackName char(32), in szDefenseName char(32), in nCombatRst int, in szData blob, out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select account_id from instance_player where instance_id=nCombatID;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_combat_log: begin

set @vMail='';

set @vCombatType=0;
set @vAccountID=0;
set @vObjID=0;
set @vMiscData='';
if not exists (select 1 from combats where combat_id=nCombatID and (@vCombatType:=combat_type)>0 and (@vAccountID:=account_id)>0 and (@vObjID:=obj_id)>0 and (@vMiscData:=misc_data) is not null) then
	set nRst=-2;
	leave label_combat_log;
end if;

set @v1st='';
set @v2nd='';
set @vStrA='';
set @vStrD='';
case
when @vCombatType=1 then
	-- 文本串
	select content into @v1st from excel_text where excel_id=100;
	select content into @v2nd from excel_text where excel_id=300+@vObjID;
	set @v1st=replace(@v1st,'%s1',@v2nd);
	select content into @v3rd from excel_text where excel_id=if(nCombatRst=1,105,106);
	set @vColor=if(nCombatRst=1,'[00FF00]','[FF0000]');
	set @vStrA=concat(@vColor,@v1st,',',@v3rd);

	-- 军事任务
	call add_private_mail(0,@vAccountID,4,2,0,@vStrA,szData,0,@vRst,@vMailID);
	if (@vRst=0) then set @vMail=concat(@vMail,',',@vAccountID,',',@vMailID); end if;

when @vCombatType=2 then
	-- 文本串
	select content into @v1st from excel_text where excel_id=100;
	set @v1st=replace(@v1st,'%s1',szDefenseName);
	select content into @v2nd from excel_text where excel_id=if(nCombatRst=1,105,106);
	set @vColor=if(nCombatRst=1,'[00FF00]','[FF0000]');
	set @vStrA=concat(@vColor,@v1st,',',@v2nd);
	select content into @v1st from excel_text where excel_id=101;
	set @v1st=replace(@v1st,'%s1',szAttackName);
	select content into @v2nd from excel_text where excel_id=if(nCombatRst!=1,105,106);
	set @vColor=if(nCombatRst!=1,'[00FF00]','[FF0000]');
	set @vStrD=concat(@vColor,@v1st,',',@v2nd);

	-- 玩家对战
	call add_private_mail(0,@vAccountID,4,2,0,@vStrA,szData,0,@vRst,@vMailID);
	if (@vRst=0) then set @vMail=concat(@vMail,',',@vAccountID,',',@vMailID); end if;
	call add_private_mail(0,@vObjID,4,2,0,@vStrD,szData,0,@vRst,@vMailID);
	if (@vRst=0) then set @vMail=concat(@vMail,',',@vObjID,',',@vMailID); end if;
	-- 类型要改一下
	call game_set_blob('private_mails','ext_data','where account_id=@vObjID and mail_id=@vMailID',true,8,0,@vCombatType+1,@vRst);

when @vCombatType=10 or @vCombatType=11 then

	call game_split_3(@vMiscData,'*',@vArea,@vClass,@vIdx);
	select content into @vArea from excel_text where excel_id=@vArea+110;
	select content into @vClass from excel_text where excel_id=@vClass+120;

	-- 文本串
	select content into @v1st from excel_text where excel_id=if(@vCombatType=10,102,103);
	set @v1st=replace(@v1st,'%s1',@vArea);
	set @v1st=replace(@v1st,'%d2',@vIdx);
	set @v1st=replace(@v1st,'%s3',@vClass);
	set @v1st=replace(@v1st,'%s4',szDefenseName);
	select content into @v2nd from excel_text where excel_id=if(nCombatRst=1,105,106);
	set @vColor=if(nCombatRst=1,'[00FF00]','[FF0000]');
	set @vStrA=concat(@vColor,@v1st,',',@v2nd);
	
	-- 世界金矿
	call add_private_mail(0,@vAccountID,4,2,0,@vStrA,szData,0,@vRst,@vMailID);
	if (@vRst=0) then set @vMail=concat(@vMail,',',@vAccountID,',',@vMailID); end if;
	if (@vCombatType=11) then
		-- 文本串
		select content into @v1st from excel_text where excel_id=104;
		set @v1st=replace(@v1st,'%s1',@vArea);
		set @v1st=replace(@v1st,'%d2',@vIdx);
		set @v1st=replace(@v1st,'%s3',@vClass);
		set @v1st=replace(@v1st,'%s4',szAttackName);
		select content into @v2nd from excel_text where excel_id=if(nCombatRst!=1,105,106);
		set @vColor=if(nCombatRst!=1,'[00FF00]','[FF0000]');
		set @vStrD=concat(@vColor,@v1st,',',@v2nd);
		
		call add_private_mail(0,@vObjID,4,2,0,@vStrD,szData,0,@vRst,@vMailID);
		if (@vRst=0) then set @vMail=concat(@vMail,',',@vObjID,',',@vMailID); end if;
		-- 类型要改一下
		call game_set_blob('private_mails','ext_data','where account_id=@vObjID and mail_id=@vMailID',true,8,0,@vCombatType+1,@vRst);
	end if;

when @vCombatType=100 or @vCombatType=101 then

	-- 上次关卡号
	set @vLastLevel=0;
	select cur_level,class_id into @vLastLevel,@_class_id from instance where instance_id=nCombatID;
	set @vLastLevel=if(nCombatRst=1, @vLastLevel-1, @vLastLevel);
	-- 打过整十关或者自动战斗失败
	if (mod(@vLastLevel,10)=0 and nCombatRst=1) then
		set nRst=0;
	elseif (@vAutoCombat=1 and nCombatRst=2) then
		set nRst=0;
	else
		set nRst=0;
		leave label_combat_log;
	end if;

	IF @vCombatType=100 THEN
		SET @vLastLevel=(@_class_id-1)*100+@vLastLevel;
	END IF;
	-- 文本串
	select content into @v1st from excel_text where excel_id=100;
	select content into @v2nd from excel_text where excel_id=300+@vCombatType;
	set @v2nd=replace(@v2nd,'%s1',@vLastLevel);
	set @v1st=replace(@v1st,'%s1',@v2nd);
	select content into @v3rd from excel_text where excel_id=if(nCombatRst=1,105,106);
	set @vColor=if(nCombatRst=1,'[00FF00]','[FF0000]');
	set @vStrA=concat(@vColor,@v1st,',',@v3rd);

	-- 副本掉落
	open vCursor;
	repeat
	fetch vCursor into nID;
	if not nDone then
		call add_private_mail(0,nID,4,2,0,@vStrA,szData,0,@vRst,@vMailID);
		if (@vRst=0) then set @vMail=concat(@vMail,',',nID,',',@vMailID); end if;
	end if;
	until nDone end repeat;
	close vCursor;

when @vCombatType=20 then
	-- 世界名城
	set @vClass=0;
	set @vOne=0;
	set @vPoxX=0;
	set @vPosY=0;
	set @vWorldCityStr='';
	if (nCombatRst=1) then
	select T2.id,T2.one,T1.pos_x,T1.pos_y into @vClass,@vOne,@vPosX,@vPosY from world_city T1 left join excel_worldcity_out T2 on T2.id=T1.class where T1.id=@vObjID and T1.win_id=@vAccountID;
	if (found_rows()=1) then
	select content into @v1st from excel_text where excel_id=100;
	set @vWorldCityStr=concat(@vWorldCityStr,@v1st);
	set @vA1='';
	select content into @vA1 from excel_text where excel_id=451;
	set @vA2='';
	select content into @vA2 from excel_text where excel_id=451+@vClass;
	set @vA1=replace(@vA1,'%s1',@vA2);
	set @vA1=replace(@vA1,'%s2',@vPosX);
	set @vA1=replace(@vA1,'%s3',@vPosY);
	set @vWorldCityStr=replace(@vWorldCityStr,'%s1',@vA1);
	select content into @vA1 from excel_text where excel_id=if(nCombatRst=1,411,461);
	set @vWorldCityStr=concat(@vWorldCityStr,' ',@vA1);
	if (nCombatRst=1) then
	set @vWorldCityStr=concat(@vWorldCityStr,',');
	select content into @vA1 from excel_text where excel_id=412;
	set @vWorldCityStr=concat(@vWorldCityStr,@vA1,' ');
	select content into @vA1 from excel_text where excel_id=450;
	set @vA1=replace(@vA1,'%s1',@vOne);
	set @vWorldCityStr=concat(@vWorldCityStr,@vA1);
	end if;
	set @vColor=if(nCombatRst=1,'[00FF00]','[FF0000]');
	set @vWorldCityStr=concat(@vColor,@vWorldCityStr);
	else
	select content into @vA1 from excel_text where excel_id=462;
	set @vWorldCityStr=concat('[FF0000]',@vWorldCityStr,' ',@vA1);
	end if;
	else
	select T2.id,T2.one,T1.pos_x,T1.pos_y into @vClass,@vOne,@vPosX,@vPosY from world_city T1 left join excel_worldcity_out T2 on T2.id=T1.class where T1.id=@vObjID;
	if (found_rows()=1) then
	select content into @v1st from excel_text where excel_id=100;
	set @vWorldCityStr=concat(@vWorldCityStr,@v1st);
	set @vA1='';
	select content into @vA1 from excel_text where excel_id=451;
	set @vA2='';
	select content into @vA2 from excel_text where excel_id=451+@vClass;
	set @vA1=replace(@vA1,'%s1',@vA2);
	set @vA1=replace(@vA1,'%s2',@vPosX);
	set @vA1=replace(@vA1,'%s3',@vPosY);
	set @vWorldCityStr=replace(@vWorldCityStr,'%s1',@vA1);
	select content into @vA1 from excel_text where excel_id=if(nCombatRst=1,411,461);
	set @vWorldCityStr=concat(@vWorldCityStr,' ',@vA1);
	set @vColor=if(nCombatRst=1,'[00FF00]','[FF0000]');
	set @vWorldCityStr=concat(@vColor,@vWorldCityStr);
	else
	select content into @vA1 from excel_text where excel_id=462;
	set @vWorldCityStr=concat('[FF0000]',@vWorldCityStr,@vA1);
	end if;
	end if;

	call add_private_mail(0,@vAccountID,4,2,0,@vWorldCityStr,szData,0,@vRst,@vMailID);
	if (@vRst=0) then set @vMail=concat(@vMail,',',@vAccountID,',',@vMailID); end if;

WHEN @vCombatType=1001 THEN
	-- 世界资源
	IF NOT EXISTS (SELECT 1 FROM world_res WHERE id=@vObjID AND (@_pos_x:=pos_x)>=0 AND (@_pos_y:=pos_y)>=0 AND (@_gold:=gold)>=0 AND (@_pop:=pop)>=0 AND (@_crystal:=crystal)>=0) THEN
		set nRst=0;
		leave label_combat_log;
	END IF;
	-- 发送邮件
	SELECT content INTO @_content FROM excel_text WHERE excel_id=IF(nCombatRst=1,471,472);
	SET @_content=REPLACE(@_content,'%s1',@_pos_x);
	SET @_content=REPLACE(@_content,'%s2',@_pos_y);
	IF nCombatRst=1 THEN
		SET @_need_comma=0;
		IF @_gold>0 THEN
			SET @_content=CONCAT(@_content,@_gold,' gold');
			SET @_need_comma=1;
		END IF;
		IF @_pop>0 THEN
			IF @_need_comma=1 THEN
				SET @_content=CONCAT(@_content,',');
			END IF;
			SET @_content=CONCAT(@_content,@_pop,' citizens');
			SET @_need_comma=1;
		END IF;
		IF @_crystal>0 THEN
			IF @_need_comma=1 THEN
				SET @_content=CONCAT(@_content,',');
			END IF;
			SET @_content=CONCAT(@_content,@_crystal,' crystal');
		END IF;
		
		-- 发送奖励
		UPDATE common_characters SET gold=gold+@_gold,crystal=crystal+@_crystal WHERE account_id=@vAccountID;
		IF (@_pop!=0) THEN
			CALL add_population(@vAccountID,@_pop,@_real_added_pop);
			IF (@_pop!=@_real_added_pop) THEN
				SET @_content=CONCAT(@_content, '(obtained ',@_real_added_pop,' citizens because pop is full)');
			END IF;
		END IF;
		-- 删除世界资源
		DELETE FROM world_res WHERE id=@vObjID;
		
		SET @_content=CONCAT('[00FF00]',@_content,'.[-]');
	ELSE
		SET @_content=CONCAT('[FF0000]',@_content,'[-]');
	END IF;
	
	CALL add_private_mail(0,@vAccountID,4,2,0,@_content,szData,0,@_result,@_mail_id);
	IF (@_result=0) THEN
		SET @vMail=CONCAT(@vMail,',',@vAccountID,',',@_mail_id);
	END IF;
else
	-- 啥都不做
	set @vRst=0;
end case;

set nRst=0;
end label_combat_log;
select nRst,@vMail;
end
//

-- 锁定武将，一般用在战斗前 --
-- nHeroID 武将ID
drop procedure if exists `game_lock_hero`;
create procedure game_lock_hero(in nHeroID bigint unsigned, in nCombatID bigint unsigned, in nCombatType int, in nSlotIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_lock_hero: begin

-- 为0就不用锁定了
if (nHeroID=0) then
	set nRst=2;
	leave label_lock_hero;
end if;

-- 更新武将状态
update hire_heros set status=nCombatType where hero_id=nHeroID and status=0 and army_num>0;
if (row_count()=0) then
	set nRst=-3;
	leave label_lock_hero;
end if;

-- 是否已经加入战斗
if exists (select 1 from combat_heros where combat_id=nCombatID and hero_id=nHeroID) then
	set nRst=-2;
	leave label_lock_hero;
end if;

insert into combat_heros (combat_id,slot_idx,hero_id) values (nCombatID,nSlotIdx,nHeroID);
set nRst=row_count();

end label_lock_hero;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 结束战斗
-- nCombatID 战斗ID
drop procedure if exists `game_stop_combat`;
create procedure game_stop_combat(in nAccountID bigint unsigned, in nCombatID bigint unsigned, out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select hero_id from combat_heros where combat_id=nCombatID order by slot_idx;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_stop_combat: begin

set @vCreatorID=0;
set @vCombatType=0;
set @vCombatRst=0;
if not exists (select 1 from combats where combat_id=nCombatID and (@vCreatorID:=account_id)>0 and (@vCombatType:=combat_type)>0 and (@vCombatRst:=combat_rst)>=0) then
	set nRst=-2;
	leave label_stop_combat;
end if;

set @vHero1=0;
set @vHero2=0;
set @vHero3=0;
set @vHero4=0;
set @vHero5=0;

set @vDrug1=0;
set @vDrug2=0;
set @vDrug3=0;
set @vDrug4=0;
set @vDrug5=0;

set @vN=1;
open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	-- 多人副本在战斗结束时不用解锁
	if (@vCombatType!=101) then
	-- 依次解锁武将
	-- <=0的都表示出错了
	set @vRst=0;
	call game_unlock_hero(nID,nCombatID,@vCombatType,@vRst);
	if (@vRst<=0) then set nRst=-3; leave label_stop_combat; end if;
	end if;

	-- 记录武将
	set @vQuery=concat('set @vHero', @vN, '=', nID);
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;
	set @vN=@vN+1;
end if;
until nDone end repeat;
close vCursor;

-- 从战斗表删除
delete from combats where combat_id=nCombatID;
set nRst=row_count();

-- 是副本战斗吗
set @vCurLevel=0;
set @vAutoCombat=0;
set @vAutoSupply=0;
set @vRetryTimes=0;
if (@vCombatType=100 OR @vCombatType=101) then
	select cur_level,auto_combat,auto_supply,retry_times into @vCurLevel,@vAutoCombat,@vAutoSupply,@vRetryTimes from instance where instance_id=nCombatID;
else
	-- 不是副本战斗就可以退出了
	leave label_stop_combat;
end if;

-- 上次战斗失败的话，还有重试次数吗
if (@vCombatRst=2) then
	if (@vRetryTimes<0) then
		set nRst=4;
	end if;
end if;

-- 要自动战斗吗
if (@vAutoCombat=1) then
case @vCombatType
when 100 then
	-- 百战不殆
	
	-- 有剩余关卡吗
	if (@vCurLevel<=100) then
		-- 要补给吗
		if (@vAutoSupply=1) then
		--	call game_auto_supply(0,@vHero1,0,@vRst); if (@vDrug>=0) then set @vDrug1=@vDrug1+@vDrug; end if;
		--	call game_auto_supply(0,@vHero2,0,@vRst); if (@vDrug>=0) then set @vDrug1=@vDrug1+@vDrug; end if;
		--	call game_auto_supply(0,@vHero3,0,@vRst); if (@vDrug>=0) then set @vDrug1=@vDrug1+@vDrug; end if;
		--	call game_auto_supply(0,@vHero4,0,@vRst); if (@vDrug>=0) then set @vDrug1=@vDrug1+@vDrug; end if;
		--	call game_auto_supply(0,@vHero5,0,@vRst); if (@vDrug>=0) then set @vDrug1=@vDrug1+@vDrug; end if;
			call game_auto_supply(0,@vHero1,0,@vRst); if (@vResDrug>=0) then set @vDrug1=@vResDrug; end if;
			call game_auto_supply(0,@vHero2,0,@vRst); if (@vResDrug>=0) then set @vDrug1=@vResDrug; end if;
			call game_auto_supply(0,@vHero3,0,@vRst); if (@vResDrug>=0) then set @vDrug1=@vResDrug; end if;
			call game_auto_supply(0,@vHero4,0,@vRst); if (@vResDrug>=0) then set @vDrug1=@vResDrug; end if;
			call game_auto_supply(0,@vHero5,0,@vRst); if (@vResDrug>=0) then set @vDrug1=@vResDrug; end if;
		end if;
		-- 继续战斗
		set @vRst=0;
		call game_start_combat(@vCreatorID,0,100,1,@vAutoSupply,@vHero1,@vHero2,@vHero3,@vHero4,@vHero5,0,0,@vRst);
		if (@vRst!=1) then set nRst=-3; call game_switch_autocombat(@vCreatorID,nCombatID,0,0,@vRst); leave label_stop_combat; end if;
		-- 
		set nRst=2;
	else
		-- 副本结束
		set nRst=3;
	end if;

when 101 then
	-- 南征北战
	
	-- 有剩余关卡吗
	if (@vCurLevel<=20) then
		-- 要补给吗
		if (@vAutoSupply=1) then
		--	call game_auto_supply(0,@vHero1,0,@vRst); if (@vDrug>=0) then set @vDrug1=@vDrug; end if;
		--	call game_auto_supply(0,@vHero2,0,@vRst); if (@vDrug>=0) then set @vDrug2=@vDrug; end if;
		--	call game_auto_supply(0,@vHero3,0,@vRst); if (@vDrug>=0) then set @vDrug3=@vDrug; end if;
		--	call game_auto_supply(0,@vHero4,0,@vRst); if (@vDrug>=0) then set @vDrug4=@vDrug; end if;
		--	call game_auto_supply(0,@vHero5,0,@vRst); if (@vDrug>=0) then set @vDrug5=@vDrug; end if;
			call game_auto_supply(0,@vHero1,0,@vRst); if (@vResDrug>=0) then set @vDrug1=@vResDrug; end if;
			call game_auto_supply(0,@vHero2,0,@vRst); if (@vResDrug>=0) then set @vDrug2=@vResDrug; end if;
			call game_auto_supply(0,@vHero3,0,@vRst); if (@vResDrug>=0) then set @vDrug3=@vResDrug; end if;
			call game_auto_supply(0,@vHero4,0,@vRst); if (@vResDrug>=0) then set @vDrug4=@vResDrug; end if;
			call game_auto_supply(0,@vHero5,0,@vRst); if (@vResDrug>=0) then set @vDrug5=@vResDrug; end if;
		end if;
		-- 继续战斗
		set @vRst=0;
		call game_start_combat(@vCreatorID,0,101,1,@vAutoSupply,@vHero1,@vHero2,@vHero3,@vHero4,@vHero5,0,0,@vRst);
		if (@vRst!=1) then set nRst=-3; call game_switch_autocombat(@vCreatorID,nCombatID,0,0,@vRst); leave label_stop_combat; end if;
		-- 
		set nRst=2;
	else
		-- 副本结束
		set nRst=3;
	end if;

else
	-- 啥都不做
	set @vRst=0;
end case;

else

case @vCombatType
when 100 then
	if (@vCurLevel>100) then
		set nRst=3;
	end if;
when 101 then
	if (@vCurLevel>20) then
		set nRst=3;
	end if;
else
	set @vRst=0;
end case;

end if;

-- 副本通关了吗
if (nRst=3) then
	set @vRst=0;
	call game_destroy_instance(@vCreatorID,@vCombatType,nCombatID,0,0,@vRst);
	if (@vRst!=0) then set nRst=-4; leave label_stop_combat; end if;
end if;
-- 副本关闭，因为重试次数用完
if (nRst=4) then
	set @vRst=0;
	call game_destroy_instance(@vCreatorID,@vCombatType,nCombatID,0,0,@vRst);
	if (@vRst!=0) then set nRst=-4; leave label_stop_combat; end if;
end if;

end label_stop_combat;
select nRst,@vCombatRst,@vCreatorID,@vHero1,@vHero2,@vHero3,@vHero4,@vHero5,@vDrug1,@vDrug2,@vDrug3,@vDrug4,@vDrug5,@vCombatID,@vGoTime,@vBackTime;
end
//

-- 解锁武将，一般用在战斗后 --
-- nHeroID 武将ID
drop procedure if exists `game_unlock_hero`;
create procedure game_unlock_hero(in nHeroID bigint unsigned, in nCombatID bigint unsigned, in nCombatType int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_unlock_hero: begin

-- 为0就不用解锁了
if (nHeroID=0) then
	set nRst=2;
	leave label_unlock_hero;
end if;

-- 更新武将状态
update hire_heros set status=0 where hero_id=nHeroID and status=nCombatType;
if (row_count()=0) then
	set nRst=-2;
	leave label_unlock_hero;
end if;

delete from combat_heros where combat_id=nCombatID and hero_id=nHeroID;
set nRst=row_count();

-- set nRst=1;
end label_unlock_hero;
end
//

-- 生成战斗数据快照 --
-- nCombatID 战斗ID
drop procedure if exists `game_gen_combat`;
create procedure game_gen_combat(in nCombatID bigint unsigned, in nCombatType int, in nClassID int, in nObjID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_gen_combat: begin

set @vAttackSide='';
set @vDefenseSide='';
set @vDrugStr='';
set @vVIP=0;
case
when nCombatType=1 then
	-- 讨伐AI
	-- 根据raid表计算战力
	set @vRst=0; call game_calc_raid(nObjID,1,@vRst); set @vArmy_1=@vArmy;
	set @vRst=0; call game_calc_raid(nObjID,2,@vRst); set @vArmy_2=@vArmy;
	set @vRst=0; call game_calc_raid(nObjID,3,@vRst); set @vArmy_3=@vArmy;
	set @vRst=0; call game_calc_raid(nObjID,4,@vRst); set @vArmy_4=@vArmy;
	set @vRst=0; call game_calc_raid(nObjID,5,@vRst); set @vArmy_5=@vArmy;
	
	select group_concat(concat_ws('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) into @vAttackSide from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T1.combat_id=nCombatID;
	select concat_ws(',',@vArmy_1,@vArmy_2,@vArmy_3,@vArmy_4,@vArmy_5) into @vDefenseSide;

when nCombatType=2 then
	-- 讨伐玩家
	
	-- tst
	-- 自动配置玩家城防
--	call game_config_citydefense_auto(nObjID, @vRst);

	-- vip自动补给城防战力
	set @vHero1=0;
	set @vHero2=0;
	set @vHero3=0;
	set @vHero4=0;
	set @vHero5=0;
	if exists (select 1 from common_characters where account_id=nObjID and (@vVIP:=vip)>0) then
		if exists (select 1 from citydefenses where account_id=nObjID and auto_supply>0) then
		select hero_1,hero_2,hero_3,hero_4,hero_5 into @vHero1,@vHero2,@vHero3,@vHero4,@vHero5 from citydefenses where account_id=nObjID;
		set @vHero=concat(',',concat_ws(',',@vHero1,@vHero2,@vHero3,@vHero4,@vHero5));
		call game_supply_strhero(nObjID,@vHero,0,@vRst);
		else
		set @vVIP=0;
		end if;
	end if;

	-- 根据当前城防计算战力
	set @vRst=0; call game_calc_citydefense(nObjID,1,@vRst); set @vArmy_1=@vArmy;
	set @vRst=0; call game_calc_citydefense(nObjID,2,@vRst); set @vArmy_2=@vArmy;
	set @vRst=0; call game_calc_citydefense(nObjID,3,@vRst); set @vArmy_3=@vArmy;
	set @vRst=0; call game_calc_citydefense(nObjID,4,@vRst); set @vArmy_4=@vArmy;
	set @vRst=0; call game_calc_citydefense(nObjID,5,@vRst); set @vArmy_5=@vArmy;
	
	select group_concat(concat_ws('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) into @vAttackSide from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T1.combat_id=nCombatID;
	select concat_ws(',',@vArmy_1,@vArmy_2,@vArmy_3,@vArmy_4,@vArmy_5) into @vDefenseSide;

when nCombatType=10 then
	-- 世界金矿，对AI
	-- 根据金矿等级计算战力
	set @vRst=0; call game_calc_worldgoldmine_pve(nObjID,1,@vRst); set @vArmy_1=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pve(nObjID,2,@vRst); set @vArmy_2=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pve(nObjID,3,@vRst); set @vArmy_3=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pve(nObjID,4,@vRst); set @vArmy_4=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pve(nObjID,5,@vRst); set @vArmy_5=@vArmy;
	
	select group_concat(concat_ws('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) into @vAttackSide from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T1.combat_id=nCombatID;
	select concat_ws(',',@vArmy_1,@vArmy_2,@vArmy_3,@vArmy_4,@vArmy_5) into @vDefenseSide;

when nCombatType=11 then
	-- 世界金矿，对玩家
	-- 根据金矿守卫配置计算战力
	set @vRst=0; call game_calc_worldgoldmine_pvp(nObjID,1,@vRst); set @vArmy_1=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pvp(nObjID,2,@vRst); set @vArmy_2=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pvp(nObjID,3,@vRst); set @vArmy_3=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pvp(nObjID,4,@vRst); set @vArmy_4=@vArmy;
	set @vRst=0; call game_calc_worldgoldmine_pvp(nObjID,5,@vRst); set @vArmy_5=@vArmy;
	
	select group_concat(concat_ws('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) into @vAttackSide from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T1.combat_id=nCombatID;
	select concat_ws(',',@vArmy_1,@vArmy_2,@vArmy_3,@vArmy_4,@vArmy_5) into @vDefenseSide;

when nCombatType=100 then
	-- 百战不殆
	-- 根据关卡号计算战力
	set @vRst=0; call game_calc_baizhanbudai(nObjID,1,nClassID,@vRst); set @vArmy_1=@vArmy;
	set @vRst=0; call game_calc_baizhanbudai(nObjID,2,nClassID,@vRst); set @vArmy_2=@vArmy;
	set @vRst=0; call game_calc_baizhanbudai(nObjID,3,nClassID,@vRst); set @vArmy_3=@vArmy;
	set @vRst=0; call game_calc_baizhanbudai(nObjID,4,nClassID,@vRst); set @vArmy_4=@vArmy;
	set @vRst=0; call game_calc_baizhanbudai(nObjID,5,nClassID,@vRst); set @vArmy_5=@vArmy;
	
	select group_concat(concat_ws('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) into @vAttackSide from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T1.combat_id=nCombatID;
	select concat_ws(',',@vArmy_1,@vArmy_2,@vArmy_3,@vArmy_4,@vArmy_5) into @vDefenseSide;

when nCombatType=101 then
	-- 南征北战
	-- 根据关卡号计算战力
	set @vRst=0; call game_calc_nanzhengbeizhan(nClassID,nObjID,1,@vRst); set @vArmy_1=@vArmy;
	set @vRst=0; call game_calc_nanzhengbeizhan(nClassID,nObjID,2,@vRst); set @vArmy_2=@vArmy;
	set @vRst=0; call game_calc_nanzhengbeizhan(nClassID,nObjID,3,@vRst); set @vArmy_3=@vArmy;
	set @vRst=0; call game_calc_nanzhengbeizhan(nClassID,nObjID,4,@vRst); set @vArmy_4=@vArmy;
	set @vRst=0; call game_calc_nanzhengbeizhan(nClassID,nObjID,5,@vRst); set @vArmy_5=@vArmy;
	
	select group_concat(concat_ws('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) into @vAttackSide from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T1.combat_id=nCombatID;
	select concat_ws(',',@vArmy_1,@vArmy_2,@vArmy_3,@vArmy_4,@vArmy_5) into @vDefenseSide;

when nCombatType=20 then
	-- 世界名城
	set @vRst=0; call game_calc_worldcity(nObjID,1,@vRst); set @vArmy_1=@vArmy;
	set @vRst=0; call game_calc_worldcity(nObjID,2,@vRst); set @vArmy_2=@vArmy;
	set @vRst=0; call game_calc_worldcity(nObjID,3,@vRst); set @vArmy_3=@vArmy;
	set @vRst=0; call game_calc_worldcity(nObjID,4,@vRst); set @vArmy_4=@vArmy;
	set @vRst=0; call game_calc_worldcity(nObjID,5,@vRst); set @vArmy_5=@vArmy;
	
	select group_concat(concat_ws('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) into @vAttackSide from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T1.combat_id=nCombatID;
	select concat_ws(',',@vArmy_1,@vArmy_2,@vArmy_3,@vArmy_4,@vArmy_5) into @vDefenseSide;
WHEN nCombatType=1001 THEN
	SET @vRst=0; CALL calc_world_res_army(nObjID,@vRst);
	SELECT GROUP_CONCAT(CONCAT_WS('*',T1.slot_idx,T2.hero_id,T2.name,T2.army_type,T2.army_level,T2.army_num,T2.army_attack,T2.army_defense,T2.army_health,T2.model,T2.level,T2.healthstate)) INTO @vAttackSide FROM combat_heros T1 INNER JOIN hire_heros T2 ON T2.hero_id=T1.hero_id WHERE T1.combat_id=nCombatID;
	SELECT CONCAT_WS(',',@_army1,@_army2,@_army3,@_army4,@_army5) INTO @vDefenseSide;
end case;

set nRst=0;
end label_gen_combat;
end
//

-- 解析raid配置 --
drop procedure if exists `game_calc_raid`;
create procedure game_calc_raid(in nRaidID bigint unsigned, in nSlotIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_raid: begin

set @vArmyID=0;
set @vArmyName='';
set @vArmyDeploy='';
select army_deploy into @vArmyDeploy from excel_combat_raid where raid_id=nRaidID;
set @vArmyType=substring_index(substring_index(@vArmyDeploy,'*',nSlotIdx),'*',-1);
set @vArmyLevel=0;
set @vArmyNum=0;
set @vModel=0;
set @vHealthState=0;

set @vTmp='';
select army_data into @vTmp from excel_combat_raid where raid_id=nRaidID;
call game_split_2(@vTmp,'*',@vArmyLevel,@vArmyNum);

set @vRst=0;
call game_calc_army(0,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
set @vArmy=concat(nSlotIdx,'*',@vArmyID,'*',@vArmyName,'*',@vArmyType,'*',@vArmyLevel,'*',@vArmyNum,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*',@vModel,'*',@vArmyLevel,'*',@vHealthState);

set nRst=0;
end label_calc_raid;
end
//

-- 解析城防配置 --
drop procedure if exists `game_calc_citydefense`;
create procedure game_calc_citydefense(in nAccountID bigint unsigned, in nSlotIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_citydefense: begin

set @vArmy='';
set @vQuery=concat('select concat_ws(\'*\',', nSlotIdx, ',hero_id,name,army_type,army_level,army_num,army_attack,army_defense,army_health,model,level,healthstate) into @vArmy from hire_heros T1 right join citydefenses T2 on T1.hero_id=T2.hero_', nSlotIdx, ' where T2.account_id=', nAccountID, ' and T2.hero_', nSlotIdx, '>0', ' and T1.army_num>0');
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set nRst=0;
end label_calc_citydefense;
end
//

-- 根据金矿等级计算战力
drop procedure if exists `game_calc_worldgoldmine_pve`;
create procedure game_calc_worldgoldmine_pve(in nClass int, in nSlotIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_worldgoldmine_pve: begin

set @vArmyID=0;
set @vArmyName='';
set @vArmyType=floor(1+(rand()*5));
set @vArmyLevel=0;
set @vArmyNum=0;

set @vTmp='';
select army_data into @vTmp from excel_worldgoldmine_combat where class=nClass;
call game_split_2(@vTmp,'*',@vArmyLevel,@vArmyNum);
set @vModel=0;
set @vHealthState=0;

set @vRst=0;
call game_calc_army(0,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
set @vArmy=concat(nSlotIdx,'*',@vArmyID,'*',@vArmyName,'*',@vArmyType,'*',@vArmyLevel,'*',@vArmyNum,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*',@vModel,'*',@vArmyLevel,'*',@vHealthState);

set nRst=0;
end label_calc_worldgoldmine_pve;
end
//

-- 解析金矿守卫配置 --
drop procedure if exists `game_calc_worldgoldmine_pvp`;
create procedure game_calc_worldgoldmine_pvp(in nAccountID bigint unsigned, in nSlotIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_worldgoldmine_pvp: begin

set @vArmy='';
set @vQuery=concat('select concat_ws(\'*\',', nSlotIdx, ',hero_id,name,army_type,army_level,army_num,army_attack,army_defense,army_health,model,level,healthstate) into @vArmy from hire_heros T1 right join world_goldmine T2 on T1.hero_id=T2.hero_', nSlotIdx, ' where T2.account_id=', nAccountID, ' and T2.hero_', nSlotIdx, '>0');
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set nRst=0;
end label_calc_worldgoldmine_pvp;
end
//

-- 根据百战不殆关卡号计算战力
drop procedure if exists `game_calc_baizhanbudai`;
create procedure game_calc_baizhanbudai(in nLevelID int, in nSlotIdx int, in nClassID int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_baizhanbudai: begin

set @vArmyID=0;
set @vArmyName='';
set @vArmyDeploy='';
CASE nClassID
WHEN 1 THEN
	select army_deploy into @vArmyDeploy from excel_baizhanbudai where level_id=nLevelID;
WHEN 2 THEN
	select army_deploy into @vArmyDeploy from excel_baizhanbudai_epic where level_id=nLevelID;
ELSE
	select army_deploy into @vArmyDeploy from excel_baizhanbudai where level_id=nLevelID;
END CASE;
set @vArmyType=substring_index(substring_index(@vArmyDeploy,'*',nSlotIdx),'*',-1);
set @vArmyLevel=0;
set @vArmyNum=0;
set @vModel=0;
set @vHealthState=0;

set @vTmp='';
CASE nClassID
WHEN 1 THEN
	select army_data into @vTmp from excel_baizhanbudai where level_id=nLevelID;
WHEN 2 THEN
	select army_data into @vTmp from excel_baizhanbudai_epic where level_id=nLevelID;
ELSE
	select army_data into @vTmp from excel_baizhanbudai where level_id=nLevelID;
END CASE;
call game_split_2(@vTmp,'*',@vArmyLevel,@vArmyNum);

set @vRst=0;
call game_calc_army(0,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
set @vArmy=concat(nSlotIdx,'*',@vArmyID,'*',@vArmyName,'*',@vArmyType,'*',@vArmyLevel,'*',@vArmyNum,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*',@vModel,'*',@vArmyLevel,'*',@vHealthState);

set nRst=0;
end label_calc_baizhanbudai;
end
//

-- 根据南征北战关卡号计算战力
drop procedure if exists `game_calc_nanzhengbeizhan`;
create procedure game_calc_nanzhengbeizhan(in nClassID int, in nLevelID int, in nSlotIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_nanzhengbeizhan: begin

set @vArmyID=0;
set @vArmyName='';
set @vArmyDeploy='';
select army_deploy into @vArmyDeploy from excel_nanzhengbeizhan where class_id=nClassID and level_id=nLevelID;
set @vArmyType=substring_index(substring_index(@vArmyDeploy,'*',nSlotIdx),'*',-1);
set @vArmyLevel=0;
set @vArmyNum=0;
set @vModel=0;
set @vHealthState=0;

set @vTmp='';
select army_data into @vTmp from excel_nanzhengbeizhan where class_id=nClassID and level_id=nLevelID;
call game_split_2(@vTmp,'*',@vArmyLevel,@vArmyNum);

set @vRst=0;
call game_calc_army(0,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
set @vArmy=concat(nSlotIdx,'*',@vArmyID,'*',@vArmyName,'*',@vArmyType,'*',@vArmyLevel,'*',@vArmyNum,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*',@vModel,'*',@vArmyLevel,'*',@vHealthState);

set nRst=0;
end label_calc_nanzhengbeizhan;
end
//

-- 根据世界名城关卡号计算战力
drop procedure if exists `game_calc_worldcity`;
create procedure game_calc_worldcity(in nID bigint unsigned, in nSlotIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_worldcity: begin

set @vArmyID=0;
set @vArmyName='';
set @vArmyType=0;
set @vArmyLevel=0;
set @vArmyNum=0;

set @vTmp='';
set @vQuery=concat('select army_', nSlotIdx, ' into @vTmp from world_city where id=', nID);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

if (length(@vTmp)=0) then
	set @vArmy='';
	leave label_calc_worldcity;
end if;

call game_split_3(@vTmp,'*',@vArmyType,@vArmyLevel,@vArmyNum);
if (@vArmyNum=0) then
	set @vArmy='';
	leave label_calc_worldcity;
end if;

set @vModel=0;
set @vHealthState=0;

call game_calc_army(0,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
set @vArmy='';
set @vArmy=concat(nSlotIdx,'*',@vArmyID,'*',@vArmyName,'*',@vArmyType,'*',@vArmyLevel,'*',@vArmyNum,'*',@vAttackArmy,'*',@vDefenseArmy,'*',@vHealthArmy,'*',@vModel,'*',@vArmyLevel,'*',@vHealthState);

set nRst=0;
end label_calc_worldcity;
end
//


-- 生成武将名字 --
-- nMale 1男生 0女生 其他随机
drop procedure if exists `game_gen_name`;
create procedure game_gen_name(in nMale int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_gen_name: begin

set @vFix=0;
if (nMale=1) then
	-- 男生
	set @vFix=1;
elseif (nMale=0) then
	-- 女生
	set @vFix=2;
else
	set @vFix=floor(1+(rand()*2));
end if;

set @vFamily='';
select family_name into @vFamily from excel_hero_name where family_name!='' order by rand() limit 1;

set @vIsDF=0;
if (length(@vFamily)>3) then
	-- 复姓
	set @vIsDF=1;
end if;

set @vName='';
set @vMod=floor(1+(rand()*2));
if (@vMod=1) then
	-- 1 单字
	set @vSingle='';
	set @vQuery=concat('select single_', @vFix, ' into @vSingle from excel_hero_name where single_', @vFix, '!=\'\' order by rand() limit 1');
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;
	set @vName=concat(@vFamily,@vSingle);
else
	-- 2 双字
	set @vMiddle='';
	set @vQuery=concat('select middle_', @vFix, ' into @vMiddle from excel_hero_name where middle_', @vFix, '!=\'\' order by rand() limit 1');
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;
	set @vSingle='';
	set @vQuery=concat('select single_', @vFix, ' into @vSingle from excel_hero_name where single_', @vFix, '!=\'\' order by rand() limit 1');
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;
	set @vName=concat(@vFamily,@vMiddle,@vSingle);
end if;

set nRst=0;
end label_gen_name;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 配置城防武将
-- nAccountID 账号ID
drop procedure if exists `game_config_citydefense`;
create procedure game_config_citydefense(in nAccountID bigint unsigned, in nHero1 bigint unsigned, in nHero2 bigint unsigned, in nHero3 bigint unsigned, in nHero4 bigint unsigned, in nHero5 bigint unsigned, in nAutoSupply int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_config_citydefense: begin

set @vRst=0;
-- 依次检测武将
-- <=0的都表示出错了
call game_check_hero(nAccountID,nHero1,@vRst);
if (@vRst<=0) then set nRst=-2; leave label_config_citydefense; end if;
call game_check_hero(nAccountID,nHero2,@vRst);
if (@vRst<=0) then set nRst=-2; leave label_config_citydefense; end if;
call game_check_hero(nAccountID,nHero3,@vRst);
if (@vRst<=0) then set nRst=-2; leave label_config_citydefense; end if;
call game_check_hero(nAccountID,nHero4,@vRst);
if (@vRst<=0) then set nRst=-2; leave label_config_citydefense; end if;
call game_check_hero(nAccountID,nHero5,@vRst);
if (@vRst<=0) then set nRst=-2; leave label_config_citydefense; end if;

update citydefenses set hero_1=nHero1,hero_2=nHero2,hero_3=nHero3,hero_4=nHero4,hero_5=nHero5,auto_supply=nAutoSupply where account_id=nAccountID;

set nRst=0;
end label_config_citydefense;
select nRst;
end
//
-- 获取城防武将ID
drop procedure if exists `game_get_citydefense`;
create procedure game_get_citydefense(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_citydefense: begin

select hero_1,hero_2,hero_3,hero_4,hero_5,auto_supply from citydefenses where account_id=nAccountID;

set @vRst=0;
end label_get_citydefense;
-- select nRst;
end
//

-- 武将是否存在
-- nAccountID 账号ID
drop procedure if exists `game_check_hero`;
create procedure game_check_hero(in nAccountID bigint unsigned, in nHeroID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_check_hero: begin

-- 为0就不检测了
if (nHeroID=0) then
	set nRst=2;
	leave label_check_hero;
end if;

if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID) then
	set nRst=0;
	leave label_check_hero;
end if;

set nRst=1;
end label_check_hero;
end
//

-- 自动配置城防武将
-- nAccountID 账号ID
drop procedure if exists `game_config_citydefense_auto`;
create procedure game_config_citydefense_auto(in nAccountID bigint unsigned, out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select hero_id from hire_heros where account_id=nAccountID limit 5;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_config_citydefense_auto: begin

set @vIdx=1;
open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	-- <=0的都表示出错了
	set @vQuery=concat('update citydefenses set hero_', @vIdx, '=', nID, ' where account_id=', nAccountID);
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;

	set @vIdx=@vIdx+1;
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_config_citydefense_auto;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 添加道具
-- nAccountID 账号ID
-- nItemID 道具ID
-- nExcelID 道具excel id
-- nNum 数量
drop procedure if exists `game_add_item`;
create procedure game_add_item(in nAccountID bigint unsigned, in nItemID bigint unsigned, in nExcelID int, in nNum int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_item: begin

set @vItem='';

-- 是否存在
set @vType=0;
if not exists (select 1 from excel_item_list where excel_id=nExcelID and (@vType:=item_type)>0) then
	set nRst=-2;
	leave label_add_item;
end if;
-- 开始生成
set @vItemID=0;
if (@vType=1) then
	-- 装备是不能堆叠的
	if (nItemID>0) then
		-- 这里指定ID，就只能添加1个
		insert into items (item_id,account_id,excel_id,num) values (nItemID,nAccountID,nExcelID,1);
		set nRst=row_count();
		leave label_add_item;
	end if;
	while nNum>0 do
		select game_global_id() into @vItemID;
		insert into items (item_id,account_id,excel_id,num) values (@vItemID,nAccountID,nExcelID,1);
		set nRst=row_count();
		set nNum=nNum-1;

		set @vItem=concat(@vItem,',',@vItemID);
	end while;
else
	if not exists (select 1 from items where account_id=nAccountID and excel_id=nExcelID and (@vItemID:=item_id)>0) then
	if (@vItemID=0) then
		-- 镶嵌列表有同类道具么
	if not exists (select 1 from item_slots where account_id=nAccountID and excel_id=nExcelID and (@vItemID:=src_id)>0) then
		if (nItemID>0) then
			-- 这个应该是以前用过并保留下来的
			set @vItemID=nItemID;
		else
			-- 新的
			select game_global_id() into @vItemID;
		end if;
	end if;
	end if;
	end if;
	insert into items (item_id,account_id,excel_id,num) values (@vItemID,nAccountID,nExcelID,nNum) on duplicate key update num=num+nNum;
	set nRst=row_count();

	set @vItem=concat(@vItem,',',@vItemID);
end if;

end label_add_item;
if (nEcho=1) then
	select nRst;
end if;
end
//

-- 删除道具
drop procedure if exists `game_del_item`;
create procedure game_del_item(in nAccountID bigint unsigned, in nItemID bigint unsigned, in nNum int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_item: begin

set @vExcelID=0;
set @vNumJK=0;
update items set num=(@vNumJK:=(num-nNum)) where account_id=nAccountID and item_id=nItemID and (@vExcelID:=excel_id)>=0;
set nRst=row_count();
if (nRst=1) then
	if (@vNumJK=0) then
		-- 数量为0的就删掉
		delete from items where item_id=nItemID and num=0;
		set nRst=row_count();
	end if;

	-- 是奖杯吗
	if (@vExcelID=3010) then
		update common_characters set cup=@vNumJK where account_id=nAccountID;

		set @vCur=0;
		select ifnull(max(id),0) into @vCur from world_city_history;

		update world_city_log set cup=cup-nNum where id=@vCur and acct_id=nAccountID;
	end if;
end if;

end label_del_item;
if (nEcho=1) then
	select nRst;
end if;
end
//

-- 获取武将装备关联数据
drop procedure if exists `game_get_equip`;
create procedure game_get_equip(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_equip: begin

select hero_id,equip_type,excel_id,item_id from hero_items where account_id=nAccountID order by excel_id;

set nRst=0;
end label_get_equip;
end
//
-- 获取装备宝石关联数据
drop procedure if exists `game_get_gem`;
create procedure game_get_gem(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_gem: begin

select item_id,slot_idx,slot_type,excel_id,src_id from item_slots where account_id=nAccountID order by slot_type;

set nRst=0;
end label_get_gem;
end
//


-- 一键换装
drop procedure if exists `game_equip_item_all`;
create procedure game_equip_item_all(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nHeadID bigint unsigned, in nChestID bigint unsigned, in nShoeID bigint unsigned, in nWeaponID bigint unsigned, in nTrinketID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_equip_item_all: begin

set @vRst=0;
-- 逐个替换
start transaction;
call game_replace_equip(nAccountID,nHeroID,1,nHeadID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_equip_item_all; end if;
call game_replace_equip(nAccountID,nHeroID,2,nChestID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_equip_item_all; end if;
call game_replace_equip(nAccountID,nHeroID,3,nShoeID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_equip_item_all; end if;
call game_replace_equip(nAccountID,nHeroID,4,nWeaponID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_equip_item_all; end if;
call game_replace_equip(nAccountID,nHeroID,5,nTrinketID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_equip_item_all; end if;
commit;

-- 
call game_calc_heroall(nHeroID,0,1,1,@vRst);

set nRst=0;
end label_equip_item_all;
select nRst,@vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd,@vGrow,@vHealthState;
end
//
-- 替换装备
drop procedure if exists `game_replace_equip`;
create procedure game_replace_equip(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEquipType int, in nEquipID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_replace_equip: begin

-- 武将是否存在
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID and status=0) then
	set nRst=-2;
	leave label_replace_equip;
end if;

-- 先卸下旧的
set @vOldExcelID=0;
set @vOldItemID=0;
delete from hero_items where hero_id=nHeroID and account_id=nAccountID and equip_type=nEquipType and (@vOldExcelID:=excel_id)>0 and (@vOldItemID:=item_id)>0;
-- 放回包裹
set @vRst=0;
call game_add_item(nAccountID,@vOldItemID,@vOldExcelID,1,0,@vRst);
if (@vRst=0) then
	set nRst=-10;
	leave label_replace_equip;
end if;

-- 
if (nEquipID=0) then
	set nRst=1;
	call game_calc_heroall(nHeroID,0,1,1,@vRst);
	leave label_replace_equip;
end if;

-- 道具是否存在
set @vExcelID=0;
if not exists (select 1 from items where item_id=nEquipID and account_id=nAccountID and (@vExcelID:=excel_id)>0) then
	set nRst=-3;
	leave label_replace_equip;
end if;
-- 是否可装备
if not exists (select 1 from excel_item_list where excel_id=@vExcelID and equip_type=nEquipType) then
	set nRst=-4;
	leave label_replace_equip;
end if;
-- 从包裹删除
set @vRst=0;
call game_del_item(nAccountID,nEquipID,1,0,@vRst);
if (@vRst!=1) then
	set nRst=-11;
	leave label_replace_equip;
end if;
-- 再装备新的
insert into hero_items (hero_id,account_id,equip_type,excel_id,item_id) values (nHeroID,nAccountID,nEquipType,@vExcelID,nEquipID);
set nRst=row_count();

-- 
call game_calc_heroall(nHeroID,0,1,1,@vRst);

end label_replace_equip;
if (nEcho=1) then
	select nRst,@vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd,@vGrow,@vHealthState,@vArmyType,@vArmyLevel,@vArmyNum,@vExp,@vLevel,@vProf;
end if;
end
//

-- 装备道具
drop procedure if exists `game_equip_item`;
create procedure game_equip_item(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nItemID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_equip_item: begin

-- 武将是否存在
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID) then
	set nRst=-2;
	leave label_equip_item;
end if;
-- 道具是否存在
set @vExcelID=0;
if not exists (select 1 from items where item_id=nItemID and account_id=nAccountID and (@vExcelID:=excel_id)>0) then
	set nRst=-3;
	leave label_equip_item;
end if;
-- 是否可装备
set @vEquipType=0;
if not exists (select 1 from excel_item_list where excel_id=@vExcelID and item_type=1 and (@vEquipType:=equip_type)>0) then
	set nRst=-4;
	leave label_equip_item;
end if;
-- 旧装备
-- 先卸下旧的
set @vOldExcelID=0;
set @vOldItemID=0;
delete from hero_items where hero_id=nHeroID and account_id=nAccountID and equip_type=@vEquipType and (@vOldExcelID:=excel_id)>0 and (@vOldItemID:=item_id)>0;
-- 放回包裹
set @vRst=0;
call game_add_item(nAccountID,@vOldItemID,@vOldExcelID,1,0,@vRst);
if (@vRst=0) then
	set nRst=-5;
	leave label_equip_item;
end if;
-- 新装备
-- 从包裹删除
set @vRst=0;
call game_del_item(nAccountID,nItemID,1,0,@vRst);
if (@vRst!=1) then
	set nRst=-6;
	leave label_equip_item;
end if;
-- 再装备新的
insert into hero_items (hero_id,account_id,equip_type,excel_id,item_id) values (nHeroID,nAccountID,@vEquipType,@vExcelID,nItemID);
set nRst=row_count();

end label_equip_item;
select nRst;
end
//
-- 解除装备
drop procedure if exists `game_disequip_item`;
create procedure game_disequip_item(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nItemID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_disequip_item: begin

-- 装备是否存在
set @vExcelID=0;
if not exists (select 1 from hero_items where hero_id=nHeroID and account_id=nAccountID and item_id=nItemID and (@vExcelID:=excel_id)>0) then
	set nRst=-2;
	leave label_disequip_item;
end if;
-- 武将是否存在
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID) then
	set nRst=-3;
	leave label_disequip_item;
end if;

-- 先删除
delete from hero_items where hero_id=nHeroID and item_id=nItemID;
set nRst=row_count();
if (nRst=1) then
	-- 再放回包裹
	set @vRst=0;
	call game_add_item(nAccountID,nItemID,@vExcelID,1,0,@vRst);
	if (@vRst=0) then
		set nRst=-4;
		leave label_disequip_item;
	end if;
end if;

end label_disequip_item;
select nRst;
end
//

-- 一键镶嵌
drop procedure if exists `game_mount_item_all`;
create procedure game_mount_item_all(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEquipID bigint unsigned, in n1ExcelID int, in n2ExcelID int, in n3ExcelID int, in n4ExcelID int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_mount_item_all: begin

-- 先卸下以前的
set @vRst=0;
call game_unmount_item(nAccountID,nHeroID,nEquipID,1,0,@vRst);
call game_unmount_item(nAccountID,nHeroID,nEquipID,2,0,@vRst);
call game_unmount_item(nAccountID,nHeroID,nEquipID,3,0,@vRst);
call game_unmount_item(nAccountID,nHeroID,nEquipID,4,0,@vRst);

-- 再镶嵌
set @vRst=0;
start transaction;
call game_mount_item(nAccountID,nHeroID,nEquipID,1,n1ExcelID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_mount_item_all; end if;
call game_mount_item(nAccountID,nHeroID,nEquipID,2,n2ExcelID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_mount_item_all; end if;
call game_mount_item(nAccountID,nHeroID,nEquipID,3,n3ExcelID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_mount_item_all; end if;
call game_mount_item(nAccountID,nHeroID,nEquipID,4,n4ExcelID,0,@vRst);
if (@vRst!=1) then rollback; set nRst=-2; leave label_mount_item_all; end if;
commit;

-- 
call game_calc_heroall(nHeroID,0,1,1,@vRst);

set nRst=0;
end label_mount_item_all;
select nRst,@vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd,@vGrow,@vHealthState;
end
//

-- 镶嵌道具
drop procedure if exists `game_mount_item`;
create procedure game_mount_item(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEquipID bigint unsigned, in nSlotIdx int, in nExcelID int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_mount_item: begin

if (nHeroID>0) then
-- 武将是否存在
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID and status=0) then
	set nRst=-12;
	leave label_mount_item;
end if;
end if;
if (nExcelID=0) then
	set nRst=1;
	leave label_mount_item;
end if;

-- 道具是否存在
set @vExcelID=0;
if not exists (select 1 from hero_items where account_id=nAccountID and item_id=nEquipID and (@vExcelID:=excel_id)>0) then
	-- 在包裹里
	if not exists (select 1 from items where item_id=nEquipID and account_id=nAccountID and (@vExcelID:=excel_id)>0) then
		set nRst=-2;
		leave label_mount_item;
	end if;
end if;
-- 装备才能被镶嵌
set @vSlotNum=0;
if not exists (select 1 from excel_item_list where excel_id=@vExcelID and item_type=1 and (@vSlotNum:=slot_num)>0) then
	set nRst=-3;
	leave label_mount_item;
end if;
-- 道具是否存在
set @vItemID=0;
if not exists (select 1 from items where account_id=nAccountID and excel_id=nExcelID and (@vItemID:=item_id)>0) then
	set nRst=-4;
	leave label_mount_item;
end if;
-- 宝石才能镶嵌
set @vSlotType=0;
if not exists (select 1 from excel_item_list where excel_id=nExcelID and item_type=2 and (@vSlotType:=slot_type)>0) then
	set nRst=-5;
	leave label_mount_item;
end if;
-- 是否还有镶嵌插槽
set @vCurSlotNum=0;
select count(1) into @vCurSlotNum from item_slots where item_id=nEquipID;
if (@vCurSlotNum>=@vSlotNum) then
	set nRst=-6;
	leave label_mount_item;
end if;
-- 装备上是否已有同类的镶嵌了
if exists (select 1 from item_slots where item_id=nEquipID and (slot_idx=nSlotIdx or slot_type=@vSlotType)) then
	set nRst=-7;
	leave label_mount_item;
end if;

-- 先删除材料
set @vRst=0;
call game_del_item(nAccountID,@vItemID,1,0,@vRst);
if (@vRst!=1) then
	set nRst=-8;
	leave label_mount_item;
end if;
-- 再镶嵌上去
insert into item_slots (item_id,account_id,slot_idx,slot_type,excel_id,src_id) values (nEquipID,nAccountID,nSlotIdx,@vSlotType,nExcelID,@vItemID);
set nRst=row_count();

-- 
call game_calc_heroall(nHeroID,0,1,1,@vRst);

end label_mount_item;
if (nEcho=1) then
	select nRst,@vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd,@vGrow,@vHealthState,@vArmyType,@vArmyLevel,@vArmyNum,@vExp,@vLevel,@vProf;
end if;
end
//
-- 解除镶嵌
drop procedure if exists `game_unmount_item`;
create procedure game_unmount_item(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEquipID bigint unsigned, in nSlotIdx int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_unmount_item: begin

if (nHeroID>0) then
-- 武将是否存在
if not exists (select 1 from hire_heros where hero_id=nHeroID and account_id=nAccountID and status=0) then
	set nRst=-12;
	leave label_unmount_item;
end if;
end if;

-- 装备是否存在
if not exists (select 1 from hero_items where account_id=nAccountID and item_id=nEquipID) then
	-- 在包裹里
	if not exists (select 1 from items where item_id=nEquipID and account_id=nAccountID) then
		set nRst=-2;
		leave label_unmount_item;
	end if;
end if;

-- 先解除镶嵌
set @vExcelID=0;
set @vSrcID=0;
delete from item_slots where item_id=nEquipID and slot_idx=nSlotIdx and (@vExcelID:=excel_id)>0 and (@vSrcID:=src_id)>0;
if (row_count()=0) then
	set nRst=-3;
	leave label_unmount_item;
end if;

-- 再放回包裹
set @vRst=0;
call game_add_item(nAccountID,@vSrcID,@vExcelID,1,0,@vRst);
if (@vRst=0) then
	set nRst=-4;
	leave label_unmount_item;
end if;

-- 
call game_calc_heroall(nHeroID,0,1,1,@vRst);

set nRst=1;
end label_unmount_item;
if (nEcho=1) then
	select nRst,@vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd,@vGrow,@vHealthState,@vArmyType,@vArmyLevel,@vArmyNum,@vExp,@vLevel,@vProf;
end if;
end
//

-- 合成道具
drop procedure if exists `game_compos_item`;
create procedure game_compos_item(in nAccountID bigint unsigned, in nItemID int, in nNum int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_compos_item: begin

-- 是否可以合成
set @vSrc='';
if not exists (select 1 from excel_item_compos where dst_id=nItemID and (@vSrc:=src) is not null) then
	set nRst=-2;
	leave label_compos_item;
end if;
-- 材料是否准备好
set @vRst=game_check_stritemnum(nAccountID,@vSrc,nNum);
if (@vRst=-1) then
	set nRst=-3;
	leave label_compos_item;
elseif (@vRst=-2) then
	set nRst=-4;
	leave label_compos_item;
end if;

-- 先删除材料
set @vRst=0;
call game_del_stritemnum(nAccountID,@vSrc,nNum,0,@vRst);
if (@vRst!=0) then
	set nRst=-5;
	leave label_compos_item;
end if;
-- 再合成新的
set @vRst=0;
call game_add_item(nAccountID,0,nItemID,nNum,0,@vRst);
if (@vRst=0) then
	set nRst=-6;
	leave label_compos_item;
end if;

set nRst=0;
end label_compos_item;
-- @vItemID，目前只有宝石会合成，所以 @vItemID 可以表示新物品ID
select nRst,@vItemID;
end
//
-- 分解道具
drop procedure if exists `game_discompos_item`;
create procedure game_discompos_item(in nAccountID bigint unsigned, in nItemID int, in nNum int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_discompos_item: begin

-- 道具是否存在
set @vSrcID=0;
set @vSrcNum=0;
if not exists (select 1 from items where account_id=nAccountID and excel_id=nItemID and (@vSrcID:=item_id)>0 and (@vSrcNum:=num)>=nNum) then
	set nRst=-2;
	leave label_discompos_item;
end if;
-- 是否可以分解
set @vDst='';
if not exists (select 1 from excel_item_discompos where src_id=nItemID and (@vDst:=dst) is not null) then
	set nRst=-3;
	leave label_discompos_item;
end if;

-- 先删除道具
set @vRst=0;
call game_del_item(nAccountID,@vSrcID,nNum,0,@vRst);
if (@vRst!=1) then
	set nRst=-4;
	leave label_discompos_item;
end if;
-- 再添加材料
set @vRst=0;
call game_add_stritemnum(nAccountID,@vDst,nNum,0,@vRst);
if (@vRst!=0) then
	set nRst=-5;
	leave label_discompos_item;
end if;

set nRst=0;
end label_discompos_item;
select nRst;
end
//

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 获取任务，当前可做的
drop procedure if exists `game_get_quest`;
create procedure game_get_quest(in nAccountID bigint unsigned, out nRst int)
begin

declare nDone			int default 0;
declare nID				int unsigned default 0;

-- 在cursor里用到的变量只能显式declare
declare vOnlyCursor		cursor for select T1.excel_id from excel_quest_list T1 where not exists (select excel_id from only_quests where account_id=nAccountID and excel_id=T1.excel_id) and T1.isdup=0;
declare vDupCursor		cursor for select T1.excel_id from excel_quest_list T1 where T1.isdup=1;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_quest: begin

set @vQuest='';

-- 不可重复的
set nDone=0;
open vOnlyCursor;
repeat_get_quest_only:
repeat
fetch vOnlyCursor into nID;
if not nDone then
	-- 检查任务普通前提
	set @vRst=game_check_questcmn(nAccountID,nID);
	if (@vRst=-1) then
		set nRst=-2;
		leave label_get_quest;
	elseif (@vRst!=1) then
		iterate repeat_get_quest_only;
	end if;
	-- 所有条件都ok
	set @vQuest=concat(@vQuest,',',nID);
	-- 是否可完成
	set @vRst=0;
	call game_cost_quest(nAccountID,nID,0,0,@vRst);
	set @vQuest=concat(@vQuest,',',@vRst);
end if;
until nDone end repeat;
close vOnlyCursor;

-- 可重复的
set nDone=0;
open vDupCursor;
repeat_get_quest_dup:
repeat
fetch vDupCursor into nID;
if not nDone then
	-- 检查任务普通前提
	set @vRst=game_check_questcmn(nAccountID,nID);
	if (@vRst=-1) then
		set nRst=-3;
		leave label_get_quest;
	elseif (@vRst!=1) then
		iterate repeat_get_quest_dup;
	end if;
	-- 检查任务重复前提
	set @vRst=game_check_questdup(nAccountID,nID);
	set nDone=0;
	if (@vRst!=1) then
		iterate repeat_get_quest_dup;
	end if;
	-- 所有条件都ok
	set @vQuest=concat(@vQuest,',',nID);
	-- 是否可完成
	set @vRst=0;
	call game_cost_quest(nAccountID,nID,0,0,@vRst);
	set @vQuest=concat(@vQuest,',',@vRst);
end if;
until nDone end repeat;
close vDupCursor;

set @vDoneQuest='';
select group_concat(excel_id,',',2) into @vDoneQuest from only_quests where account_id=nAccountID;
if (@vDoneQuest is not null) then
set @vQuest=concat(@vQuest,',',@vDoneQuest);
end if;

set nRst=0;
end label_get_quest;
select @vQuest;
end
//

-- 检查玩家接受某个任务的普通前提
drop function if exists `game_check_questcmn`;
create function game_check_questcmn(nAccountID bigint unsigned, nQuestID int) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vLevel=0;
set @vVIP=0;
set @vDevelop=0;
set @vPopulation=0;
set @vSoldier=0;
set @vHero=0;
set @vBuild='';
set @vItem='';
set @vPVPAttack=0;
set @vPVPAttackWin=0;
set @vPVPDefense=0;
set @vPVPDefenseWin=0;
set @vWangZhe=0;
set @vZhengZhan1=0;
set @vZhengZhan2=0;
set @vZhengZhan3=0;
set @vZhengZhan4=0;
set @vZhengZhan5=0;
set @vZhengZhan6=0;
set @vZhengZhan7=0;
set @vZhengZhan8=0;
set @vZhengZhan9=0;
set @vZhengZhan10=0;
set @vZhengZhan11=0;
set @vZhengZhan12=0;
set @vZhengZhan13=0;
set @vZhengZhan14=0;
set @vZhengZhan15=0;
set @vZhengZhan16=0;
set @vZhengZhan17=0;
set @vZhengZhan18=0;
set @vZhengZhan19=0;
set @vZhengZhan20=0;
set @vOnlyVIP=0;
select pre_level,pre_vip,pre_develop,pre_population,pre_soldier,pre_hero,pre_build,pre_item,pvp_attack,pvp_attack_win,pvp_defense,pvp_defense_win,wangzhe
	,zhengzhan_1,zhengzhan_2,zhengzhan_3,zhengzhan_4,zhengzhan_5,zhengzhan_6,zhengzhan_7,zhengzhan_8,zhengzhan_9,zhengzhan_10
	,zhengzhan_11,zhengzhan_12,zhengzhan_13,zhengzhan_14,zhengzhan_15,zhengzhan_16,zhengzhan_17,zhengzhan_18,zhengzhan_19,zhengzhan_20
	,vip 
	into @vLevel,@vVIP,@vDevelop,@vPopulation,@vSoldier,@vHero,@vBuild,@vItem,@vPVPAttack,@vPVPAttackWin,@vPVPDefense,@vPVPDefenseWin,@vWangZhe
	,@vZhengZhan1,@vZhengZhan2,@vZhengZhan3,@vZhengZhan4,@vZhengZhan5,@vZhengZhan6,@vZhengZhan7,@vZhengZhan8,@vZhengZhan9,@vZhengZhan10
	,@vZhengZhan11,@vZhengZhan12,@vZhengZhan13,@vZhengZhan14,@vZhengZhan15,@vZhengZhan16,@vZhengZhan17,@vZhengZhan18,@vZhengZhan19,@vZhengZhan20
	,@vOnlyVIP from excel_quest_list where excel_id=nQuestID;

-- 更新每日pvp次数
update common_characters set pvp_attack_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),0,pvp_attack_win_day),
							pvp_defense_win_day=if(to_days(date(from_unixtime(pvp_done_tick)))!=to_days(date(now())),0,pvp_defense_win_day),
							pvp_done_tick=unix_timestamp() where account_id=nAccountID;

-- 等级，VIP，发展度，人口
set @vCurVIP=0;
if not exists (select 1 from common_characters where account_id=nAccountID and 
	level>=@vLevel and 
	(@vCurVIP:=vip)>=@vVIP and 
	development>=@vDevelop and 
	population>=@vPopulation and 
	pvp_attack>=@vPVPAttack and 
	pvp_attack_win>=@vPVPAttackWin and 
	pvp_defense>=@vPVPDefense and 
	pvp_defense_win>=@vPVPDefenseWin and 
	instance_wangzhe>=@vWangZhe and 
	zhengzhan_1>=@vZhengZhan1 and 
	zhengzhan_2>=@vZhengZhan2 and 
	zhengzhan_3>=@vZhengZhan3 and 
	zhengzhan_4>=@vZhengZhan4 and 
	zhengzhan_5>=@vZhengZhan5 and 
	zhengzhan_6>=@vZhengZhan6 and 
	zhengzhan_7>=@vZhengZhan7 and 
	zhengzhan_8>=@vZhengZhan8 and 
	zhengzhan_9>=@vZhengZhan9 and 
	zhengzhan_10>=@vZhengZhan10 and
	zhengzhan_11>=@vZhengZhan11 and 
	zhengzhan_12>=@vZhengZhan12 and 
	zhengzhan_13>=@vZhengZhan13 and 
	zhengzhan_14>=@vZhengZhan14 and 
	zhengzhan_15>=@vZhengZhan15 and 
	zhengzhan_16>=@vZhengZhan16 and 
	zhengzhan_17>=@vZhengZhan17 and 
	zhengzhan_18>=@vZhengZhan18 and 
	zhengzhan_19>=@vZhengZhan19 and 
	zhengzhan_20>=@vZhengZhan20
	) then 
	return 0;
end if;
-- VIP专属
if (@vOnlyVIP>0) then
if (@vCurVIP!=@vOnlyVIP) then
	return 0;
end if;
end if;
-- 士兵数量
set @vCurSoldier=0;
if (@vSoldier>0) then
select sum(num) into @vCurSoldier from soldiers where account_id=nAccountID;
if ((@vCurSoldier is null) or (@vCurSoldier<@vSoldier)) then
	return 0;
end if;
end if;
-- 武将数量
set @vCurHero=0;
if (@vHero>0) then
select count(1) into @vCurHero from hire_heros where account_id=nAccountID;
if ((@vCurHero is null) or (@vCurHero<@vHero)) then
	return 0;
end if;
end if;
-- 建筑
set @vRst=game_check_strbuildlevel(nAccountID,@vBuild);
if (@vRst=-1) then
	return -1;
elseif (@vRst=-2) then
	return 0;
end if;
-- 道具
set @vRst=game_check_stritemnum(nAccountID,@vItem,1);
if (@vRst=-1) then
	return -1;
elseif (@vRst=-2) then
	return 0;
end if;

return 1;
end
//
-- 检查玩家接受某个任务的重复前提
drop function if exists `game_check_questdup`;
create function game_check_questdup(nAccountID bigint unsigned, nQuestID int) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vYear='';
set @vMonth='';
set @vDay='';
set @vWeekday='';
set @vHour='';
set @vDaytimes=0;
set @vUseTime=0;
select open_year,open_month,open_day,open_weekday,open_hour,day_times,use_time into @vYear,@vMonth,@vDay,@vWeekday,@vHour,@vDaytimes,@vUseTime from excel_quest_list where excel_id=nQuestID;

set @vDoneTick=0;
set @vDoneDayTimes=0;
select done_tick,day_times into @vDoneTick,@vDoneDayTimes from dup_quests where account_id=nAccountID and excel_id=nQuestID;
-- 上次执行是否还未完成
if ((@vDoneTick+@vUseTime)>unix_timestamp()) then
	return 0;
end if;
-- 是否在开放时段
if (game_check_tick(@vYear,@vMonth,@vDay,@vWeekday,@vHour)<0) then
	return 0;
end if;
-- 重置之前是否还有剩余次数
if (to_days(date(from_unixtime(@vDoneTick)))=to_days(date(now()))) then
	if (@vDoneDayTimes>=@vDayTimes) then
		return 0;
	end if;
end if;

return 1;
end
//
-- 任务消耗
drop procedure if exists `game_cost_quest`;
create procedure game_cost_quest(in nAccountID bigint unsigned, in nQuestID int, in nPayNow int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_cost_quest: begin

set @vCostGold=0;
set @vCostCrystal=0;
set @vCostDiamond=0;
set @vCostItem='';
set @vPVPAttackWinDay=0;
set @vPVPDefenseWinDay=0;
select cost_gold,cost_crystal,cost_diamond,cost_item,pvp_attack_win_day,pvp_defense_win_day into @vCostGold,@vCostCrystal,@vCostDiamond,@vCostItem,@vPVPAttackWinDay,@vPVPDefenseWinDay from excel_quest_list where excel_id=nQuestID;

-- 金币，水晶，钻石，pvp
if not exists (select 1 from common_characters where account_id=nAccountID and 
	gold>=@vCostGold and 
	crystal>=@vCostCrystal and 
	diamond>=@vCostDiamond and 
	pvp_attack_win_day>=@vPVPAttackWinDay and 
	pvp_defense_win_day>=@vPVPDefenseWinDay
	) then 
	set nRst=0;
	leave label_cost_quest;
end if;
-- 道具
set @vRst=game_check_stritemnum(nAccountID,@vCostItem,1);
if (@vRst=-1) then
	set nRst=-2;
	leave label_cost_quest;
elseif (@vRst=-2) then
	set nRst=0;
	leave label_cost_quest;
end if;

-- 现在支付吗
if (nPayNow=1) then
	-- 金币，水晶，钻石
	if (@vCostGold>0 or @vCostCrystal>0 or @vCostDiamond>0) then
	update common_characters set gold=gold-@vCostGold,crystal=crystal-@vCostCrystal,diamond=diamond-@vCostDiamond where account_id=nAccountID and gold>=@vCostGold and crystal>=@vCostCrystal and diamond>=@vCostDiamond;
	if (row_count()=0) then
		set nRst=-4;
		leave label_cost_quest;
	end if;
	end if;

	-- 道具
	set @vRst=0;
	call game_del_stritemnum(nAccountID,@vCostItem,1,0,@vRst);
	if (@vRst!=0) then
		set nRst=-5;
		leave label_cost_quest;
	end if;
end if;

set nRst=1;
end label_cost_quest;
if (nEcho=1) then
	select nRst;
end if;
end
//

-- 计算字符出现次数
drop function if exists `game_wc`;
create function game_wc(szStr blob, szTag char) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vWC=0;
set @vSize=length(szStr);
while @vSize>0 do
	if (strcmp(substring(szStr,@vSize,1),szTag)=0) then
		set @vWC=@vWC+1;
	end if;
	set @vSize=@vSize-1;
end while;
return @vWC;
end
//

-- 依次检查字符串中的建筑是否达到指定等级
drop function if exists `game_check_strbuildlevel`;
create function game_check_strbuildlevel(nAccountID bigint unsigned, szStr blob) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

-- 为空就返回成功
if (length(szStr)=0) then
	return 0;
end if;

-- 字符串是否合法
select game_wc(szStr,'*') into @vWC;
if (mod(@vWC,2)!=0) then
	return -1;
end if;

set @vN=1;
while @vN<=@vWC do
	select substring_index(substring_index(szStr,'*',@vN),'*',-1) into @vBuildID;
	select substring_index(substring_index(szStr,'*',@vN+1),'*',-1) into @vLevel;
	select substring_index(substring_index(szStr,'*',@vN+2),'*',-1) into @vNum;
	if (game_check_buildlevel(nAccountID,@vBuildID,@vLevel,@vNum)=0) then
		-- 条件不满足
		return -2;
	end if;
	set @vN=@vN+2;
end while;

return 0;
end
//
-- 检查玩家某个建筑是否达到指定等级
drop function if exists `game_check_buildlevel`;
create function game_check_buildlevel(nAccountID bigint unsigned, nBuildID int, nLevel int, nNum int) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vNum=0;
select count(1) into @vNum from buildings where account_id=nAccountID and excel_id=nBuildID and level>=nLevel;
if (@vNum>=nNum) then
	return 1;
else
	return 0;
end if;
end
//

-- 依次检查字符串中的道具是否达到指定数量
drop function if exists `game_check_stritemnum`;
create function game_check_stritemnum(nAccountID bigint unsigned, szStr blob, nMulti int) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

-- 为空就返回成功
if (length(szStr)=0) then
	return 0;
end if;

-- 字符串是否合法
select game_wc(szStr,'*') into @vWC;
if (mod(@vWC,2)=0) then
	return -1;
end if;

set @vN=1;
while @vN<=@vWC do
	select substring_index(substring_index(szStr,'*',@vN),'*',-1) into @vItemID;
	select substring_index(substring_index(szStr,'*',@vN+1),'*',-1) into @vNum;
	if (game_check_itemnum(nAccountID,@vItemID,@vNum*nMulti)=0) then
		-- 条件不满足
		return -2;
	end if;
	set @vN=@vN+2;
end while;

return 0;
end
//
-- function 可以调用 function，但不能调用 procedure -_-!!
-- 依次添加字符串中的道具
drop procedure if exists `game_add_stritemnum`;
create procedure game_add_stritemnum(in nAccountID bigint unsigned, in szStr blob, in nMulti int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

label_add_stritemnum: begin

set @vMyAddItem='';

select game_wc(szStr,'*') into @vWC;
if (mod(@vWC,2)=0) then
	set nRst=-1;
	leave label_add_stritemnum;
end if;

set @vN=1;
while @vN<=@vWC do
	select substring_index(substring_index(szStr,'*',@vN),'*',-1) into @vMyExcelID;
	select substring_index(substring_index(szStr,'*',@vN+1),'*',-1) into @vNum;
	set @vRst=0;
	call game_add_item(nAccountID,0,@vMyExcelID,@vNum*nMulti,0,@vRst);
	if (@vRst=0) then
		-- 添加失败
		set nRst=-2;
		leave label_add_stritemnum;
	end if;

	-- 记录 nItemID,nExcelID
	set @vMyAddItem=if(length(@vMyAddItem)=0,concat_ws('*',@vItemID,@vMyExcelID,@vNum),concat_ws('*',@vMyAddItem,@vItemID,@vMyExcelID,@vNum));

	set @vN=@vN+2;
end while;

set nRst=0;
end label_add_stritemnum;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 依次删除字符串中的道具
drop procedure if exists `game_del_stritemnum`;
create procedure game_del_stritemnum(in nAccountID bigint unsigned, in szStr blob, in nMulti int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_stritemnum: begin

-- 为空就返回成功
if (length(szStr)=0) then
	set nRst=0;
	leave label_del_stritemnum;
end if;

select game_wc(szStr,'*') into @vWC;
if (mod(@vWC,2)=0) then
	set nRst=-1;
	leave label_del_stritemnum;
end if;

start transaction;
set @vN=1;
while @vN<=@vWC do
	select substring_index(substring_index(szStr,'*',@vN),'*',-1) into @vExcelID;
	select substring_index(substring_index(szStr,'*',@vN+1),'*',-1) into @vNum;
	
	set @vItemType=0;
	select item_type into @vItemType from excel_item_list where excel_id=@vExcelID;
	if (@vItemType=1) then
		-- 装备
		set @vEquipNum=1;
		while @vEquipNum<=@vNum*nMulti do
			set @vItemID=0;
			select item_id into @vItemID from items where account_id=nAccountID and excel_id=@vExcelID limit 1;
			set @vRst=0;
			call game_del_item(nAccountID,@vItemID,1,0,@vRst);
			if (@vRst!=1) then
				-- 删除失败
				set nRst=-2;
				rollback;
				leave label_del_stritemnum;
			end if;

			set @vEquipNum=@vEquipNum+1;
		end while;
	else
		-- 其他
		set @vItemID=0;
		select item_id into @vItemID from items where account_id=nAccountID and excel_id=@vExcelID limit 1;

		set @vRst=0;
		call game_del_item(nAccountID,@vItemID,@vNum*nMulti,0,@vRst);
		if (@vRst!=1) then
			-- 删除失败
			set nRst=-2;
			rollback;
			leave label_del_stritemnum;
		end if;
	end if;
	
	set @vN=@vN+2;
end while;
commit;

set nRst=0;
end label_del_stritemnum;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 检查玩家某个道具是否达到指定数量
drop function if exists `game_check_itemnum`;
create function game_check_itemnum(nAccountID bigint unsigned, nItemID int, nNum int) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vCurNum=0;
select sum(num) into @vCurNum from items where account_id=nAccountID and excel_id=nItemID;
if (@vCurNum>=nNum) then
	return 1;
else
	return 0;
end if;
end
//

-- 检查当前时刻是否在指定时段内
drop function if exists `game_check_tick`;
create function game_check_tick(szYear char(16), szMonth char(8), szDay char(8), szWeekday char(8), szHour char(8)) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set @vNow=now();
-- 年
if (length(szYear)>0) then
	set @vYear=extract(year from @vNow);
	call game_split_2(szYear,'*',@vBegin,@vEnd);
	if (@vBegin>0) and (@vBegin>@vYear) then
		return -1;
	end if;
	if (@vEnd>0) and (@vEnd<@vYear) then
		return -2;
	end if;
end if;
-- 月
if (length(szMonth)>0) then
	set @vMonth=extract(month from @vNow);
	call game_split_2(szMonth,'*',@vBegin,@vEnd);
	if (@vBegin>0) and (@vBegin>@vMonth) then
		return -3;
	end if;
	if (@vEnd>0) and (@vEnd<@vMonth) then
		return -4;
	end if;
end if;
-- 日
if (length(szDay)>0) then
	set @vDay=extract(day from @vNow);
	call game_split_2(szDay,'*',@vBegin,@vEnd);
	if (@vBegin>0) and (@vBegin>@vDay) then
		return -5;
	end if;
	if (@vEnd>0) and (@vEnd<@vDay) then
		return -6;
	end if;
end if;
-- 周天
if (length(szWeekday)>0) then
	set @vWeekday=dayofweek(@vNow);
	call game_split_2(szWeekday,'*',@vBegin,@vEnd);
	if (@vBegin>0) and (@vBegin>@vWeekday) then
		return -7;
	end if;
	if (@vEnd>0) and (@vEnd<@vWeekday) then
		return -8;
	end if;
end if;
-- 小时
if (length(szHour)>0) then
	set @vHour=extract(hour from @vNow);
	call game_split_2(szHour,'*',@vBegin,@vEnd);
	if (@vBegin>0) and (@vBegin>@vHour) then
		return -9;
	end if;
	if (@vEnd>0) and (@vEnd<@vHour) then
		return -10;
	end if;
end if;

return 0;
end
//

-- 拆分2段字符串
drop procedure if exists `game_split_2`;
create procedure game_split_2(in szSrc blob, in szTag char, out sz1st blob, out sz2nd blob)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

label_split_2: begin

set sz1st='';
set sz2nd='';

set @vWC=0;
select game_wc(szSrc,szTag) into @vWC;
if (@vWC!=1) then
	leave label_split_2;
end if;

select substring_index(substring_index(szSrc,szTag,1),szTag,-1) into sz1st;
select substring_index(substring_index(szSrc,szTag,2),szTag,-1) into sz2nd;

end label_split_2;
end
//
-- 拆分3段字符串
drop procedure if exists `game_split_3`;
create procedure game_split_3(in szSrc blob, in szTag char, out sz1st blob, out sz2nd blob, out sz3rd blob)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

label_split_3: begin

set sz1st='';
set sz2nd='';
set sz3rd='';

set @vWC=0;
select game_wc(szSrc,szTag) into @vWC;
if (@vWC!=2) then
	leave label_split_3;
end if;

select substring_index(substring_index(szSrc,szTag,1),szTag,-1) into sz1st;
select substring_index(substring_index(szSrc,szTag,2),szTag,-1) into sz2nd;
select substring_index(substring_index(szSrc,szTag,3),szTag,-1) into sz3rd;

end label_split_3;
end
//
-- 拆分4段字符串
drop procedure if exists `game_split_4`;
create procedure game_split_4(in szSrc blob, in szTag char, out sz1st blob, out sz2nd blob, out sz3rd blob, out sz4th blob)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

label_split_4: begin

set sz1st='';
set sz2nd='';
set sz3rd='';
set sz4th='';

set @vWC=0;
select game_wc(szSrc,szTag) into @vWC;
if (@vWC!=3) then
	leave label_split_4;
end if;

select substring_index(substring_index(szSrc,szTag,1),szTag,-1) into sz1st;
select substring_index(substring_index(szSrc,szTag,2),szTag,-1) into sz2nd;
select substring_index(substring_index(szSrc,szTag,3),szTag,-1) into sz3rd;
select substring_index(substring_index(szSrc,szTag,4),szTag,-1) into sz4th;

end label_split_4;
end
//
-- 拆分5段字符串
drop procedure if exists `game_split_5`;
create procedure game_split_5(in szSrc blob, in szTag char, out sz1st blob, out sz2nd blob, out sz3rd blob, out sz4th blob, out sz5th blob)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

label_split_5: begin

set sz1st='';
set sz2nd='';
set sz3rd='';
set sz4th='';
set sz5th='';

set @vWC=0;
select game_wc(szSrc,szTag) into @vWC;
-- if (@vWC!=4) then
--	leave label_split_5;
-- end if;

if (@vWC>=0) then
select substring_index(substring_index(szSrc,szTag,1),szTag,-1) into sz1st;
end if;
if (@vWC>=1) then
select substring_index(substring_index(szSrc,szTag,2),szTag,-1) into sz2nd;
end if;
if (@vWC>=2) then
select substring_index(substring_index(szSrc,szTag,3),szTag,-1) into sz3rd;
end if;
if (@vWC>=3) then
select substring_index(substring_index(szSrc,szTag,4),szTag,-1) into sz4th;
end if;
if (@vWC>=4) then
select substring_index(substring_index(szSrc,szTag,5),szTag,-1) into sz5th;
end if;

end label_split_5;
end
//

-- 完成任务
drop procedure if exists `game_done_quest`;
create procedure game_done_quest(in nAccountID bigint unsigned, in nQuestID int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_done_quest: begin

set @vIsDup=0;
if not exists (select 1 from excel_quest_list where excel_id=nQuestID and (@vIsDup:=isdup)>=0) then
	set nRst=-20;
	leave label_done_quest;
end if;
if (@vIsDup=0) then
	-- 已经做过了吗
	if exists (select 1 from only_quests where account_id=nAccountID and excel_id=nQuestID) then
		set nRst=-2;
		leave label_done_quest;
	end if;
end if;
-- 普通任务前提满足吗
set @vRst=game_check_questcmn(nAccountID,nQuestID);
if (@vRst=-1) then
	set nRst=-3;
	leave label_done_quest;
elseif (@vRst!=1) then
	set nRst=-4;
	leave label_done_quest;
end if;
-- 重复的呢
if (@vIsDup=1) then
	set @vRst=game_check_questdup(nAccountID,nQuestID);
	if (@vRst!=1) then
		set nRst=-5;
		leave label_done_quest;
	end if;
end if;

-- 任务消耗
call game_cost_quest(nAccountID,nQuestID,1,0,@vRst);
if (@vRst!=1) then
	set nRst=-10;
	leave label_done_quest;
end if;
-- 完成标记
if (@vIsDup=0) then
	-- 不可重复的
	insert into only_quests (account_id,excel_id,done_tick) values (nAccountID,nQuestID,unix_timestamp());
else
	-- 可重复的
	insert into dup_quests (account_id,excel_id,done_tick,day_times) values (nAccountID,nQuestID,unix_timestamp(),1) on duplicate key update day_times=day_times+1;
	-- 是否需要重置计数
	update dup_quests set day_times=if(to_days(date(from_unixtime(done_tick)))!=to_days(date(now())),1,day_times),done_tick=unix_timestamp() where account_id=nAccountID and excel_id=nQuestID;
end if;

-- 奖励
set @vGold=0;
set @vCrystal=0;
set @vDiamond=0;
set @vExp=0;
set @vItem='';
set @vGoldBonus=0;
select awa_gold,awa_crystal,awa_diamond,awa_exp,awa_item,gold_bonus into @vGold,@vCrystal,@vDiamond,@vExp,@vItem,@vGoldBonus from excel_quest_list where excel_id=nQuestID;
set @vLevel=0;
select level into @vLevel from common_characters where account_id=nAccountID;
-- 金币加成
set @vGoldBonus=pow(1+@vGoldBonus,@vLevel-1);
-- 金币，水晶，钻石
update common_characters set gold=gold+floor(@vGold*@vGoldBonus),crystal=crystal+@vCrystal,diamond=diamond+@vDiamond where account_id=nAccountID;
-- 经验
call add_char_exp(nAccountID,@vExp,@vCurExp,@vCurLevel);

-- 道具
set @vMyAddItem='';
if (length(@vItem)>0) then
	set @vRst=0;
	call game_add_stritemnum(nAccountID,@vItem,1,0,@vRst);
	if (@vRst!=0) then
		set nRst=-6;
		leave label_done_quest;
	end if;
end if;


set nRst=0;
end label_done_quest;
select nRst,@vMyAddItem;
end
//

-- 获取指定副本的所有实例摘要
drop procedure if exists `game_get_instancedesc`;
create procedure game_get_instancedesc(in nAccountID bigint unsigned, in nExcelID int, in nClassID int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;

select T1.instance_id,T2.name,T1.cur_level,T2.level,T1.cur_player,T1.creator_id from instance T1 left join common_characters T2 on T1.creator_id=T2.account_id where T1.excel_id=nExcelID and T1.class_id=nClassID and T1.cur_status=0;

set nRst=0;
-- select nRst;
end
//

-- 刷新玩家所有副本的状态
drop procedure if exists `game_flush_allinstance`;
create procedure game_flush_allinstance(in nAccountID bigint unsigned, out nRst int)
begin

declare nDone			int default 0;
declare nID				int unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select excel_id from instance_player where account_id=nAccountID;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_flush_allinstance: begin

open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	call game_flush_instance(nAccountID,nID,@vRst);
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_flush_allinstance;
-- select nRst;
end
//
-- 刷新指定副本状态
drop procedure if exists `game_flush_instance`;
create procedure game_flush_instance(in nAccountID bigint unsigned, in nExcelID int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;

update instance_player set done_tick=unix_timestamp(),day_times_free=0,day_times_fee=0 where account_id=nAccountID and excel_id=nExcelID and to_days(date(from_unixtime(done_tick)))!=to_days(date(now()));

set nRst=0;
-- select nRst;
end
//

-- 获取玩家副本状态，比如每日次数
drop procedure if exists `game_get_instancestatus`;
create procedure game_get_instancestatus(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_instancestatus: begin

-- 先刷新一下
set @vRst=0;
call game_flush_allinstance(nAccountID,@vRst);
if (@vRst!=0) then
	set nRst=-2;
	leave label_get_instancestatus;
end if;

-- vip特权
set @vVIP=0;
select vip into @vVIP from common_characters where account_id=nAccountID;
set @vDaytimesfree_1=0;
set @vDaytimesfee_1=0;
select instance_day_times_free,instance_day_times_fee into @vDaytimesfree_1,@vDaytimesfee_1 from excel_vip_right where level=@vVIP;

select T1.excel_id,ifnull(T2.day_times_free,0),T1.day_times_free+@vDaytimesfree_1,ifnull(T2.day_times_fee,0),T1.day_times_fee+@vDaytimesfee_1,T2.instance_id,ifnull(T3.cur_status,0),ifnull(T3.class_id,0),ifnull(T3.creator_id,0),ifnull(T3.cur_level,0) from excel_instance_list T1 left join instance_player T2 on T2.excel_id=T1.excel_id and T2.account_id=nAccountID left join instance T3 on T3.instance_id=T2.instance_id;

set nRst=0;
end label_get_instancestatus;
end
//

-- 创建副本
drop procedure if exists `game_create_instance`;
create procedure game_create_instance(in nAccountID bigint unsigned, in nExcelID int, in nClassID int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_create_instance: begin

-- 先刷新一下
set @vRst=0;
call game_flush_instance(nAccountID,nExcelID,@vRst);
-- 当前能否进入这个副本
call game_check_playerinstance(nAccountID,nExcelID,nClassID,0,0,nRst);
if (nRst!=1) then
	leave label_create_instance;
end if;

select game_global_id() into @vInstanceID;
insert into instance (instance_id,excel_id,creator_id,class_id,cur_player,cur_level,loot_list,auto_combat,auto_supply,retry_times,create_time) values (@vInstanceID,nExcelID,nAccountID,nClassID,1,1,'',0,0,@vRetryTimes,unix_timestamp());


if (nExcelID=101) then

-- 多人副本，在开打前都不算真正进入
if (@vDonedaytimesfree<@vDaytimesfree) then
	insert into instance_player (account_id,excel_id,instance_id,hero_id,loot_list,done_tick,day_times_free,day_times_fee,day_times_free_buffer,day_times_fee_buffer,join_time) values (nAccountID,nExcelID,@vInstanceID,'','',unix_timestamp(),0,0,1,0,unix_timestamp()) on duplicate key update instance_id=@vInstanceID,hero_id='',day_times_free_buffer=1,join_time=unix_timestamp();
end if;
	update instance_player set instance_id=@vInstanceID where account_id=nAccountID and excel_id=nExcelID;
else

-- 增加免费次数或者收费次数
if (@vDonedaytimesfree<@vDaytimesfree) then
	insert into instance_player (account_id,excel_id,instance_id,hero_id,loot_list,done_tick,day_times_free,day_times_fee,join_time) values (nAccountID,nExcelID,@vInstanceID,'','',unix_timestamp(),1,0,unix_timestamp()) on duplicate key update instance_id=@vInstanceID,hero_id='',day_times_free=day_times_free+1,join_time=unix_timestamp();
else
	update instance_player set instance_id=@vInstanceID,hero_id='',day_times_fee=day_times_fee+1,join_time=unix_timestamp() where account_id=nAccountID and excel_id=nExcelID;
end if;

end if;

set nRst=0;
end label_create_instance;
select nRst,@vInstanceID;
end
//
-- 加入副本
drop procedure if exists `game_join_instance`;
create procedure game_join_instance(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_join_instance: begin

-- 先刷新一下
set @vRst=0;
call game_flush_instance(nAccountID,nExcelID,@vRst);
-- 副本存在吗
set @vClassID=0;
if not exists (select 1 from instance where instance_id=nInstanceID and (@vClassID:=class_id)>=0) then
	set nRst=-16;
	leave label_join_instance;
end if;
-- 当前能否进入这个副本
call game_check_playerinstance(nAccountID,nExcelID,@vClassID,nInstanceID,0,nRst);
if (nRst!=1) then
	leave label_join_instance;
end if;

update instance set cur_player=cur_player+1 where instance_id=nInstanceID and excel_id=nExcelID and cur_player<@vPlayernum;
if (row_count()=0) then
	-- 满员了，跟game_check_playerinstance一样返回 -8
	set nRst=-8;
	leave label_join_instance;
end if;


if (nExcelID=101) then

-- 多人副本，在开打前都不算真正进入
if (@vDonedaytimesfree<@vDaytimesfree) then
	insert into instance_player (account_id,excel_id,instance_id,hero_id,loot_list,done_tick,day_times_free,day_times_fee,day_times_free_buffer,day_times_fee_buffer,join_time) values (nAccountID,nExcelID,nInstanceID,'','',unix_timestamp(),0,0,1,0,unix_timestamp()) on duplicate key update instance_id=nInstanceID,hero_id='',day_times_free_buffer=1,join_time=unix_timestamp();
end if;
	update instance_player set instance_id=nInstanceID where account_id=nAccountID and excel_id=nExcelID;
else

-- 增加免费次数或者收费次数
if (@vDonedaytimesfree<@vDaytimesfree) then
	insert into instance_player (account_id,excel_id,instance_id,hero_id,loot_list,done_tick,day_times_free,day_times_fee,join_time) values (nAccountID,nExcelID,nInstanceID,'','',unix_timestamp(),1,0,unix_timestamp()) on duplicate key update instance_id=nInstanceID,hero_id='',day_times_free=day_times_free+1,join_time=unix_timestamp();
else
	update instance_player set instance_id=nInstanceID,hero_id='',day_times_fee=day_times_fee+1,join_time=unix_timestamp() where account_id=nAccountID and excel_id=nExcelID;
end if;

end if;

set nRst=0;
end label_join_instance;
select nRst;
end
//
-- 副本就绪
drop procedure if exists `game_prepare_instance`;
create procedure game_prepare_instance(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, in nVal int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_prepare_instance: begin

-- 是多人副本吗
if (nExcelID!=101) then
	set nRst=-2;
	leave label_prepare_instance;
end if;
-- 必须配置武将
if not exists (select 1 from instance_player where instance_id=nInstanceID and account_id=nAccountID and length(hero_id)>0) then
	set nRst=-3;
	leave label_prepare_instance;
end if;

set @vStatus=nVal;
case nVal
when 0 then
	update instance_player set status=0 where instance_id=nInstanceID and account_id=nAccountID;
when 1 then
	update instance_player set status=1 where instance_id=nInstanceID and account_id=nAccountID;
else
	update instance_player set status=(@vStatus:=if(status=0,1,0)) where instance_id=nInstanceID and account_id=nAccountID;
end case;

set nRst=0;
end label_prepare_instance;
if (nEcho=1) then
	select nRst,@vStatus;
end if;
end
//
-- 副本开始
drop procedure if exists `game_start_instance`;
create procedure game_start_instance(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_start_instance: begin

-- 是多人副本吗
if (nExcelID!=101) then
	set nRst=-2;
	leave label_start_instance;
end if;
-- 是队长吗
if not exists (select 1 from instance where instance_id=nInstanceID and creator_id=nAccountID) then
	set nRst=-3;
	leave label_start_instance;
end if;
-- 都准备就绪了吗
set @vID='';
select group_concat(account_id) into @vID from instance_player where instance_id=nInstanceID and status=0;
if (length(@vID)>0) then
	set nRst=-4;
	leave label_start_instance;
end if;
-- 
select group_concat(account_id) into @vID from instance_player where instance_id=nInstanceID and status=1;
if (length(@vID)=0) then
	set nRst=-5;
	leave label_start_instance;
end if;

-- 修改状态
update instance set cur_status=1 where instance_id=nInstanceID and cur_status=0;
if (row_count()=0) then
	set nRst=-6;
	leave label_start_instance;
end if;
-- 增加进入次数
update instance_player set day_times_free=if(day_times_free_buffer>0,day_times_free+1,day_times_free),day_times_free_buffer=0,day_times_fee_buffer=0 where instance_id=nInstanceID;

set nRst=0;
end label_start_instance;
select nRst,@vID;
end
//
-- 踢人
drop procedure if exists `game_kick_instance`;
create procedure game_kick_instance(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, in nObjID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_kick_instance: begin

-- 是多人副本吗
if (nExcelID!=101) then
	set nRst=-2;
	leave label_kick_instance;
end if;
-- 是队长吗
if not exists (select 1 from instance where instance_id=nInstanceID and creator_id=nAccountID) then
	set nRst=-3;
	leave label_kick_instance;
end if;

call game_quit_instance(nObjID,nExcelID,nInstanceID,0,0,@vRst);

set nRst=0;
end label_kick_instance;
select nRst;
end
//
-- 副本补给
drop procedure if exists `game_supply_instance`;
create procedure game_supply_instance(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
declare szHero			blob;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select account_id,hero_id from instance_player where instance_id=nInstanceID;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_supply_instance: begin

-- 是多人副本吗
if (nExcelID!=101) then
	set nRst=-2;
	leave label_supply_instance;
end if;
-- 是队长吗
set @vStatus=0;
if not exists (select 1 from instance where instance_id=nInstanceID and creator_id=nAccountID and (@vStatus:=cur_status)>=0) then
	set nRst=-3;
	leave label_supply_instance;
end if;

set @vDrugStr='';

open vCursor;
repeat
fetch vCursor into nID,szHero;
if not nDone then
	call game_supply_strhero(nID,szHero,0,@vRst);
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_supply_instance;
select nRst,@vStatus,@vDrugStr;
end
//

-- 检查玩家是否可以创建或加入某个副本
drop procedure if exists `game_check_playerinstance`;
create procedure game_check_playerinstance(in nAccountID bigint unsigned, in nExcelID int, in nClassID int, in nInstanceID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_check_playerinstance: begin

set @vInstanceID=0;
set @vDoneTick=0;
set @vDoneDaytimesfree=0;
set @vDoneDaytimesfee=0;
select instance_id,done_tick,day_times_free,day_times_fee into @vInstanceID,@vDoneTick,@vDoneDaytimesfree,@vDoneDaytimesfee from instance_player where account_id=nAccountID and excel_id=nExcelID;
-- 上一次退出了吗
if (@vInstanceID>0) then
	set nRst=-2;
	leave label_check_playerinstance;
end if;

set @vYear='';
set @vMonth='';
set @vDay='';
set @vWeekday='';
set @vHour='';
set @vDaytimesfree=0;
set @vDaytimesfee=0;
set @vPlayernum=0;
set @vLevel=0;
set @vRetryTimes=0;
set @vCostGold=0;
set @vCostCrystal=0;
set @vCostDiamond=0;
set @vCostItem='';
select open_year,open_month,open_day,open_weekday,open_hour,day_times_free,day_times_fee,player_num,pre_level,retry_times,cost_gold,cost_crystal,cost_diamond,cost_item into @vYear,@vMonth,@vDay,@vWeekday,@vHour,@vDaytimesfree,@vDaytimesfee,@vPlayernum,@vLevel,@vRetryTimes,@vCostGold,@vCostCrystal,@vCostDiamond,@vCostItem from excel_instance_list where excel_id=nExcelID;

-- 副本存在吗
if (row_count()=0) then
	set nRst=-3;
	leave label_check_playerinstance;
end if;

-- vip特权
set @vVIP=0;
select vip into @vVIP from common_characters where account_id=nAccountID;
set @vDaytimesfree_1=0;
set @vDaytimesfee_1=0;
select instance_day_times_free,instance_day_times_fee into @vDaytimesfree_1,@vDaytimesfee_1 from excel_vip_right where level=@vVIP;
set @vDaytimesfree=@vDaytimesfree+@vDaytimesfree_1;
set @vDaytimesfee=@vDaytimesfee+@vDaytimesfee_1;

-- 
case
when nExcelID=101 then
	-- 南征北战
	if not exists (select 1 from excel_nanzhengbeizhan where class_id=nClassID) then
		set nRst=-11;
		leave label_check_playerinstance;
	end if;
when nExcelID=100 then
	-- 百战不殆
	if (nClassID>1) then
		if not exists (select 1 from common_characters where account_id=nAccountID and instance_wangzhe>=100) then
			set nRst=-12;
			leave label_check_playerinstance;
		end if;
	end if;
else
	set @vRst=0;
end case;
-- 等级
if not exists (select 1 from common_characters where account_id=nAccountID and level>=@vLevel) then 
	set nRst=-4;
	leave label_check_playerinstance;
end if;
-- 是否在开放时段
if (game_check_tick(@vYear,@vMonth,@vDay,@vWeekday,@vHour)<0) then
	set nRst=-5;
	leave label_check_playerinstance;
end if;
-- 重置之前是否还有剩余次数
if (to_days(date(from_unixtime(@vDoneTick)))=to_days(date(now()))) then
	if (@vDonedaytimesfree>=@vDaytimesfree) then
		-- 没有免费次数了
		if (@vDonedaytimesfee>=@vDaytimesfee) then
			-- 没有收费次数了
			set nRst=-6;
			leave label_check_playerinstance;
		else
			-- 消耗品够吗
			call game_cost_instance(nAccountID,@vCostGold,@vCostCrystal,@vCostDiamond,@vCostItem,1,0,@vRst);
			if (@vRst!=1) then
				set nRst=-7;
				leave label_check_playerinstance;
			end if;
			if (nExcelID=101) then
				update instance_player set hero_id='',day_times_fee=day_times_fee+1,day_times_fee_buffer=1,join_time=unix_timestamp(),day_times_free_buffer=1,day_times_free=day_times_free-1 where account_id=nAccountID and excel_id=nExcelID;
			end if;
		end if;
	end if;
end if;

-- 如果是加入，副本满员了吗
if (nInstanceID>0) then
	if exists (select 1 from instance where instance_id=nInstanceID and cur_player>=@vPlayernum) then
		set nRst=-8;
		leave label_check_playerinstance;
	end if;
	-- 已经出征了吗
	if not exists (select 1 from instance where instance_id=nInstanceID and cur_status=0) then
		set nRst=-9;
		leave label_check_playerinstance;
	end if;
end if;

set nRst=1;
end label_check_playerinstance;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 副本消耗检查
drop procedure if exists `game_cost_instance`;
create procedure game_cost_instance(in nAccountID bigint unsigned, in nGold int, in nCrystal int, in nDiamond int, in szItem blob, in nPayNow int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_cost_instance: begin

-- 金币，水晶，钻石
if not exists (select 1 from common_characters where account_id=nAccountID and 
	gold>=nGold and 
	crystal>=nCrystal and 
	diamond>=nDiamond
	) then 
	set nRst=0;
	leave label_cost_instance;
end if;
-- 道具
set @vRst=game_check_stritemnum(nAccountID,szItem,1);
if (@vRst=-1) then
	set nRst=-2;
	leave label_cost_instance;
elseif (@vRst=-2) then
	set nRst=-3;
	leave label_cost_instance;
end if;

-- 现在支付吗
if (nPayNow=1) then
	-- 金币，水晶，钻石
	if (nGold>0 or nCrystal>0 or nDiamond>0) then
	update common_characters set gold=gold-nGold,crystal=crystal-nCrystal,diamond=diamond-nDiamond where account_id=nAccountID and gold>=nGold and crystal>=nCrystal and diamond>=nDiamond;
	if (row_count()=0) then
		set nRst=-4;
		leave label_cost_instance;
	end if;
	end if;

	-- 道具
	set @vRst=0;
	call game_del_stritemnum(nAccountID,szItem,1,0,@vRst);
	if (@vRst!=0) then
		set nRst=-5;
		leave label_cost_instance;
	end if;
end if;

set nRst=1;
end label_cost_instance;
if (nEcho=1) then
	select nRst;
end if;
end
//


-- 销毁副本
drop procedure if exists `game_destroy_instance`;
create procedure game_destroy_instance(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, in nForce int, in nEcho int, out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select account_id from instance_player where instance_id=nInstanceID;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_destroy_instance: begin

set @vID1=nAccountID;
set @vID2=0;
set @vID3=0;
set @vID4=0;
set @vID5=0;

-- 是创建人吗
if (nForce=0) then
if not exists (select 1 from instance where instance_id=nInstanceID and creator_id=nAccountID) then
	set nRst=-2;
	leave label_destroy_instance;
end if;
-- 在战斗中吗
if exists (select 1 from combats where combat_id=nInstanceID) then
	set nRst=-4;
	leave label_destroy_instance;
end if;
else
	-- 如果是多人副本战斗不能踢出
	IF EXISTS (SELECT 1 FROM instance WHERE instance_id=nInstanceID AND excel_id=101 AND cur_status!=0 AND (@_create_time:=create_time)>=0) THEN
		SET @_fight_timeout=0;
		SELECT val_int INTO @_fight_timeout FROM excel_cmndef WHERE id=27;
		IF (@_create_time+@_fight_timeout)>UNIX_TIMESTAMP() THEN
			-- 战斗未超时
			SET nRst=-5;
			leave label_destroy_instance;
		END IF;
	END IF;
end if;

set @vN=1;
open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	-- 逐个离开
	set @vRst=0;
	call game_quit_instance(nID,nExcelID,nInstanceID,nForce,0,@vRst);
	if (@vRst!=0) then
		set nRst=-3;
		leave label_destroy_instance;
	end if;
	
	set @vQuery=concat('set @vID', @vN, '=', nID);
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;

	set @vN=@vN+1;

end if;
until nDone end repeat;
close vCursor;

-- 销毁副本
-- 最后一人 quit 时，instance就会删除了

set nRst=0;
end label_destroy_instance;
if (nEcho=1) then
	select nRst,@vID1,@vID2,@vID3,@vID4,@vID5;
end if;
end
//
-- 离开副本
drop procedure if exists `game_quit_instance`;
create procedure game_quit_instance(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, in nForce int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_quit_instance: begin

if (nExcelID=101) then
set @vHero='';
if not exists (select 1 from instance_player where account_id=nAccountID and instance_id=nInstanceID and (@vHero:=hero_id) is not null) then
	set nRst=-5;
	leave label_quit_instance;
end if;
-- 在战斗中吗
if exists (select 1 from combats where combat_id=nInstanceID) then
	set nRst=-4;
	leave label_quit_instance;
end if;
-- 武将解锁
if (length(@vHero)>0) then
	set @vHeroID=0;
	call game_split_2(@vHero,',',@vNull,@vHeroID);
	call game_unlock_hero(@vHeroID,nInstanceID,nExcelID,@vRst);
	if (@vRst<=0) then set nRst=-6; leave label_quit_instance; end if;
end if;
end if;

-- 是队长吗
if exists (select 1 from instance where instance_id=nInstanceID and creator_id=nAccountID) then
	-- 交给其他队员
	update instance set creator_id=ifnull((select account_id from instance_player where instance_id=nInstanceID and account_id!=nAccountID limit 1),nAccountID) where instance_id=nInstanceID;
	if (row_count()=0) then
		-- 没人了？那销毁吧
		update instance_player set instance_id=0,hero_id='',status=0 where account_id=nAccountID and excel_id=nExcelID and instance_id=nInstanceID;
		delete from instance where instance_id=nInstanceID;
		set nRst=0;
		leave label_quit_instance;
	end if;
end if;

update instance_player set instance_id=0,hero_id='',status=0 where account_id=nAccountID and excel_id=nExcelID and instance_id=nInstanceID;
if (row_count()=0) then
	set nRst=-2;
	leave label_quit_instance;
end if;
update instance set cur_player=cur_player-1 where instance_id=nInstanceID and excel_id=nExcelID and cur_player>0;
if (row_count()=0) then
	set nRst=-3;
	leave label_quit_instance;
end if;

set nRst=0;
end label_quit_instance;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 获取指定副本的成员摘要
drop procedure if exists `game_get_instancedata`;
create procedure game_get_instancedata(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;

select T1.account_id,T2.name,T1.hero_id,T1.status from instance_player T1 left join common_characters T2 on T1.account_id=T2.account_id where T1.instance_id=nInstanceID and excel_id=nExcelID order by join_time;

set nRst=0;
-- select nRst;
end
//
-- 获取指定副本已获得的奖励
drop procedure if exists `game_get_instanceloot`;
create procedure game_get_instanceloot(in nInstanceID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;

select loot_list from instance where instance_id=nInstanceID;

set nRst=0;
-- select nRst;
end
//
-- 获取指定副本已获得的奖励，通知client添加道具
drop procedure if exists `game_get_instanceloot_additem`;
create procedure game_get_instanceloot_additem(in nCombatType int, in nInstanceID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;

case
when nCombatType=1 then
-- 军事任务
select account_id,loot_list from combats where combat_id=nInstanceID;
-- 只能取一次，清除吧
update combats set loot_list='' where combat_id=nInstanceID;

when nCombatType=100 OR nCombatType=101 then
-- 副本
select account_id,loot_list from instance_player where instance_id=nInstanceID;
-- 只能取一次，清除吧
update instance_player set loot_list='' where instance_id=nInstanceID;

when nCombatType=20 then
-- 世界名城
select T1.account_id,T1.loot_list,T2.cup from combats T1 left join common_characters T2 on T2.account_id=T1.account_id where T1.combat_id=nInstanceID;
-- 只能取一次，清除吧
update combats set loot_list='' where combat_id=nInstanceID;

else
	set @vRst=0;

end case;

set nRst=0;
-- select nRst;
end
//


-- 多人副本需要预先配置武将，因为战斗只能由一人发起
drop procedure if exists `game_config_instancehero`;
create procedure game_config_instancehero(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, in nHero1 bigint unsigned, in nHero2 bigint unsigned, in nHero3 bigint unsigned, in nHero4 bigint unsigned, in nHero5 bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_config_instancehero: begin

-- 获取副本人数
set @vPlayernum=0;
if not exists (select 1 from excel_instance_list T1 right join instance_player T2 on T2.excel_id=T1.excel_id where T2.account_id=nAccountID and T2.instance_id=nInstanceID and (@vPlayernum:=player_num)>0) then
	set nRst=-2;
	leave label_config_instancehero;
end if;
-- 副本里没有战斗时才可以
if exists (select 1 from combats where combat_id=nInstanceID) then
	set nRst=-3;
	leave label_config_instancehero;
end if;

set @vHero='';
if (@vPlayernum=1) then
	-- 单人副本可配最多5个
	call game_check_instancehero(nHero1,@vRst);
	if (@vRst<=0) then set nRst=-4; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero1); end if;
	call game_check_instancehero(nHero2,@vRst);
	if (@vRst<=0) then set nRst=-4; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero2); end if;
	call game_check_instancehero(nHero3,@vRst);
	if (@vRst<=0) then set nRst=-4; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero3); end if;
	call game_check_instancehero(nHero4,@vRst);
	if (@vRst<=0) then set nRst=-4; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero4); end if;
	call game_check_instancehero(nHero5,@vRst);
	if (@vRst<=0) then set nRst=-4; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero5); end if;
else
	-- 先解锁
	select hero_id into @vHero from instance_player where account_id=nAccountID and excel_id=nExcelID;
	if (length(@vHero)>0) then
		set @vHeroID=0;
		call game_split_2(@vHero,',',@vNull,@vHeroID);
		call game_unlock_hero(@vHeroID,nInstanceID,nExcelID,@vRst);
		if (@vRst<=0) then set nRst=-5; leave label_config_instancehero; end if;
		set @vHero='';
		update instance_player set hero_id=@vHero where account_id=nAccountID and excel_id=nExcelID and instance_id=nInstanceID;
	end if;
	-- 多人副本只能1个
	set @vHero='';
	call game_check_instancehero(nHero1,@vRst);
	if (@vRst<=0) then set nRst=-6; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero1); end if;
	-- 再锁定
	call game_lock_hero(nHero1,nInstanceID,nExcelID,0,@vRst);
	if (@vRst<=0) then set nRst=-7; leave label_config_instancehero; end if;
end if;

-- 更新副本武将
-- if (length(@vHero)>0) then
	update instance_player set hero_id=@vHero where account_id=nAccountID and excel_id=nExcelID and instance_id=nInstanceID;
-- end if;

-- 是队长还要设置准备就绪
if exists (select 1 from instance where instance_id=nInstanceID and creator_id=nAccountID) then
	call game_prepare_instance(nAccountID,nExcelID,nInstanceID,1,0,@vRst);
end if;

set nRst=0;
end label_config_instancehero;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 检测是否可被配置
drop procedure if exists `game_check_instancehero`;
create procedure game_check_instancehero(in nHeroID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_check_instancehero: begin

-- 为0就不用锁定了
if (nHeroID=0) then
	set nRst=2;
	leave label_check_instancehero;
end if;

set nRst=1;
end label_check_instancehero;
end
//


-- 下面这个是假设副本需要完全锁定武将，不过现在不用这样
-- 配置副本武将
drop procedure if exists `game_config_instancehero_1`;
create procedure game_config_instancehero_1(in nAccountID bigint unsigned, in nExcelID int, in nInstanceID bigint unsigned, in nHero1 bigint unsigned, in nHero2 bigint unsigned, in nHero3 bigint unsigned, in nHero4 bigint unsigned, in nHero5 bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_config_instancehero: begin

-- 获取副本人数
set @vPlayernum=0;
if not exists (select 1 from excel_instance_list T1 right join instance_player T2 on T2.excel_id=T1.excel_id where T2.account_id=nAccountID and T2.instance_id=nInstanceID and (@vPlayernum:=player_num)>0) then
	set nRst=-2;
	leave label_config_instancehero;
end if;

-- 先解锁
set @vRst=0;
call game_unlock_instancehero(nInstanceID,nAccountID,0,@vRst);
if (@vRst=-2) then
	-- 副本当前正在战斗
	set nRst=-3;
	leave label_config_instancehero;
end if;

-- 再锁定
set @vHero='';
if (@vPlayernum=1) then
	-- 单人副本可配最多5个
	call game_lock_hero(nHero1,nInstanceID,nExcelID,1,@vRst);
	if (@vRst<=0) then set nRst=-3; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero1); end if;
	call game_lock_hero(nHero2,nInstanceID,nExcelID,1,@vRst);
	if (@vRst<=0) then set nRst=-3; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero2); end if;
	call game_lock_hero(nHero3,nInstanceID,nExcelID,1,@vRst);
	if (@vRst<=0) then set nRst=-3; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero3); end if;
	call game_lock_hero(nHero4,nInstanceID,nExcelID,1,@vRst);
	if (@vRst<=0) then set nRst=-3; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero4); end if;
	call game_lock_hero(nHero5,nInstanceID,nExcelID,1,@vRst);
	if (@vRst<=0) then set nRst=-3; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero5); end if;
else
	-- 多人副本只能1个
	call game_lock_hero(nHero1,nInstanceID,nExcelID,1,@vRst);
	if (@vRst<=0) then set nRst=-3; leave label_config_instancehero; elseif (@vRst=1) then set @vHero=concat(@vHero,',',nHero1); end if;
end if;

-- 更新副本武将
if (length(@vHero)>0) then
update instance_player set hero_id=@vHero where account_id=nAccountID and excel_id=nExcelID and instance_id=nInstanceID;
if (row_count()=0) then
	set nRst=-4;
	leave label_config_instancehero;
end if;
end if;

set nRst=0;
end label_config_instancehero;
select nRst;
end
//
-- 解锁指定副本里账号的所有武将
drop procedure if exists `game_unlock_instancehero`;
create procedure game_unlock_instancehero(in nInstanceID bigint unsigned, in nAccountID bigint unsigned, in nEcho int, out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select T1.hero_id from combat_heros T1 inner join hire_heros T2 on T2.hero_id=T1.hero_id where T2.account_id=nAccountID and T1.combat_id=nInstanceID;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_unlock_instancehero: begin

-- 副本里没有战斗时才能解锁
set @vCombatType=0;
if exists (select 1 from combats where combat_id=nInstanceID and (@vCombatType:=combat_type)>0) then
	set nRst=-2;
	leave label_unlock_instancehero;
end if;

open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	-- 逐个解锁
	call game_unlock_hero(nID,nInstanceID,@vCombatType,@vRst);
end if;
until nDone end repeat;
close vCursor;

-- 更新副本武将
update instance_player set hero_id='' where account_id=nAccountID and instance_id=nInstanceID;

set nRst=0;
end label_unlock_instancehero;
if (nEcho=1) then
	select nRst;
end if;
end
//

-- 添加定时事件索引
drop procedure if exists `game_add_te`;
create procedure game_add_te(in nTimeout int, in nType int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_te: begin

select game_global_id() into @vTEID;
set @vCurTime=unix_timestamp();
insert into time_events (event_id,begin_time,end_time,type,locked) value (@vTEID,@vCurTime,@vCurTime+nTimeout,nType,0);

set nRst=0;
end label_add_te;
if (nEcho=1) then
	select nRst,@vTEID;
end if;
end
//
-- 添加定时事件数据，战斗
drop procedure if exists `game_add_te_combat`;
create procedure game_add_te_combat(in nCombatID bigint unsigned, in nTimeout int, in nType int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_te_combat: begin

-- 添加定时事件索引
set @vRst=0;
call game_add_te(nTimeout,nType,0,@vRst);
if (@vRst!=0) then
	set nRst=-2;
	leave label_add_te_combat;
end if;

insert into te_combat (te_id,te_type,combat_id) values (@vTEID,nType,nCombatID);

set nRst=0;
end label_add_te_combat;
if (nEcho=1) then
	select nRst,@vTEID;
end if;
end
//
-- 获取定时事件数据，战斗
drop procedure if exists `game_get_te_combat`;
create procedure game_get_te_combat(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_te_combat: begin

select T1.combat_id,T2.combat_type,T2.status from te_combat T1 left join combats T2 on T2.combat_id=T1.combat_id where T1.te_id=nTEID;

-- 可以删除了
call game_del_te_combat(nTEID,@vRst);

set nRst=0;
end label_get_te_combat;
end
//
-- 删除定时事件索引
drop procedure if exists `game_del_te`;
create procedure game_del_te(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_te: begin

delete from time_events where event_id=nTEID;

set nRst=0;
end label_del_te;
end
//
-- 删除定时事件数据，战斗
drop procedure if exists `game_del_te_combat`;
create procedure game_del_te_combat(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_te_combat: begin

delete from te_combat where te_id=nTEID;
call game_del_te(nTEID,@vRst);

set nRst=0;
end label_del_te_combat;
end
//

-- 获取战斗摘要
drop procedure if exists `game_list_combat`;
create procedure game_list_combat(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_list_combat: begin

-- union = union distinct
-- 普通战斗，攻方
select combat_id,combat_type,obj_id,defense_name,cast(use_time as signed) - (cast(unix_timestamp() as signed) - cast(begin_time as signed)),status from combats where account_id=nAccountID union 
-- 普通对战，守方
select combat_id,combat_type+1,account_id,attack_name,cast(use_time as signed) - (cast(unix_timestamp() as signed) - cast(begin_time as signed)),status from combats where obj_id=nAccountID union 
-- 副本
select T1.combat_id,T1.combat_type,T1.obj_id,T1.defense_name,cast(T1.use_time as signed) - (cast(unix_timestamp() as signed) - cast(T1.begin_time as signed)),T1.status from combats T1 left join instance_player T2 on T1.combat_id=T2.instance_id where T2.account_id=nAccountID;

set nRst=0;
end label_list_combat;
end
//

-- 自动战斗开关
drop procedure if exists `game_switch_autocombat`;
create procedure game_switch_autocombat(in nAccountID bigint unsigned, in nInstanceID bigint unsigned, in nVal int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_switch_autocombat: begin

set @vAutoCombat=0;

case nVal
when 0 then
	update instance set auto_combat=0 where instance_id=nInstanceID and creator_id=nAccountID and (@vAutoCombat:=auto_combat)>=0;
when 1 then
	if not exists (select 1 from combats where combat_id=nInstanceID and status=1) then
		update instance set auto_combat=1 where instance_id=nInstanceID and creator_id=nAccountID;
	end if;
else
	update instance set auto_combat=if(auto_combat=0,1,0) where instance_id=nInstanceID and creator_id=nAccountID;
end case;

set nRst=0;
end label_switch_autocombat;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 自动补给开关
drop procedure if exists `game_switch_autosupply`;
create procedure game_switch_autosupply(in nAccountID bigint unsigned, in nInstanceID bigint unsigned, in nVal int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_switch_autosupply: begin

case nVal
when 0 then
	update instance set auto_supply=0 where instance_id=nInstanceID and creator_id=nAccountID;
when 1 then
	update instance set auto_supply=1 where instance_id=nInstanceID and creator_id=nAccountID;
else
	update instance set auto_supply=if(auto_supply=0,1,0) where instance_id=nInstanceID and creator_id=nAccountID;
end case;

set nRst=0;
end label_switch_autosupply;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 自动补给
drop procedure if exists `game_auto_supply`;
create procedure game_auto_supply(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_auto_supply: begin

set @vAccountID=0;
set @vDrug=0;
set @vResDrug=0;
-- nHeroID为0就算了
if (nHeroID=0) then
	set nRst=1;
	leave label_auto_supply;
end if;

-- 
set @vLevel=0;
set @vArmyType=0;
set @vArmyLevel=0;
set @vArmyNum=0;
set @vStatus=0;
if not exists (select 1 from hire_heros where hero_id=nHeroID and (@vAccountID:=account_id)>=0 and (@vLevel:=level)>=0 and (@vArmyType:=army_type)>=0 and (@vArmyLevel:=army_level)>=0 and (@vArmyNum:=army_num)>=0 and (@vStatus:=status)>=0) then
	set nRst=-2;
	leave label_auto_supply;
end if;

-- 出征时不能补给
if (@vStatus=2) then
	set nRst=-10;
	leave label_auto_supply;
end if;

-- 先清空原来的
if (revert_soldier(@vAccountID,@vArmyType,@vArmyLevel,@vArmyNum)!=0) then
	set nRst=-3;
	leave label_auto_supply;
end if;

-- 疗伤
call game_use_item(@vAccountID,nHeroID,2014,-1,0,@vRst);

-- 所扣草药
set @vDrug=@vNeedNum;

-- 统率与带兵的关系是啥？
set @vArmyNum=@vLeaderBase+@vLeaderAdd;

-- 再重新配置
set @vNum=0;
select ifnull(num,0) into @vNum from soldiers where account_id=@vAccountID and excel_id=@vArmyType and level=@vArmyLevel;
set @vArmyNum=if(@vArmyNum>@vNum,@vNum,@vArmyNum);
if (borrow_soldier(@vAccountID,@vArmyType,@vArmyLevel,@vArmyNum)!=0) then
	set nRst=-4;
	leave label_auto_supply;
end if;

-- 计算配兵后的总属性
set @vRst=0;
call game_calc_army(nHeroID,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
update hire_heros set army_num=@vArmyNum,army_attack=@vAttackArmy,army_defense=@vDefenseArmy,army_health=@vHealthArmy,army_prof=@vProf where hero_id=nHeroID;
set nRst=row_count();

end label_auto_supply;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 计算武将疗伤需要的草药数量
drop procedure if exists `game_calc_heal`;
create procedure game_calc_heal(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_heal: begin

set @vNeedNum=-1;

set @vLevel=0;
set @vHealthState=0;
if not exists (select 1 from hire_heros where hero_id=nHeroID and (@vLevel:=level)>=0 and (@vHealthState:=healthstate)>=0) then
	set nRst=-2;
	leave label_calc_heal;
end if;
set @vVal=0;
if not exists (select 1 from excel_heal where level=@vLevel and (@vVal:=val)>=0) then
	set nRst=-3;
	leave label_calc_heal;
end if;
set @vNum=0;
if not exists (select 1 from common_characters where account_id=nAccountID and (@vNum:=drug)>=0) then
	set nRst=-4;
	leave label_calc_heal;
end if;

set @vNeedNum=CEIL((100-@vHealthState)/@vVal);
-- 受伤惩罚
-- 轻伤 0
-- 中伤 1
-- 重伤 2
set @vBaseNum=@vNeedNum;
set @vPunishNum=0;

if (@vHealthState>80) then
	set @vPunishNum=@vNeedNum*0;
elseif (@vHealthState>50) then
	set @vPunishNum=@vNeedNum*1;
else
	set @vPunishNum=@vNeedNum*2;
end if;

set @vNeedNum=@vBaseNum+@vPunishNum;
set @vOldNeedNum=@vNeedNum;
set @vNeedNum=if(@vNeedNum<@vNum,@vNeedNum,@vNum);
set @vBaseNum=ceil(@vBaseNum*@vNeedNum/@vOldNeedNum);

set nRst=0;
end label_calc_heal;
if (nEcho=1) then
	select nRst,@vVal,@vNeedNum;
end if;
end
//
-- 补给武将，ID字符串 ,id1,id2,id3,...,idn
drop procedure if exists `game_supply_strhero`;
create procedure game_supply_strhero(in nAccountID bigint unsigned, in szStr blob, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_supply_strhero: begin

set @vWC_SS=0;
select game_wc(szStr,',')+1 into @vWC_SS;

set @vN_SS=2;
while @vN_SS<=@vWC_SS do
	select substring_index(substring_index(szStr,',',@vN_SS),',',-1) into @vHeroID;
	set @vRst=0;
	call game_auto_supply(nAccountID,@vHeroID,0,@vRst);

	set @vDrugStr=concat(@vDrugStr,',',@vAccountID,',',@vResDrug);
	set @vN_SS=@vN_SS+1;
end while;

set nRst=0;
end label_supply_strhero;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 手动补给
drop procedure if exists `game_manual_supply`;
create procedure game_manual_supply(in nAccountID bigint unsigned, in nInstanceID bigint unsigned, in nCombatType int, in n1Hero bigint unsigned, in n2Hero bigint unsigned, in n3Hero bigint unsigned, in n4Hero bigint unsigned, in n5Hero bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_manual_supply: begin

set @vStatus=1;
set @vHero='';
set @vDrugStr='';
case
when nCombatType=10 or nCombatType=11 then
	-- 世界金矿
	select concat(',',concat_ws(',',hero_1,hero_2,hero_3,hero_4,hero_5)) into @vHero from world_goldmine where account_id=nAccountID;
	if (length(@vHero)=0) then
		set nRst=-3;
		leave label_manual_supply;
	end if;

when nCombatType=100 or nCombatType=101 then
	-- 副本，百战不殆/南征北战
	if not exists (select 1 from instance where instance_id=nInstanceID and (@vStatus:=cur_status)>=0) then
		set nRst=-4;
		leave label_manual_supply;
	end if;
	if not exists (select 1 from instance_player where account_id=nAccountID and instance_id=nInstanceID and (@vHero:=hero_id) is not null) then
		set nRst=-5;
		leave label_manual_supply;
	end if;

else
	-- 其他，指定武将
	set @vHero=concat(',',concat_ws(',',n1Hero,n2Hero,n3Hero,n4Hero,n5Hero));

end case;

call game_supply_strhero(nAccountID,@vHero,0,@vRst);

set nRst=0;
end label_manual_supply;
select nRst,@vStatus,@vDrugStr;
end
//

-- 获取副本武将ID
drop procedure if exists `game_get_instanceheroid`;
create procedure game_get_instanceheroid(in nAccountID bigint unsigned, in nInstanceID bigint unsigned, in nCombatType int, in nAll int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_instanceheroid: begin

set @vHero1=0;
set @vHero2=0;
set @vHero3=0;
set @vHero4=0;
set @vHero5=0;

set @vHero='';
if (nAll=0) then
-- 
if not exists (select 1 from instance_player where account_id=nAccountID and instance_id=nInstanceID and (@vHero:=hero_id) is not null) then
	set nRst=-2;
	leave label_get_instanceheroid;
end if;
else
-- 多人副本所有英雄
select group_concat(hero_id separator '') into @vHero from instance_player where instance_id=nInstanceID;
end if;

set @vWC=0;
select game_wc(@vHero,',')+1 into @vWC;

set @vN=2;
while @vN<=@vWC do
	select substring_index(substring_index(@vHero,',',@vN),',',-1) into @vHeroID;
	
	set @vQuery=concat('set @vHero', @vN-1, '=', @vHeroID);
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;
	
	set @vN=@vN+1;
end while;

set nRst=0;
end label_get_instanceheroid;
end
//
-- 获取副本武将摘要
-- 世界金矿的也可以用这个
drop procedure if exists `game_get_instancehero`;
create procedure game_get_instancehero(in nAccountID bigint unsigned, in nInstanceID bigint unsigned, in nCombatType int, in n1Hero bigint unsigned, in n2Hero bigint unsigned, in n3Hero bigint unsigned, in n4Hero bigint unsigned, in n5Hero bigint unsigned, in nAll int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_instancehero: begin

set @vHero1=0;
set @vHero2=0;
set @vHero3=0;
set @vHero4=0;
set @vHero5=0;

case
when nCombatType=10 or nCombatType=11 then
	-- 世界金矿
	select hero_1,hero_2,hero_3,hero_4,hero_5 into @vHero1,@vHero2,@vHero3,@vHero4,@vHero5 from world_goldmine where account_id=nAccountID;

when nCombatType=100 or nCombatType=101 then
	-- 副本，百战不殆/南征北战
	call game_get_instanceheroid(nAccountID,nInstanceID,nCombatType,nAll,@vRst);

else
	-- 其他，指定武将
	select n1Hero,n2hero,n3Hero,n4Hero,n5Hero into @vHero1,@vHero2,@vHero3,@vHero4,@vHero5;

end case;

select hero_id,army_num,army_prof,healthstate,level,leader+leader_add from hire_heros where hero_id=@vHero1 union
select hero_id,army_num,army_prof,healthstate,level,leader+leader_add from hire_heros where hero_id=@vHero2 union
select hero_id,army_num,army_prof,healthstate,level,leader+leader_add from hire_heros where hero_id=@vHero3 union
select hero_id,army_num,army_prof,healthstate,level,leader+leader_add from hire_heros where hero_id=@vHero4 union
select hero_id,army_num,army_prof,healthstate,level,leader+leader_add from hire_heros where hero_id=@vHero5;

set nRst=0;
end label_get_instancehero;
end
//
-- 获取武将战斗数据
drop procedure if exists `game_get_herocombat`;
create procedure game_get_herocombat(in n1Hero bigint unsigned, in n2Hero bigint unsigned, in n3Hero bigint unsigned, in n4Hero bigint unsigned, in n5Hero bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_herocombat: begin

select hero_id,name,army_type,army_level,army_num,army_attack,army_defense,army_health,model,level,healthstate from hire_heros where hero_id=n1Hero union
select hero_id,name,army_type,army_level,army_num,army_attack,army_defense,army_health,model,level,healthstate from hire_heros where hero_id=n2Hero union
select hero_id,name,army_type,army_level,army_num,army_attack,army_defense,army_health,model,level,healthstate from hire_heros where hero_id=n3Hero union
select hero_id,name,army_type,army_level,army_num,army_attack,army_defense,army_health,model,level,healthstate from hire_heros where hero_id=n4Hero union
select hero_id,name,army_type,army_level,army_num,army_attack,army_defense,army_health,model,level,healthstate from hire_heros where hero_id=n5Hero;

set nRst=0;
end label_get_herocombat;
end
//


-- 骰子圆桌
drop function if exists `game_roll_table`;
create function game_roll_table(szRollTable blob) returns int
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

-- 字符串是否合法
select game_wc(szRollTable,'*') into @vWC;
if (mod(@vWC-2,3)!=0) then
	return -1;
end if;
-- roll
set @vRoll=rand()*100;
-- 落在哪个区间
set @vCur=0;
set @vN=1;
while @vN<=@vWC do
	select substring_index(substring_index(szRollTable,'*',@vN),'*',-1) into @vID;
	select substring_index(substring_index(szRollTable,'*',@vN+1),'*',-1) into @vPro;
	select substring_index(substring_index(szRollTable,'*',@vN+2),'*',-1) into @vNum;

	set @vCur=@vCur+@vPro;
	if (@vRoll<=@vCur) then
		return @vID;
	end if;

	set @vN=@vN+3;
end while;

return 0;
end
//

-- 计算武将的加成数据
drop procedure if exists `game_calc_heroadd`;
create procedure game_calc_heroadd(in nHeroID bigint unsigned, in nAttackBonusVal int, in fAttackBonusPct float(32,2), in nDefenseBonusVal int, in fDefenseBonusPct float(32,2), in nHealthBonusVal int, in fHealthBonusPct float(32,2), in nLeaderBonusVal int, in fLeaderBonusPct float(32,2), in nEcho int, out nRst int)
begin

declare nDone			int default 0;
declare nEquipID		int unsigned default 0;
declare szGem			blob;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select T1.excel_id,ifnull(group_concat(T2.excel_id),'') from hero_items T1 left join item_slots T2 on T2.item_id=T1.item_id where T1.hero_id=nHeroID group by T1.excel_id;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_heroadd: begin

set @vAttackAdd=0;
set @vDefenseAdd=0;
set @vHealthAdd=0;
set @vLeaderAdd=0;

open vCursor;
repeat
fetch vCursor into nEquipID,szGem;
if not nDone then
	-- 逐个计算
	-- 装备
	select attack+@vAttackAdd,defense+@vDefenseAdd,health+@vHealthAdd,leader+@vLeaderAdd into @vAttackAdd,@vDefenseAdd,@vHealthAdd,@vLeaderAdd from excel_item_list where excel_id=nEquipID and item_type=1;
	-- 宝石
	if (length(szGem)>0) then
		set @vWC=0;
		select game_wc(szGem,',')+1 into @vWC;

		set @vN=1;
		while @vN<=@vWC do
			select substring_index(substring_index(szGem,',',@vN),',',-1) into @vGemID;
			select attack+@vAttackAdd,defense+@vDefenseAdd,health+@vHealthAdd,leader+@vLeaderAdd into @vAttackAdd,@vDefenseAdd,@vHealthAdd,@vLeaderAdd from excel_item_list where excel_id=@vGemID and item_type=2;
			
			set @vN=@vN+1;
		end while;
	end if;
end if;
until nDone end repeat;
close vCursor;

set @vAttackAdd=floor((@vAttackAdd+nAttackBonusVal)*(1+fAttackBonusPct));
set @vDefenseAdd=floor((@vDefenseAdd+nDefenseBonusVal)*(1+fDefenseBonusPct));
set @vHealthAdd=floor((@vHealthAdd+nHealthBonusVal)*(1+fHealthBonusPct));
set @vLeaderAdd=floor((@vLeaderAdd+nLeaderBonusVal)*(1+fLeaderBonusPct));

set nRst=0;
end label_calc_heroadd;
if (nEcho=1) then
	select @vAttackAdd,@vDefenseAdd,@vHealthAdd,@vLeaderAdd;
end if;
end
//

-- 使用道具
drop procedure if exists `game_use_item`;
create procedure game_use_item(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nExcelID int, in nNum int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_use_item: begin

-- 关联类型
set @vType=0;

label_use_item_1: begin

case nExcelID
when 2014 then
-- 如果是疗伤，要先计算将使用的草药数
	-- 类型，武将
	set @vType=1;
	-- 
	set @vResDrug=0;
	call game_calc_heal(nAccountID,nHeroID,0,@vRst);
	call game_calc_heroall(nHeroID,1,1,1,@vRst);
	if (@vRst!=0) then
		set nRst=-12;
		leave label_use_item;
	end if;
	-- 扣掉草药
	if (@vNeedNum>0) then
		update common_characters set drug=drug-@vNeedNum,is_drug_max=0 where account_id=nAccountID AND drug>=@vNeedNum;
		IF ROW_COUNT()=0 THEN
			set nRst=-12;
			leave label_use_item;
		END IF;
	end if;
	-- 稍后才更新健康度
--	set nNum=@vNeedNum;
	set nNum=@vBaseNum;

	select drug into @vResDrug from common_characters where account_id=nAccountID;

when 2010 then
	-- 英雄榜不能这样使用，在酒馆刷新
	set nRst=-21;
	leave label_use_item;

else
-- 道具是否存在
set @vItemID=0;
if not exists (select 1 from items where account_id=nAccountID and excel_id=nExcelID and num>=nNum and (@vItemID:=item_id)>0) then
	case
	when nExcelID=2009 then
		set @vType=1;
	when nExcelID=2015 then
		set @vType=4;
		set @vChgNum=0;
	when nExcelID=2017 or nExcelID=2018 or nExcelID=2019 or nExcelID=2020 or nExcelID=2050  or nExcelID=2051  or nExcelID=2052  or nExcelID=2053  or nExcelID=2054  or nExcelID=2055  or nExcelID=2056  or nExcelID=2057  or nExcelID=2058 or nExcelID=2059 or nExcelID=2060 or nExcelID=2061 or nExcelID=2062 or nExcelID=2063 or nExcelID=2064 or nExcelID=2065 or nExcelID=2066 or nExcelID=2067 or nExcelID=2068 or nExcelID=2069 or nExcelID=2070 or nExcelID=2071 or nExcelID=2072 or nExcelID=2073 or nExcelID=2074 or nExcelID=2075 or nExcelID=4001 then
		set @vType=3;
	when nExcelID=2022 then
		set @vType=2;
	when nExcelID=2021 then
		set @vType=2;
	when nExcelID=2006 then
		set @vType=1;
	when nExcelID=2007 then
		set @vType=2;
	when nExcelID=2013 or nExcelID=2040 or nExcelID=2041 or nExcelID=2042 then
		set @vType=2;
	when nExcelID=2005 or nExcelID=2080 or nExcelID=2081 then
		set @vType=2;
	when nExcelID=2012 or nExcelID=2030 or nExcelID=2031 or nExcelID=2032 then
		set @vType=2;
	else
		set @vRst=0;
	end case;
	set nRst=-2;
	leave label_use_item_1;

end if;

case
when nExcelID=2006 or nExcelID=2009 then
	set @vType=1;
	-- 武将存在吗
	if not exists (select 1 from hire_heros where account_id=nAccountID and hero_id=nHeroID) then
		set nRst=-12;
		leave label_use_item;
	end if;
	
when nExcelID=2012 or nExcelID=2030 or nExcelID=2031 or nExcelID=2032 then
	set @vType=2;
	-- 工匠之书的时间还没结束
	if exists (select 1 from once_perperson_time_events where account_id=nAccountID and type=3) then
		set nRst=-13;
		leave label_use_item;
	end if;

when nExcelID=2005 or nExcelID=2080 or nExcelID=2081 then
	SET @vType=2;
	-- 有奖杯就不能免战
	if exists (select 1 from common_characters where account_id=nAccountID AND (@_proctect_time:=protect_time)>=0 and cup>0) then
		set nRst=-14;
		leave label_use_item;
	end if;
	-- 不能累积使用
	IF @_proctect_time>UNIX_TIMESTAMP() THEN
		SET nRst=-13;
		LEAVE label_use_item;
	END IF;
	-- 有自己发起的PVP战斗则不能免战
	IF EXISTS (SELECT 1 FROM combats WHERE account_id=nAccountID AND combat_type IN (2,20)) THEN
		SET nRst=-15;
		LEAVE label_use_item;
	END IF;
else
	set @vRst=0;
end case;

-- 道具属性
set @vValInt=0;
set @vValStr='';
select val_int,val_str into @vValInt,@vValStr from excel_item_list where excel_id=nExcelID;
end case;

while nNum>0 do

-- 先删除道具
if (nExcelID!=2014) then
call game_del_item(nAccountID,@vItemID,1,0,@vRst);
if (@vRst!=1) then
	set nRst=-11;
	leave label_use_item;
end if;
end if;

-- 开始使用
case
when nExcelID=2009 then
	-- 类型，武将
	set @vType=1;
	-- 强化丹
	call game_split_2(@vValStr,'*',@vGrowMin,@vGrowMax);
	set @vGrow=@vGrowMin+floor(rand()*(@vGrowMax/@vGrowMin))/10;
	call game_add_grow(nAccountID,nHeroID,@vGrow,0,@vRst);

when nExcelID=2014 then
	-- 类型，武将
	set @vType=1;
	-- 草药
	select T1.val into @vVal from excel_heal T1 right join hire_heros T2 on T1.level=T2.level where T2.hero_id=nHeroID;
	-- 先更新 healthstate ，trigger会重新计算有关属性
	update hire_heros set healthstate=(@vHealthState:=healthstate+@vVal) where hero_id=nHeroID;

when nExcelID=2015 then
	-- 类型，草药包
	set @vType=4;
	-- 草药包
	call game_split_2(@vValStr,'*',@vDrugID,@vDrugNum);
	-- 
	call game_add_drug(nAccountID,@vDrugNum,0,@vRst);

when nExcelID=2017 or nExcelID=2018 or nExcelID=2019 or nExcelID=2020 or nExcelID=2050 or nExcelID=2051  or nExcelID=2052  or nExcelID=2053  or nExcelID=2054  or nExcelID=2055  or nExcelID=2056  or nExcelID=2057  or nExcelID=2058 or nExcelID=2059 or nExcelID=2060 or nExcelID=2061 or nExcelID=2062 or nExcelID=2063 or nExcelID=2064 or nExcelID=2065 or nExcelID=2066 or nExcelID=2067 or nExcelID=2068 or nExcelID=2069 or nExcelID=2070 or nExcelID=2071 or nExcelID=2072 or nExcelID=2073 or nExcelID=2074 or nExcelID=2075 or nExcelID=4001 then
	-- 类型，道具
	set @vType=3;
	-- 新手礼包
	call game_add_stritemnum(nAccountID,@vValStr,1,0,@vRst);
	if (@vRst!=0) then set nRst=-20; leave label_use_item; end if;

	-- 金币
	update common_characters set gold=gold+@vValInt where account_id=nAccountID;

	-- @vType在game_add_stritemnum中也使用了,重新赋值,防止污染
	SET @vType=3;
when nExcelID=2022 or nExcelID=2024 then
	-- 类型，君主
	set @vType=2;
	-- 水晶
	update common_characters set crystal=crystal+@vValInt where account_id=nAccountID;

when nExcelID=2021 or nExcelID=2023 or nExcelID=2025 then
	-- 类型，君主
	set @vType=2;
	-- 金币
	update common_characters set gold=gold+@vValInt where account_id=nAccountID;

when nExcelID=2006 then
	-- 武将
	set @vType=1;
	-- 经验
	call add_hero_exp(nAccountID,nHeroID,@vValInt,@vRst);

when nExcelID=2007 then
	-- 领主
	set @vType=2;
	-- 经验
	call add_char_exp(nAccountID,@vValInt,@vRstExp,@vRstLevel);

when nExcelID=2013 or nExcelID=2040 or nExcelID=2041 or nExcelID=2042 then
	-- 领主
	set @vType=2;
	-- 人口
--	call add_population(nAccountID,@vValInt,@vManReal);
	update common_characters set population=population+@vValInt where account_id=nAccountID;

when nExcelID=2005 or nExcelID=2080 or nExcelID=2081 then
	-- 领主
	set @vType=2;
	-- 免战
	update common_characters set protect_time=unix_timestamp()+@vValInt where account_id=nAccountID;
	-- 别人的出征队伍全部返回
	CALL system_other_army_back(nAccountID);
when nExcelID=2012 or nExcelID=2030 or nExcelID=2031 or nExcelID=2032 then
	-- 领主
	set @vType=2;
	-- 工匠之书
	call add_build_num(nAccountID,nExcelID,0,@vRst);

else
	set @vRst=0;
end case;

set nNum=nNum-1;
end while;
end label_use_item_1;

-- 计算关联数据
case @vType
when 1 then
	-- 武将
	call game_calc_heroall(nHeroID,1,1,1,@vRst);

when 2 then
	-- 君主
	select level,exp,diamond,crystal,gold,vip,population,if(protect_time>0,if(protect_time>unix_timestamp(),protect_time-unix_timestamp(),0),0) into @vLevel,@vExp,@vDiamond,@vCrystal,@vGold,@vVIP,@vPopulation,@vProtectTime from common_characters where account_id=nAccountID;
	set @vRst=0;

when 3 then
	-- 道具
	set @vRst=0;

when 4 then
	-- 草药包
	set @vRst=0;

else
	set @vRst=0;
end case;

set nRst=0;
end label_use_item;
if (nEcho=1) then
-- 返回关联数据
case @vType
when 1 then
	-- 武将
	select @vType,nRst,@vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd,@vGrow,@vHealthState,@vArmyType,@vArmyLevel,@vArmyNum,@vExp,@vLevel,@vProf;

when 2 then
	-- 君主
	select @vType,nRst,@vLevel,@vExp,@vDiamond,@vCrystal,@vGold,@vVIP,@vPopulation,@vProtectTime;

when 3 then
	-- 道具
	select @vType,nRst,@vMyAddItem;

when 4 then
	select @vType,nRst,@vChgNum;

else
	select @vType,nRst;
end case;
end if;
end
//

-- 同步数据，君主
drop procedure if exists `game_sync_char`;
create procedure game_sync_char(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_sync_char: begin

set @vExp=0;
set @vDiamond=0;
set @vCrystal=0;
set @vGold=0;
set @vVip=0;

select exp,diamond,crystal,gold,vip into @vExp,@vDiamond,@vCrystal,@vGold,@vVip from common_characters where account_id=nAccountID;

set nRst=0;
end label_sync_char;
select nRst,@vExp,@vDiamond,@vCrystal,@vGold,@vVip;
end
//

-- 同步数据，武将
drop procedure if exists `game_sync_hero`;
create procedure game_sync_hero(in nAccountID bigint unsigned, in nHeroID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_sync_hero: begin

call game_calc_heroall(nHeroID,1,1,1,@vRst);

set nRst=0;
end label_sync_hero;
select nRst,@vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd,@vGrow,@vHealthState;
end
//

-- 重新计算武将所有数据
-- nBase 基础部分
-- nAdd 加成部分
-- nArmy 部队
drop procedure if exists `game_calc_heroall`;
create procedure game_calc_heroall(in nHeroID bigint unsigned, in nBase int, in nAdd int, in nArmy int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_heroall: begin

set @vAccountID=0;
set @vLevel=0;
set @vExp=0;
set @vAttackBase=0;
set @vAttackBonusVal=0;
set @vAttackBonusPct=0;
set @vDefenseBase=0;
set @vDefenseBonusVal=0;
set @vDefenseBonusPct=0;
set @vHealthBase=0;
set @vHealthBonusVal=0;
set @vHealthBonusPct=0;
set @vLeaderBase=0;
set @vLeaderBonusVal=0;
set @vLeaderBonusPct=0;
set @vGrow=0;
set @vArmyType=0;
set @vArmyLevel=0;
set @vArmyNum=0;
set @vHealthState=0;
select 
	account_id,level,exp
	,attack_0,attack_bonus_val,attack_bonus_pct
	,defense_0,defense_bonus_val,defense_bonus_pct
	,health_0,health_bonus_val,health_bonus_pct
	,leader_0,leader_bonus_val,leader_bonus_pct
	,grow,army_type,army_level,army_num
	,healthstate 
	into 
	@vAccountID,@vLevel,@vExp
	,@vAttackBase,@vAttackBonusVal,@vAttackBonusPct
	,@vDefenseBase,@vDefenseBonusVal,@vDefenseBonusPct
	,@vHealthBase,@vHealthBonusVal,@vHealthBonusPct
	,@vLeaderBase,@vLeaderBonusVal,@vLeaderBonusPct
	,@vGrow,@vArmyType,@vArmyLevel,@vArmyNum
	,@vHealthState 
	from 
	hire_heros where hero_id=nHeroID;

if (nBase=1) then
	-- 基础部分
	call game_calc_herobase(@vAttackBase,@vAttackBonusPct,@vDefenseBase,@vDefenseBonusPct,@vHealthBase,@vHealthBonusPct,@vLeaderBase,@vLeaderBonusPct,@vGrow,@vLevel,0,@vRst);
	-- 写回
	update hire_heros set attack=@vAttackBase,defense=@vDefenseBase,health=@vHealthBase,leader=@vLeaderBase where hero_id=nHeroID;
	-- 更新部队数量
	call game_update_armynum(@vAccountID,nHeroID,0,@vRst);
end if;
if (nAdd=1) then
	-- 加成部分
	call game_calc_heroadd(nHeroID,@vAttackBonusVal,@vAttackBonusPct,@vDefenseBonusVal,@vDefenseBonusPct,@vHealthBonusVal,@vHealthBonusPct,@vLeaderBonusVal,@vLeaderBonusPct,0,@vRst);
	-- 写回
	update hire_heros set attack_add=@vAttackAdd,defense_add=@vDefenseAdd,health_add=@vHealthAdd,leader_add=@vLeaderAdd where hero_id=nHeroID;
	-- 更新部队数量
	call game_update_armynum(@vAccountID,nHeroID,0,@vRst);
end if;
if (nArmy=1) then
	-- 部队
	call game_calc_army(nHeroID,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
	-- 写回
	update hire_heros set army_attack=@vAttackArmy,army_defense=@vDefenseArmy,army_health=@vHealthArmy,army_prof=@vProf where hero_id=nHeroID;
end if;

-- 因为有 trigger，所以再取一次
select attack,attack_add,defense,defense_add,health,health_add,leader,leader_add into @vAttackBase,@vAttackAdd,@vDefenseBase,@vDefenseAdd,@vHealthBase,@vHealthAdd,@vLeaderBase,@vLeaderAdd from hire_heros where hero_id=nHeroID;

set nRst=0;
end label_calc_heroall;
end
//
-- 更新部队数量
drop procedure if exists `game_update_armynum`;
create procedure game_update_armynum(in nAccountID bigint unsigned, in nHeroID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_update_armynum: begin

if not exists (select 1 from hire_heros where hero_id=nHeroID and (@vLeaderBase:=leader)>=0 and (@vLeaderAdd:=leader_add)>=0) then
	set nRst=-2;
	leave label_update_armynum;
end if;

-- 先清空原来的
if (revert_soldier(nAccountID,@vArmyType,@vArmyLevel,@vArmyNum)!=0) then
	set nRst=-3;
	leave label_update_armynum;
end if;

-- 统率与带兵的关系是啥？
set @vNewArmyNum=@vLeaderBase+@vLeaderAdd;
if (@vArmyNum>@vNewArmyNum) then
	set @vArmyNum=@vNewArmyNum;
end if;

-- 再重新配置
if (borrow_soldier(nAccountID,@vArmyType,@vArmyLevel,@vArmyNum)!=0) then
	set nRst=-4;
	leave label_update_armynum;
end if;

-- 部队
call game_calc_army(nHeroID,@vArmyType,@vArmyLevel,@vArmyNum,@vRst);
-- 写回
update hire_heros set army_num=@vArmyNum,army_attack=@vAttackArmy,army_defense=@vDefenseArmy,army_health=@vHealthArmy,army_prof=@vProf where hero_id=nHeroID;

set nRst=0;
end label_update_armynum;
if (nEcho=1) then
	select @vArmyNum;
end if;
end
//


-- 初始化世界金矿
drop procedure if exists `game_init_worldgoldmine`;
create procedure game_init_worldgoldmine(out nRst int)
begin

declare nDone			int default 0;
declare nID				int unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select area_id from excel_worldgoldmine;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_init_worldgoldmine: begin

-- 要初始化吗
if exists (select 1 from world_goldmine limit 1) then
	set nRst=0;
--	leave label_init_worldgoldmine;
end if;

open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	if not exists (select 1 from world_goldmine where area=nID limit 1) then
	call game_get_worldgoldmine_cfg(nID,1,0,@vRst); while @vNum>0 do insert into world_goldmine (area,class,idx) values (nID,1,@vNum); set @vNum=@vNum-1; end while;
	call game_get_worldgoldmine_cfg(nID,2,0,@vRst); while @vNum>0 do insert into world_goldmine (area,class,idx) values (nID,2,@vNum); set @vNum=@vNum-1; end while;
	call game_get_worldgoldmine_cfg(nID,3,0,@vRst); while @vNum>0 do insert into world_goldmine (area,class,idx) values (nID,3,@vNum); set @vNum=@vNum-1; end while;
	call game_get_worldgoldmine_cfg(nID,4,0,@vRst); while @vNum>0 do insert into world_goldmine (area,class,idx) values (nID,4,@vNum); set @vNum=@vNum-1; end while;
	end if;
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_init_worldgoldmine;
select nRst;
end
//
-- 获取世界金矿配置
drop procedure if exists `game_get_worldgoldmine_cfg`;
create procedure game_get_worldgoldmine_cfg(in nArea int, in nClass int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_worldgoldmine_cfg: begin

set @vStr='';
case nClass
when 1 then
	select top_cfg into @vStr from excel_worldgoldmine where area_id=nArea;
when 2 then
	select sen_cfg into @vStr from excel_worldgoldmine where area_id=nArea;
when 3 then
	select mid_cfg into @vStr from excel_worldgoldmine where area_id=nArea;
when 4 then
	select bas_cfg into @vStr from excel_worldgoldmine where area_id=nArea;
else
	set @vRst=0;
end case;

call game_split_4(@vStr,'*',@vNum,@vOutNum,@vOutFreq,@vSafeTime);

end label_get_worldgoldmine_cfg;
if (nEcho=1) then
	select @vNum,@vOutNum,@vOutFreq,@vSafeTime;
end if;
end
//

-- 获取世界金矿
drop procedure if exists `game_get_worldgoldmine`;
create procedure game_get_worldgoldmine(in nArea int, in nClass int, in nIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_worldgoldmine: begin

-- 开启数量
set @vNum=0;
select val_int into @vNum from excel_cmndef where id=6;
if (nArea>@vNum) then
	leave label_get_worldgoldmine;
end if;

set @vCurTick=unix_timestamp();
if (nClass=0 and nIdx=0) then
	select T1.class,T1.idx,T1.account_id,ifnull(T2.name,''),@vCurTick-T1.safe_time from world_goldmine T1 left join common_characters T2 on T2.account_id=T1.account_id where T1.area=nArea;
elseif (nIdx=0) then
	select T1.class,T1.idx,T1.account_id,ifnull(T2.name,''),@vCurTick-T1.safe_time from world_goldmine T1 left join common_characters T2 on T2.account_id=T1.account_id where T1.area=nArea and T1.class=nClass;
else
	select T1.class,T1.idx,T1.account_id,ifnull(T2.name,''),@vCurTick-T1.safe_time from world_goldmine T1 left join common_characters T2 on T2.account_id=T1.account_id where T1.area=nArea and T1.class=nClass and T1.idx=nIdx;
end if;

set nRst=0;
end label_get_worldgoldmine;
end
//
-- 设置世界金矿
drop procedure if exists `game_set_worldgoldmine`;
create procedure game_set_worldgoldmine(in nArea int, in nClass int, in nIdx int, in nAccountID bigint unsigned, in nObjID bigint unsigned, in nLock int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_set_worldgoldmine: begin

-- 开启数量
set @vNum=0;
select val_int into @vNum from excel_cmndef where id=6;
if (nArea>@vNum) then
	set nRst=0;
	leave label_set_worldgoldmine;
end if;

set @vCurTick=unix_timestamp();

if (nLock=1) then
	update world_goldmine set account_id=nAccountID,safe_time=@vCurTick,xlock=1 where area=nArea and class=nClass and idx=nIdx and account_id=if(nObjID<5,0,nObjID) and xlock=0;
else
	update world_goldmine set account_id=nAccountID,safe_time=@vCurTick where area=nArea and class=nClass and idx=nIdx and account_id=if(nObjID<5,0,nObjID) and xlock=0;
end if;

set nRst=row_count();
end label_set_worldgoldmine;
end
//
-- 配置金矿武将
drop procedure if exists `game_config_worldgoldmine_hero`;
create procedure game_config_worldgoldmine_hero(in nAccountID bigint unsigned, in nHero1 bigint unsigned, in nHero2 bigint unsigned, in nHero3 bigint unsigned, in nHero4 bigint unsigned, in nHero5 bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_config_worldgoldmine_hero: begin

update world_goldmine set hero_1=nHero1,hero_2=nHero2,hero_3=nHero3,hero_4=nHero4,hero_5=nHero5 where account_id=nAccountID;
set nRst=row_count();

end label_config_worldgoldmine_hero;
if (nEcho=1) then
	select nRst;
end if;
end
//
-- 夺取金矿
drop procedure if exists `game_rob_worldgoldmine`;
create procedure game_rob_worldgoldmine(in nArea int, in nClass int, in nIdx int, in nAccountID bigint unsigned, in nHero1 bigint unsigned, in nHero2 bigint unsigned, in nHero3 bigint unsigned, in nHero4 bigint unsigned, in nHero5 bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_rob_worldgoldmine: begin

-- 已经占领过了吗
if exists (select 1 from world_goldmine where account_id=nAccountID) then
	set nRst=-2;
	leave label_rob_worldgoldmine;
end if;
-- 开启数量
set @vNum=0;
select val_int into @vNum from excel_cmndef where id=6;
if (nArea>@vNum) then
	set nRst=-10;
	leave label_rob_worldgoldmine;
end if;
-- 金矿上有人吗
set @vObjID=0;
if not exists (select 1 from world_goldmine where area=nArea and class=nClass and idx=nIdx and xlock=0 and (@vObjID:=account_id)>=0) then
	set nRst=-3;
	leave label_rob_worldgoldmine;
end if;
-- 在保护时间里吗
call game_get_worldgoldmine_cfg(nArea,nClass,0,@vRst);
if exists (select 1 from world_goldmine where area=nArea and class=nClass and account_id>0 and (unix_timestamp()-safe_time)<=@vSafeTime) then
	set nRst=-4;
	leave label_rob_worldgoldmine;
end if;

-- 目标
if (@vObjID=0) then
	-- 对AI
	set @vObjID=nClass;
	set @vCombatType=10;
else
	-- 对玩家
	set @vCombatType=11;
end if;
-- 发起挑战
call game_start_combat(nAccountID,@vObjID,@vCombatType,0,0,nHero1,nHero2,nHero3,nHero4,nHero5,0,0,@vRst);
if (@vRst!=1) then set nRst=-5; leave label_rob_worldgoldmine; end if;
-- 附加数据
update combats set misc_data=concat(nArea,'*',nClass,'*',nIdx) where combat_id=@vCombatID;

set nRst=0;
end label_rob_worldgoldmine;
select nRst,@vCombatID;
end
//
-- 放弃世界金矿
drop procedure if exists `game_drop_worldgoldmine`;
create procedure game_drop_worldgoldmine(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_drop_worldgoldmine: begin

set @vArea=0;
set @vClass=0;
set @vIdx=0;
update world_goldmine set account_id=0,hero_1=0,hero_2=0,hero_3=0,hero_4=0,hero_5=0 where account_id=nAccountID and (@vArea:=area)>0 and (@vClass:=class)>0 and (@vIdx:=idx)>0;
set nRst=row_count();

-- 删除时间事件
if (nRst=1) then
	call game_del_te_worldgoldmine_1(@vArea,@vClass,@vIdx,@vRst);
end if;

end label_drop_worldgoldmine;
select nRst;
end
//
-- 收获世界金矿
drop procedure if exists `game_gain_worldgoldmine`;
create procedure game_gain_worldgoldmine(in nArea int, in nClass int, in nIdx int, in nAccountID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_gain_worldgoldmine: begin

-- 基础产量
call game_get_worldgoldmine_cfg(nArea,nClass,0,@vRst);
-- 产出度
set @vPct=0;
select T1.pct into @vPct from excel_worldgoldmine_out T1 right join common_characters T2 on T1.level=T2.level where T2.account_id=nAccountID;

set @vGold=0;
-- 锁定
update world_goldmine set xlock=1 where area=nArea and class=nClass and idx=nIdx and account_id=nAccountID and xlock=0;
if (row_count()=1) then
	-- 增加金币
	set @vGold=@vOutNum*@vPct;
	update common_characters set gold=gold+@vGold where account_id=nAccountID;
	-- 下一次收获超时 105
	call game_add_te_worldgoldmine(nArea,nClass,nIdx,nAccountID,@vOutFreq,105,0,@vRst);
	-- 解锁
	update world_goldmine set xlock=0 where area=nArea and class=nClass and idx=nIdx and account_id=nAccountID and xlock=1;
end if;

set nRst=0;
end label_gain_worldgoldmine;
if (nEcho=1) then
	select nRst,@vGold;
end if;
end
//

-- 添加定时事件数据，世界金矿
drop procedure if exists `game_add_te_worldgoldmine`;
create procedure game_add_te_worldgoldmine(in nArea int, in nClass int, in nIdx int, in nAccountID bigint unsigned, in nTimeout int, in nType int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_te_worldgoldmine: begin

-- 添加定时事件索引
set @vRst=0;
call game_add_te(nTimeout,nType,0,@vRst);
if (@vRst!=0) then
	set nRst=-2;
	leave label_add_te_worldgoldmine;
end if;

insert into te_worldgoldmine (te_id,te_type,area,class,idx,account_id) values (@vTEID,nType,nArea,nClass,nIdx,nAccountID);

set nRst=0;
end label_add_te_worldgoldmine;
if (nEcho=1) then
	select nRst,@vTEID;
end if;
end
//
-- 处理定时事件，世界金矿收获
drop procedure if exists `game_deal_te_worldgoldmine_gain`;
create procedure game_deal_te_worldgoldmine_gain(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_deal_te_worldgoldmine_gain: begin

set @vArea=0;
set @vClass=0;
set @vIdx=0;
set @vAccountID=0;
-- 获取数据
select area,class,idx,account_id into @vArea,@vClass,@vIdx,@vAccountID from te_worldgoldmine where te_id=nTEID;
set nRst=found_rows();

if (nRst=1) then
	-- 删除时间事件
	call game_del_te_worldgoldmine(nTEID,@vRst);
	-- 增加金币
	call game_gain_worldgoldmine(@vArea,@vClass,@vIdx,@vAccountID,0,@vRst);
end if;

set nRst=0;
end label_deal_te_worldgoldmine_gain;
select nRst,@vAccountID,@vArea,@vClass,@vIdx,@vGold;
end
//
-- 删除定时事件数据，世界金矿
drop procedure if exists `game_del_te_worldgoldmine`;
create procedure game_del_te_worldgoldmine(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_te_worldgoldmine: begin

delete from te_worldgoldmine where te_id=nTEID;
call game_del_te(nTEID,@vRst);

set nRst=0;
end label_del_te_worldgoldmine;
end
//
drop procedure if exists `game_del_te_worldgoldmine_1`;
create procedure game_del_te_worldgoldmine_1(in nArea int, in nClass int, in nIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_te_worldgoldmine_1: begin

set @vTEID=0;
delete from te_worldgoldmine where area=nArea and class=nClass and idx=nIdx and (@vTEID:=te_id)>0;
if (row_count()=1) then
	call game_del_te(@vTEID,@vRst);
end if;

set nRst=0;
end label_del_te_worldgoldmine_1;
end
//
-- 获取某个账号的世界金矿
drop procedure if exists `game_my_worldgoldmine`;
create procedure game_my_worldgoldmine(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_my_worldgoldmine: begin

set @vArea=0;
set @vClass=0;
set @vIdx=0;
set @vSafeTime=0;
set @vCurTick=unix_timestamp();
set @vHero1=0;
set @vHero2=0;
set @vHero3=0;
set @vHero4=0;
set @vHero5=0;
-- 获取数据
select area,class,idx,@vCurTick-safe_time,hero_1,hero_2,hero_3,hero_4,hero_5 into @vArea,@vClass,@vIdx,@vSafeTime,@vHero1,@vHero2,@vHero3,@vHero4,@vHero5 from world_goldmine where account_id=nAccountID;
set nRst=found_rows();

end label_my_worldgoldmine;
select nRst,@vArea,@vClass,@vIdx,@vSafeTime,@vHero1,@vHero2,@vHero3,@vHero4,@vHero5;
end
//
-- 重置世界金矿
drop procedure if exists `game_reset_worldgoldmine`;
create procedure game_reset_worldgoldmine(out nRst int)
begin

DECLARE _done INT DEFAULT 0;
DECLARE _account_id BIGINT UNSIGNED DEFAULT 0;

DECLARE _cursor CURSOR FOR SELECT account_id FROM world_goldmine;

declare exit            handler for sqlexception        begin end;
DECLARE CONTINUE				HANDLER FOR SQLSTATE '02000'		BEGIN SET _done=1; END; 
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_reset_worldgoldmine: begin

-- 删除时间事件
delete from time_events where type=105;
delete from te_worldgoldmine;

SELECT IFNULL(content,'') INTO @_text FROM excel_text WHERE excel_id=464;
OPEN _cursor;
REPEAT
	FETCH _cursor INTO _account_id;
	IF NOT _done THEN
		CALL add_private_mail(0,_account_id,4,1,0,@_text,NULL,0,@vRst,@vMailID);
	END IF;
UNTIL _done END REPEAT;
CLOSE _cursor;

-- 清空占领
update world_goldmine set account_id=0,hero_1=0,hero_2=0,hero_3=0,hero_4=0,hero_5=0,xlock=0;

set nRst=0;
end label_reset_worldgoldmine;
end
//

-- 初始化定时事件（固定小时）
drop procedure if exists `game_init_fixhour`;
create procedure game_init_fixhour(out nRst int)
begin

declare nDone			int default 0;
declare nID				int unsigned default 0;
declare szHour			blob;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select id,hour from excel_fixhour;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_init_fixhour: begin

-- 清楚以前的
delete from te_fixhour;
delete from time_events where type=110;

-- 当前时刻
set @vTick=unix_timestamp();

open vCursor;
repeat
fetch vCursor into nID,szHour;
if not nDone then

	call game_calc_fixhour(@vTick,szHour,0,@vRst);
	-- fixhour超时，110
	call game_add_te_fixhour(nID,@vDstHour,@vTimeout,110,0,@vRst);

end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_init_fixhour;
select nRst;
end
//
-- 计算下一次触发时刻
drop procedure if exists `game_calc_fixhour`;
create procedure game_calc_fixhour(in nCurTick int unsigned, in szHour blob, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_calc_fixhour: begin

-- 当前小时
set @vCurHour=hour(from_unixtime(nCurTick));

-- 触发时刻
set @vDstHour=-1;
	
set @vWC=0;
select game_wc(szHour,'*')+1 into @vWC;
set @vN=1;
while @vN<=@vWC do
	select substring_index(substring_index(szHour,'*',@vN),'*',-1) into @vNextHour;
	set @vDstHour=if(@vDstHour=-1,@vNextHour,@vDstHour);
	if (@vNextHour>@vCurHour) then
		set @vDstHour=@vNextHour;
		-- 为了跳出循环
		set @vWC=0;
	end if;

	set @vN=@vN+1;
end while;

-- timeout
set @vDstHourStr=concat_ws(':',@vDstHour,0,0);
set @vTimeout=time_to_sec(timediff(time(from_unixtime(nCurTick)),@vDstHourStr));
set @vTimeout=if(@vTimeout>0,60*60*24-@vTimeout,abs(@vTimeout));

set nRst=0;
end label_calc_fixhour;
if (nEcho=1) then
	select nRst,@vDstHour,@vTimeout;
end if;
end
//
-- 添加定时事件数据，fixhour
drop procedure if exists `game_add_te_fixhour`;
create procedure game_add_te_fixhour(in nID int, in nNextHour int, in nTimeout int, in nType int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_te_fixhour: begin

-- 添加定时事件索引
set @vRst=0;
call game_add_te(nTimeout,nType,0,@vRst);
if (@vRst!=0) then
	set nRst=-2;
	leave label_add_te_fixhour;
end if;

insert into te_fixhour (te_id,te_type,id,next_hour) values (@vTEID,nType,nID,nNextHour);

set nRst=0;
end label_add_te_fixhour;
if (nEcho=1) then
	select nRst,@vTEID;
end if;
end
//
-- 处理定时事件，fixhour
drop procedure if exists `game_deal_te_fixhour`;
create procedure game_deal_te_fixhour(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_deal_te_fixhour: begin

set @vFHID=0;
set @vNextHour=0;
select id,next_hour into @vFHID,@vNextHour from te_fixhour where te_id=nTEID;
set nRst=found_rows();
if (nRst=0) then
	set nRst=-2;
	leave label_deal_te_fixhour;
end if;

-- 删除时间事件
call game_del_te_fixhour(nTEID,@vRst);

-- 根据 id 分派
case @vFHID
when 10 then
	-- 世界金矿
	call game_reset_worldgoldmine(@vRst);
	
WHEN 100 THEN
	CALL game_reset_instance_wangzhe(@vRst);

when 101 then
	-- 南征北战
	call game_reset_instance(@vRst);

when 1000 then
	-- 刷新草药
	call game_gen_drug(@vRst);

else
	set @vRst=0;
end case;

-- 获取触发时刻
set @vHour='';
if not exists (select 1 from excel_fixhour where id=@vFHID and (@vHour:=hour) is not null) then
	set nRst=-3;
	leave label_deal_te_fixhour;
end if;

set @vTick=unix_timestamp();
call game_calc_fixhour(@vTick,@vHour,0,@vRst);
-- 下一次超时，110
call game_add_te_fixhour(@vFHID,@vDstHour,@vTimeout,110,0,@vRst);

set nRst=0;
end label_deal_te_fixhour;
case @vFHID
when 1000 then
	select nRst,@vFHID,@vAccountID;
else
	select nRst,@vFHID;
end case;
end
//
-- 删除定时事件数据，fixhour
drop procedure if exists `game_del_te_fixhour`;
create procedure game_del_te_fixhour(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_te_fixhour: begin

delete from te_fixhour where te_id=nTEID;
call game_del_te(nTEID,@vRst);

set nRst=0;
end label_del_te_fixhour;
end
//

-- 获取兵种战力
drop function if exists `game_strarmy_prof`;
create function game_strarmy_prof(szStr blob) returns int
begin

declare exit			handler for sqlexception		begin end;
declare exit			handler for sqlstate '22003'	begin end;

set @vWC=0;
select game_wc(szStr,'*') into @vWC;
set @vType=0;
set @vLevel=0;
set @vNum=0;
if (@vWC=1) then
call game_split_2(szStr,'*',@vLevel,@vNum);
elseif (@vWC=2) then
call game_split_3(szStr,'*',@vType,@vLevel,@vNum);
end if;

set @vProf=0;
select attack*@vNum+defense*@vNum*0.9+life*@vNum*0.8 into @vProf from excel_soldier where excel_id=1 and level=@vLevel;
set @vProf=ceiling(@vProf*1/3);

return @vProf;
end
//
drop function if exists `game_strarmy5_prof`;
create function game_strarmy5_prof(szStr1 blob, szStr2 blob, szStr3 blob, szStr4 blob, szStr5 blob) returns int
begin

declare exit			handler for sqlexception		begin end;
declare exit			handler for sqlstate '22003'	begin end;

set @vSumProf=0;
set @vSumProf=@vSumProf+game_strarmy_prof(szStr1);
set @vSumProf=@vSumProf+game_strarmy_prof(szStr2);
set @vSumProf=@vSumProf+game_strarmy_prof(szStr3);
set @vSumProf=@vSumProf+game_strarmy_prof(szStr4);
set @vSumProf=@vSumProf+game_strarmy_prof(szStr5);

return @vSumProf;
end
//
-- 获取兵种战力
drop procedure if exists `game_army_prof`;
create procedure game_army_prof(in nArmyType int, in nArmyLevel int, in nArmyNum int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_army_prof: begin

set @vProf=0;
select attack*nArmyNum+defense*nArmyNum*0.9+life*nArmyNum*0.8 into @vProf from excel_soldier where excel_id=nArmyType and level=nArmyLevel;

set @vProf=ceiling(@vProf*1/3);
set nRst=0;
end label_army_prof;

end
//
-- 获取战力
drop procedure if exists `game_combat_prof`;
create procedure game_combat_prof(in nClassID int, in nObjID bigint unsigned, in nCombatType int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_combat_prof: begin

set @vSum=0;

case
when nCombatType=1 or nCombatType=100 or nCombatType=101 then
	-- 军事任务/百战不殆/南征北战
	set @vArmyDeploy='';
	set @vArmyData='';
	
	case
	when nCombatType=1 then
		select army_deploy,army_data into @vArmyDeploy,@vArmyData from excel_combat_raid where raid_id=nObjID;
	when nCombatType=100 then
		CASE nClassID
		WHEN 1 THEN
			select army_deploy,army_data into @vArmyDeploy,@vArmyData from excel_baizhanbudai where level_id=nObjID;
		WHEN 2 THEN
			select army_deploy,army_data into @vArmyDeploy,@vArmyData from excel_baizhanbudai_epic where level_id=nObjID;
		ELSE
			select army_deploy,army_data into @vArmyDeploy,@vArmyData from excel_baizhanbudai where level_id=nObjID;
		END CASE;
	when nCombatType=101 then
		select army_deploy,army_data into @vArmyDeploy,@vArmyData from excel_nanzhengbeizhan where class_id=nClassID and level_id=nObjID;
	else
		set @vRst=0;
	end case;

	call game_split_5(@vArmyDeploy,'*',@vType1,@vType2,@vType3,@vType4,@vType5);
	call game_split_2(@vArmyData,'*',@vLevel,@vNum);
	-- 计算战力
	call game_army_prof(@vType1,@vLevel,@vNum,@vRst); set @vSum=@vSum+@vProf;
	call game_army_prof(@vType2,@vLevel,@vNum,@vRst); set @vSum=@vSum+@vProf;
	call game_army_prof(@vType3,@vLevel,@vNum,@vRst); set @vSum=@vSum+@vProf;
	call game_army_prof(@vType4,@vLevel,@vNum,@vRst); set @vSum=@vSum+@vProf;
	call game_army_prof(@vType5,@vLevel,@vNum,@vRst); set @vSum=@vSum+@vProf;
else
	set nRst=-2;
	leave label_combat_prof;
end case;

set nRst=0;
end label_combat_prof;
select nRst,floor(@vSum),@vType1,@vType2,@vType3,@vType4,@vType5;
end
//
-- 获取兵种数据
drop procedure if exists `game_load_army`;
create procedure game_load_army(out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_load_army: begin

select crush,bonus from excel_army order by id asc;

set nRst=0;
end label_load_army;
end
//

-- 增加草药
drop procedure if exists `game_add_drug`;
create procedure game_add_drug(in nAccountID bigint unsigned, in nNum int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_drug: begin

set @vDrugMax=0;
select T2.max into @vDrugMax from buildings T1 left join excel_drug T2 on T2.level=T1.level where T1.account_id=nAccountID and T1.excel_id=6;

set @vDrugCur=0;
case
when nNum=-1 then
	-- 补满
	update common_characters set drug=(@vChgNum:=@vDrugMax),is_drug_max=1 where account_id=nAccountID and (@vDrugCur:=drug)>=0;
when nNum=-2 then
	-- 补给1/24
	set nNum=@vDrugMax/24;
	update common_characters set drug=(@vChgNum:=if((drug+nNum)>=@vDrugMax,@vDrugMax,(drug+nNum))),is_drug_max=if((drug+nNum)>=@vDrugMax,1,0) where account_id=nAccountID and (@vDrugCur:=drug)>=0;
else
	update common_characters set drug=(@vChgNum:=if((drug+nNum)>=@vDrugMax,@vDrugMax,(drug+nNum))),is_drug_max=if((drug+nNum)>=@vDrugMax,1,0) where account_id=nAccountID and (@vDrugCur:=drug)>=0;
end case;

-- set @vChgNum=@vChgNum-@vDrugCur;
set nRst=0;
end label_add_drug;
if (nEcho=1) then
	select nRst,@vChgNum;
end if;
end
//
-- 刷新草药
drop procedure if exists `game_gen_drug`;
create procedure game_gen_drug(out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select account_id from common_characters where is_drug_max=0;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_gen_drug: begin

set @vAccountID='';
set nDone=0;
open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	call game_add_drug(nID,-2,0,@vRst);
	set @vAccountID=concat(@vAccountID,',',nID,',',@vChgNum);
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_gen_drug;
end
//

-- 行军加速
drop procedure if exists `game_army_accelerate`;
create procedure game_army_accelerate(in nAccountID bigint unsigned, in nCombatID bigint unsigned, in nType int, in nReqTime int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_army_accelerate: begin

set @vUseTime=0;
set @vBeginTime=0;
if not exists (select 1 from combats where combat_id=nCombatID and account_id=nAccountID and (@vBeginTime:=begin_time)>=0 and (@vUseTime:=use_time)>=0) then
	set nRst=-2;
	leave label_army_accelerate;
end if;
set @vTEID=0;
if not exists (select 1 from te_combat where combat_id=nCombatID and (@vTEID:=te_id)>0) then
	set nRst=-3;
	leave label_army_accelerate;
end if;

set @vResTime=@vUseTime+@vBeginTime-unix_timestamp();
set @vFinTime=if(@vResTime<nReqTime,@vResTime,nReqTime);
-- 
set @vCost1=0;
select val_int into @vCost1 from excel_cmndef where id=2;
set @vCostCrystal=ceiling(@vFinTime/@vCost1);

if (nType=2) then
-- 水晶够吗
update common_characters set crystal=crystal-@vCostCrystal where account_id=nAccountID and crystal>=@vCostCrystal;
if (row_count()=0) then
	set nRst=-4;
	leave label_army_accelerate;
end if;
else
-- 钻石
update common_characters set diamond=diamond-@vCostCrystal where account_id=nAccountID and diamond>=@vCostCrystal;
if (row_count()=0) then
	set nRst=-4;
	leave label_army_accelerate;
end if;
end if;

update time_events set end_time=begin_time+@vUseTime-@vFinTime where event_id=@vTEID;
update combats set use_time=use_time-@vFinTime where combat_id=nCombatID;

set nRst=0;
end label_army_accelerate;
select nRst,@vCostCrystal,@vFinTime;
end
//
-- 回城
drop procedure if exists `game_army_back`;
create procedure game_army_back(in nAccountID bigint unsigned, in nCombatID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_army_back: begin

set @vCombatType=0;
set @vMiscData='';
set @vBeginTime=0;
if not exists (select 1 from combats where combat_id=nCombatID and account_id=nAccountID and status=0 and (@vCombatType:=combat_type)>=0 and (@vMiscData:=misc_data) is not null and (@vBeginTime:=begin_time)>=0) then
	set nRst=-2;
	leave label_army_back;
end if;
set @vTEID=0;
if not exists (select 1 from te_combat where combat_id=nCombatID and (@vTEID:=te_id)>0) then
	set nRst=-3;
	leave label_army_back;
end if;

-- 
call game_del_te_combat(@vTEID,@vRst);

-- 获取返回时间
set @vBackTime=unix_timestamp()-@vBeginTime;
-- if (@vCombatType=2) then
--	call game_split_2(@vMiscData,'*',@vGoTime,@vBackTime);
-- else
--	select back_time into @vBackTime from excel_combat where excel_id=@vCombatType;
-- end if;

update combats set begin_time=unix_timestamp(),use_time=@vBackTime,status=2,combat_rst=0 where combat_id=nCombatID;
if (row_count()=0) then
	set nRst=-4;
	leave label_army_back;
end if;

-- 战斗返回超时 102
call game_add_te_combat(nCombatID,@vBackTime,102,0,@vRst);

set nRst=0;
end label_army_back;
select nRst,@vBackTime;
end
//

-- 重置多人副本
drop procedure if exists `game_reset_instance`;
create procedure game_reset_instance(out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select T1.instance_id from instance T1 left join excel_cmndef T2 on unix_timestamp()-T1.create_time>=T2.val_int where T2.id=4 and T1.excel_id=101;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_reset_instance: begin

set nDone=0;
open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	call game_destroy_instance(0,101,nID,1,0,@vRst);
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_reset_instance;
end
//

-- 重置多人副本
drop procedure if exists `game_reset_instance_wangzhe`;
create procedure game_reset_instance_wangzhe(out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select T1.instance_id from instance T1 left join excel_cmndef T2 on unix_timestamp()-T1.create_time>=T2.val_int where T2.id=26 and T1.excel_id=100;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_reset_instance: begin

set nDone=0;
open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	call game_destroy_instance(0,100,nID,1,0,@vRst);
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_reset_instance;
end
//

-- 刷新世界名城
drop procedure if exists `game_gen_worldcity`;
create procedure game_gen_worldcity(in nID bigint unsigned, in nClass int unsigned, in nPosX int unsigned, in nPosY int unsigned, in nCanBuild int unsigned, in nFloor2 int unsigned, in nTerrainType int unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_gen_worldcity: begin

-- ID重复
if exists (select 1 from world_city where id=nID) then
	set nRst=-2;
	leave label_gen_worldcity;
end if;
-- 坐标重复
if exists (select 1 from world_city where pos_x=nPosX and pos_y=nPosY) then
	set nRst=-3;
	leave label_gen_worldcity;
end if;
-- 类型错误
set @vRes=0;
set @vName='';
if not exists (select 1 from excel_worldcity_out where id=nClass and (@vRes:=total)>=0 and (@vName:=name) is not null) then
	set nRst=-4;
	leave label_gen_worldcity;
end if;
-- 兵力
set @vNum=1;
while @vNum<=5 do
set @vArmyType=floor(1+(rand()*5));
set @vArmyLevel=0;
set @vArmyNum=0;
set @vTmp='';
select army_data into @vTmp from excel_worldcity_combat where id=nClass;
set @vTmp=concat_ws('*',@vArmyType,@vTmp);

set @vQuery=concat('set @vArmy', @vNum, '=''', @vTmp, '''');
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

set @vNum=@vNum+1;
end while;

insert into world_city (id,class,pos_x,pos_y,res,can_build,floor2,terrain_type,army_1,army_2,army_3,army_4,army_5,combat_id,name) values (nID,nClass,nPosX,nPosY,@vRes,nCanBuild,nFloor2,nTerrainType,@vArmy1,@vArmy2,@vArmy3,@vArmy4,@vArmy5,'',@vName);

set nRst=0;
end label_gen_worldcity;
if (nEcho=1) then
select nRst;
end if;
end
//

-- 删除世界名城
drop procedure if exists `game_del_worldcity`;
create procedure game_del_worldcity(in nID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_worldcity: begin

if (nID>0) then
	delete from world_city where id=nID;
	call game_del_te_worldcityrecover_byid(nID,@vRst);
else
	delete from world_city;
	delete from time_events where type=120;
	delete from te_worldcityrecover;
end if;

set nRst=0;
end label_del_worldcity;
if (nEcho=1) then
select nRst;
end if;
end
//

-- 获得奖杯
drop procedure if exists `game_rob_cup`;
create procedure game_rob_cup(in nAccountID bigint unsigned, in nID bigint unsigned, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_rob_cup: begin

set @vCur=0;
select ifnull(max(id),0) into @vCur from world_city_history;
if (@vCur=0) then
	set nRst=-10;
	leave label_rob_cup;
end if;

-- 名城存在吗
set @vClass=0;
set @vPosX=0;
set @vPosY=0;
if not exists (select 1 from world_city where id=nID and (@vClass:=class)>0 and (@vPosX:=pos_x)>=0 and (@vPosY:=pos_y)>=0 and res>0) then
	set nRst=-2;
	leave label_rob_cup;
end if;
-- 单次产出量
set @vOne=0;
select one into @vOne from excel_worldcity_out where id=@vClass;
-- 
set @vRes=0;
update world_city set res=(@vRes:=res-@vOne),win_id=nAccountID where id=nID and res>=@vOne;
if (row_count()=0) then
	set nRst=-3;
	leave label_rob_cup;
end if;
-- 
set @vAllianceID=0;
update common_characters set cup=cup+@vOne,cup_history=cup_history+@vOne where account_id=nAccountID and (@vAllianceID:=alliance_id)>=0;
call game_add_item(nAccountID,0,3010,@vOne,0,@vRst);
-- 
insert into world_city_log (id,acct_id,alliance_id,cup) values (@vCur,nAccountID,@vAllianceID,@vOne) on duplicate key update alliance_id=@vAllianceID,cup=cup+@vOne;

set nRst=0;
end label_rob_cup;
if (nEcho=1) then
	select nRst,@vPosX,@vPosY,@vOne,@vRes;
end if;
end
//

-- 添加定时事件数据，worldcityrecover
drop procedure if exists `game_add_te_worldcityrecover`;
create procedure game_add_te_worldcityrecover(in nID bigint unsigned, in nTimeout int, in nType int, in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_add_te_worldcityrecover: begin

-- 添加定时事件索引
set @vRst=0;
call game_add_te(nTimeout,nType,0,@vRst);
if (@vRst!=0) then
	set nRst=-2;
	leave label_add_te_worldcityrecover;
end if;

insert into te_worldcityrecover (te_id,te_type,id) values (@vTEID,nType,nID);

set nRst=0;
end label_add_te_worldcityrecover;
if (nEcho=1) then
	select nRst,@vTEID;
end if;
end
//
-- 处理定时事件，worldcityrecover
drop procedure if exists `game_deal_te_worldcityrecover`;
create procedure game_deal_te_worldcityrecover(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_deal_te_worldcityrecover: begin

-- 恢复兵力
set @vID=0;
if not exists (select 1 from te_worldcityrecover where te_id=nTEID and (@vID:=id)>=0) then
	set nRst=-2;
	leave label_deal_te_worldcityrecover;
end if;

-- 
set @vTmp='';
select T1.army_data into @vTmp from excel_worldcity_combat T1 left join world_city T2 on T1.id=T2.class where T2.id=@vID;
set @vArmyLevel0=0;
set @vArmyNum0=0;
call game_split_2(@vTmp,'*',@vArmyLevel0,@vArmyNum0);

set @vNum=1;
while @vNum<=5 do
	set @vTmp='';
	set @vQuery=concat('select army_', @vNum, ' into @vTmp from world_city where id=', @vID);
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;

	if (length(@vTmp)>0) then
	set @vArmyType=0;
	set @vArmyLevel=0;
	set @vArmyNum=0;
	call game_split_3(@vTmp,'*',@vArmyType,@vArmyLevel,@vArmyNum);
	set @vTmp=concat_ws('*',@vArmyType,@vArmyLevel0,@vArmyNum0);

	set @vQuery=concat('update world_city set army_', @vNum, '=@vTmp where id=', @vID);
	prepare smt from @vQuery;
	execute smt;
	deallocate prepare smt;

	end if;

	set @vNum=@vNum+1;
end while;

-- 删除时间事件
call game_del_te_worldcityrecover(nTEID,@vRst);

set nRst=0;
end label_deal_te_worldcityrecover;
select nRst;
end
//
-- 删除定时事件数据，worldcityrecover
drop procedure if exists `game_del_te_worldcityrecover`;
create procedure game_del_te_worldcityrecover(in nTEID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_te_worldcityrecover: begin

delete from te_worldcityrecover where te_id=nTEID;
call game_del_te(nTEID,@vRst);

set nRst=0;
end label_del_te_worldcityrecover;
end
//
-- 删除定时事件数据，worldcityrecover
drop procedure if exists `game_del_te_worldcityrecover_byid`;
create procedure game_del_te_worldcityrecover_byid(in nID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_del_te_worldcityrecover_byid: begin

set @vTEID=0;
select te_id into @vTEID from te_worldcityrecover where id=nID;
call game_del_te_worldcityrecover(@vTEID,@vRst);

set nRst=0;
end label_del_te_worldcityrecover_byid;
end
//

-- 获取世界名城
drop procedure if exists `game_get_worldcity`;
create procedure game_get_worldcity(out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_worldcity: begin

select T1.id,T1.class,T1.pos_x,T1.pos_y,ifnull((T3.end_time-unix_timestamp()),0),T1.res,T1.name,game_strarmy_prof(T4.army_data)*5,T4.recover,game_strarmy5_prof(T1.army_1,T1.army_2,T1.army_3,T1.army_4,T1.army_5) from world_city T1 left join te_worldcityrecover T2 on T2.id=T1.id left join time_events T3 on T3.event_id=T2.te_id left join excel_worldcity_combat T4 on T4.id=T1.class where T1.res>0 order by T1.class asc;

set nRst=0;
end label_get_worldcity;
end
//
-- 获取世界名城活动结束时间
drop procedure if exists `game_get_worldcity_endtime`;
create procedure game_get_worldcity_endtime(out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_get_worldcity_endtime: begin

set @vRefreshTime=0;
set @vBonusTime=0;
select next_tick into @vRefreshTime from wait_deal_excel_time_event where excel_id=3;
select get_cup_activity_left_time() into @vBonusTime;

set @vCurTick=unix_timestamp();
set @vRefreshTime=if(@vRefreshTime>@vCurTick,@vRefreshTime-@vCurTick,0);
-- set @vBonusTime=if(@vBonusTime>@vCurTick,@vBonusTime-@vCurTick,0);
select @vRefreshTime,@vBonusTime;

set nRst=0;
end label_get_worldcity_endtime;
end
//

-- 刷新世界名城 初始化
drop procedure if exists `game_gen_worldcity_init`;
create procedure game_gen_worldcity_init(in nEcho int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_gen_worldcity_init: begin

set @vMax=0;
select ifnull(max(id),0)+1 into @vMax from world_city_history;

insert into world_city_history (id,create_time) values (@vMax,unix_timestamp());

set nRst=0;
end label_gen_worldcity_init;
if (nEcho=1) then
select nRst;
end if;
end
//

-- 获取奖杯个人战绩
drop procedure if exists `game_worldcity_log_man`;
create procedure game_worldcity_log_man(in nPageNum int, in nPageIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_worldcity_log_man: begin

set @vCur=0;
select ifnull(max(id),0) into @vCur from world_city_history;
if (@vCur<=1) then
	set nRst=0;
	leave label_worldcity_log_man;
end if;
set @vCur=@vCur-1;

set @vMaxPageNum=0;
select ceiling(count(1)/nPageNum) into @vMaxPageNum from world_city_log where id=@vCur and cup>0;

set @vQuery=concat('select ifnull(T2.account_id,0),ifnull(T2.name,0),ifnull(T2.head_id,0),T1.diamond,T1.cup from world_city_log T1 left join common_characters T2 on T2.account_id=T1.acct_id where T1.id=@vCur and T1.cup>0 order by diamond desc limit ', nPageNum*nPageIdx, ',', nPageNum);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

-- select @vMaxPageNum;
set nRst=0;
end label_worldcity_log_man;
end
//
-- 获取奖杯联盟战绩
drop procedure if exists `game_worldcity_log_alliance`;
create procedure game_worldcity_log_alliance(in nPageNum int, in nPageIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_worldcity_log_alliance: begin

set @vCur=0;
select ifnull(max(id),0) into @vCur from world_city_history;
if (@vCur<=1) then
	set nRst=0;
	leave label_worldcity_log_alliance;
end if;
set @vCur=@vCur-1;

set @vMaxPageNum=0;
select ceiling(count(distinct(alliance_id))/nPageNum) into @vMaxPageNum from world_city_log where id=@vCur and cup>0;

DROP TEMPORARY TABLE if exists `tmp_worldcity_log_alliance`;
create temporary table `tmp_worldcity_log_alliance`
(
	id bigint unsigned not null default 0,
	name char(32) not null default '',
	diamond int unsigned not null default 0,
	cup int unsigned not null default 0
);

set @vQuery=concat('insert into tmp_worldcity_log_alliance (select ifnull(T2.alliance_id,0),ifnull(T2.name,\'\'),sum(T1.diamond),sum(T1.cup) from world_city_log T1 inner join alliances T2 on T2.alliance_id=T1.alliance_id where T1.id=@vCur and T1.cup>0 group by T1.alliance_id order by sum(T1.diamond) desc limit ', nPageNum*nPageIdx, ',', nPageNum, ')');
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

select id,name,diamond,cup from tmp_worldcity_log_alliance;

-- select @vMaxPageNum;
set nRst=0;
end label_worldcity_log_alliance;
end
//

-- 获取当前奖杯个人排行榜
drop procedure if exists `game_worldcity_rank_man`;
create procedure game_worldcity_rank_man(in nPageNum int, in nPageIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_worldcity_rank_man: begin

set @vCur=0;
select ifnull(max(id),0) into @vCur from world_city_history;
if (@vCur=0) then
	set nRst=0;
	leave label_worldcity_rank_man;
end if;

set @vMaxPageNum=0;
select ceiling(count(1)/nPageNum) into @vMaxPageNum from world_city_log where id=@vCur and cup>0;

set @vQuery=concat('select ifnull(T2.account_id,0),ifnull(T2.name,0),ifnull(T2.head_id,0),ifnull(T2.level,0),T1.cup from world_city_log T1 left join common_characters T2 on T2.account_id=T1.acct_id where T1.id=@vCur and T1.cup>0 order by T1.cup desc,T2.level desc limit ', nPageNum*nPageIdx, ',', nPageNum);
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

-- select @vMaxPageNum;
set nRst=0;
end label_worldcity_rank_man;
end
//
-- 获取当前奖杯联盟排行榜
drop procedure if exists `game_worldcity_rank_alliance`;
create procedure game_worldcity_rank_alliance(in nPageNum int, in nPageIdx int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_worldcity_rank_alliance: begin

set @vCur=0;
select ifnull(max(id),0) into @vCur from world_city_history;
if (@vCur=0) then
	set nRst=0;
	leave label_worldcity_rank_alliance;
end if;

set @vMaxPageNum=0;
select ceiling(count(distinct(alliance_id))/nPageNum) into @vMaxPageNum from world_city_log where id=@vCur and cup>0;

DROP TEMPORARY TABLE if exists `tmp_worldcity_rank_alliance`;
create temporary table `tmp_worldcity_rank_alliance`
(
	id bigint unsigned not null default 0,
	name char(32) not null default '',
	level int unsigned not null default 0,
	cup int unsigned not null default 0
);

set @vQuery=concat('insert into tmp_worldcity_rank_alliance (select ifnull(T2.alliance_id,0),ifnull(T2.name,\'\'),ifnull(T3.level,0),sum(T1.cup) from world_city_log T1 inner join alliances T2 on T2.alliance_id=T1.alliance_id inner join alliance_buildings T3 on T3.alliance_id=T1.alliance_id and T3.excel_id=1 where T1.id=@vCur and T1.cup>0 group by T1.alliance_id order by sum(T1.cup) desc,T2.development desc limit ', nPageNum*nPageIdx, ',', nPageNum, ')');
prepare smt from @vQuery;
execute smt;
deallocate prepare smt;

select id,name,level,cup from tmp_worldcity_rank_alliance;

-- select @vMaxPageNum;
set nRst=0;
end label_worldcity_rank_alliance;
end
//
-- 售出奖杯
drop procedure if exists `game_sale_cup`;
create procedure game_sale_cup(in nAccountID bigint unsigned, in nNum int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_sale_cup: begin

if not exists (select 1 from common_characters where account_id=nAccountID and cup>=nNum) then
	set nRst=-2;
	leave label_sale_cup;
end if;

set @vPrice=0;
select val_int into @vPrice from excel_cmndef where id=12;

update common_characters set cup=cup-nNum,gold=gold+nNum*@vPrice where account_id=nAccountID and cup>=nNum;
if (row_count()=0) then
	set nRst=-2;
	leave label_sale_cup;
end if;

set nRst=0;
end label_sale_cup;
select nRst;
end
//
-- 世界名城活动结算
drop procedure if exists `game_gen_worldcity_fini`;
create procedure game_gen_worldcity_fini(in nEcho int, out nRst int)
begin

declare nWCID			int default 0;
declare nDone			int default 0;
declare nAID			bigint unsigned default 0;
declare nID				bigint unsigned default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursorA		cursor for select ifnull(T2.alliance_id,0) from world_city_log T1 left join alliances T2 on T2.alliance_id=T1.alliance_id where T1.id=nWCID group by T1.alliance_id order by sum(T1.cup) desc,T2.development desc limit 20;
declare vCursor			cursor for select acct_id from world_city_log where id=nWCID;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_worldcity_fini: begin

set @vCurWorldCityID=0;
set nWCID=0;
select ifnull(max(id),0) into nWCID from world_city_history;
if (nWCID=0) then
	set nRst=0;
	leave label_worldcity_fini;
end if;
set @vCurWorldCityID=nWCID;

-- 奖励发放范围
set @vNum=0;
select val_int into @vNum from excel_cmndef where id=20;
set @vTD=0;
select val_int into @vTD from excel_cmndef where id=19;

-- 
set @vIdx=1;
set @vTC=0;
set nDone=0;
-- 计算参与奖励分配联盟的奖杯数
open vCursorA;
repeat
fetch vCursorA into nID;
if not nDone then
	set @_vTC=0;
	if (nID>0) then
		select sum(cup) into @_vTC from world_city_log where id=nWCID and alliance_id=nID;
		set @vTC=@vTC+@_vTC;

		set @vIdx=@vIdx+1;
		set nDone=0;
		if (@vIdx>@vNum) then
			set nDone=1;
		end if;
	end if;
end if;
until nDone end repeat;
close vCursorA;

-- 奖励分配
set @vIdx=1;
set nDone=0;
open vCursorA;
repeat
fetch vCursorA into nID;
if not nDone then
	if (nID>0) then
		-- 按比例发放
		update world_city_log set diamond=ceiling(cup/@vTC*@vTD) where id=nWCID and alliance_id=nID;
	
		set @vIdx=@vIdx+1;
		set nDone=0;
		
		if (@vIdx>@vNum) then
			set nDone=1;
		end if;
	end if;
end if;
until nDone end repeat;
close vCursorA;

-- 删除奖杯
set @vAcct='';
set nDone=0;
open vCursor;
repeat
fetch vCursor into nID;
if not nDone then
	set @vItemID=0;
	set @vNum=0;
	select item_id,num into @vItemID,@vNum from items where account_id=nID and excel_id=3010;
	call game_del_item(nID,@vItemID,@vNum,0,@vRst);
	update common_characters set cup=0 where account_id=nID;
	update world_city_log set cup=@vNum where id=nWCID and acct_id=nID;
	set @vAcct=concat(@vAcct,',',nID);
	
	set nDone=0;
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_worldcity_fini;
if (nEcho=1) then
select nRst,@vAcct;
end if;
end
//
-- 发送奖励邮件
drop procedure if exists `game_worldcity_bonus`;
create procedure game_worldcity_bonus(in nEcho int, out nRst int)
begin

declare nDone			int default 0;
declare nID				bigint unsigned default 0;
declare nDiamond		int default 0;
-- 在cursor里用到的变量只能显式declare
declare vCursor			cursor for select acct_id,diamond from world_city_log where id=@vCurWorldCityID and diamond>0;
declare continue        handler for sqlstate '02000'    begin set nDone=1; end;

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_worldcity_bonus: begin

set @vMail='';

-- 标题
set @vTitle='';
select content into @vTitle from excel_text where excel_id=463;
set @vTitle=concat('[FFFF00]',@vTitle);
-- 奖励邮件
set nDone=0;
open vCursor;
repeat
fetch vCursor into nID,nDiamond;
if not nDone then
	set @vData=concat('2*',nDiamond,'*0,');
	call add_private_mail(0,nID,4,3,0,@vTitle,@vData,0,@vRst,@vMailID);
	if (@vRst=0) then set @vMail=concat(@vMail,',',nID,',',@vMailID); end if;
end if;
until nDone end repeat;
close vCursor;

set nRst=0;
end label_worldcity_bonus;
if (nEcho=1) then
select nRst,@vMail;
end if;
end
//
-- 修改当前奖杯排行信息，当玩家变换联盟时
drop procedure if exists `game_update_cup`;
create procedure game_update_cup(in nAccountID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_update_cup: begin

set @vCur=0;
select ifnull(max(id),0) into @vCur from world_city_history;
if (@vCur=0) then
	set nRst=0;
	leave label_update_cup;
end if;

set @vAllianceID=0;
select alliance_id into @vAllianceID from common_characters where account_id=nAccountID;
update world_city_log set alliance_id=@vAllianceID where id=@vCur and acct_id=nAccountID;

set nRst=0;
end label_update_cup;
end
//

-- 名城被打掉后的扫尾工作
drop procedure if exists `game_rob_end`;
create procedure game_rob_end(in nID bigint unsigned, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_rob_end: begin

select combat_id from world_city where id=nID;
delete from world_city where id=nID;

set nRst=0;
end label_rob_end;
end
//

-- 找出指定时段内的充值信息
drop procedure if exists `game_rechargeinfo`;
create procedure game_rechargeinfo(in szBeginTime char(32), in szEndTime char(32), out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_rechargeinfo: begin

set @vBeginTick=unix_timestamp(szBeginTime);
set @vEndTick=unix_timestamp(szEndTime);
set @vCurTick=unix_timestamp();

select T1.account_id,T2.name,sum(T1.val) from billslog T1 inner join common_characters T2 on T2.account_id=T1.account_id where donetime between @vBeginTick and @vEndTick and T1.val is not null group by T1.account_id order by sum(T1.val) desc;

set nRst=0;
end label_rechargeinfo;
end
//

-- 尝试充值记录
drop procedure if exists `game_recharge_try`;
create procedure game_recharge_try(in nAccountID bigint unsigned, in nDiamond int, out nRst int)
begin

declare exit            handler for sqlexception        begin end;
declare exit            handler for sqlstate '22003'    begin end;

set nRst=-1;
label_recharge_try: begin

insert into recharge_try (acct_id,time,diamond) values (nAccountID,unix_timestamp(),nDiamond);

set nRst=0;
end label_recharge_try;
select nRst;
end
//


-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- 健康度对武将属性的影响
drop trigger if exists `game_health_hireheros`;
create trigger game_health_hireheros before update on hire_heros
for each row
begin

-- 健康度和其他属性不能同时修改

if (new.healthstate!=old.healthstate) then
-- 只改变健康度
set new.healthstate=if(new.healthstate>=100,100,new.healthstate);
-- set new.healthstate=if(new.healthstate<=10,10,new.healthstate);
-- set @vPct=new.healthstate/old.healthstate;
set @vPct=1;

set new.attack=floor(new.attack*@vPct);
set new.attack_add=floor(new.attack_add*@vPct);
set new.defense=floor(new.defense*@vPct);
set new.defense_add=floor(new.defense_add*@vPct);
set new.health=floor(new.health*@vPct);
set new.health_add=floor(new.health_add*@vPct);
set new.leader=floor(new.leader*@vPct);
set new.leader_add=floor(new.leader_add*@vPct);

else
-- 只改变其他属性
-- set @vPct=new.healthstate/100;
set @vPct=1;

if (new.attack!=old.attack) then
set new.attack=floor(new.attack*@vPct);
end if;
if (new.attack_add!=old.attack_add) then
set new.attack_add=floor(new.attack_add*@vPct);
end if;
if (new.defense!=old.defense) then
set new.defense=floor(new.defense*@vPct);
end if;
if (new.defense_add!=old.defense_add) then
set new.defense_add=floor(new.defense_add*@vPct);
end if;
if (new.health!=old.health) then
set new.health=floor(new.health*@vPct);
end if;
if (new.health_add!=old.health_add) then
set new.health_add=floor(new.health_add*@vPct);
end if;
if (new.leader!=old.leader) then
set new.leader=floor(new.leader*@vPct);
end if;
if (new.leader_add!=old.leader_add) then
set new.leader_add=floor(new.leader_add*@vPct);
end if;

end if;

end;
//





delimiter ;



