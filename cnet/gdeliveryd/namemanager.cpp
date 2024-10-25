#include "namemanager.h"
#include "dbrolenamelist.hrp"
#include "matcher.h"
#include "mapuser.h"
#include "gdeliveryserver.hpp"

namespace GNET
{

bool NameManager::AddName(int id, const Octets & name)
{
	Thread::Mutex::Scoped l(locker_namemanager);
	if (!m_init)
		return false;
	m_namemap[id] = name;
	return true;
}

bool NameManager::DelName(int id)
{
	Thread::Mutex::Scoped l(locker_namemanager);
	if (!m_init)
		return false;
	if (m_namemap.find(id) == m_namemap.end())
		return false;
	m_namemap.erase(id);	
	return true;
}

bool NameManager::FindName(int id, Octets &name)
{
	Thread::Mutex::Scoped l(locker_namemanager);
	if (!m_init)
		return false;
	NameMap::iterator it = m_namemap.find(id);
	if (it == m_namemap.end())
		return false;
	name = it->second;
	return true;	
}

int NameManager::CheckNewName(Octets &name)
{
	Thread::Mutex::Scoped l(locker_namemanager);
	if (!IsInit())
		return ERR_PR_OUTOFSERVICE;
	int tmpid = 0;
	if (UserContainer::GetInstance().FindRoleId(name, tmpid))
	{
		return ERR_PR_DUPLICATE;
	}
	if (!(name.size()>=2 && name.size()<=GDeliveryServer::GetInstance()->max_name_len &&
			Matcher::GetInstance()->Match((char*)name.begin(),name.size())==0))
		return ERR_PR_VALIDNAME;
	return ERR_SUCCESS;
}

bool NameManager::IsInit()
{
	return m_init;
}

void NameManager::OnDBConnect(Protocol::Manager *manager, int sid)
{
	m_init = false;               
	manager->Send(sid, Rpc::Call(RPC_DBROLENAMELIST, DBRoleNameListArg()));
}

void NameManager::InitName(std::vector<GRoleIDandName> & rolenamelist, char finish)
{
	if (m_init)
	{
		m_init = false;
		m_namemap.clear();
	}
	Thread::Mutex::Scoped l(locker_namemanager);
	for (size_t i = 0; i < rolenamelist.size(); i++)
	{
		GRoleIDandName & ridn = rolenamelist[i];
		m_namemap[ridn.roleid] = ridn.rolename;
	}
	if (finish)
	{
		m_init = true;
	}
}

}
