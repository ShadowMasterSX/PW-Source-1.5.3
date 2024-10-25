#ifndef __ONLINEGAME_GS_PLAYER_TITLE_H__
#define __ONLINEGAME_GS_PLAYER_TITLE_H__
#include "common/packetwrapper.h"
#include "titlemanager.h"
/**
 *	这个类保存在玩家称号管理时需要保存的一些状态和处理内容
 */

class gplayer_imp;
class dispatcher;
class player_title
{
public:
	typedef abase::vector<TITLE_ID> 	TITLE_ID_VEC;
	typedef	abase::vector<TITLE_EXPIRE> TITLE_EXPIRE_VEC;
	
	player_title(gplayer_imp* p) : _owner(p)  	   { _current_title = 0;}
	~player_title() { _delivered_titles.clear(); _expire_titles.clear();}
public:
	bool 	Save(archive & ar);
	bool 	Load(archive & ar);
	void 	Swap(player_title & rhs);
	void 	OnHeartbeat(int curtime);

public:
	bool 	CalcAllTitleEnhance(bool updateplayer);
	bool	ObtainTitle(TITLE_ID tid,int expire);
	bool	LoseTitle(TITLE_ID tid);
	bool	IsExistTitle(TITLE_ID tid);
	bool	ChangeCurrTitle(TITLE_ID tid);
	bool	QueryTitleData(dispatcher* runner);
	bool	InitFromDB(archive & ar);
	bool	SaveToDB(archive & ar);

private:
	
	TITLE_ID_VEC 		_delivered_titles;
	TITLE_EXPIRE_VEC   	_expire_titles;
	gplayer_imp* const	_owner;
	TITLE_ID			_current_title;
	
};

#endif
