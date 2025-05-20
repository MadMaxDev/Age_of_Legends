-- ----------------------------
-- Table structure for `common_accounts`
-- ----------------------------
DROP TABLE IF EXISTS `common_accounts`;
CREATE TABLE `common_accounts` (
  `name` varchar(32) NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `pass` varchar(256) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL DEFAULT '',
  `last_login_time` int(10) unsigned NOT NULL DEFAULT '0',
  `last_logout_time` int(10) unsigned NOT NULL DEFAULT '0',
  `device_id` varchar(64) DEFAULT NULL,
  `device_type` int(10) unsigned NOT NULL DEFAULT '0',
  `binded` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `create_time` int(10) unsigned NOT NULL,
  `appid` int(10) unsigned NOT NULL,
  `banned` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `group_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`),
  UNIQUE KEY `account_name` (`name`),
  UNIQUE KEY `device` (`device_id`,`group_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `banned_device`
-- ----------------------------
DROP TABLE IF EXISTS `banned_device`;
CREATE TABLE `banned_device` (
  `device_id` varchar(64) NOT NULL,
  PRIMARY KEY (`device_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guid`
-- ----------------------------
-- CREATE TABLE IF NOT EXISTS `guid` (
--  `guid` bigint(20) unsigned NOT NULL
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `notification`
-- ----------------------------
DROP TABLE IF EXISTS `notification`;
CREATE TABLE `notification` (
  `id` int(10) unsigned NOT NULL,
  `group_id` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  `title` blob NOT NULL,
  `content` blob NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `notification_id`
-- ----------------------------
CREATE TABLE IF NOT EXISTS `notification_id` (
  `id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 64位ID生成 --
create table global_id (
	svrgrp_id bigint not null default 0,
	dbs_id bigint not null default 0,
	asc_id bigint not null default 0,
	last_gen_time int unsigned not null default 0,
	asc_id_in1s int not null default 0
)engine=innodb default charset=utf8;

-- ----------------------------
-- Table structure for `gifts`
-- ----------------------------
DROP TABLE IF EXISTS `gifts`;
CREATE TABLE `gifts` (
  `device_id` varchar(64) NOT NULL,
  `appid` int(10) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`device_id`,`appid`),
  UNIQUE KEY `account` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `adcolony_logs`
-- ----------------------------
DROP TABLE IF EXISTS `adcolony_logs`;
CREATE TABLE `adcolony_logs` (
  `transaction_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `amount` int(10) unsigned DEFAULT NULL,
  `name` varchar(32) DEFAULT NULL,
  `time` int(10) unsigned DEFAULT NULL,
  `account_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`transaction_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `adcolony_transactions`
-- ----------------------------
DROP TABLE IF EXISTS `adcolony_transactions`;
CREATE TABLE `adcolony_transactions` (
  `transaction_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `amount` int(10) unsigned DEFAULT NULL,
  `name` varchar(32) DEFAULT NULL,
  `time` int(10) unsigned DEFAULT NULL,
  `account_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`transaction_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
