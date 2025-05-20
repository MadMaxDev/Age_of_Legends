SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `log_gold_cost`
-- ----------------------------
DROP TABLE IF EXISTS `log_gold_cost`;
CREATE TABLE `log_gold_cost` (
  `log_datetime` datetime NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  KEY `time_idx` (`log_datetime`),  
  KEY `account` (`account_id`),
  KEY `type_idx` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
PARTITION BY LIST (TO_DAYS(log_datetime))( 
PARTITION p_20121024 VALUES IN (735165)
);

-- ----------------------------
-- Table structure for `log_gold_produce`
-- ----------------------------
DROP TABLE IF EXISTS `log_gold_produce`;
CREATE TABLE `log_gold_produce` (
  `log_datetime` datetime NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `gold` int(10) unsigned NOT NULL,
  KEY `time_idx` (`log_datetime`),
  KEY `account` (`account_id`),
  KEY `type_idx` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
PARTITION BY LIST (TO_DAYS(log_datetime))( 
PARTITION p_20121024 VALUES IN (735165)
);

-- ----------------------------
-- Table structure for `log_money_cost`
-- ----------------------------
DROP TABLE IF EXISTS `log_money_cost`;
CREATE TABLE `log_money_cost` (
  `log_datetime` datetime NOT NULL,
  `account_id` bigint(20) unsigned NOT NULL,
  `use_type` int(10) unsigned NOT NULL,
  `money_type` int(10) unsigned NOT NULL,
  `money` int(10) unsigned NOT NULL,
  `ext_data_0` int(10) unsigned NOT NULL,
  `ext_data_1` int(10) unsigned NOT NULL,
  `ext_data_2` int(10) unsigned NOT NULL,
  `ext_data_3` int(10) unsigned NOT NULL,
  KEY `time_idx` (`log_datetime`),
  KEY `account` (`account_id`),
  KEY `money_type_idx` (`money_type`),
  KEY `use_type_idx` (`use_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
PARTITION BY LIST (TO_DAYS(log_datetime))( 
PARTITION p_20121024 VALUES IN (735165)
);

-- ----------------------------
-- Table structure for `log_lottery`
-- ----------------------------
DROP TABLE IF EXISTS `log_lottery`;
CREATE TABLE `log_lottery` (
  `log_datetime` datetime NOT NULL,
  `account_id` bigint(20) NOT NULL,
  `lottery_type` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `data` int(10) unsigned NOT NULL,
  `value` int(10) unsigned NOT NULL,
  KEY `time` (`log_datetime`),
  KEY `account` (`account_id`),
  KEY `idex_lottery_type` (`lottery_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
PARTITION BY LIST (TO_DAYS(log_datetime))( 
PARTITION p_20121024 VALUES IN (735165)
);

DROP TABLE IF EXISTS `log_item`;
CREATE TABLE `log_item` (
  `log_datetime` datetime NOT NULL,
  `account_id` bigint(20) NOT NULL,
  `item_id` bigint(20) not null,
  `type` int not null,
  `excel_id` int not null,
  `num` int not null,
  `gold` int not null,
  KEY `time` (`log_datetime`),
  KEY `account` (`account_id`),
  KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
PARTITION BY LIST (TO_DAYS(log_datetime))( 
PARTITION p_20121024 VALUES IN (735165)
);



