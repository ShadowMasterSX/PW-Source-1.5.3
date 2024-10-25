
#ifndef __GNET_FORCE_MANAGER_H__
#define __GNET_FORCE_MANAGER_H__

#include <vector>
#include <gforceglobaldata>
#include <gforceglobaldatalist>

namespace GNET
{

class ForceManager : public IntervalTimer::Observer
{
public:
	enum
	{
		UPDATE_INTERVAL 		= 3000000,
		WRITEDB_CHECK_INTERVAL 	= 37000000,
		SYNCGS_CHECK_INTERVAL	= 3000000,
	};
	enum
	{
		ST_INIT,
		ST_OPEN,
	};	
	enum
	{
		ACTIVITY_LEVEL_0 = 0,	//正常,
		ACTIVITY_LEVEL_1,		//低落，前一天活跃度最低
		ACTIVITY_LEVEL_2,		//奋进，前一天活跃度最高
		ACTIVITY_LEVEL_3,		//激昂，前两天活跃度最高
	};
	typedef std::vector<GForceGlobalData> FORCE_LIST;
	typedef std::vector<bool> 			FLAG_LIST;
private:
	ForceManager() : _status(ST_INIT),_dirty_flag(false),_writeback_flag(false),_writedb_timer(0),
					 _syncgs_timer(0),
					 _update_time(0)
	{}
	bool IsOpen(){ return _status != ST_INIT; }
	int GetForceIndex(int force_id);
	void UpdateForceData();
	void WriteToDB();
	void SyncAllToGS(int sid = -1);
public:
	~ForceManager(){}
	static ForceManager * GetInstance() { static ForceManager instance; return &instance; }

	bool Initialize();
	void OnDBConnect(Protocol::Manager * manager, int sid);
	void OnGSConnect(Protocol::Manager * manager, int sid);
	void OnDBLoad(const GForceGlobalDataList & list);
	bool Update();

	void OnDBPutForce(int retcode);

	void PlayerJoinOrLeave(int force_id, bool is_join);
	void IncreaseActivity(int force_id, int activity);
	
private:
	int 			_status;
	
	bool			_dirty_flag;
	bool			_writeback_flag;
	int				_writedb_timer;
	
	FLAG_LIST		_syncgs_flag_list;
	int				_syncgs_timer;
	//数据库存储的数据
	int				_update_time;
	FORCE_LIST		_force_list;
};


}

#endif
