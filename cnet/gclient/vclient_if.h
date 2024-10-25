#ifndef __VCLIENT_IF_H
#define __VCLIENT_IF_H


namespace VCLIENT
{
	void Init();
	void Quit();
	void AddPlayer(int roleid);
	void RemovePlayer(int roleid);
	void OnEnterWorld(int roleid);
	void SendC2SGameData(int roleid, void * buf, size_t size);
	void RecvS2CGameData(int roleid, void * buf, size_t size);
	void RecvConsoleCmd(int target, int source, char * buf, size_t size);
}

#endif
