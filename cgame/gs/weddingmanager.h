#ifndef __ONLINEGAME_GS_WEDDINGMANAGER_H__
#define __ONLINEGAME_GS_WEDDINGMANAGER_H__

class wedding_manager
{
public:
	struct DAY
	{
		int year;
		int month;
		int day;
		bool Valid()
		{
			static int month_days[13] = {0/*û��*/,31,29/*������������*/,31,30,31,30,31,31,30,31,30,31};
			return year >= 0 
				&& month >= 1 && month <= 12
				&& day >= 1 && day <= month_days[month]; 
		}
	};
	struct PERIOD 
	{
		int start_hour;
		int start_min;
		int end_hour;
		int end_min;
		bool Valid()
		{ 
			return start_hour >= 0 && start_hour <= 23
				&& start_min >= 0 && start_min <= 59
				&& end_hour >= 0 && end_hour <= 23
				&& end_min >= 0 && end_min <= 59
				&& (start_hour < end_hour || (start_hour==end_hour && start_min<end_min));
		}
	};
	enum{
		ST_INVALID = 0,
		ST_UNBOOKED,
		ST_BOOKED,
		ST_ONGOING,
		ST_FINISH,
	};
	struct wedding 
	{
		int start_time;
		int end_time;
		int groom;
		int bride;
		int scene;
		char status;
		bool special;
		wedding():start_time(0),end_time(0),groom(0),bride(0),scene(0),status(ST_INVALID),special(false){}
	};

public:
	wedding_manager():initialized(false),lock(0),tick_counter(0),t_base(0)
	{
		for(int i=0; i<WEDDING_SCENE_COUNT; i++)
			ongoing_index[i] = -1;
	}
	bool Initialize();
	void RunTick();

	bool CheckOngoing(int groom, int bride, int scene);
	bool CheckOngoing(int id);
	bool SendBookingList(int id, int cs_index, int cs_sid);
	bool CheckBook(int start_time, int end_time, int scene, int card_year, int card_month, int card_day);
	bool TryBook(int start_time, int end_time, int groom, int bride, int scene);
	bool CheckCancelBook(int start_time, int end_time, int groom, int bride, int scene);
	bool TryCancelBook(int start_time, int end_time, int groom, int bride, int scene);
	bool DBLoad(archive & ar);
	bool DBSave(archive & ar);

private:
	void DelWedding(int tbase);				//ɾ��tbase��ǰ�Ļ���
	void AddWedding(int tbase, int day_num);//����tbase��ʼ��num�����Ļ���
	bool UpdateWedding(int start_time, int end_time, int groom, int bride, int scene, char status);
	
private:
	bool initialized;
	int lock;
	int tick_counter;
	int t_base;		//��������ʼʱ��: ���� 00:00:00
	
	int ongoing_index[WEDDING_SCENE_COUNT];
	abase::vector<wedding> booking_list;
	
	abase::vector<DAY> special_days;
	abase::vector<PERIOD> schedule_periods; 
};
#endif
