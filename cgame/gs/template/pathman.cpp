#include "pathman.h"
#include "sevbezier.h"


bool
path_manager::Init(const char * filename)
{
	CSevBezierMan CBM;
	if(!CBM.Load(filename)) return false;
	typedef CSevBezierMan::BezierTable TABLE; 
	const TABLE & tab = CBM.GetTable();

	TABLE::const_iterator it = tab.begin();
	for(;it != tab.end(); ++it)
	{
		int id = *it.key();
		CSevBezier * pBezier = *it.value();
		int global_id = pBezier->GetGlobalID();
		single_path * pPath = new single_path();
		pPath->id = id;
		do
		{
			CSevBezierWalker walker;
			walker.BindBezier(pBezier);
			walker.SetSpeed(8.0f);
			walker.StartWalk(false,true);
			do{
				A3DVECTOR3 pos = walker.GetPos();
				pPath->Push(pos);
				walker.Tick(1000);
				A3DVECTOR3 pos2 = walker.GetPos();
				if(!walker.IsWalking() || (pos2-pos).Normalize() < 1e-3) break;
			}while(1);
			if(pBezier->GetNextGlobalID() < 0)
			{
				break;
			}
			int id2 = pBezier->GetNextGlobalID();
			TABLE::const_iterator it2 = tab.begin();
			CSevBezier * pBezier2 = NULL;
			for(;it2 != tab.end(); ++it2)
			{
				if(*it.key() == -1) continue;
				pBezier2 = *it2.value();
				if(pBezier2->GetGlobalID() == id2) break;
			}
			if(pBezier2 == NULL || pBezier2->GetGlobalID() != id2)
			{
				pBezier2 = NULL;
				printf("未发现合适的后继路线 %d\n",id2);
			}
			/*else
			  {
			  printf("发现合适的后继路线%d\n",id2);
			  }*/
			pBezier = pBezier2;
		}while(pBezier);
		bool bRst = _path_tab.put(id,pPath);
		if(!bRst){
			printf("重复的路线ID被发现\n");
			return false;
		}
		if(global_id > 0)
		{
			bool bRst2 = _convert_tab.put(global_id,id);
			if(!bRst2){
				printf("重复的路线GlobalID被发现\n");
				return false;
			}
		}
	}
	return true;
}

