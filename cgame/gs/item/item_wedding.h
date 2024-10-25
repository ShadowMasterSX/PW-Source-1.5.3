#ifndef _ITEM_WEDDING_H_
#define _ITEM_WEDDING_H_

#include <stddef.h>
#include "../item.h" 

struct wedding_bookcard_essence
{
	int year;
	int month;
	int day;
};

class wedding_bookcard_item : public item_body
{
	struct wedding_bookcard_essence ess;
	
public:
	DECLARE_SUBSTANCE(wedding_bookcard_item);
	wedding_bookcard_item():_crc(0){memset(&ess, 0, sizeof(ess));}

	virtual bool Load(archive & ar)
	{
		ar.pop_back(&ess, sizeof(ess));
		CalcCRC();
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &ess;
		len = sizeof(ess);
 	}
	
	unsigned short _crc;
	unsigned short GetDataCRC() { return _crc; }
	void CalcCRC()
	{   
		_crc = crc16( (unsigned char *)&ess, sizeof(ess));
	}

public:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_WEDDING_BOOKCARD;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new wedding_bookcard_item(*this);}

	virtual bool GetBookCardData(int & year, int & month, int & day)
	{
		year = ess.year; month = ess.month; day = ess.day;
		return true;
	}
};

struct wedding_invitecard_essence
{
	int start_time;
	int end_time;
	int groom;
	int bride;
	int scene;
	int invitee;
};

class wedding_invitecard_item : public item_body
{
	struct wedding_invitecard_essence ess;
	
public:
	DECLARE_SUBSTANCE(wedding_invitecard_item);
	wedding_invitecard_item():_crc(0){memset(&ess, 0, sizeof(ess));}

	virtual bool Load(archive & ar)
	{
		ar.pop_back(&ess, sizeof(ess));
		CalcCRC();
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &ess;
		len = sizeof(ess);
 	}
	
	unsigned short _crc;
	unsigned short GetDataCRC() { return _crc; }
	void CalcCRC()
	{   
		_crc = crc16( (unsigned char *)&ess, sizeof(ess));
	}

public:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_WEDDING_INVITECARD;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new wedding_invitecard_item(*this);}

	virtual bool SetInviteCardData(int start_time, int end_time, int groom, int bride, int scene, int invitee)
	{
		ess.start_time = start_time;
		ess.end_time = end_time;
		ess.groom = groom;
		ess.bride = bride;
		ess.scene = scene;
		ess.invitee = invitee;
		return true;
	}
	virtual bool GetInviteCardData(int& start_time, int& end_time, int& groom, int& bride, int& scene, int& invitee)
	{
		start_time = ess.start_time;
		end_time = ess.end_time;
		groom = ess.groom;
		bride = ess.bride;
		scene = ess.scene;
		invitee = ess.invitee;
		return true;
	}
private:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual int OnGetUseDuration() { return 0;}	//0 代表要排队，但使用时间为0
};
#endif
