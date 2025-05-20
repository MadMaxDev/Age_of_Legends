-- ----------------------------
-- Table structure for `alliances`
-- ----------------------------
DROP TABLE IF EXISTS `alliances`;
CREATE TABLE `alliances` (
  `alliance_id` bigint(20) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `development` int(10) unsigned NOT NULL,
  `introduction` blob NOT NULL,
  `total_development` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`alliance_id`),
  UNIQUE KEY `leader` USING BTREE (`account_id`),
  UNIQUE KEY `uniname` (`name`),
  KEY `t_dev` (`total_development`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alliance_members`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_members`;
CREATE TABLE `alliance_members` (
  `alliance_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `position` int(10) unsigned NOT NULL,
  `development` int(10) unsigned NOT NULL default '0',
  `total_development` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`),
  KEY `alliance` (`alliance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `buildings`
-- ----------------------------
DROP TABLE IF EXISTS `buildings`;
CREATE TABLE `buildings` (
  `account_id` bigint(20) unsigned NOT NULL,
  `auto_id` int(10) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`auto_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `building_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `building_time_events`;
CREATE TABLE `building_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `auto_id` int(10) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`event_id`),
  UNIQUE KEY `uni_key` USING BTREE (`account_id`,`auto_id`,`type`),
  KEY `account` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `common_characters`
-- ----------------------------
DROP TABLE IF EXISTS `common_characters`;
CREATE TABLE `common_characters` (
  `account_id` bigint(20) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `last_login_time` int(10) unsigned NOT NULL DEFAULT '0',
  `last_logout_time` int(10) unsigned NOT NULL DEFAULT '0',
  `exp` int(10) unsigned NOT NULL DEFAULT '0',
  `level` int(10) unsigned NOT NULL DEFAULT '0',
  `diamond` int(10) unsigned NOT NULL DEFAULT '0',
  `crystal` int(10) unsigned NOT NULL DEFAULT '0',
  `vip` int(10) unsigned NOT NULL DEFAULT '0',
  `development` int(10) unsigned NOT NULL DEFAULT '0',
  `alliance_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `sex` int(10) unsigned NOT NULL DEFAULT '0',
  `head_id` int(10) unsigned NOT NULL DEFAULT '0',
  `pos_x` int(10) unsigned NOT NULL,
  `pos_y` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL DEFAULT '0',
  `population` int(10) unsigned NOT NULL DEFAULT '0',
  `build_num` tinyint(1) unsigned NOT NULL,
  `added_build_num` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `drug` int(10) unsigned NOT NULL DEFAULT '0',
  `is_drug_max` int(10) unsigned NOT NULL DEFAULT '0',
  `draw_lottery_date` date NOT NULL DEFAULT '0000-00-00',
  `draw_lottery_num` int(10) unsigned NOT NULL DEFAULT '0',
  `trade_date` date NOT NULL DEFAULT '0000-00-00',
  `trade_num` int(10) unsigned NOT NULL DEFAULT '0',
  `total_diamond` int(10) unsigned NOT NULL DEFAULT '0',
  `signature` blob,
  `change_name_num` int(10) unsigned NOT NULL DEFAULT '0',
  `protect_time` int(10) unsigned NOT NULL DEFAULT '0',
  `notification_id` int(10) unsigned NOT NULL DEFAULT '0',
  `alliance_lottery_date` date NOT NULL DEFAULT '0000-00-00',
  `alliance_lottery_num` int(10) unsigned NOT NULL DEFAULT '0',
  `instance_wangzhe` int(10) unsigned NOT NULL DEFAULT '0',
  `instance_zhengzhan` int(10) unsigned NOT NULL DEFAULT '0',
  `pvp_attack` int(10) unsigned NOT NULL DEFAULT '0',
  `pvp_attack_win` int(10) unsigned NOT NULL DEFAULT '0',
  `pvp_defense` int(10) unsigned NOT NULL DEFAULT '0',
  `pvp_defense_win` int(10) unsigned NOT NULL DEFAULT '0',
  `pvp_done_tick` int(10) unsigned NOT NULL DEFAULT '0',
  `pvp_attack_win_day` int(10) unsigned NOT NULL DEFAULT '0',
  `pvp_defense_win_day` int(10) unsigned NOT NULL DEFAULT '0',
  `create_time` int(10) unsigned NOT NULL DEFAULT '0',
  `cup` int(10) unsigned NOT NULL DEFAULT '0',
  `cup_history` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_1` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_2` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_3` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_4` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_5` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_6` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_7` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_8` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_9` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_10` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_11` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_12` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_13` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_14` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_15` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_16` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_17` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_18` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_19` int(10) unsigned NOT NULL DEFAULT '0',
  `zhengzhan_20` int(10) unsigned NOT NULL DEFAULT '0',
  `ip` int(10) unsigned NOT NULL DEFAULT '0',
  `binded` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `today_online_time` int(10) unsigned NOT NULL DEFAULT '0',
  `total_online_time` int(10) unsigned NOT NULL DEFAULT '0',
  `banned` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `vip_display` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`account_id`),
  UNIQUE KEY `char_name` (`name`),
  UNIQUE KEY `position` (`pos_x`,`pos_y`),
  KEY `alliance` (`alliance_id`),
  KEY `idx_create_time` (`create_time`),
  KEY `idx_last_logout` (`last_logout_time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_barrack_production`
-- ----------------------------
DROP TABLE IF EXISTS `excel_barrack_production`;
CREATE TABLE `excel_barrack_production` (
  `level` int(10) unsigned NOT NULL,
  `capacity` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_building_level_limit`
-- ----------------------------
DROP TABLE IF EXISTS `excel_building_level_limit`;
CREATE TABLE `excel_building_level_limit` (
  `development` int(10) unsigned NOT NULL,
  `castle_max_level` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`development`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_building_list`
-- ----------------------------
DROP TABLE IF EXISTS `excel_building_list`;
CREATE TABLE `excel_building_list` (
  `excel_id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `build_time` int(10) unsigned NOT NULL,
  `build_gold` int(10) unsigned NOT NULL,
  `build_population` int(10) unsigned NOT NULL,
  `exp` int(10) unsigned NOT NULL,
  `development` int(10) unsigned NOT NULL,
  `max_num` int(10) unsigned NOT NULL,
  `auto_id_begin` int(10) unsigned NOT NULL,
  `auto_id_end` int(10) unsigned NOT NULL,
  `auto_build` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`excel_id`,`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_building_num_limit`
-- ----------------------------
DROP TABLE IF EXISTS `excel_building_num_limit`;
CREATE TABLE `excel_building_num_limit` (
  `char_level` int(10) unsigned NOT NULL,
  `goldore_max_num` int(10) unsigned NOT NULL,
  `house_max_num` int(10) unsigned NOT NULL,
  `barrack_max_num` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`char_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_goldore_production`
-- ----------------------------
DROP TABLE IF EXISTS `excel_goldore_production`;
CREATE TABLE `excel_goldore_production` (
  `level` int(10) unsigned NOT NULL,
  `produce_time_0` int(10) unsigned NOT NULL,
  `production_0` int(10) unsigned NOT NULL,
  `produce_time_1` int(10) unsigned NOT NULL,
  `production_1` int(10) unsigned NOT NULL,
  `produce_time_2` int(10) unsigned NOT NULL,
  `production_2` int(10) unsigned NOT NULL,
  `produce_time_3` int(10) unsigned NOT NULL,
  `production_3` int(10) unsigned NOT NULL,
  `can_steal` int(10) unsigned NOT NULL,
  `steal_percent` float unsigned NOT NULL,
  `steal_num` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_house_production`
-- ----------------------------
DROP TABLE IF EXISTS `excel_house_production`;
CREATE TABLE `excel_house_production` (
  `level` int(10) unsigned NOT NULL,
  `production` int(10) unsigned NOT NULL,
  `produce_time` int(10) unsigned NOT NULL,
  `capacity` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_item_list`
-- ----------------------------
DROP TABLE IF EXISTS `excel_item_list`;
CREATE TABLE `excel_item_list` (
  `excel_id` int(10) unsigned NOT NULL DEFAULT '0',
  `name` char(64) NOT NULL DEFAULT '',
  `description` blob NOT NULL,
  `item_type` int(10) unsigned NOT NULL DEFAULT '0',
  `file_name` char(64) NOT NULL DEFAULT '',
  `price` int(10) unsigned NOT NULL DEFAULT '0',
  `attack` int(10) unsigned NOT NULL DEFAULT '0',
  `defense` int(10) unsigned NOT NULL DEFAULT '0',
  `health` int(10) unsigned NOT NULL DEFAULT '0',
  `leader` int(10) unsigned NOT NULL DEFAULT '0',
  `val_int` int(10) unsigned NOT NULL DEFAULT '0',
  `val_str` blob NOT NULL,
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `lot_id` int(10) unsigned NOT NULL DEFAULT '0',
  `level` int(10) unsigned NOT NULL DEFAULT '0',
  `slot_num` int(10) unsigned NOT NULL DEFAULT '0',
  `equip_type` int(10) unsigned NOT NULL DEFAULT '0',
  `slot_type` int(10) unsigned NOT NULL DEFAULT '0',
  `theroy_diamond` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_soldier`
-- ----------------------------
DROP TABLE IF EXISTS `excel_soldier`;
CREATE TABLE `excel_soldier` (
  `excel_id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  `attack` int(10) unsigned NOT NULL,
  `defense` int(10) unsigned NOT NULL,
  `life` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`excel_id`,`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_store_conversion_rate`
-- ----------------------------
DROP TABLE IF EXISTS `excel_store_conversion_rate`;
CREATE TABLE `excel_store_conversion_rate` (
  `money_type` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`money_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_store_item`
-- ----------------------------
DROP TABLE IF EXISTS `excel_store_item`;
CREATE TABLE `excel_store_item` (
  `excel_id` int(10) unsigned NOT NULL,
  `crystal` int(10) unsigned NOT NULL,
  `diamond` int(10) unsigned NOT NULL,
  `can_crystal_buy` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_technology`
-- ----------------------------
DROP TABLE IF EXISTS `excel_technology`;
CREATE TABLE `excel_technology` (
  `level` int(10) unsigned NOT NULL,
  `institute_level` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  `time` float unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_text`
-- ----------------------------
DROP TABLE IF EXISTS `excel_text`;
CREATE TABLE `excel_text` (
  `excel_id` int(10) unsigned NOT NULL,
  `content` varchar(256) NOT NULL,
  PRIMARY KEY  (`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `once_perperson_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `once_perperson_time_events`;
CREATE TABLE `once_perperson_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  PRIMARY KEY (`event_id`),
  UNIQUE KEY `account` (`account_id`,`type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `online_characters`
-- ----------------------------
DROP TABLE IF EXISTS `online_characters`;
CREATE TABLE `online_characters` (
  `account_id` bigint(20) NOT NULL,
  `name` varchar(32) NOT NULL,
  `login_time` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `production_events`
-- ----------------------------
DROP TABLE IF EXISTS `production_events`;
CREATE TABLE `production_events` (
  `account_id` bigint(20) unsigned NOT NULL,
  `auto_id` int(10) unsigned NOT NULL,
  `production` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`auto_id`,`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `production_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `production_time_events`;
CREATE TABLE `production_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `auto_id` int(10) unsigned NOT NULL,
  `production` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`event_id`),
  UNIQUE KEY `unikey` (`account_id`,`auto_id`,`type`),
  KEY `account` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `relation_events`
-- ----------------------------
DROP TABLE IF EXISTS `relation_events`;
CREATE TABLE `relation_events` (
  `account_id` bigint(20) unsigned NOT NULL,
  `auto_id` int(10) unsigned NOT NULL,
  `p_account_id` bigint(20) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  PRIMARY KEY (`account_id`,`auto_id`,`p_account_id`,`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `research_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `research_time_events`;
CREATE TABLE `research_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`event_id`),
  UNIQUE KEY `unikey` USING BTREE (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `soldiers`
-- ----------------------------
DROP TABLE IF EXISTS `soldiers`;
CREATE TABLE `soldiers` (
  `account_id` bigint(20) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `num` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`excel_id`,`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `technologys`
-- ----------------------------
DROP TABLE IF EXISTS `technologys`;
CREATE TABLE `technologys` (
  `account_id` bigint(20) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `time_events`
-- ----------------------------
DROP TABLE IF EXISTS `time_events`;
CREATE TABLE `time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `begin_time` int(10) unsigned NOT NULL,
  `end_time` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `locked` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`event_id`),
  KEY `deadline` (`end_time`),
  KEY `bigtype` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alliance_buildings`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_buildings`;
CREATE TABLE `alliance_buildings` (
  `alliance_id` bigint(20) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`alliance_id`,`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alliance_building_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_building_time_events`;
CREATE TABLE `alliance_building_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `alliance_id` bigint(20) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`event_id`),
  UNIQUE KEY `unikey` (`alliance_id`,`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alliance_join_events`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_join_events`;
CREATE TABLE `alliance_join_events` (
  `account_id` bigint(20) unsigned NOT NULL,
  `alliance_id` bigint(20) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`),
  KEY `alliance` (`alliance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_accelerate_cost`
-- ----------------------------
DROP TABLE IF EXISTS `excel_accelerate_cost`;
CREATE TABLE `excel_accelerate_cost` (
  `time` int(10) unsigned NOT NULL,
  `diamond` int(10) unsigned NOT NULL,
  `crystal` int(10) unsigned NOT NULL,
  PRIMARY KEY (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_alliance_building_list`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alliance_building_list`;
CREATE TABLE `excel_alliance_building_list` (
  `excel_id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `build_time` int(10) unsigned NOT NULL,
  `build_development` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`excel_id`,`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_alliance_congress`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alliance_congress`;
CREATE TABLE `excel_alliance_congress` (
  `level` int(10) unsigned NOT NULL,
  `development` int(10) unsigned NOT NULL,
  `member_num` int(10) unsigned NOT NULL,
  `vice_leader_num` int(10) unsigned NOT NULL,
  `manager_num` int(10) unsigned NOT NULL,
  `added_trade_rate` float unsigned NOT NULL default '0',
  `max_instance_level` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_alliance_contribute_rate`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alliance_contribute_rate`;
CREATE TABLE `excel_alliance_contribute_rate` (
  `gold` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`gold`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_alliance_create_cost`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alliance_create_cost`;
CREATE TABLE `excel_alliance_create_cost` (
  `gold` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`gold`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_alliance_development`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alliance_development`;
CREATE TABLE `excel_alliance_development` (
  `action_id` int(10) unsigned NOT NULL,
  `illustration` varchar(32) NOT NULL,
  `development` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`action_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_alliance_position_right`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alliance_position_right`;
CREATE TABLE `excel_alliance_position_right` (
  `excel_id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `build` int(10) unsigned NOT NULL,
  `abdicate` int(11) NOT NULL,
  `dismiss` int(10) unsigned NOT NULL,
  `operate_event` int(10) unsigned NOT NULL,
  `designate_vice_leader` int(10) unsigned NOT NULL,
  `designate_manager` int(10) unsigned NOT NULL,
  `expel_member` int(10) unsigned NOT NULL,
  `invite_member` int(10) unsigned NOT NULL,
  `approve_member` int(10) unsigned NOT NULL,
  `send_mail` int(11) unsigned NOT NULL,
  `can_change_introduction` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_vip_right`
-- ----------------------------
DROP TABLE IF EXISTS `excel_vip_right`;
CREATE TABLE `excel_vip_right` (
  `level` int(10) unsigned NOT NULL,
  `total_diamond` int(10) unsigned NOT NULL DEFAULT '0',
  `mail_num` int(10) unsigned NOT NULL,
  `friend_num` int(10) unsigned NOT NULL,
  `send_mail_num` int(10) unsigned NOT NULL,
  `added_build_num` int(10) unsigned NOT NULL,
  `goldore_protection_interval` int(10) unsigned NOT NULL,
  `christmas_tree_ripe_interval` int(10) unsigned NOT NULL,
  `instance_day_times_free` int unsigned not null default 0,
  `instance_day_times_fee` int unsigned not null default 0,
  `army_accelerate` float(32,2) not null default 0,
  `pvp_queue` int unsigned not null default 0,
  PRIMARY KEY (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `mail_id`
-- ----------------------------
DROP TABLE IF EXISTS `mail_id`;
CREATE TABLE `mail_id` (
  `account_id` bigint(20) unsigned NOT NULL,
  `mail_id` int(10) unsigned NOT NULL,
  `today_send_num` int(10) unsigned NOT NULL,
  `last_send_day` date NOT NULL default '0000-00-00',
  PRIMARY KEY  (`account_id`,`mail_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `private_mails`
-- ----------------------------
DROP TABLE IF EXISTS `private_mails`;
CREATE TABLE `private_mails` (
  `account_id` bigint(20) unsigned NOT NULL,
  `mail_id` int(10) unsigned NOT NULL,
  `sender_id` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `flag` tinyint(3) unsigned NOT NULL,
  `readed` tinyint(3) unsigned NOT NULL,
  `text` blob,
  `time` int(10) unsigned NOT NULL,
  `ext_data` blob,
  PRIMARY KEY (`account_id`,`mail_id`),
  KEY `idx_time` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_character_template`
-- ----------------------------
DROP TABLE IF EXISTS `excel_character_template`;
CREATE TABLE `excel_character_template` (
  `build_num` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  `crystal` int(10) unsigned NOT NULL,
  `diamond` int(10) unsigned NOT NULL,
  `population` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`build_num`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `personal_relations`
-- ----------------------------
DROP TABLE IF EXISTS `personal_relations`;
CREATE TABLE `personal_relations` (
  `account_id` bigint(20) unsigned NOT NULL,
  `peer_account_id` bigint(20) unsigned NOT NULL,
  `relation_type` int(10) unsigned NOT NULL,
  PRIMARY KEY (`account_id`,`peer_account_id`,`relation_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `friend_apply_events`
-- ----------------------------
DROP TABLE IF EXISTS `friend_apply_events`;
CREATE TABLE `friend_apply_events` (
  `account_id` bigint(20) unsigned NOT NULL,
  `peer_account_id` bigint(20) NOT NULL,
  PRIMARY KEY  (`account_id`,`peer_account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_terrain_info`
-- ----------------------------
DROP TABLE IF EXISTS `excel_terrain_info`;
CREATE TABLE `excel_terrain_info` (
  `pos_x` int(10) unsigned NOT NULL,
  `pos_y` int(10) unsigned NOT NULL,
  `can_build` tinyint(3) unsigned NOT NULL,
  `floor_1` tinyint(3) unsigned NOT NULL,
  `floor_2` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`pos_y`,`pos_x`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_char_level_exp`
-- ----------------------------
DROP TABLE IF EXISTS `excel_char_level_exp`;
CREATE TABLE `excel_char_level_exp` (
  `level` int(10) unsigned NOT NULL,
  `exp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
-- ----------------------------
-- Table structure for `alliance_logs`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_logs`;
CREATE TABLE `alliance_logs` (
  `alliance_id` bigint(20) unsigned NOT NULL,
  `log_id` int(10) unsigned NOT NULL,
  `text` varchar(256) NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`alliance_id`,`log_id`),
  KEY `time_idx` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alliance_log_id`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_log_id`;
CREATE TABLE `alliance_log_id` (
  `alliance_id` bigint(20) unsigned NOT NULL,
  `log_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`alliance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alliance_mails`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_mails`;
CREATE TABLE `alliance_mails` (
  `alliance_id` bigint(20) unsigned NOT NULL,
  `mail_id` int(10) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `text` varchar(1024) NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`alliance_id`,`mail_id`),
  KEY `account` (`alliance_id`),
  KEY `time_idx` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alliance_mail_id`
-- ----------------------------
DROP TABLE IF EXISTS `alliance_mail_id`;
CREATE TABLE `alliance_mail_id` (
  `alliance_id` bigint(20) unsigned NOT NULL,
  `mail_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`alliance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `private_logs`
-- ----------------------------
DROP TABLE IF EXISTS `private_logs`;
CREATE TABLE `private_logs` (
  `account_id` bigint(20) unsigned NOT NULL,
  `log_id` int(10) unsigned NOT NULL,
  `text` varchar(256) NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`log_id`),
  KEY `time_idx` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `private_log_id`
-- ----------------------------
DROP TABLE IF EXISTS `private_log_id`;
CREATE TABLE `private_log_id` (
  `account_id` bigint(20) unsigned NOT NULL,
  `log_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_christmas_tree`
-- ----------------------------
DROP TABLE IF EXISTS `excel_christmas_tree`;
CREATE TABLE `excel_christmas_tree` (
  `char_level` int(10) unsigned NOT NULL,
  `ripe_watering_num` int(10) unsigned NOT NULL,
  `self_watering_interval` int(10) unsigned NOT NULL,
  `ripe_time_interval` int(10) unsigned NOT NULL,
  `award_crystal_string` varchar(1024) NOT NULL,
  `award_item_string` varchar(1024) NOT NULL,
  `last_award_crystal` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`char_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `relation_logs`
-- ----------------------------
DROP TABLE IF EXISTS `relation_logs`;
CREATE TABLE `relation_logs` (
  `account_id` bigint(20) unsigned NOT NULL,
  `log_id` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `p_account_id` bigint(20) unsigned NOT NULL,
  `text` varchar(256) NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`account_id`,`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `buildings_ext_data`
-- ----------------------------
DROP TABLE IF EXISTS `buildings_ext_data`;
CREATE TABLE `buildings_ext_data` (
  `account_id` bigint(20) unsigned NOT NULL,
  `auto_id` int(10) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `data_0` int(10) unsigned NOT NULL default '0',
  `data_1` int(10) unsigned NOT NULL default '0',
  `data_2` int(10) unsigned NOT NULL default '0',
  `data_3` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`auto_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_hero_level_exp`
-- ----------------------------
DROP TABLE IF EXISTS `excel_hero_level_exp`;
CREATE TABLE `excel_hero_level_exp` (
  `level` int(10) unsigned NOT NULL,
  `exp` int(10) unsigned NOT NULL,
  `training_exp` int(10) unsigned NOT NULL,
  `training_gold` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_training_hall`
-- ----------------------------
DROP TABLE IF EXISTS `excel_training_hall`;
CREATE TABLE `excel_training_hall` (
  `level` int(10) unsigned NOT NULL,
  `hero_level_max` int(10) unsigned NOT NULL,
  `training_max_time` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


-- ----------------------------
-- Table structure for `hero_training_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `hero_training_time_events`;
CREATE TABLE `hero_training_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `hero_id` bigint(20) unsigned NOT NULL,
  `exp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`event_id`),
  UNIQUE KEY `uni_key` USING BTREE (`hero_id`),
  KEY `account` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_lottery_cfg`
-- ----------------------------
DROP TABLE IF EXISTS `excel_lottery_cfg`;
CREATE TABLE `excel_lottery_cfg` (
  `free_draw_num` int(10) unsigned NOT NULL,
  `paid_draw_price` int(10) unsigned NOT NULL,
  `paid_draw_alliance_contribute` int(10) unsigned NOT NULL,
  `alliance_lottery_num` int(10) unsigned NOT NULL,
  `top_log_value` int(10) unsigned NOT NULL,
  `notify_world_value` int(10) unsigned NOT NULL,
  PRIMARY KEY (`free_draw_num`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_embassy_cfg`
-- ----------------------------
DROP TABLE IF EXISTS `excel_embassy_cfg`;
CREATE TABLE `excel_embassy_cfg` (
  `level` int(10) unsigned NOT NULL,
  `trade_num` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `trade_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `trade_time_events`;
CREATE TABLE `trade_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`event_id`),
  UNIQUE KEY `account` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_accelerate_cost_discount`
-- ----------------------------
DROP TABLE IF EXISTS `excel_accelerate_cost_discount`;
CREATE TABLE `excel_accelerate_cost_discount` (
  `price` int(10) unsigned NOT NULL,
  `discount` float unsigned NOT NULL,
  PRIMARY KEY  (`price`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_change_name`
-- ----------------------------
DROP TABLE IF EXISTS `excel_change_name`;
CREATE TABLE `excel_change_name` (
  `excel_id` int(10) unsigned NOT NULL,
  `free_num` int(10) unsigned NOT NULL,
  `diamond` int(10) unsigned NOT NULL,
  PRIMARY KEY (`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_recharge_diamond`
-- ----------------------------
DROP TABLE IF EXISTS `excel_recharge_diamond`;
CREATE TABLE `excel_recharge_diamond` (
  `money` int(10) unsigned NOT NULL,
  `diamond` int(10) unsigned NOT NULL,
  `added_diamond` int(10) unsigned NOT NULL,
  PRIMARY KEY (`money`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `top_lottery_reward_log`
-- ----------------------------
DROP TABLE IF EXISTS `top_lottery_reward_log`;
CREATE TABLE `top_lottery_reward_log` (
  `account_id` bigint(20) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `data` int(10) unsigned NOT NULL,
  `value` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  KEY `account` (`account_id`),
  KEY `type_idx` (`type`),
  KEY `value_idx` (`value`),
  KEY `time_idx` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `gm_mail`
-- ----------------------------
DROP TABLE IF EXISTS `gm_mail`;
CREATE TABLE `gm_mail` (
  `account_id` bigint(20) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  `text` blob NOT NULL,
  `mail_id` int(10) unsigned NOT NULL,
  `status` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`mail_id`,`account_id`),
  KEY `account` (`account_id`),
  KEY `time_idx` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

drop table if exists gm_mail_reply;
create table gm_mail_reply (
	account_id bigint unsigned not null,
	mail_id int unsigned not null,
	time int unsigned not null,
	text blob not null,
	index (account_id,mail_id)
)engine=innodb default charset=utf8;

-- ----------------------------
-- Table structure for `wait_deal_excel_time_event`
-- ----------------------------
DROP TABLE IF EXISTS `wait_deal_excel_time_event`;
CREATE TABLE `wait_deal_excel_time_event` (
  `excel_id` int(10) unsigned NOT NULL,
  `next_tick` int(10) unsigned NOT NULL,
  `dealed` int(10) unsigned NOT NULL,
  `ext_data` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_time_event`
-- ----------------------------
DROP TABLE IF EXISTS `excel_time_event`;
CREATE TABLE `excel_time_event` (
  `excel_id` int(10) unsigned NOT NULL,
  `begin_date` datetime NOT NULL,
  `is_cycle` int(10) unsigned NOT NULL,
  `cycle_time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`excel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `bills`
-- ----------------------------
DROP TABLE IF EXISTS `bills`;
CREATE TABLE `bills` (
  `id` int(10) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `client_time` int(10) unsigned NOT NULL,
  `server_time` int(10) unsigned NOT NULL,
  `bill` blob,
  `appid` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `billslog`
-- ----------------------------
DROP TABLE IF EXISTS `billslog`;
CREATE TABLE `billslog` (
  `id` int(10) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `client_time` int(10) unsigned NOT NULL,
  `server_time` int(10) unsigned NOT NULL,
  `bill` blob,
  `donetime` int(10) unsigned DEFAULT NULL,
  `status` int(11) DEFAULT NULL,
  `val` int(11) DEFAULT NULL,
  `purchase_date` int(10) unsigned DEFAULT NULL,
  `bvrs` char(8) DEFAULT NULL,
  `transaction_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `appid` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `transaction_id` (`transaction_id`),
  KEY `donetime` (`donetime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `bill_id`
-- ----------------------------
DROP TABLE IF EXISTS `bill_id`;
CREATE TABLE `bill_id` (
  `id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `bill_id`
-- ----------------------------
-- DROP TABLE IF EXISTS `bill_id`;
-- CREATE TABLE `bill_id` (
--   `id` int(10) unsigned NOT NULL
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- 统计登录情况
-- ----------------------------
DROP TABLE IF EXISTS `stat_login`;
CREATE TABLE `stat_login` (
    `day` int(10) unsigned NOT NULL,
    `todaynew` int(10) unsigned DEFAULT NULL,
    `todaylogin` int(10) unsigned DEFAULT NULL,
    `player7` int(10) unsigned DEFAULT NULL,
    `player30` int(10) unsigned DEFAULT NULL,
    `remain1` int(10) unsigned DEFAULT NULL,
    `remain3` int(10) unsigned DEFAULT NULL,
    `total` int(10) unsigned NOT NULL DEFAULT '0',
    PRIMARY KEY (`day`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `unfetched_lottery`
-- ----------------------------
DROP TABLE IF EXISTS `unfetched_lottery`;
CREATE TABLE `unfetched_lottery` (
  `id` int(10) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `data` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`,`account_id`),
  KEY `i_time` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `char_online_stat`
-- ----------------------------
DROP TABLE IF EXISTS `char_online_stat`;
CREATE TABLE `char_online_stat` (
  `stat_date` date NOT NULL,
  `online_num` int(10) unsigned NOT NULL DEFAULT '0',
  `online_time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`stat_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_hero_name_en`
-- ----------------------------
DROP TABLE IF EXISTS `excel_hero_name_en`;
CREATE TABLE `excel_hero_name_en` (
  `lastname` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_app_gift`
-- ----------------------------
DROP TABLE IF EXISTS `excel_app_gift`;
CREATE TABLE `excel_app_gift` (
  `appid` int(10) unsigned NOT NULL,
  `diamond` int(10) unsigned NOT NULL,
  `end_date` datetime NOT NULL,
  PRIMARY KEY (`appid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `locks`
-- locked=1代表能增加,不可删除,其他进程不可处理
-- ----------------------------
DROP TABLE IF EXISTS `locks`;
CREATE TABLE `locks` (
  `type` int(10) unsigned NOT NULL,
  `locked` int(10) unsigned NOT NULL,
  PRIMARY KEY (`type`)
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

-- ----------------------------
-- Table structure for `excel_alliance_donate_soldier`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alliance_donate_soldier`;
CREATE TABLE `excel_alliance_donate_soldier` (
  `level` int(10) unsigned NOT NULL,
  `day_max_num` int(10) unsigned NOT NULL,
  `accelerate_rate` decimal(12,2) unsigned NOT NULL,
  PRIMARY KEY (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `char_donate_soldier`
-- ----------------------------
DROP TABLE IF EXISTS `char_donate_soldier`;
CREATE TABLE `char_donate_soldier` (
  `account_id` bigint(20) unsigned NOT NULL,
  `donated_num` int(10) unsigned NOT NULL,
  `date` date NOT NULL,
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `donate_soldier_time_events`
-- ----------------------------
DROP TABLE IF EXISTS `donate_soldier_time_events`;
CREATE TABLE `donate_soldier_time_events` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `obj_id` bigint(20) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `num` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  PRIMARY KEY (`event_id`),
  UNIQUE KEY `account` (`account_id`),
  KEY `obj` (`obj_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_gold_market_level`
-- ----------------------------
DROP TABLE IF EXISTS `excel_gold_market_level`;
CREATE TABLE `excel_gold_market_level` (
  `level` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  PRIMARY KEY (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `gold_market`
-- ----------------------------
DROP TABLE IF EXISTS `gold_market`;
CREATE TABLE `gold_market` (
  `transaction_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `num` int(10) unsigned NOT NULL,
  `price` int(10) unsigned NOT NULL,
  `total_price` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`transaction_id`),
  UNIQUE KEY `account` (`account_id`),
  KEY `submit_time` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `gold_market_log`
-- ----------------------------
DROP TABLE IF EXISTS `gold_market_log`;
CREATE TABLE `gold_market_log` (
  `transaction_id` bigint(20) unsigned NOT NULL,
  `seller_id` bigint(20) unsigned NOT NULL,
  `buyer_id` bigint(20) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  `total_price` int(10) unsigned NOT NULL,
  `action` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`transaction_id`,`action`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `char_attack_protect`
-- ----------------------------
DROP TABLE IF EXISTS `char_attack_protect`;
CREATE TABLE `char_attack_protect` (
  `account_id` bigint(20) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_world_res`
-- ----------------------------
DROP TABLE IF EXISTS `excel_world_res`;
CREATE TABLE `excel_world_res` (
  `type` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `num` int(10) unsigned NOT NULL,
  `pos_x_begin` int(10) unsigned NOT NULL,
  `pos_y_begin` int(10) unsigned NOT NULL,
  `pos_x_end` int(10) unsigned NOT NULL,
  `pos_y_end` int(10) unsigned NOT NULL,
  `army_data` blob NOT NULL,
  `gold_loot` blob NOT NULL,
  `pop_loot` blob NOT NULL,
  `crystal_loot` blob NOT NULL,
  PRIMARY KEY (`type`,`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `world_res`
-- ----------------------------
DROP TABLE IF EXISTS `world_res`;
CREATE TABLE `world_res` (
  `id` bigint(20) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `pos_x` int(10) unsigned NOT NULL,
  `pos_y` int(10) unsigned NOT NULL,
  `army_deploy` blob NOT NULL,
  `army_data` blob NOT NULL,
  `combat_id` blob NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  `pop` int(10) unsigned NOT NULL,
  `crystal` int(10) unsigned NOT NULL,
  `can_build` int(10) unsigned NOT NULL,
  `floor2` int(10) unsigned NOT NULL,
  `terrain_type` int(10) unsigned NOT NULL,
  `prof` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `instance_drop_log`
-- ----------------------------
DROP TABLE IF EXISTS `instance_drop_log`;
CREATE TABLE `instance_drop_log` (
  `account_id` bigint(20) unsigned NOT NULL,
  `instance_id` bigint(20) unsigned NOT NULL,
  `class` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `excel_id` int(10) unsigned NOT NULL,
  `num` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  KEY `account_id` (`account_id`),
  KEY `instance_id` (`instance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alli_instance`
-- ----------------------------
DROP TABLE IF EXISTS `alli_instance`;
CREATE TABLE `alli_instance` (
  `instance_id` bigint(20) unsigned NOT NULL,
  `creator_id` bigint(20) unsigned NOT NULL,
  `alliance_id` bigint(20) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `status` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `player_num` int(10) unsigned NOT NULL,
  `create_time` int(10) unsigned NOT NULL,
  `start_time` int(10) unsigned NOT NULL,
  `auto_combat` tinyint(3) unsigned NOT NULL,
  `auto_supply` tinyint(3) unsigned NOT NULL,
  `stop_level` int(10) unsigned NOT NULL,
  `retry_times` int(10) unsigned NOT NULL,
  `loot` blob NOT NULL,
  `combat_result` int(10) unsigned NOT NULL,
  `combat_log` blob NOT NULL,
  PRIMARY KEY (`instance_id`),
  KEY `creator_id` (`creator_id`),
  KEY `create_time` (`create_time`),
  KEY `begin_time` (`start_time`),
  KEY `alliance_id` (`alliance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alli_instance_combat`
-- ----------------------------
DROP TABLE IF EXISTS `alli_instance_combat`;
CREATE TABLE `alli_instance_combat` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `obj_id` bigint(20) unsigned NOT NULL,
  `combat_type` int(10) unsigned NOT NULL,
  `attack_restore_percent` decimal(4,2) unsigned NOT NULL,
  `defense_restore_percent` decimal(4,2) unsigned NOT NULL,
  `error_flag` int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`event_id`),
  KEY (`account_id`),
  KEY (`obj_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alli_instance_combat_event`
-- ----------------------------
DROP TABLE IF EXISTS `alli_instance_combat_event`;
CREATE TABLE `alli_instance_combat_event` (
  `event_id` bigint(20) unsigned NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `march_type` int(10) unsigned NOT NULL,
  `combat_type` int(10) unsigned NOT NULL,
  `obj_id` bigint(20) unsigned NOT NULL,
  `hero_deploy` blob NOT NULL,
  `combat_log` blob NOT NULL,
  `combat_result` int(10) unsigned NOT NULL,
  PRIMARY KEY (`event_id`),
  KEY `account_id` (`account_id`),
  KEY (`obj_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alli_instance_player`
-- ----------------------------
DROP TABLE IF EXISTS `alli_instance_player`;
CREATE TABLE `alli_instance_player` (
  `account_id` bigint(20) unsigned NOT NULL,
  `instance_id` bigint(20) unsigned NOT NULL,
  `status` int(10) NOT NULL,
  `hero_ids` blob NOT NULL,
  `day_times_free` int(10) unsigned NOT NULL,
  `last_time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`account_id`),
  KEY `instance_id` (`instance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `t_kick_client_all`
-- ----------------------------
DROP TABLE IF EXISTS `t_kick_client_all`;
CREATE TABLE `t_kick_client_all` (
  `version_appid` blob NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_alli_instance`
-- ----------------------------
DROP TABLE IF EXISTS `excel_alli_instance`;
CREATE TABLE `excel_alli_instance` (
  `level_id` int(10) unsigned NOT NULL DEFAULT '0',
  `army_deploy` char(10) NOT NULL DEFAULT '',
  `army_data` char(10) NOT NULL DEFAULT '',
  `awa_exp` int(10) unsigned NOT NULL DEFAULT '0',
  `awa_gold` int(10) unsigned NOT NULL DEFAULT '0',
  `loot_table` blob NOT NULL,
  `loot_num` int(10) unsigned NOT NULL DEFAULT '0',
  `awa_exp_char` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`level_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `alli_instance_combat_result`
-- ----------------------------
DROP TABLE IF EXISTS `alli_instance_combat_result`;
CREATE TABLE `alli_instance_combat_result` (
  `event_id` bigint(20) unsigned NOT NULL,
  `result` tinyint(3) unsigned NOT NULL,
  `attack_data` blob NOT NULL,
  `defense_data` blob NOT NULL,
  `ext_data` blob NOT NULL,
  `loot_list` blob NOT NULL,
  `locked` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`event_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `arena`
-- ----------------------------
DROP TABLE IF EXISTS `arena`;
CREATE TABLE `arena` (
  `account_id` bigint(20) unsigned NOT NULL,
  `rank` int(10) unsigned NOT NULL,
  `total_force` int(10) unsigned NOT NULL,
  `combat_data` blob NOT NULL,
  `used_times` int(10) unsigned NOT NULL,
  `used_pay_times` int(10) unsigned NOT NULL,
  `last_reset_time` int(10) unsigned NOT NULL,
  `last_upload_time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`account_id`),
  KEY `rank` (`rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_arena`
-- ----------------------------
DROP TABLE IF EXISTS `excel_arena`;
CREATE TABLE `excel_arena` (
  `rank_diff` int(10) unsigned NOT NULL,
  `free_times` int(10) unsigned NOT NULL,
  `pay_times` int(10) unsigned NOT NULL,
  `upload_cd` int(10) unsigned NOT NULL,
  `min_level` int(10) unsigned NOT NULL,
  `pay_price` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `excel_arena_reward`
-- ----------------------------
DROP TABLE IF EXISTS `excel_arena_reward`;
CREATE TABLE `excel_arena_reward` (
  `rank` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  PRIMARY KEY (`rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `position_mark`
-- ----------------------------
DROP TABLE IF EXISTS `position_mark`;
CREATE TABLE `position_mark` (
  `account_id` bigint(20) unsigned NOT NULL,
  `pos_x` int(10) unsigned NOT NULL,
  `pos_y` int(10) unsigned NOT NULL,
  `head_id` int(10) unsigned NOT NULL,
  `time` int(10) unsigned NOT NULL,
  `prompt` tinyblob NOT NULL,
  PRIMARY KEY (`account_id`,`pos_x`,`pos_y`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


-- ----------------------------
-- Table structure for `excel_position_mark`
-- ----------------------------
DROP TABLE IF EXISTS `excel_position_mark`;
CREATE TABLE `excel_position_mark` (
  `capacity` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
