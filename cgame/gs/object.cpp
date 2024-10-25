#include <stdlib.h>
#include <ASSERT.h>
#include "object.h"
#include "grid.h"
#include "world.h"

gobject_imp * CF_CreateImp(int guid,world * plane, gobject * obj)
{
	gobject_imp * pTmp = CreateDerivedInstance<gobject_imp>(guid);
	ASSERT(pTmp);
	pTmp->Init(plane,obj);
	return pTmp;

}

dispatcher * CF_CreateDispatcher(int guid, gobject_imp * imp)
{
	dispatcher * pTmp = CreateDerivedInstance<dispatcher>(guid);
	pTmp->init(imp);
	return pTmp;
}

controller * CF_CreateCtrl(int guid, gobject_imp * imp)
{
	controller * pTmp = CreateDerivedInstance<controller>(guid);
	pTmp->Init(imp);
	return pTmp;
}

