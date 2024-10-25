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
	if(_initialized) return false;   //�Ѿ����ع�������
	
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
		GLog::log(GLOG_ERR, "DPS���а����ݼ���ʧ��.ar.size()=%d",ar.size());
		Clear(); //���ݲ���ȷ���������
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
	
	/*rank_mask����ͻ���Э����maskֵ��������λ��Ч��Ϣ
	���и���λ�������а����
	DPH_ALL_RANK = 00,
	DPH_CLS_RANK = 01,
	DPS_ALL_RANK = 10,
	DPS_CLS_RANK = 11,
	
	����λ��������ְҵ����ֵӦ����[0, 10)֮�������
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
	if(!rank.empty()) min_val = rank.begin()->first; //���а�Ŀǰ��͵Ĺ�����ֵ
	
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
	//���а���RunTick����ˢ�£�RunTickÿ10�������Ӧ���߼�����
	//Normal״̬�£����а�ÿ600��ˢ��һ�Σ���ʱ���ؾ����´�ˢ�µ�ʣ��ʱ��
	//����״̬�����а񲻻����б仯����ͳһ����600��
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
	
	MakeRankLogEntry(DPH_ALL_RANK, _dph_rank, DPH_RANK_SIZE); //dph�ܰ�
	for(int i = 0; i < USER_CLASS_COUNT; ++i) {
		MakeRankLogEntry(DPH_CLS_RANK, _cls_dph_rank[i], CLS_DPH_RANK_SIZE); //dph����ְҵ��
	}
	//Ŀǰlog�����dps�񵥣����޸�����
}

void dps_rank_manager::OutputRankLog()
{
	if(!_log_entry_list.empty()) { //��log���ݴ���������Ѿ�����InitLogList
		unsigned int log_cnt = ((_log_entry_list.size() < 10) ? _log_entry_list.size() : 10); //һ�����д10��log

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

	if(++_tick_counter < 200) return;	//10��һ��
	_tick_counter = 0;

	time_t now = time(NULL);
	now += time_adjust; //debugģʽ��
	struct tm* tm1 = localtime(&now);
		
	const int sunday = 0;
	const int rank_locktime = 19;

	if(_state == STATE_NORMAL) {
		++_write_tickcnt; 
		if(_write_tickcnt >= 60) { //10��һ�Σ�60�μ�600�룬10����
			SortRank();
			_write_tickcnt = 0;
		}

		if((tm1->tm_wday == sunday) && (tm1->tm_hour >= rank_locktime)) { //����19����������а�
			SortRank(); //дlog�͹���ǰ��������
			InitLogEntryList(); //��ʼ���������log����

			_state = STATE_ANNOUNCE; //�����а����ڹ������а�ھ���״̬
		}
	} else if(_state == STATE_ANNOUNCE) {
		if(tm1->tm_wday != sunday) { //0���ѹ���������а�����״̬����ʼһ���µ�����
			Clear(); //������а�
			_last_announce_hour = 0;
			_state = STATE_NORMAL; //�����а����û�����״̬
		} else { //ÿ��һ����Ȼ���㣬����
			if((tm1->tm_hour > _last_announce_hour) && (!_dph_rank.empty()) ) {
				RANK::reverse_iterator it = _dph_rank.rbegin();
				player_rank_info * pInfo = it->second;

				if(pInfo != NULL) {
					BroadcastResult(pInfo->roleid, pInfo->level, pInfo->dps, pInfo->dph, 1/*rank*/);
					_last_announce_hour = tm1->tm_hour;
				}
			}
		}
	} else { //����ȷ��״̬
		Clear(); //������а�
		_last_announce_hour = 0;
		_state = STATE_NORMAL;
	}

	OutputRankLog(); //�����а����������log
}

