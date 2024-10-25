#include <time.h>
#include <glog.h>
#include "string.h"
#include "world.h"
#include "common/message.h"

#include "usermsg.h"
#include "worldmanager.h"
#include "dpsrankmanager.h"

int dps_rank_manager::time_adjust = 0;

bool dps_rank_manager::Initialize()
{
	return true; 
}

bool dps_rank_manager::DBLoad(archive & ar)
{
	spin_autolock keeper(_lock);
	if(_initialized) return false;   //已经加载过数据了
	
	try
	{
		if(ar.size())
		{
			size_t size;
			int roleid, level, cls, dps, dph;
			ar >> size;
			while(size--)
			{
				ar >> roleid >> level >> cls >> dps >> dph;
				InitAdd(roleid, level, cls, dps, dph);
			}
		}
	}
	catch(...)
	{
		GLog::log(GLOG_ERR, "DPS排行榜数据加载失败.ar.size()=%d",ar.size());
		Clear(); //数据不正确，清除数据
	}
	_initialized = true;
	_state = STATE_NORMAL;
	SortRank();
	return true;
}

void dps_rank_manager::SaveRankData(archive& ar, const RANK& rank)
{
	for(RANK::const_reverse_iterator it = rank.rbegin(); it != rank.rend(); ++it) {
		player_rank_info * pInfo = it->second;
		if(pInfo != NULL) ar << pInfo->roleid << pInfo->level << pInfo->cls << pInfo->dps << pInfo->dph;
	}
}

bool dps_rank_manager::DBSave(archive & ar)
{
	spin_autolock keeper(_lock);
	if(!_initialized) return false;
	
	int total_cnt = _dps_rank.size() + _dph_rank.size();
	for(int i = 0; i < USER_CLASS_COUNT; ++i) {
		total_cnt += _cls_dps_rank[i].size() + _cls_dph_rank[i].size();;
	}
	
	ar << total_cnt;
	SaveRankData(ar, _dps_rank);
	SaveRankData(ar, _dph_rank);

	for(int i = 0; i < USER_CLASS_COUNT; ++i) {
		SaveRankData(ar, _cls_dps_rank[i]);
		SaveRankData(ar, _cls_dph_rank[i]);
	}

	return true;
}

bool dps_rank_manager::UpdateRankInfo(int roleid, int level, int cls, int dps, int dph)
{
	if((cls < USER_CLASS_SWORDSMAN) && (cls >= USER_CLASS_COUNT)) return false;

	spin_autolock keeper(_lock);
	if(!_initialized) return false;
	if(_state != STATE_NORMAL) return false;
		
	player_rank_info *& pInfo = _map[roleid];
	if(!pInfo) pInfo = new player_rank_info();
	pInfo->roleid = roleid;
	pInfo->level = level;
	pInfo->cls = cls;
	if(dps > pInfo->dps) pInfo->dps = dps;
	if(dph > pInfo->dph) pInfo->dph = dph;

	_changed = true;
	return true;
}

void dps_rank_manager::DoSendRankData(int link_id, int roleid, int link_sid, const RANK& rank, unsigned char rank_mask)
{
	using namespace S2C;
	packet_wrapper h1(sizeof(CMD::dps_dph_rank::_entry) * (rank.size()) + sizeof(CMD::dps_dph_rank));
	CMD::Make<CMD::dps_dph_rank>::From(h1, NextSortTime(), rank.size(), rank_mask);
	
	for(RANK::const_reverse_iterator it = rank.rbegin(); it != rank.rend(); ++it) {
		int attack_val = it->first;
		player_rank_info* pInfo = it->second;
		if(pInfo != NULL) CMD::Make<CMD::dps_dph_rank>::AddEntry(h1, pInfo->roleid, pInfo->level, attack_val);
	}
	
	send_ls_msg(link_id, roleid, link_sid, h1);
}

bool dps_rank_manager::SendRank(int link_id, int roleid, int link_sid, unsigned char rank_mask)
{
	spin_autolock keeper(_lock);
	if(!_initialized) return false;
	
	/*rank_mask是与客户端协定的mask值，包含六位有效信息
	其中高两位表明排行榜类别
	DPH_ALL_RANK = 00,
	DPH_CLS_RANK = 01,
	DPS_ALL_RANK = 10,
	DPS_CLS_RANK = 11,
	
	低四位用来表明职业，其值应该是[0, 10)之间的整数
	*/
	
	unsigned char rank_type = (rank_mask >> 4);
	unsigned char cls_idx = (rank_mask & 0x0F);
	if(cls_idx >= USER_CLASS_COUNT) return false;

	switch(rank_type)
	{
		case DPH_ALL_RANK:
			DoSendRankData(link_id, roleid, link_sid, _dph_rank, rank_mask);
			break;
		case DPH_CLS_RANK:
			DoSendRankData(link_id, roleid, link_sid, _cls_dph_rank[cls_idx], rank_mask);
			break;
		case DPS_ALL_RANK:
			DoSendRankData(link_id, roleid, link_sid, _dps_rank, rank_mask);
			break;
		case DPS_CLS_RANK:
			DoSendRankData(link_id, roleid, link_sid, _cls_dps_rank[cls_idx], rank_mask);
			break;
		default:
			return false;
	}
	
	return true;
}

void dps_rank_manager::InitAdd(int roleid, int level, int cls, int dps, int dph)
{
	if((cls < USER_CLASS_SWORDSMAN) && (cls >= USER_CLASS_COUNT)) return;

	player_rank_info *& pInfo = _map[roleid];
	if(!pInfo) 
	{
		pInfo = new player_rank_info(roleid,level,cls,dps,dph);
		_changed = true;
	}
}

void dps_rank_manager::UpdatePlayerRank(player_rank_info* pInfo, int attack_value, RANK& rank, unsigned int rank_capacity)
{	
	int min_val = 0;
	if(!rank.empty()) min_val = rank.begin()->first; //排行榜目前最低的攻击力值
	
	if(rank.size() < rank_capacity) {
		rank.insert(std::make_pair(attack_value, pInfo));	
	} else if(attack_value > min_val) {
		rank.erase(rank.begin());
		rank.insert(std::make_pair(attack_value, pInfo));
	}
}

void dps_rank_manager::SortRank()
{
	if(!_changed) return;

	_dps_rank.clear();
	_dph_rank.clear();
	for(int i = 0; i < USER_CLASS_COUNT; ++i) {
		_cls_dps_rank[i].clear();
		_cls_dph_rank[i].clear();
	}

	for(MAP::iterator it=_map.begin(); it!=_map.end(); ++it) {
		player_rank_info* pInfo = it->second;

		UpdatePlayerRank(pInfo, pInfo->dps, _dps_rank, DPS_RANK_SIZE);
		UpdatePlayerRank(pInfo, pInfo->dph, _dph_rank, DPH_RANK_SIZE);
		
		ASSERT((pInfo->cls >= USER_CLASS_SWORDSMAN) && (pInfo->cls < USER_CLASS_COUNT));
		UpdatePlayerRank(pInfo, pInfo->dps, _cls_dps_rank[pInfo->cls], CLS_DPS_RANK_SIZE);
		UpdatePlayerRank(pInfo, pInfo->dph, _cls_dph_rank[pInfo->cls], CLS_DPH_RANK_SIZE);
	}
	
	_changed = false;
}

int dps_rank_manager::NextSortTime()
{
	//排行榜在RunTick心跳刷新，RunTick每10秒进行相应的逻辑处理
	//Normal状态下，排行榜每600秒刷新一次，此时返回距离下次刷新的剩余时间
	//其他状态，排行榜不会再有变化，但统一返回600秒
	int ret = (60 - _write_tickcnt) * 10;
	if(_state != STATE_NORMAL) ret = 600;
	return ret;
}

void dps_rank_manager::BroadcastResult(int roleid, int level, int dps, int dph, char rank)
{
#pragma pack(1)
	struct {
		int roleid;
		int level;
		int dps;
		int dph;
		char rank;
	} data;
#pragma pack()

	memset(&data, 0,sizeof(data)); 
	
	data.roleid = roleid;
	data.level = level;
	data.dps = dps;
	data.dph = dph;
	data.rank = rank;

	broadcast_chat_msg(DPS_MAN_CHAT_MSG_ID, &data, sizeof(data), GMSV::CHAT_CHANNEL_SYSTEM, 0, 0, 0);
}

void dps_rank_manager::MakeRankLogEntry(char mask, const RANK& rank, int rank_capacity)
{
	short rank_idx = 1;
	for(RANK::const_reverse_iterator it = rank.rbegin(); it != rank.rend(); ++it) {
		player_rank_info * pInfo = it->second;
		if(pInfo != NULL) {
			log_entry entry;
			entry.mask = mask;
			entry.cls = pInfo->cls;
			entry.rank = rank_idx;
			entry.roleid = pInfo->roleid;
			entry.dps = pInfo->dps;;
			entry.dph = pInfo->dph;

			_log_entry_list.push_back(entry);
			++rank_idx;
			if(rank_idx > rank_capacity) break;
		}
	}
}

void dps_rank_manager::InitLogEntryList()
{
	_log_entry_list.clear();
	
	MakeRankLogEntry(DPH_ALL_RANK, _dph_rank, DPH_RANK_SIZE); //dph总榜
	for(int i = 0; i < USER_CLASS_COUNT; ++i) {
		MakeRankLogEntry(DPH_CLS_RANK, _cls_dph_rank[i], CLS_DPH_RANK_SIZE); //dph各个职业榜
	}
	//目前log不输出dps榜单，尚无该需求
}

void dps_rank_manager::OutputRankLog()
{
	if(!_log_entry_list.empty()) { //有log数据待输出，即已经经过InitLogList
		unsigned int log_cnt = ((_log_entry_list.size() < 10) ? _log_entry_list.size() : 10); //一次最多写10条log

		for(unsigned int i = 0; i < log_cnt; ++i) {
			log_entry& entry = _log_entry_list[i];
			GLog::formatlog("formatlog:dph_rank::mask=%d:cls=%d:rank=%d:roleid=%d:dps=%d:dph=%d", entry.mask, entry.cls, entry.rank, entry.roleid, entry.dps, entry.dph);
		}
		
		_log_entry_list.erase(_log_entry_list.begin(), _log_entry_list.begin() + log_cnt);
	}
}

void dps_rank_manager::RunTick()
{
	spin_autolock keeper(_lock);
	if(!_initialized) return;

	if(++_tick_counter < 200) return;	//10秒一次
	_tick_counter = 0;

	time_t now = time(NULL);
	now += time_adjust; //debug模式用
	struct tm* tm1 = localtime(&now);
		
	const int sunday = 0;
	const int rank_locktime = 19;

	if(_state == STATE_NORMAL) {
		++_write_tickcnt; 
		if(_write_tickcnt >= 60) { //10秒一次，60次即600秒，10分钟
			SortRank();
			_write_tickcnt = 0;
		}

		if((tm1->tm_wday == sunday) && (tm1->tm_hour >= rank_locktime)) { //周日19点后，锁定排行榜
			SortRank(); //写log和公告前重新排序
			InitLogEntryList(); //初始化待输出的log数据

			_state = STATE_ANNOUNCE; //将排行榜置于公告排行榜冠军的状态
		}
	} else if(_state == STATE_ANNOUNCE) {
		if(tm1->tm_wday != sunday) { //0点已过，清空排行榜，重置状态，开始一个新的周期
			Clear(); //清空排行榜
			_last_announce_hour = 0;
			_state = STATE_NORMAL; //将排行榜重置回正常状态
		} else { //每过一个自然整点，公告
			if((tm1->tm_hour > _last_announce_hour) && (!_dph_rank.empty()) ) {
				RANK::reverse_iterator it = _dph_rank.rbegin();
				player_rank_info * pInfo = it->second;

				if(pInfo != NULL) {
					BroadcastResult(pInfo->roleid, pInfo->level, pInfo->dps, pInfo->dph, 1/*rank*/);
					_last_announce_hour = tm1->tm_hour;
				}
			}
		}
	} else { //不正确的状态
		Clear(); //清空排行榜
		_last_announce_hour = 0;
		_state = STATE_NORMAL;
	}

	OutputRankLog(); //将排行榜数据输出到log
}

