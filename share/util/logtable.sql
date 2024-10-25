
CREATE DATABASE IF NOT EXISTS logdb;

use logdb;

CREATE TABLE `accounting` (
  `userid` int(11) NOT NULL default '0',
  `type` int(11) NOT NULL default '0',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `login` (
  `account` varchar(64) NOT NULL default '',
  `userid` int(11) NOT NULL default '0',
  `sid` int(11) NOT NULL default '0',
  `peer` varchar(32) NOT NULL default '',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `logout` (
  `account` varchar(64) NOT NULL default '',
  `userid` int(11) NOT NULL default '0',
  `sid` int(11) NOT NULL default '0',
  `peer` varchar(32) NOT NULL default '',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `rolelogin` (
  `roleid` int(11) NOT NULL default '0',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`roleid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `rolelogout` (
  `roleid` int(11) NOT NULL default '0',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`roleid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `gmoperate` (
  `userid` int(11) NOT NULL default '0',
  `type` int(11) NOT NULL default '0',
  `content` varchar(255) default '',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `trade` (
  `roleidA` int(11) NOT NULL default '0',
  `roleidB` int(11) NOT NULL default '0',
  `moneyA` int(11) NOT NULL default '0',
  `moneyB` int(11) NOT NULL default '0',
  `objectA` varchar(255) default '',
  `objectB` varchar(255) default '',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`roleidA`,`roleidB`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `upgrade` (
  `roleid` int(11) NOT NULL default '0',
  `level` int(11) NOT NULL default '0',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`roleid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `die` (
  `roleid` int(11) NOT NULL default '0',
  `type` int(11) NOT NULL default '0',
  `attacker` int(11) NOT NULL default '0',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`roleid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;

CREATE TABLE `keyobject` (
  `id` int(11) NOT NULL default '0',
  `delta` int(11) NOT NULL default '0',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `host` varchar(32) default '',
  `service` varchar(32) default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;



