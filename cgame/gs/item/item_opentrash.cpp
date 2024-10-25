#include "../world.h"
#include "../clstab.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_opentrash.h"
#include "../task/taskman.h"
#include "../player_imp.h"
#include "../actsession.h"

DEFINE_SUBSTANCE(item_open_trash,item_body,10000)

int
item_open_trash::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	gplayer_imp * pImp = (gplayer_imp *)obj;
	if(pImp->HasSession()) 
	{
		pImp->_runner->error_message(S2C::ERR_OTHER_SESSION_IN_EXECUTE);
		return -1;
	}
	if(!pImp->_trashbox.CheckPassword("",0))
	{
		pImp->_runner->error_message(S2C::ERR_PASSWD_NOT_MATCH);
		return -1;
	}

	session_use_trashbox *pSession = new session_use_trashbox(pImp);
	pImp->AddSession(pSession);
	pImp->StartSession();
	return 1;
}


