#ifndef __GNET_UNIQUEDATAMANAGER_H__
#define __GNET_UNIQUEDATAMANAGER_H__

#include <itimer.h>
#include "guniquedataelem"
#include "guniquedataelemnode"
#include <map>
#include <deque>

namespace GNET
{

class UniqueDataServer : public IntervalTimer::Observer
{
	UniqueDataServer(): _clientsynctick(0) , _initialized(false) { }
	~UniqueDataServer() { }
public:
	static UniqueDataServer * GetInstance(){ static UniqueDataServer instance; return &instance; }
	
	bool Initialize();
	virtual bool Update();
	
	void OnDBConnect(Protocol::Manager * manager, int sid);
	void OnDBLoad(GUniqueDataElemNodeVector& values,bool finish);
	void OnGSConnect(Protocol::Manager * manager, int sid);
public:
	void ModifyUniqueData(int worldtag,int key, int vtype, Octets& val,Octets& oldval, bool excl, bool broadcast, int sid, int version,bool timeout);
	void InitGSData(int worldtag,int sid);
private:
	void SyncModifyToDB();
	void SyncModifyToClient();
	void SyncModifyToGS(int worldtag,int key, int vtype, Octets& val,Octets& oldval, bool excl,int retcode ,int version,int sid = -1);
	void SyncAllToGS(int sid = -1);

protected:	
	// GUniqueData数据 壳
	struct SGUniqueData
	{
		// 数据类型安全读写及脏写标志 壳
		struct SGUniqueDataElem
		{
			SGUniqueDataElem() : wflag(false),broadcast(false){}

			int modify(int vtype,Octets& newdata,Octets& olddata,bool exclusive,int& version,bool timeout);

			bool wflag;
			bool broadcast;
			GUniqueDataElem elem;
		};

		enum
		{
			SAVE_TO_DB,
			SAVE_TO_GS,
			SAVE_TO_CL,
		};

		typedef std::map<int, SGUniqueDataElem> SGUMap;
		typedef int GUHandle;
		
		void load( GUniqueDataElemNodeVector& values )
		{
			GUniqueDataElemNodeVector::iterator iter = values.begin();
			GUniqueDataElemNodeVector::iterator iend = values.end();

			for(;iter != iend; ++iter)
			{
				elems[iter->key].elem = iter->val;
				elems[iter->key].wflag = false;
			}
		}
		
		bool save( GUniqueDataElemNodeVector& values ,int type, GUHandle& handle )
		{
			SGUMap::iterator iter = (handle == -1) ? elems.begin() : elems.find(handle);
			SGUMap::iterator iend = elems.end();
			
			int count = 0;
			int totalsize = 0;
			for(; iter!=iend ; ++iter) // same as db
			{
				if(count > 255 || totalsize > 65535)
				{
					handle = iter->first; 
					return false;
				}
				
				switch(type)
				{
					case SAVE_TO_DB:
						if(!iter->second.wflag)
							continue;
						iter->second.wflag = false;
						break;
					case SAVE_TO_CL:
						if(!iter->second.broadcast)
							continue;
						iter->second.broadcast = false;
						break;
					case SAVE_TO_GS:
						break;
				}

				values.push_back(GUniqueDataElemNode(iter->first,iter->second.elem)); 
				totalsize += sizeof(iter->first) + sizeof(iter->second.elem.vtype) + iter->second.elem.value.size();
				++count;	
			}	

			return true;
		}

		SGUMap elems;
	} _data;

	int  _clientsynctick;

	bool _initialized;
	Thread::Mutex   lock;
};

}

#endif
