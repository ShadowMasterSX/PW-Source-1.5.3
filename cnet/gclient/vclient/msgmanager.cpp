#include "dbgprt.h"
#include "msg.h"
#include "msgmanager.h"
#include "playermanager.h"
#include "virtualclient.h"

MsgQueue::~MsgQueue()
{
	std::list<MSG *>::iterator it = _list.begin();
	for( ; it!=_list.end(); ++it)
	{
		FreeMessage(*it);
	}
	_list.clear();
}

void MsgQueue::Run()
{
	std::list<MSG *>::iterator it = _list.begin();
	for( ; it!=_list.end(); ++it)
	{
		g_virtualclient.GetPlayerManager()->DispatchMessage(*it);	
	}
	delete this;
}


