# table auth

CREATE TABLE [dbo].[auth] (
	[userid] [int] NOT NULL ,
	[zoneid] [int] NOT NULL ,
	[rid] [int] NOT NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[auth] WITH NOCHECK ADD 
	CONSTRAINT [PK_auth] PRIMARY KEY  CLUSTERED 
	(
		[userid],
		[zoneid],
		[rid]
	)  ON [PRIMARY] 
GO

# table forbid

CREATE TABLE [dbo].[forbid] (
	[userid] [int] NOT NULL ,
	[type] [int] NOT NULL ,
	[ctime] [datetime] NOT NULL ,
	[forbid_time] [int] NOT NULL ,
	[reason] [varbinary] (255) NOT NULL ,
	[gmroleid] [int] NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[forbid] WITH NOCHECK ADD 
	CONSTRAINT [PK_forbid] PRIMARY KEY  CLUSTERED 
	(
		[userid],
		[type]
	)  ON [PRIMARY] 
GO

# table iplimit

CREATE TABLE [dbo].[iplimit] (
	[uid] [int] NOT NULL ,
	[ipaddr1] [int] NULL ,
	[ipmask1] [varchar] (2) NULL ,
	[ipaddr2] [int] NULL ,
	[ipmask2] [varchar] (2) NULL ,
	[ipaddr3] [int] NULL ,
	[ipmask3] [varchar] (2) NULL ,
	[enable] [char] (1) NULL ,
	[lockstatus] [char] (1) NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[iplimit] WITH NOCHECK ADD 
	CONSTRAINT [PK_iplimit] PRIMARY KEY  CLUSTERED 
	(
		[uid]
	)  ON [PRIMARY] 
GO

# table point

CREATE TABLE [dbo].[point] (
	[uid] [int] NOT NULL ,
	[aid] [int] NOT NULL ,
	[time] [int] NOT NULL ,
	[zoneid] [int] NULL ,
	[zonelocalid] [int] NULL ,
	[accountstart] [datetime] NULL ,
	[lastlogin] [datetime] NULL ,
	[enddate] [datetime] NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[point] WITH NOCHECK ADD 
	CONSTRAINT [PK_point] PRIMARY KEY  CLUSTERED 
	(
		[uid],
		[aid]
	)  ON [PRIMARY] 
GO

CREATE  INDEX [IX_point_aidzoneid] ON [dbo].[point]([aid], [zoneid]) ON [PRIMARY]
GO

# table usecashlog

CREATE TABLE [dbo].[usecashlog] (
	[userid] [int] NOT NULL ,
	[zoneid] [int] NOT NULL ,
	[sn] [int] NOT NULL ,
	[aid] [int] NOT NULL ,
	[point] [int] NOT NULL ,
	[cash] [int] NOT NULL ,
	[status] [int] NOT NULL ,
	[creatime] [datetime] NOT NULL ,
	[fintime] [datetime] NOT NULL 
) ON [PRIMARY]
GO

CREATE  CLUSTERED  INDEX [IX_usecashlog_uzs] ON [dbo].[usecashlog]([userid], [zoneid], [sn]) ON [PRIMARY]
GO

CREATE  INDEX [IX_usecashlog_creatime] ON [dbo].[usecashlog]([creatime]) ON [PRIMARY]
GO

# table usecashnow

CREATE TABLE [dbo].[usecashnow] (
	[userid] [int] NOT NULL ,
	[zoneid] [int] NOT NULL ,
	[sn] [int] NOT NULL ,
	[aid] [int] NOT NULL ,
	[point] [int] NOT NULL ,
	[cash] [int] NOT NULL ,
	[status] [int] NOT NULL ,
	[creatime] [datetime] NOT NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[usecashnow] WITH NOCHECK ADD 
	CONSTRAINT [PK_usecashnow_1] PRIMARY KEY  CLUSTERED 
	(
		[userid],
		[zoneid],
		[sn]
	)  ON [PRIMARY] 
GO

CREATE  INDEX [IX_usecashnow_status] ON [dbo].[usecashnow]([status]) ON [PRIMARY]
GO

CREATE  INDEX [IX_usecashnow_creatime] ON [dbo].[usecashnow]([creatime]) ON [PRIMARY]
GO

# table users

CREATE TABLE [dbo].[users] (
	[ID] [int] NOT NULL ,
	[name] [varchar] (32) NOT NULL ,
	[passwd] [binary] (16) NOT NULL ,
	[Prompt] [varchar] (32) NOT NULL ,
	[answer] [varchar] (32) NOT NULL ,
	[truename] [varchar] (32) NOT NULL ,
	[idnumber] [varchar] (32) NOT NULL ,
	[email] [varchar] (64) NOT NULL ,
	[mobilenumber] [varchar] (32) NULL ,
	[province] [varchar] (32) NULL ,
	[city] [varchar] (32) NULL ,
	[phonenumber] [varchar] (32) NULL ,
	[address] [varchar] (64) NULL ,
	[postalcode] [varchar] (8) NULL ,
	[gender] [int] NULL ,
	[birthday] [datetime] NULL ,
	[creatime] [datetime] NOT NULL ,
	[qq] [varchar] (32) NULL ,
	[passwd2] [binary] (16) NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[users] WITH NOCHECK ADD 
	CONSTRAINT [PK_users] PRIMARY KEY  CLUSTERED 
	(
		[ID]
	)  ON [PRIMARY] 
GO

CREATE  UNIQUE  INDEX [IX_users_name] ON [dbo].[Users]([name]) ON [PRIMARY]
GO

CREATE  INDEX [IX_users_creatime] ON [dbo].[Users]([creatime]) ON [PRIMARY]
GO

