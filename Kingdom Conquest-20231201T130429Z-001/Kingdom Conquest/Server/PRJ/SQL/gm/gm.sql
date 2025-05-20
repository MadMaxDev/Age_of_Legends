SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `account`
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `gid` int(11) NOT NULL DEFAULT '0',
  `user` char(32) DEFAULT NULL,
  `pass` char(32) DEFAULT NULL,
  `name` char(32) DEFAULT NULL,
  `comment` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`uid`),
  UNIQUE KEY `user` (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `session`
-- ----------------------------
DROP TABLE IF EXISTS `session`;
CREATE TABLE `session` (
  `uid` int(11) NOT NULL,
  `sessionid` char(32) DEFAULT NULL,
  `startime` int(11) DEFAULT NULL,
  `dbname` char(32) DEFAULT NULL,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

SET GLOBAL log_bin_trust_function_creators=ON;
-- ----------------------------
-- Function structure for `func_passwd`
-- ----------------------------
DROP FUNCTION IF EXISTS `func_passwd`;
DELIMITER ;;
CREATE FUNCTION `func_passwd`(_uid int, _oldpass char(32), _newpass char(32)) RETURNS int(11)
BEGIN
SET @U=0;
select uid into @U from account where uid=_uid and pass=_oldpass;
IF @U=0 THEN return -1;
END IF;
update account set pass=_newpass where uid=_uid;
return  0;
END
;;
DELIMITER ;
