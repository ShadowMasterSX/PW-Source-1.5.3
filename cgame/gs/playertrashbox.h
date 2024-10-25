#ifndef __ONLINEG_GAME_GS_PLAYER_TRASH_BOX_H__
#define __ONLINEG_GAME_GS_PLAYER_TRASH_BOX_H__

#include "item.h"
#include "item_list.h"
#include "config.h"
#include <string.h>
#include <vector.h>

class player_trashbox
{
	size_t 	  	_money;
	item_list 	_box;
	unsigned char	_passwd[16];
	bool		_has_passwd;

	item_list 	_box2;		//²ÄÁÏ²Ö¿â
	item_list 	_box3;		//Ê±×°²Ö¿â
	item_list 	_box4;		//¿¨ÅÆ²Ö¿â
public:
	enum
	{
		MAX_PASSWORD_SIZE = 24,
	};
	static bool IsPasswordValid(const char * str, size_t size)
	{
		static char pass_char[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()+-_='`~;:,<.>/? \"";
		if(size == 0) return true;
		if(size > MAX_PASSWORD_SIZE) return false;
		for(size_t i = 0; i < size;i ++)
		{
			if(strchr(pass_char,str[i]) == NULL) return false;
		}
		return true;
	}

	void SetPassword(const char * str, size_t size);
	void SetPasswordMD5(const char * str, size_t size);
	bool CheckPassword(const char * str, size_t size);

	bool HasPassword()
	{
		return _has_passwd;
	}

	const char * GetPassword(size_t & size)
	{
		if(_has_passwd)
		{
			size = 16;
			return (const char*) _passwd;
		}
		else
		{
			size = 0;
			return NULL;
		}
	}
	
	
protected:
	player_trashbox():_money(0),_box(item::BACKPACK,TRASHBOX_BASE_SIZE),_has_passwd(false),_box2(item::BACKPACK,0),_box3(item::BACKPACK,0),_box4(item::BACKPACK,TRASHBOX_BASE_SIZE4)
	{
	}

public:
	player_trashbox(int size, int size4):_money(0),_box(item::BACKPACK,size),_has_passwd(false),_box2(item::BACKPACK,0),_box3(item::BACKPACK,0),_box4(item::BACKPACK,size4)
	{
	}

	void SetOwner(gactive_imp * obj)
	{
		_box.SetOwner(obj);
		_box2.SetOwner(obj);
		_box3.SetOwner(obj);
		_box4.SetOwner(obj);
	}

	size_t & GetMoney() { return _money; }
	item_list & GetBackpack1() { return _box;}
	item_list & GetBackpack2() { return _box2;}
	item_list & GetBackpack3() { return _box3;}
	item_list & GetBackpack4() { return _box4;}

	int GetTrashBoxSize()
	{
		return _box.Size();
	}

	int GetTrashBoxSize2()
	{
		return _box2.Size();
	}

	int GetTrashBoxSize3()
	{
		return _box3.Size();
	}

	int GetTrashBoxSize4()
	{
		return _box4.Size();
	}

	void SetTrashBoxSize(int size)
	{
		if((int)_box.Size() >= size) return ;
		if(size > TRASHBOX_MAX_SIZE) return;
		_box.SetSize(size);
	}

	void SetTrashBoxSize2(int size)
	{
		if((int)_box2.Size() >= size) return ;
		if(size > TRASHBOX_MAX_SIZE) return;
		_box2.SetSize(size);
	}

	void SetTrashBoxSize3(int size)
	{
		if((int)_box3.Size() >= size) return ;
		if(size > TRASHBOX_MAX_SIZE) return;
		_box3.SetSize(size);
	}
	
	void SetTrashBoxSize4(int size)
	{
		if((int)_box4.Size() >= size) return ;
		if(size > TRASHBOX_MAX_SIZE) return;
		_box4.SetSize(size);
	}
	

	bool Save(archive & ar)
	{
		ar << _money;
		bool nosave;
		_box.Save(ar,nosave);
		_box2.Save(ar,nosave);
		_box3.Save(ar,nosave);
		_box4.Save(ar,nosave);
		ar << _has_passwd;
		ar.push_back(_passwd,16);
		return true;
	}

	bool Load(archive & ar)
	{
		ar >> _money;
		_box.Load(ar);
		_box2.Load(ar);
		_box3.Load(ar);
		_box4.Load(ar);
		ar >> _has_passwd;
		ar.pop_back(_passwd,16);
		return true;
	}

	void Swap(player_trashbox & rhs)
	{
		abase::swap(_money,rhs._money);
		_box.Swap(rhs._box);
		_box2.Swap(rhs._box2);
		_box3.Swap(rhs._box3);
		_box4.Swap(rhs._box4);
		abase::swap(_has_passwd,rhs._has_passwd);

		unsigned char	passwd[16];
		ASSERT(sizeof(passwd) == sizeof(_passwd));
		memcpy(passwd,_passwd,sizeof(_passwd));
		memcpy(_passwd,rhs._passwd,sizeof(_passwd));
		memcpy(rhs._passwd,passwd,sizeof(_passwd));
	}
};

#endif

