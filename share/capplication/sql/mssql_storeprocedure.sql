
/***************************************************
  addForbid
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  addForbid 

@userid                   integer,
@type                      integer,
@forbid_time          integer,
@reason                  varbinary(255),
@gmroleid              integer

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE forbid SET ctime=getdate(),forbid_time=@forbid_time,reason=@reason,gmroleid=@gmroleid WHERE userid=@userid AND type=@type
	IF @@rowcount = 0	INSERT INTO forbid VALUES(@userid,@type,getdate(),@forbid_time,@reason,@gmroleid)
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

/***************************************************
  addUserPriv
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  addUserPriv 

@userid     integer,
@zoneid    integer,
@rid          integer

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	INSERT INTO auth VALUES( @userid, @zoneid, @rid )
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

/***************************************************
  acquireuserpasswd
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  acquireuserpasswd 

@name                  varchar(64),
@uid                   integer out,
@passwd                varchar(64) out

AS
SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	DECLARE @passwdtemp binary(16)
	SELECT @uid = id, @passwdtemp = passwd FROM users WHERE name = @name
	IF @@rowcount = 0
		return -1
	SELECT @passwd = master.dbo.fn_varbintohexsubstring(1,@passwdtemp,1,0)
	return 0

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

/***************************************************
  acquireuserpasswd2
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  acquireuserpasswd2 

@name                  varchar(64),
@uid                   integer out,
@passwd2               varchar(64) out

AS
SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	DECLARE @passwdtemp binary(16)
	SELECT @uid = id, @passwdtemp = passwd2 FROM users WHERE name = @name
	IF @@rowcount = 0
		return -1
	SELECT @passwd2 = master.dbo.fn_varbintohexsubstring(1,@passwdtemp,1,0)
	return 0

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

/***************************************************
  getuseridbyname
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  getuseridbyname 

@name                  varchar(64),
@uid                   integer out,

AS
SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	SELECT @uid = id FROM users WHERE name = @name
	IF @@rowcount = 0
		return -1
	return 0

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

/***************************************************
  getuserinfobyname
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  getuserinfobyname 

@name                  varchar(32),
@id                    integer out,
@prompt                varchar(32) out,
@answer                varchar(32) out,
@truename              varchar(32) out,
@idnumber              varchar(32) out,
@email                 varchar(64) out,
@mobilenumber          varchar(32) out,
@province              varchar(32) out,
@city                  varchar(32) out,
@phonenumber           varchar(32) out,
@address               varchar(64) out,
@postalcode            varchar(8) out,
@gender                integer out,
@birthday              varchar(32) out,
@creatime              varchar(32) out,
@qq                    varchar(32) out

AS
SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	DECLARE @birthday_tmp datetime
	DECLARE @creatime_tmp datetime
	SELECT @id = id, @prompt = prompt, @answer = answer, @truename = truename, @idnumber = idnumber, @email = email, @mobilenumber = mobilenumber, @province = province, @city = city, @phonenumber = phonenumber, @address = address, @postalcode = postalcode, @gender = gender, @birthday_tmp = birthday, @creatime_tmp = creatime, @qq = qq FROM users WHERE name = @name
	IF @@rowcount = 0
		return -1
	SELECT @birthday = convert(varchar(32), @birthday_tmp, 120)
	SELECT @creatime = convert(varchar(32), @creatime_tmp, 121)
	return 0

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

/***************************************************
  adduser
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE PROCEDURE  adduser

@name               varchar(64),
@passwd            binary(16),
@prompt            varchar(32),
@answer            varchar(32),
@truename        varchar(32),
@idnumber        varchar(32),
@email               varchar(64),
@mobilenumber varchar(32),
@province         varchar(32),
@city                 varchar(32),
@phonenumber  varchar(32),
@address            varchar(64),
@postalcode       varchar(8) ,
@gender             integer,
@birthday          varchar(32),
@qq                   varchar(32),
@passwd2         binary(16)

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON
SET NOCOUNT ON
SET XACT_ABORT  ON

	BEGIN TRAN
	DECLARE @id integer
	SELECT @id = ISNULL(max(id), 16) + 16 FROM users HOLDLOCK
	INSERT INTO users (id,name,passwd,prompt,answer,truename,idnumber,email,mobilenumber,province,city,phonenumber,address,postalcode,gender,birthday,creatime,qq,passwd2) VALUES( @id, @name, @passwd, @prompt, @answer, @truename, @idnumber, @email, @mobilenumber, @province, @city, @phonenumber, @address, @postalcode, @gender, @birthday, getdate(), @qq, @passwd2 )
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  adduserpoint
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  adduserpoint 

@uid                   integer,
@aid                   integer,
@time                 integer

AS
SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE point SET time=ISNULL(time,0)+@time WHERE @uid=uid AND @aid=aid
	IF @@rowcount = 0	INSERT INTO point (uid,aid,time) VALUES (@uid,@aid,@time)
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  changePasswd
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE PROCEDURE  changePasswd 

@name                  varchar(64),
@passwd                binary(16)

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE users SET passwd=@passwd WHERE name=@name
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  changePasswd2
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE PROCEDURE  changePasswd2 

@name                  varchar(64),
@passwd2              binary(16)

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE users SET passwd2=@passwd2 WHERE name=@name
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  clearonlinerecords
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  clearonlinerecords 
@zoneid             integer,
@aid                   integer

AS
SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE point SET zoneid = NULL, zonelocalid = NULL WHERE aid = @aid AND zoneid = @zoneid
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  delUserPriv
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  delUserPriv 

@userid     integer,
@zoneid    integer,
@rid          integer,
@deltype  integer

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	IF @deltype = 0 DELETE FROM auth WHERE userid = @userid AND zoneid = @zoneid AND rid = @rid 
	ELSE IF @deltype = 1 DELETE FROM auth WHERE userid = @userid AND zoneid = @zoneid
	ELSE IF @deltype = 2 DELETE FROM auth WHERE userid = @userid 
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  deleteTimeoutForbid
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  deleteTimeoutForbid 

@userid                   integer

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	DELETE 	FROM forbid WHERE userid=@userid AND datediff(ss,ctime,getdate())>forbid_time
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  enableiplimit
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  enableiplimit 

@uid     integer,
@enable  char(1)

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE iplimit SET enable=@enable WHERE uid=@uid
	IF @@rowcount = 0	INSERT INTO iplimit (uid,enable) VALUES (@uid,@enable)
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  lockuser
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  lockuser 

@uid     integer,
@lockstatus  char(1)

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE iplimit SET lockstatus=@lockstatus WHERE uid=@uid
	IF @@rowcount = 0	INSERT INTO iplimit (uid,lockstatus,enable) VALUES (@uid,@lockstatus,'t')
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  recordoffline
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  recordoffline 

@uid                   integer,
@aid                   integer,
@zoneid              integer out,
@zonelocalid       integer out,
@overwrite          integer out

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE point SET zoneid = NULL, zonelocalid = NULL WHERE uid = @uid AND aid = @aid AND zoneid = @zoneid
	SELECT @overwrite = @@rowcount
	IF @overwrite = 0 SELECT @zoneid = zoneid, @zonelocalid = zonelocalid FROM point WHERE uid = @uid AND aid = @aid
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  recordonline
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  recordonline 

@uid                   integer,
@aid                   integer,
@zoneid              integer out,
@zonelocalid       integer out,
@overwrite          integer out

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	DECLARE @tmp_zoneid      integer
	DECLARE @tmp_zonelocalid integer
	SELECT @tmp_zoneid = zoneid, @tmp_zonelocalid = zonelocalid FROM point WHERE uid = @uid AND aid = @aid
	IF @@rowcount = 0
		INSERT INTO point (uid,aid,time,zoneid,zonelocalid,lastlogin) VALUES (@uid,@aid,0,@zoneid,@zonelocalid,getdate())
	ELSE IF @tmp_zoneid = NULL OR @overwrite = 1
		UPDATE point SET zoneid = @zoneid, zonelocalid = @zonelocalid, lastlogin = getdate() WHERE uid = @uid AND aid = @aid
	IF @tmp_zoneid = NULL
		SELECT @overwrite = 1
	ELSE
		SELECT @zoneid = @tmp_zoneid, @zonelocalid = @tmp_zonelocalid
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  remaintime
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  remaintime 

@uid                   integer,
@aid                   integer,
@remain              integer out,
@freetimeleft       integer out

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	DECLARE @enddate datetime
	DECLARE @now     datetime
	SELECT @now = getdate()
	IF @aid = 0
	BEGIN
		SELECT @remain = 86313600
		SELECT @enddate = dateadd(day,30,@now)
	END
	ELSE
	BEGIN
		SELECT @remain=time, @enddate=ISNULL(enddate,@now) FROM point WHERE @uid = uid AND @aid = aid
		IF @@rowcount = 0
		BEGIN
			SELECT @remain = 0
			INSERT INTO point (uid,aid,time) VALUES (@uid,@aid,@remain)
		END
	END
	SELECT @freetimeleft = 0
	IF @enddate > @now	SELECT @freetimeleft = datediff(second, @now, @enddate )
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  setiplimit
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE PROCEDURE  setiplimit 

@uid     integer,
@ipaddr1 integer,
@ipmask1 varchar(2),
@ipaddr2 integer,
@ipmask2 varchar(2),
@ipaddr3 integer,
@ipmask3 varchar(2)
--@enable  char(1)

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	UPDATE iplimit SET ipaddr1=@ipaddr1,ipmask1=@ipmask1,ipaddr2=@ipaddr2,ipmask2=@ipmask2,ipaddr3=@ipaddr3,ipmask3=@ipmask3 WHERE uid=@uid
	IF @@rowcount = 0	INSERT INTO iplimit (uid,ipaddr1,ipmask1,ipaddr2,ipmask2,ipaddr3,ipmask3,enable) VALUES (@uid,@ipaddr1,@ipmask1,@ipaddr2,@ipmask2,@ipaddr3,@ipmask3,'t')
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  updateUserInfo
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE PROCEDURE  updateUserInfo

@name                   varchar(32),
@prompt                varchar(32),
@answer                varchar(32),
@truename            varchar(32),
@idnumber            varchar(32),
@email                  varchar(64),
@mobilenumber   varchar(32),
@province            varchar(32),
@city                    varchar(32),
@phonenumber    varchar(32),
@address              varchar(64),
@postalcode         varchar(8),
@gender               integer,
@birthday            varchar(32),
@qq                      varchar(32)

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	update users set prompt=@prompt,answer=@answer,truename=@truename,idnumber=@idnumber,email=@email,mobilenumber=@mobilenumber,province=@province,city=@city,phonenumber=@phonenumber,address=@address,postalcode=@postalcode,gender=@gender,birthday=@birthday,qq=@qq where name=@name
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO


/***************************************************
  usecash
***************************************************/

SET QUOTED_IDENTIFIER ON 
GO
SET ANSI_NULLS ON 
GO

CREATE  PROCEDURE  usecash 

@userid                   integer,
@zoneid                  integer,
@sn                         integer,
@aid                        integer,
@point                    integer,
@cash                    integer,
@status                   integer,
@error                    integer out

AS

SET ANSI_NULLS ON
SET ANSI_WARNINGS ON
SET ANSI_NULL_DFLT_ON ON
SET QUOTED_IDENTIFIER ON

	BEGIN TRAN
	DECLARE @sn_old integer
	DECLARE @aid_old integer
	DECLARE @point_old integer
	DECLARE @cash_old integer
	DECLARE @status_old integer
	DECLARE @creatime_old datetime
	DECLARE @time_old integer
	DECLARE @need_restore integer
	DECLARE @exists integer

	SELECT @error = 0
	SELECT @need_restore = 0
	SELECT @sn_old=sn,@aid_old=aid,@point_old=point,@cash_old=cash,@status_old=status,@creatime_old=creatime FROM usecashnow WHERE @userid=userid AND @zoneid=zoneid AND sn>=0

	IF @@rowcount = 1	SELECT @exists = 1
	ELSE				SELECT @exists = 0

	IF @status = 0
	BEGIN
		IF @exists = 0
		BEGIN
			SELECT @aid=aid,@point=point FROM usecashnow WHERE @userid=userid AND @zoneid=zoneid AND @sn=sn
			SELECT @point = ISNULL(@point,0)
			UPDATE point SET time=time-@point WHERE @userid=uid AND @aid=aid AND time>=@point
			IF @@rowcount = 1
				UPDATE usecashnow SET sn=0, status=1 WHERE userid=@userid AND zoneid=@zoneid AND sn=@sn
			ELSE
				SELECT @error = -8
		END
		ELSE
			SELECT @error = -7
	END
	ELSE IF @status = 1
	BEGIN
		IF @exists = 0
		BEGIN
			UPDATE point SET time=time-@point WHERE @userid=uid AND @aid=aid AND time>=@point
			IF @@rowcount = 1
				INSERT INTO usecashnow (userid, zoneid, sn, aid, point, cash, status, creatime) VALUES(@userid, @zoneid, @sn, @aid, @point, @cash, @status, getdate() )
			ELSE
			BEGIN
				INSERT INTO usecashnow SELECT @userid, @zoneid, ISNULL(min(sn),0)-1, @aid, @point, @cash, 0, getdate() FROM usecashnow WHERE userid=@userid and zoneid=@zoneid and 0>=sn
				SELECT @error = -8
			END
		END
		ELSE
		BEGIN
			INSERT INTO usecashnow SELECT @userid, @zoneid, ISNULL(min(sn),0)-1, @aid, @point, @cash, 0, getdate() FROM usecashnow WHERE userid=@userid and zoneid=@zoneid and 0>=sn
			SELECT @error = -7
		END
	END
	ELSE IF @status = 2
	BEGIN
		IF @exists = 1 AND @status_old = 1 AND @sn_old = 0
		BEGIN
			UPDATE usecashnow set sn = @sn, status=@status WHERE @userid=userid AND @zoneid=zoneid AND @sn_old=sn
		END
		ELSE
		BEGIN
			SELECT @error = -9
		END
	END
	ELSE IF @status = 3
	BEGIN
		IF @exists = 1 AND @status_old = 2
			UPDATE usecashnow set status=@status WHERE @userid=userid AND @zoneid=zoneid AND @sn_old=sn
		ELSE
		BEGIN
			SELECT @error = -10
		END
	END
	ELSE IF @status = 4
	BEGIN
		IF @exists = 1
		BEGIN
			DELETE FROM usecashnow WHERE @userid=userid AND @zoneid=zoneid AND @sn_old=sn
			INSERT INTO usecashlog (userid, zoneid, sn, aid, point, cash, status, creatime, fintime) VALUES(@userid, @zoneid, @sn_old, @aid_old, @point_old, @cash_old, @status, @creatime_old, getdate() )
		END
		IF NOT (@exists = 1 AND @status_old = 3)	SELECT @error = -11
	END
	ELSE
	BEGIN
		SELECT @error = -12
	END

	IF @need_restore = 1
	BEGIN
		UPDATE point SET time=time+@point_old WHERE @userid=uid AND @aid_old=aid
		DELETE FROM usecashnow WHERE @userid=userid AND @zoneid=zoneid AND @sn_old=sn
		INSERT INTO usecashlog (userid, zoneid, sn, aid, point, cash, status, creatime, fintime) VALUES(@userid, @zoneid, @sn_old, @aid_old, @point_old, @cash_old, @status, @creatime_old, getdate() )
	END
	COMMIT TRAN

GO
SET QUOTED_IDENTIFIER OFF 
GO
SET ANSI_NULLS ON 
GO

