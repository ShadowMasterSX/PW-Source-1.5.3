#ifndef __ONLINEGAME_GS_TEAMRELATIONJOB_H__
#define __ONLINEGAME_GS_TEAMRELATIONJOB_H__

class TeamRelationJob : public ONET::Thread::Runnable
{
protected:
	world * _plane; 
	int _leader;
	abase::vector<int , abase::fast_alloc<> > _list;
	int error_msg;

	void SetErrorMsg(int err){ error_msg = err; }
public:
	TeamRelationJob(gplayer_imp * pImp, const XID * list, size_t count);
	virtual ~TeamRelationJob(){}

	virtual void Run()
	{
		OnRun();
		delete this;
	}

private:
	bool OnRun();
	virtual bool OnTeamCheck(gplayer ** list, size_t count) = 0;
	virtual bool OnLeaderCheck(gplayer_imp * pImp) = 0;
	virtual bool OnMemberCheck(gplayer_imp * pImp) = 0;
	virtual bool OnExecute(gplayer ** list, size_t count) = 0;
};

class WeddingBookJob : public TeamRelationJob
{
	int wedding_start_time;
	int wedding_end_time;
	int wedding_scene;
	int bookcard_index;
public:
	WeddingBookJob(gplayer_imp * pImp, const XID * list, size_t count, int start_time, int end_time, int scene, int index)
		:TeamRelationJob(pImp,list,count),
		wedding_start_time(start_time),wedding_end_time(end_time),wedding_scene(scene),bookcard_index(index){}
private:
	virtual bool OnTeamCheck(gplayer ** list, size_t count);
	virtual bool OnLeaderCheck(gplayer_imp * pImp);
	virtual bool OnMemberCheck(gplayer_imp * pImp);
	virtual bool OnExecute(gplayer ** list, size_t count);
};

class WeddingCancelBookJob : public TeamRelationJob
{
	int wedding_start_time;
	int wedding_end_time;
	int wedding_scene;
	int leader_invitecard_index;
	int member_invitecard_index;
public:
	WeddingCancelBookJob(gplayer_imp * pImp, const XID * list, size_t count, int start_time, int end_time, int scene)
		:TeamRelationJob(pImp,list,count),
		wedding_start_time(start_time),wedding_end_time(end_time),wedding_scene(scene),
		leader_invitecard_index(-1),member_invitecard_index(-1){}
private:
	virtual bool OnTeamCheck(gplayer ** list, size_t count);
	virtual bool OnLeaderCheck(gplayer_imp * pImp);
	virtual bool OnMemberCheck(gplayer_imp * pImp);
	virtual bool OnExecute(gplayer ** list, size_t count);
};

class CountryJoinApplyJob: public TeamRelationJob
{
	int level;
	int ticket;
public:
	CountryJoinApplyJob(gplayer_imp * pImp, const XID * list, size_t count, int lvl, int t)
		:TeamRelationJob(pImp,list,count),
		level(lvl),ticket(t){}
private:
	virtual bool OnTeamCheck(gplayer ** list, size_t count);
	virtual bool OnLeaderCheck(gplayer_imp * pImp);
	virtual bool OnMemberCheck(gplayer_imp * pImp);
	virtual bool OnExecute(gplayer ** list, size_t count);
};
#endif
