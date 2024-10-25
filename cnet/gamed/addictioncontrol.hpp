
#ifndef __GNET_ADDICTIONCONTROL_HPP
#define __GNET_ADDICTIONCONTROL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gpair"
#include <map>


void wallow_control(int userid, int rate, std::map<int,int> & data, int msg);
namespace GNET
{

class AddictionControl : public GNET::Protocol
{
	#include "addictioncontrol"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// rate:     0-����״̬, 1-�����, 2-����
		// data.key: 1-�ۼ�����ʱ�䣬 2-��������״̬ʱ���,  3-�������״̬
		// msg:		 0-�û��������֤�Ų�����δ������ 1-�û�δ�����֤��
		std::map<int,int> mm;
		for(size_t i = 0; i < data.size(); i ++)
		{
			mm[data[i].key] = data[i].value;
		}

		wallow_control(userid, rate, mm, msg);
	}
};

};

#endif
