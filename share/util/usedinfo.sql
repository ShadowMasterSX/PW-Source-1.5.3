use passport
go

create table #billinfo (
	uid                             int                              not null  ,
	aid                             int                              not null  ,
	addupmoney                      int                              not    null  ,
	adduppoint                      float                            not    null  , 
	time                            int                              not    null  ,
	monthtime                       int                              not    null  ,
	enddate                         datetime                         not    null  ,
	primary key(uid, aid)
)
lock allpages
 on 'default'
go 

create table #transbillinfo (
	uid                             int                              not null  ,
	aid                             int                              not null  ,
	addupmoney                      int                              not    null  ,
	adduppoint                      float                            not    null  ,
	flags                           int                              not    null  ,
	primary key(uid, aid)
)
lock allpages
 on 'default'
go 


create table #result
(
	now				datetime			null,
	addall				integer				null,
	consumedall			integer				null,
	exchgtomonthall			integer				null,
	remainall			integer				null,
	presentall			integer				null,
	addtoday			integer				null,
	addtodaymoney			integer				null,
	exchgtomonthtoday		integer				null,
	difftoday			integer				null,
	consumedtoday			integer				null,
	consumedbymonthtoday		integer				null,
	player_transinmonth		integer				null,
	player_transnoadd		integer				null,
	player_trans			integer				null,
	payremainall			integer				null,
	paynetremainall			integer				null,
	paypresentall			integer				null,
	netconsumedtoday		integer				null,
	netconsumedbymonthtoday		integer				null,
	monthremainall			integer				null,
	monthnetremainall		integer				null,
	paynetremainall2		integer				null,
	monthnetremainall2		integer				null
)
go

use passport
go

	INSERT into #billinfo SELECT a.uid,a.aid,sum(a.money),sum(a.addpoint*1.0),0,0,'' from agentbill a where a.uid>32 and a.aid>0 and 9>a.aid group by a.uid,a.aid
	INSERT into #transbillinfo SELECT t.buyer,t.aid,convert(int,sum(t.point*1.0)/90),sum(t.point*1.0),0 from translog t where t.status=1 and t.aid>0 and 9>t.aid group by t.buyer,t.aid
	
go
	update statistics #billinfo
	update statistics #transbillinfo
go

	UPDATE #transbillinfo set t.flags=1 from #billinfo b,#transbillinfo t where b.uid=t.uid and b.aid=t.aid
	INSERT into #billinfo select t.uid,t.aid,t.addupmoney,t.adduppoint,0,0,'' from #transbillinfo t where NOT t.flags=1
	UPDATE #billinfo set b.adduppoint=b.adduppoint+t.adduppoint,b.addupmoney=b.addupmoney+t.addupmoney from #billinfo b,#transbillinfo t where b.uid=t.uid and b.aid=t.aid and t.flags=1
	TRUNCATE table #transbillinfo
go
	INSERT into #transbillinfo select t.seller,t.aid,-convert(int,sum(t.point*1.0)/90),-sum(t.point*1.0),0 from translog t where t.status=1 and t.aid>0 and 9>t.aid group by t.seller,t.aid
go	
	update statistics #transbillinfo
go
	UPDATE #transbillinfo set t.flags=1 from #billinfo b,#transbillinfo t where b.uid=t.uid and b.aid=t.aid
	INSERT into #billinfo select t.uid,t.aid,t.addupmoney,t.adduppoint,0,0,'' from #transbillinfo t where NOT t.flags=1
go
	UPDATE #billinfo set b.adduppoint=b.adduppoint+t.adduppoint,b.addupmoney=b.addupmoney+t.addupmoney from #billinfo b,#transbillinfo t where b.uid=t.uid and b.aid=t.aid and t.flags=1
go
	update statistics #billinfo
go
	UPDATE #billinfo set b.time=p.time,b.enddate=ISNULL(p.enddate,'') from #billinfo b,point p where b.uid=p.uid and b.aid=p.aid
go
	update statistics #billinfo
go

	DECLARE @now datetime
	SELECT @now = max(creatime) from account
	SELECT @now = dateadd(second,-1,convert(varchar(64),@now,102))
	INSERT INTO #result ( now ) VALUES ( @now )
	UPDATE #billinfo set b.monthtime=datediff(second,@now,b.enddate) from #billinfo b where b.enddate>@now
go


	DECLARE @addall integer
	DECLARE @consumedall integer
	DECLARE @exchgtomonthall integer
	DECLARE @remainall integer
	DECLARE @presentall integer
	DECLARE @addtoday integer
	DECLARE @addtodaymoney integer
	DECLARE @exchgtomonthtoday integer
	DECLARE @difftoday integer
	DECLARE @consumedtoday integer
	DECLARE @consumedbymonthtoday integer

	DECLARE @player_transinmonth integer
	DECLARE @player_transnoadd integer
	DECLARE @player_trans integer

	DECLARE @payremainall integer
	DECLARE @paynetremainall integer
	DECLARE @paypresentall integer
	DECLARE @netconsumedtoday integer
	DECLARE @netconsumedbymonthtoday integer
	DECLARE @monthremainall integer
	DECLARE @monthnetremainall integer
	DECLARE @paynetremainall2 integer
	DECLARE @monthnetremainall2 integer

	DECLARE @now datetime
	DECLARE @last datetime
	SELECT @now = now from #result
	SELECT @last = dateadd(day,-1,@now)


	SELECT @addall=convert(int,sum(addpoint*1.0)/3600) from agentbill where uid>32 and aid>0 and 9>aid and @now>=usedate at isolation read uncommitted
	SELECT @exchgtomonthall=convert(int,sum(usepoint*1.0)/3600) from monthbill where aid>0 and 9>aid and @now>=usedate
	SELECT @remainall=convert(int,sum(time*1.0)/3600),@presentall=10*count(time) from point where aid>0 and 9>aid at isolation read uncommitted
	SELECT @consumedall = @addall + @presentall - @remainall - @exchgtomonthall


	SELECT @addtoday=convert(int,sum(addpoint*1.0)/3600),@addtodaymoney=convert(int,sum(money*0.01)) from agentbill where uid>32 and aid>0 and 9>aid and usedate>@last and @now>=usedate at isolation read uncommitted
	SELECT @exchgtomonthtoday=convert(int,sum(usepoint*1.0)/3600) from monthbill where aid>0 and 9>aid and usedate>@last and @now>=usedate
	SELECT @difftoday = convert(int,@addtodaymoney*2.5) - @exchgtomonthtoday


	SELECT @consumedbymonthtoday=sum(datediff(hour,@last,enddate)) from point where aid>0 and 9>aid and enddate>@last and @now>=enddate at isolation read uncommitted
	SELECT @consumedbymonthtoday=@consumedbymonthtoday+sum(datediff(hour,@last,@now)) from point where aid>0 and 9>aid and enddate>@now at isolation read uncommitted


	SELECT @player_transinmonth=count(distinct buyer) from translog where aid>0 and 9>aid and date>dateadd(day,-30,@now) at isolation read uncommitted
	SELECT @player_transnoadd=count(distinct buyer) from translog,agentbill where translog.buyer=agentbill.uid and translog.aid>0 and 9>translog.aid and agentbill.usedate>dateadd(day,-30,@now)
	SELECT @player_trans = @player_transinmonth - @player_transnoadd






	SELECT @payremainall=convert(int,sum(b.time*1.0)/3600) from #billinfo b where b.addupmoney>0
	SELECT @paynetremainall=isnull(convert(int,sum(b.time*1.0)/3600),0) from #billinfo b where b.addupmoney>0 and b.addupmoney*90>b.time
	SELECT @paynetremainall=@paynetremainall+isnull(convert(int,sum(b.addupmoney*90.0)/3600),0) from #billinfo b where b.addupmoney>0 and b.time>b.addupmoney*90
	SELECT @paypresentall=10*count(*) from #billinfo


	SELECT @monthremainall=sum(datediff(hour,@now,enddate)) from #billinfo where enddate>@now at isolation read uncommitted


	SELECT @monthnetremainall=isnull(convert(int,sum(b.time*1.0+b.monthtime*58.0*2.5/720)/3600),0) from #billinfo b where b.addupmoney>0 and b.addupmoney*90.0>b.time*1.0+b.monthtime*58.0*2.5/720
	SELECT @monthnetremainall=@monthnetremainall+isnull(convert(int,sum(b.addupmoney*90.0)/3600),0) from #billinfo b where b.addupmoney>0 and b.time*1.0+b.monthtime*58.0*2.5/720>b.addupmoney*90.0
	SELECT @monthnetremainall = convert(int,(@monthnetremainall - @paynetremainall)/(58*2.5)*720)


	SELECT @paynetremainall2=isnull(convert(int,sum(b.time*1.0)/3600),0) from #billinfo b where b.addupmoney>0 and b.adduppoint>b.time
	SELECT @paynetremainall2=@paynetremainall2+isnull(convert(int,sum(b.adduppoint*1.0)/3600),0) from #billinfo b where b.addupmoney>0 and b.time>b.adduppoint
	SELECT @monthnetremainall2=isnull(convert(int,sum(b.time*1.0+b.monthtime*58.0*2.5/720)/3600),0) from #billinfo b where b.addupmoney>0 and b.adduppoint*1.0>b.time*1.0+b.monthtime*58.0*2.5/720
	SELECT @monthnetremainall2=@monthnetremainall2+isnull(convert(int,sum(b.adduppoint*1.0)/3600),0) from #billinfo b where b.addupmoney>0 and b.time*1.0+b.monthtime*58.0*2.5/720>b.adduppoint*1.0
	SELECT @monthnetremainall2 = convert(int,(@monthnetremainall2 - @paynetremainall2)/(58*2.5)*720)


	update #result set addall = @addall,
	consumedall = @consumedall,
	exchgtomonthall = @exchgtomonthall,
	remainall = @remainall,
	presentall = @presentall,
	addtoday = @addtoday,
	addtodaymoney = @addtodaymoney,
	exchgtomonthtoday = @exchgtomonthtoday,
	difftoday = @difftoday,
	consumedtoday = @consumedtoday,
	consumedbymonthtoday = @consumedbymonthtoday,
	player_transinmonth = @player_transinmonth,
	player_transnoadd = @player_transnoadd,
	player_trans = @player_trans,
	payremainall = @payremainall,
	paynetremainall = @paynetremainall,
	paypresentall = @paypresentall,
	netconsumedtoday = @netconsumedtoday,
	netconsumedbymonthtoday = @netconsumedbymonthtoday,
	monthremainall = @monthremainall,
	monthnetremainall = @monthnetremainall,
	paynetremainall2 = @paynetremainall2,
	monthnetremainall2 = @monthnetremainall2
go

	SELECT now, addall, consumedall, exchgtomonthall, remainall, presentall, addtoday, addtodaymoney, exchgtomonthtoday, difftoday, consumedtoday, consumedbymonthtoday, player_transinmonth, player_transnoadd, player_trans, payremainall, paynetremainall, paypresentall, netconsumedtoday, netconsumedbymonthtoday, monthremainall, monthnetremainall, 1.0, paynetremainall2, monthnetremainall2 FROM #result
go

