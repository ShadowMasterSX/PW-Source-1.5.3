#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "usermsg.h"
#include "weddingmanager.h"

bool wedding_manager::Initialize()
{
	DATA_TYPE dt;
	WEDDING_CONFIG * cfg = (WEDDING_CONFIG *)world_manager::GetDataMan().get_data_ptr(WEDDING_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if(cfg == NULL || dt != DT_WEDDING_CONFIG) return false;
	//��ʼ������ʱ���
	int last_end_time = 0;
	for(size_t i=0; i<sizeof(cfg->wedding_session)/sizeof(cfg->wedding_session[0]); i++)
	{
		if(cfg->wedding_session[i].start_hour <= 0) break;
		PERIOD p;
		p.start_hour = cfg->wedding_session[i].start_hour;
		p.start_min = cfg->wedding_session[i].start_min;
		p.end_hour = cfg->wedding_session[i].end_hour;
		p.end_min = cfg->wedding_session[i].end_min;
		if(!p.Valid())
		{
			__PRINTF("����ʱ��β��Ϸ�1\n");	
			return false;
		}
		int start_time = p.start_hour*3600+p.start_min*60; 
		if(start_time < last_end_time)
		{
			__PRINTF("����ʱ��β��Ϸ�2\n");	
			return false;
		}
		//
		last_end_time = p.end_hour*3600+p.end_min*60;
		schedule_periods.push_back(p);
	}
	if(!schedule_periods.size()) return false;
	//��ʼ����������
	for(size_t i=0; i<sizeof(cfg->reserved_day)/sizeof(cfg->reserved_day[0]); i++)
	{
		if(cfg->reserved_day[i].year <= 0) break;
		DAY d;
		d.year = cfg->reserved_day[i].year; 
		d.month = cfg->reserved_day[i].month; 
		d.day = cfg->reserved_day[i].day; 
		if(!d.Valid())
		{
			__PRINTF("�����������ڲ��Ϸ�\n");	
			return false;
		}
		special_days.push_back(d);
	}
	//����booking list
	time_t t1 = time(NULL);
	struct tm * tm1 = localtime(&t1);
	t_base = t1 - 86400*tm1->tm_wday - 3600*tm1->tm_hour - 60*tm1->tm_min - tm1->tm_sec; //��������ʼʱ��: ���� 00:00:00
	AddWedding(t_base - 604800, 21);
	
	return true;
}

void wedding_manager::RunTick()
{
	spin_autolock keeper(lock);
	if(!initialized) return;
	//
	if(++ tick_counter < 200) return;	//10��һ��
	tick_counter = 0;

	time_t t1 = time(NULL);
	bool has_ongoing = false;
	for(int i=0; i<WEDDING_SCENE_COUNT; i++)
	{
		if(ongoing_index[i] >= 0) has_ongoing = true;
	}
	if(!has_ongoing)
	{
		//�ǻ���ʱ���У����ж��Ƿ�������ʱ��
		for(size_t i=0; i<booking_list.size(); i++)
		{
			if(t1 >= booking_list[i].start_time && t1 < booking_list[i].end_time)
			{
				int scene = booking_list[i].scene;
				ASSERT(scene >= 0 && scene < WEDDING_SCENE_COUNT);
				ASSERT(ongoing_index[scene] < 0);

				has_ongoing = true;
				ongoing_index[scene] = (int)i;
				if(booking_list[i].status == ST_BOOKED)
					booking_list[i].status = ST_ONGOING;
			}		
		}
		if(!has_ongoing)
		{
			//û�н������ʱ�Σ��ж��Ƿ���Ҫ����booking list
			if(t1 - t_base > 604800)
			{
				//�����µ�һ��
				t_base += 604800;
				for(int i=0; i<WEDDING_SCENE_COUNT; i++)
					ongoing_index[i] = -1;
				//ɾ�����ڵ�booking list
				DelWedding(t_base - 604800);
				//�����µ�booking list
				AddWedding(t_base + 604800, 7);	
			}
		}
	}
	else
	{
		//�ڻ���ʱ���У������жϵ�ǰʱ���Ƿ����	
		for(int i=0; i<WEDDING_SCENE_COUNT; i++)
		{
			if(ongoing_index[i] < 0) continue;
			wedding & wedding_ongoing = booking_list[ongoing_index[i]];
			if(t1 >= wedding_ongoing.end_time)
			{
				if(wedding_ongoing.status == ST_ONGOING)
					wedding_ongoing.status = ST_FINISH;
				ongoing_index[i] = -1;
			}
		}
	}
}

//ע��:�˲�����ʹongoing_indexʧЧ
void wedding_manager::DelWedding(int tbase)
{
	abase::vector<wedding>::iterator it = booking_list.begin();
	for( ; it!=booking_list.end(); ++it)
	{
		if(it->start_time >= tbase) 
			break;				
		if(it->status != ST_UNBOOKED && it->status != ST_FINISH)
		{
			GLog::log(GLOG_ERR,"�������ʱ���ֻ���(����%d ����%d ��ʼʱ��%d ����ʱ��%d ��ǰ״̬%d)δ��������",it->groom,it->bride,it->start_time,it->end_time,it->status);
		}
	}
	booking_list.erase(booking_list.begin(),it);
}

void wedding_manager::AddWedding(int tbase, int day_num)
{
	time_t t1 = tbase;
	struct tm * tm1;
	for(int i=0; i<day_num; i++)
	{
	 	tm1 = localtime(&t1);
		bool special = false;
		for(size_t j=0; j<special_days.size(); j++)
		{
			if(tm1->tm_year+1900 == special_days[j].year
					&& tm1->tm_mon+1 == special_days[j].month
					&& tm1->tm_mday == special_days[j].day)
			{
				special = true;
				break;
			}
		}
		for(size_t j=0; j<schedule_periods.size(); j++)
		{
			wedding w;
			w.start_time = t1 + schedule_periods[j].start_hour*3600 + schedule_periods[j].start_min*60;
			w.end_time = t1 + schedule_periods[j].end_hour*3600 + schedule_periods[j].end_min*60;
			w.status = ST_UNBOOKED;
			w.special = special;
			for(int sc=0; sc<WEDDING_SCENE_COUNT; sc++)
			{
				w.scene = sc;
				booking_list.push_back(w);	
			}
		}
		t1 += 86400;
	}
}

bool wedding_manager::UpdateWedding(int start_time, int end_time, int groom, int bride, int scene, char status)
{
	for(size_t i=0; i< booking_list.size(); i++)
	{
		wedding & w = booking_list[i];
		if(w.start_time == start_time && w.end_time == end_time && w.scene == scene)
		{
			w.groom = groom;
			w.bride = bride;
			w.status = status;
			return true;
		}
	}
	return false;
}

bool wedding_manager::CheckOngoing(int groom, int bride, int scene)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;
	
	if(scene < 0 || scene >= WEDDING_SCENE_COUNT) return false;
	if(ongoing_index[scene] < 0) return false;
	wedding & wedding_ongoing = booking_list[ongoing_index[scene]];

	return wedding_ongoing.status == ST_ONGOING 
		&& groom == wedding_ongoing.groom
		&& bride == wedding_ongoing.bride;
}

bool wedding_manager::CheckOngoing(int id)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;

	for(int scene=0; scene<WEDDING_SCENE_COUNT; scene++)
	{
		if(ongoing_index[scene] < 0) continue;
		wedding & wedding_ongoing = booking_list[ongoing_index[scene]];
		if(wedding_ongoing.status == ST_ONGOING
				&& (id == wedding_ongoing.groom || id == wedding_ongoing.bride))
			return true;
	}
	return false;
}

bool wedding_manager::SendBookingList(int id, int cs_index, int cs_sid)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;

	packet_wrapper h1(1024);
	using namespace S2C;
	CMD::Make<CMD::wedding_book_list>::From(h1,booking_list.size());
	for(size_t i=0; i<booking_list.size(); i++)
	{
		wedding & w = booking_list[i];
		CMD::Make<CMD::wedding_book_list>::AddEntry(h1,w.start_time,w.end_time,w.groom,w.bride,w.scene,w.status,w.special);			
	}
	send_ls_msg(cs_index, id, cs_sid, h1);	
	return true;
}

bool wedding_manager::CheckBook(int start_time, int end_time, int scene, int card_year, int card_month, int card_day)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;
	
	if(start_time <= time(NULL)) return false;
	//�޸�Ϊͬһʱ���ֻ�ܽ���һ������
	bool find = false;
	for(size_t i=0; i<booking_list.size(); i++)
	{
		if(booking_list[i].start_time == start_time && booking_list[i].end_time == end_time)
		{
			if(booking_list[i].status != ST_UNBOOKED) return false;
			if(booking_list[i].scene == scene)
			{
				if(booking_list[i].special)
				{
					time_t t1 = booking_list[i].start_time;
					struct tm * tm1 = localtime(&t1);
					if(tm1->tm_year+1900 != card_year || tm1->tm_mon+1 != card_month || tm1->tm_mday != card_day) return false;
				}
				find = true;
			}
		}
	}
	return find;
}

bool wedding_manager::TryBook(int start_time, int end_time, int groom, int bride, int scene)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;

	if(start_time <= time(NULL)) return false;
	for(size_t i=0; i<booking_list.size(); i++)
	{
		if(booking_list[i].start_time == start_time && booking_list[i].end_time == end_time && booking_list[i].scene == scene)
		{
			if(booking_list[i].status != ST_UNBOOKED) return false;
			booking_list[i].status = ST_BOOKED;
			booking_list[i].groom = groom;
			booking_list[i].bride = bride;
			return true;
		}
	}
	return false;
}

bool wedding_manager::CheckCancelBook(int start_time, int end_time, int groom, int bride, int scene)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;
	
	for(size_t i=0; i<booking_list.size(); i++)
	{
		if(booking_list[i].start_time == start_time && booking_list[i].end_time == end_time
				&& booking_list[i].groom == groom && booking_list[i].bride == bride
				&& booking_list[i].scene == scene)
		{
			if(booking_list[i].status != ST_BOOKED) return false;
			return true;
		}
	}
	return false;
}

bool wedding_manager::TryCancelBook(int start_time, int end_time, int groom, int bride, int scene)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;

	for(size_t i=0; i<booking_list.size(); i++)
	{
		if(booking_list[i].start_time == start_time && booking_list[i].end_time == end_time
				&& booking_list[i].groom == groom && booking_list[i].bride == bride
				&& booking_list[i].scene == scene)
		{
			if(booking_list[i].status != ST_BOOKED) return false;
			booking_list[i].status = ST_UNBOOKED;
			booking_list[i].groom = 0;
			booking_list[i].bride = 0;
			return true;
		}
	}
	return false;
}

bool wedding_manager::DBLoad(archive & ar)
{
	spin_autolock keeper(lock);
	if(initialized) return false;	//�Ѿ����ع�������
	
	try
	{
		if(ar.size())
		{
			size_t size;
			int start_time, end_time, groom, bride,	scene;
			char status;
			ar >> size;
			while(size--)
			{
				ar >> start_time >> end_time >> groom >> bride >> scene >> status;	
				if(!UpdateWedding(start_time, end_time, groom, bride, scene, status))
				{
					//���ݿ��ܴ��˻����ǹ�����
					GLog::log(GLOG_WARNING, "�������ݼ���ʱ���ִ�������Ч.start_time=%d,end_time=%d,groom=%d,bride=%d,scene=%d,status=%d",
							start_time, end_time, groom, bride, scene, status);
				}
			}
		}
	}
	catch(...)
	{
		GLog::log(GLOG_ERR, "�������ݼ���ʧ��.ar.size()=%d",ar.size());
		return false;
	}
	initialized = true;
	return true;
}

bool wedding_manager::DBSave(archive & ar)
{
	spin_autolock keeper(lock);
	if(!initialized) return false;

	size_t size = 0;
	for(size_t i=0; i<booking_list.size(); i++)
	{
		wedding & w = booking_list[i];
		if(w.status != ST_UNBOOKED)
			size++;
	}
	ar << size;
	for(size_t i=0; i<booking_list.size(); i++)
	{
		wedding & w = booking_list[i];
		if(w.status != ST_UNBOOKED)
		{
			ar << w.start_time << w.end_time << w.groom << w.bride << w.scene << w.status;	
		}
	}
	return true;
}
