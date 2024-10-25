#ifndef __ONLINEGAME_GS_HISTORY_MANAGER_H__
#define __ONLINEGAME_GS_HISTORY_MANAGER_H__

#include <vector.h>

class itemdataman;
// ��uniquedataclient���в������л����Ա��ز�����
class history_manager
{
public:
	history_manager() : _initialized(false), _stagelimit(0), _stagevalue(0), _stageversion(-1) {}
	~history_manager() 	{}
public:
	bool Initialize(itemdataman & dataman);
	bool OnSetVersion(int version);									// �汾����
	bool OnSetValue(int value, int stageid);						// ��ʷ��������
	bool OnAdvance(int version, bool localflag, int retcode); 		// �汾�ƽ�
	bool OnStep(int value,int stageid,bool advance,int retcode ,bool localflag); 	// ��ʷ�������

	int  GetLocalStageVal()  { return _stagevalue; }
	int  GetStageVersion() { return _stageversion +1; }
	int  GetStageLimit() { return _stagelimit; }	
	
private:
	bool _initialized;
	int  _stagelimit;
	int  _stagevalue;
	int  _stageversion;
};

#endif

