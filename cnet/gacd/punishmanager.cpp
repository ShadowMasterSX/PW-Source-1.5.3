
#include "gacdutil.h"
#include "punishmanager.hpp"
#include "logmanager.hpp"
#include "cheater.hpp"
#include "forbidlibrary.hpp"
#include "mempatternlibrary.hpp"
#include "punishmanager.hpp"
#include "usercodemanager.hpp"
#include "stringhelper.hpp"
#include "ganticheaterserver.hpp"
#include "preparedcodelibrary.hpp"

namespace GNET
{

PunishManager PunishManager::s_instance;

int PunishManager::KickRule::CalForbidTime(int iTimes)
{
    if( iTimes <= 1 ) return m_iForbidTime[0];
	if( iTimes > FT_CLASS ) return m_iForbidTime[FT_CLASS-1];
	return m_iForbidTime[iTimes-1];

}   
/*
        
void PunishManager::PunishCodeQueue::SendPunishCode(int uid)
{
    for(std::vector<ACRemoteCode>::const_iterator it = codes.begin(), ie = codes.end(); it != ie; ++it)
    {
        ACRemoteCode code = (*it);
        code.dstroleid = uid;
        GAntiCheaterServer::GetInstance()->SendProtocol(code);
    }
}

void PunishManager::CheckPunishCode(int cid, int subid, int uid)
{
    int rid = -1;
    BindCodeCheatMap::iterator it = bindcode_map.find(cid);
    if( it != bindcode_map.end() )
    {
        rid = (*it).second;
    }
    BindCodeCheatSubMap::iterator it3 = bindcode_sub_map.find(std::pair<int,int>(cid, subid));
    if( it3 != bindcode_sub_map.end() )
    {
        rid = (*it3).second;
    }
    if( rid == -1 ) return;
    PunishCodeMap::iterator it2 = punish_code_map.find(rid);
    if( it2 == punish_code_map.end() ) return;
    (*it2).second.SendPunishCode(uid);
}
*/

void PunishManager::RunPunisher(const Punisher *punisher)
{
	m_punisherList.push_back(std::make_pair(punisher->GetDelayTime()+1, punisher));
}


void PunishManager::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
	ClearRules();
  //  punish_code_map.clear();
  //  bindcode_map.clear();
  //  bindcode_sub_map.clear();
        
    const XmlConfig::Node *manager = pRoot->GetFirstChild("punishmanager");
            
	/*
    nodes = manager->GetChildren("punishcode");
    for(XmlConfig::Nodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        std::string sid = (*it)->GetAttr("id");
        std::string sname = (*it)->GetAttr("name");
        if( sid.empty() || sname.empty() ) continue;
        PunishCodeQueue pcq;
        pcq.id = atoi(sid.c_str());
        pcq.name = sname;
        // load punish codes here
        XmlConfig::Nodes codes = (*it)->GetChildren("bindcode");
        int in = 0;
        for(XmlConfig::Nodes::const_iterator it2 = codes.begin(); it2 != codes.end(); ++it2)
        {
            std::string code_name = (*it2)->GetAttr("name");
            if( code_name.empty() ) continue;
            std::map<short,short> refs;
            std::vector<std::string> params;
            std::string param = (*it2)->GetAttr("param");
            if( ! param.empty() ) params.push_back(StringHelper::utf8_to_iso1(param));
            ACRemoteCode pro;
            PreparedCodeLibrary::GetInstance()->MakeCode(code_name, pro.content, refs, params);
            pcq.codes.push_back(pro);
			++in;
        }
        if( in == 0 ) continue;
        punish_code_map[pcq.id] = pcq;
        DEBUG_PRINT_INIT("-=-=-make punish code queue %d\n", pcq.id);
        XmlConfig::Nodes binders = (*it)->GetChildren("bindcheater");
        for(XmlConfig::Nodes::const_iterator it2 = binders.begin(); it2 != binders.end(); ++it2)
        {
            std::string tmpstr = (*it2)->GetAttr("type");
            if( !tmpstr.empty())
            {
                std::string tmpsubid = (*it2)->GetAttr("subid");
                if( tmpsubid.empty() )
                {
                    bindcode_map[atoi(tmpstr.c_str())] = pcq.id;
                    DEBUG_PRINT_INIT("-=-=-=-cheat type %d bind to punishcode %d\n", atoi(tmpstr.c_str()), pcq.id);
                }
                else
                {
                    bindcode_sub_map[std::pair<int,int>(atoi(tmpstr.c_str()), atoi(tmpsubid.c_str()))] = pcq.id
;
                    DEBUG_PRINT_INIT("-=-=-=-cheat type %d, subid %d to bind to punishcode %d\n",
                        atoi(tmpstr.c_str()), atoi(tmpsubid.c_str()), pcq.id);
                }
            }
        }
    }
	*/
	const XmlConfig::Nodes kickrules = manager->GetChildren("kickrule");
    for(XmlConfig::Nodes::const_iterator it = kickrules.begin(), ie = kickrules.end(); it != ie; ++it)
    {
		KickRule *pKickRule = new KickRule();
		(*it)->GetIntAttr("id", &pKickRule->m_sID);
		(*it)->GetIntAttr("delay_min", &pKickRule->m_iMinDelay);
		(*it)->GetIntAttr("delay_max", &pKickRule->m_iMaxDelay);
		(*it)->GetStrAttr("name", pKickRule->m_name);
		if( (*it)->GetStrAttr("reason", pKickRule->m_reason) )
			pKickRule->m_reason = StringHelper::utf8_to_iso1(pKickRule->m_reason);
		(*it)->GetIntAttr("first", &pKickRule->m_iForbidTime[0]);
		(*it)->GetIntAttr("second", &pKickRule->m_iForbidTime[1]);
		(*it)->GetIntAttr("max", &pKickRule->m_iForbidTime[2]);

        DEBUG_PRINT_INIT("-=-=-make rule %d\n", pKickRule->m_sID);
        XmlConfig::Nodes binders = (*it)->GetChildren("bindcheater");
        for(XmlConfig::Nodes::const_iterator it2 = binders.begin(); it2 != binders.end(); ++it2)
        {
			BindItem bindItem;
            if( (*it2)->GetIntAttr("type", &bindItem.m_iCheatID) )
            {
            	if( (*it2)->GetIntAttr("subid", &bindItem.m_iSubID) )
                {
					bindItem.m_iBindType = BindItem::BT_CHEATID_AND_SUBID;
                    DEBUG_PRINT_INIT("-=-=-=-cheat type %d, subid %d to bind rule %d\n",
                        bindItem.m_iCheatID, bindItem.m_iSubID, pKickRule->m_sID);
                }
                else
                {
					bindItem.m_iBindType = BindItem::BT_CHEATID;
                    DEBUG_PRINT_INIT("-=-=-=-cheat type %d bind to rule %d\n", bindItem.m_iCheatID, pKickRule->m_sID);
                }
            }
			if( bindItem.m_iBindType != BindItem::BT_NULL )
			{
				BindMap::iterator it3 = m_bindMap.find(bindItem);
				if( it3 != m_bindMap.end() ) it3->second.m_pKickRule = pKickRule;
				else { m_bindMap[bindItem] = BindRules(pKickRule); }
	
			}
        }

		m_kickRules.push_back(pKickRule);
    }
	DEBUG_PRINT_INIT("bind map size %d\n", m_bindMap.size());

}

void PunishManager::DeliverCheater(const Cheater &cheater)
{
	LogManager::GetInstance()->Log(cheater);
	
	BindRules bindRules;
	if( FindBindRules(cheater, bindRules ) )
	{

		DEBUG_PRINT_PUNISH("find bind rules\n");
		KickRule *pKickRule = bindRules.m_pKickRule;
		if( pKickRule != NULL )
		{
			DEBUG_PRINT_PUNISH("kick rule is not null\n");
			int iForbidTime = pKickRule->CalForbidTime(cheater.m_iTimes);
			DEBUG_PRINT_PUNISH("forbid time = %d\n", iForbidTime);
			if( iForbidTime >= KickRule::FT_DISCONNECT )
			{
				int iDelay = ( iForbidTime <= 0 ) ? 
					0 : (pKickRule->m_iMinDelay + rand()%(pKickRule->m_iMaxDelay - pKickRule->m_iMinDelay));
            	RunPunisher(new KickPunisher(cheater.m_iUserID, iForbidTime>KickRule::FT_DISCONNECT
						, iForbidTime, pKickRule->m_reason, iDelay));
			}
		}

		else 
		{
		}
    }

}

void PunishManager::OnTimer()
{
	for(PunisherList::iterator it = m_punisherList.begin(), ie = m_punisherList.end(); it != ie;)
	{
		it->first = it->first -1;
		if( it->first <= 0 )
		{
			PunisherList::iterator oldit = it;
			++it;
			oldit->second->Process();
			delete oldit->second;
			m_punisherList.erase(oldit);
			continue;
		}
		++it;
	}
}

};

