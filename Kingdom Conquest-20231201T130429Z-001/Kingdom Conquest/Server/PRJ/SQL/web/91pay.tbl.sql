-- 准备发起购买（由客户端请求，调用gen91PaySerial）
DROP TABLE IF EXISTS `pay91_prepare`;
CREATE TABLE `pay91_prepare` (
`serial` char(36) primary key,
`accountid` bigint unsigned,
`createTime` int unsigned,
INDEX(`createTime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 购买成功（主要是用做日志，可以保留一段时间，把老的（如超过一个月的）删除或者放到另外的表中）
DROP TABLE IF EXISTS `pay91_done`;
CREATE TABLE `pay91_done` (
`serial` char(36) PRIMARY KEY,
`accountid` bigint unsigned,
`accountname` char(32),
`createTime` int unsigned,
`overTime` int unsigned,
`GoodsId` int,
`GoodsCount` int,
`ConsumeStreamId` char(36),
`trycount` int,
`ip` int unsigned ,
INDEX(`overTime`),
INDEX(`accountid`),
INDEX(`accountname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 目前想到的reason都是超时没有购买（0），还有就是91服务器发来的序列号这边不存在（-1）
DROP TABLE IF EXISTS `pay91_fail`;
CREATE TABLE `pay91_fail` (
`serial` char(36),
`accountid` bigint unsigned,
`accountname` char(32),
`createTime` int unsigned,
`failTime` int unsigned,
`reason` int,
`ip` int unsigned,
INDEX(`createTime`),
INDEX(`failTime`),
INDEX(`accountid`),
INDEX(`accountname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 给应用看的（应用每2秒检查一下，有就通知给客户端）
DROP TABLE IF EXISTS `pay91_procqueue`;
CREATE TABLE `pay91_procqueue` (
`serial` char(36) PRIMARY KEY,
`accountid` bigint unsigned,
`GoodsId` int,
`GoodsCount` int
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


