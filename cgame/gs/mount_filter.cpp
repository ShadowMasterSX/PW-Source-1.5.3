#include "string.h"
#include "world.h"
#include "mount_filter.h"
#include "clstab.h"
#include "arandomgen.h"
#include "actobject.h"
#include "world.h"

DEFINE_SUBSTANCE(mount_filter,filter,CLS_FILTER_MOUNT)

void 
mount_filter::AdjustDamage(damage_entry&, const XID &, const attack_msg&,float)
{
	//每次被击中都有一定概率掉下马
	if(abase::Rand(0.f,1.f) < _drop_rate)
	{
		//发送让玩家召回宠物的消息
		//_is_deleted = true;
		_parent.GetImpl()->SendTo<0>(GM_MSG_PLAYER_RECALL_PET,_parent.GetSelfID(),0);
	}
}

void 
mount_filter::OnAttach()
{
	//1:进入骑乘状态 
	_parent.ActiveMountState(_mount_id,_mount_color);

	//2:增加移动速度
	_parent.EnhanceOverrideSpeed(_speedup);
	_parent.UpdateSpeedData();
	_parent.SendClientCurSpeed();
}

void 
mount_filter::OnRelease()
{
	//1:离开骑乘状态
	_parent.DeactiveMountState();

	//2:修改移动速度
	_parent.ImpairOverrideSpeed(_speedup);
	_parent.UpdateSpeedData();
	_parent.SendClientCurSpeed();
}
	
bool 
mount_filter::Save(archive & ar)
{
	filter::Save(ar);
	ar << _mount_id << _mount_color << _speedup << _drop_rate;
	return true;
}

bool 
mount_filter::Load(archive & ar)
{
	filter::Load(ar);
	ar >> _mount_id >> _mount_color >> _speedup >> _drop_rate;
	return true;
}

void 
mount_filter::Merge(filter * f)
{
	mount_filter * pFilter = substance::DynamicCast<mount_filter>(f);
	ASSERT(pFilter);
	if(!pFilter) return ;
	
	//更新数据
	_mount_id = pFilter->_mount_id;
	_mount_color = pFilter->_mount_color;

	//速度有更改，重新进行计算
	if(_speedup != pFilter->_speedup)
	{
		_parent.ImpairOverrideSpeed(_speedup);
		_speedup = pFilter->_speedup;
		_parent.EnhanceOverrideSpeed(_speedup);
		_parent.UpdateSpeedData();
		_parent.SendClientCurSpeed();
	}

	_drop_rate = pFilter->_drop_rate;
}

