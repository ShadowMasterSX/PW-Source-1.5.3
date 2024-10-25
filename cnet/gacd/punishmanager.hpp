#ifndef __GNET_PUNISHMANAGER_HPP
#define __GNET_PUNISHMANAGER_HPP

#include <list>
#include <vector>
#include <map>
#include "cheater.hpp"
#include "punisher.hpp"
#include "acremotecode.hpp"
#include "xmlconfig.h"

namespace GNET
{

class PunishManager
{
	static PunishManager s_instance;

	typedef std::list<std::pair<int, const Punisher*> > PunisherList;
	PunisherList m_punisherList;

	struct KickRule
    {
		enum { FT_DISCONNECT = -1 };
		enum { FT_CLASS = 3 };
		enum { FDT_MIN = 30, FDT_MAX = 60 };

        short m_sID;
        int m_iForbidTime[FT_CLASS];
		int m_iMinDelay;
		int m_iMaxDelay;

        std::string m_name;
		std::string m_reason;

        KickRule() : m_sID(0) 
			, m_iMinDelay(FDT_MIN), m_iMaxDelay(FDT_MAX) 
		{
			for(int i=0; i< FT_CLASS; ++i) m_iForbidTime[i] = 0;
			m_name = "null";
			m_reason = "请勿使用非法的第三方软件。"; 
		}
        int CalForbidTime(int iTimes);
    };

	typedef std::vector<KickRule*> KickRules;
	KickRules m_kickRules;

	struct BindItem
	{
		enum { BT_NULL, BT_CHEATID, BT_CHEATID_AND_SUBID };
		int m_iBindType;
		int m_iCheatID;
		int m_iSubID;

		BindItem() : m_iBindType(BT_NULL), m_iCheatID(0), m_iSubID(0) { }

		bool operator==(const BindItem &bi) const
		{
			return this == &bi 
				|| m_iBindType == bi.m_iBindType && m_iCheatID == bi.m_iCheatID && m_iSubID == bi.m_iSubID;
		}

		bool operator<(const BindItem &bi) const
		{
			if( this == &bi ) return false;
			if( m_iBindType < bi.m_iBindType ) return true;
			if( m_iBindType == bi.m_iBindType && m_iCheatID < bi.m_iCheatID  ) return true;
			if( m_iBindType == bi.m_iBindType && m_iCheatID == bi.m_iCheatID 
				&& m_iSubID < bi.m_iSubID ) return true;
			return false;
		}
	};

	struct BindRules	
	{
		KickRule *m_pKickRule;
		// other
		BindRules() : m_pKickRule(NULL) { }
		BindRules(KickRule *pKickRule) : m_pKickRule(pKickRule) { }
	};

	typedef std::map<BindItem, BindRules> BindMap;
	BindMap m_bindMap;

	bool FindBindRules(const Cheater &cheater, BindRules &bindRules) const
	{
		for(BindMap::const_iterator it = m_bindMap.begin(), ie = m_bindMap.end(); it != ie; ++it)
		{
			if( (*it).first.m_iBindType == BindItem::BT_CHEATID_AND_SUBID 
					&& (*it).first.m_iCheatID == cheater.m_iCheatID 
					&& (*it).first.m_iSubID == cheater.m_iSubID )
			{
				bindRules = (*it).second;
				return true;
			}
		}
		for(BindMap::const_iterator it = m_bindMap.begin(), ie = m_bindMap.end(); it != ie; ++it)
		{
			if( (*it).first.m_iBindType == BindItem::BT_CHEATID 
					&& (*it).first.m_iCheatID == cheater.m_iCheatID ) 
			{
				bindRules = (*it).second;
				return true;
			}
		}
		return false;
	}
	/*
    struct PunishCodeQueue
    {
        short id;
        std::string name;
        std::vector<ACRemoteCode> codes;
        PunishCodeQueue() : id(0) { }
        void SendPunishCode(int uid);
    };
    typedef std::map<short, PunishCodeQueue> PunishCodeMap;
    typedef std::map<int, short> BindCodeCheatMap;
    typedef std::map<std::pair<int, int>, short> BindCodeCheatSubMap;
    PunishCodeMap punish_code_map;
    BindCodeCheatMap bindcode_map;
    BindCodeCheatSubMap bindcode_sub_map;
	*/
	/*
    typedef std::map<short, KickTimeRule> KickTimeRuleMap;
    typedef std::map<int, short> BindCheatMap;
    typedef std::map<std::pair<int, int>, short> BindCheatSubMap;

    KickTimeRuleMap rule_map;
    BindCheatMap bind_map;
	BindCheatSubMap bind_sub_map;
	*/
	void ClearRules()
	{
		m_bindMap.clear();
		for(KickRules::iterator it = m_kickRules.begin(), ie = m_kickRules.end(); it != ie; ++it )
			delete (*it);
		m_kickRules.clear();
	}

	void ClearPunishers()
	{
		for(PunisherList::iterator it = m_punisherList.begin(), ie = m_punisherList.end(); it != ie; ++it )
			delete (*it).second;  
		m_punisherList.clear();
	}

public:

    //void CheckPunishCode(int cid, int subid, int uid);

	~PunishManager() { 
		ClearRules();
		ClearPunishers();
	}

	static PunishManager *GetInstance() { return &s_instance; }

	void RunPunisher(const Punisher* punisher);
	void DeliverCheater(const Cheater &cheater);

	void OnTimer();
	void OnUpdateConfig(const XmlConfig::Node *pRoot);

private:
	PunishManager() { m_punisherList.clear();}
	PunishManager(const PunishManager &pm);
	PunishManager& operator=(const PunishManager &pm);
	bool operator==(const PunishManager &pm) const;
};

};

#endif
