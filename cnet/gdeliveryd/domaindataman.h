#ifndef _DOMAINDATAMAN_H_
#define _DOMAINDATAMAN_H_

#include <vector>

// data and methods for domain
enum DOMAIN_TYPE
{
	DOMAIN_TYPE_NULL = 0,
	DOMAIN_TYPE_3RD_CLASS,
	DOMAIN_TYPE_2ND_CLASS,
	DOMAIN_TYPE_1ST_CLASS,
};

typedef struct _DOMAIN_INFO_SERV
{
	int						id;				// id of the domain
	DOMAIN_TYPE				type;			// type of the domain
	int						reward;			// money rewarded per week
	std::vector<int>		neighbours;		// neighbours of this domain

} DOMAIN_INFO_SERV;

typedef struct _BATTLETIME_SERV
{
	int						nDay;
	int						nHour;
	int						nMinute;

} BATTLETIME_SERV;

enum DOMAIN2_TYPE
{
	DOMAIN2_TYPE_SINGLE = 0,
	DOMAIN2_TYPE_CROSS,
};

typedef struct _DOMAIN2_INFO_SERV
{
	int						id;
	int						point;			//	积分
	int						wartype;		//	战争类型
	int						country_id;		//	初始归属阵营
	int						is_capital;		//	是否是阵营首都
	float					mappos[4][3];	//	对应地宫中三维位置
	std::vector<int>		neighbours;
	std::vector<int>		time_neighbours;	
} DOMAIN2_INFO_SERV;

///////////////////////////////////////////////////////////////////////////////
// load data from a config file
// return	0 if succeed
//			-1 if failed.
///////////////////////////////////////////////////////////////////////////////
int domain_data_load();
int domain2_data_load(char domain2_type);

// get the data of a domain by id
// return NULL if not found
DOMAIN_INFO_SERV * domain_data_getbyid(int id);

// get the data of a domain by index
// return NULL if not found
DOMAIN_INFO_SERV * domain_data_getbyindex(int index);

// get the number of domain data
int domain_data_getcount();

// get battle time list
const std::vector<BATTLETIME_SERV>& getbattletimelist();
// get number of battle at the same time
int getbattletimemax();

DOMAIN2_INFO_SERV * domain2_data_getbyid(int id);
DOMAIN2_INFO_SERV * domain2_data_getbyindex(int index);
DOMAIN2_INFO_SERV * domain2_get_capital_data(int camp_id);
const std::vector<DOMAIN2_INFO_SERV>* get_domain2_infos();
unsigned int get_domain2_data_timestamp();

#endif//_DOMAINDATAMAN_H_

