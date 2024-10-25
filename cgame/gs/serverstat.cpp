#include "world.h"
#include "worldmanager.h"
#include <glog.h>

/*
 *	��������������������
 */

namespace GNET
{
void SetDoubleExp(unsigned char factor)
{
	//factorΪ�������������10������ЧֵΪ10��15��20 ... 100
	if(factor < 10 || factor > 100 || (factor%5) != 0) return;

	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]��˫�����鿪��Ϊ(%s),�������������%f",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			factor>10?"On":"Off",
			factor/10.f);
	if(factor > 10)
	{
		world_manager::GetWorldParam().double_exp = true;	
		world_manager::SetDoubleExpFactor(factor/10.f);
	}
	else
	{
		world_manager::GetWorldParam().double_exp = false;	
		world_manager::SetDoubleExpFactor(1.f);
	}
}

void SetNoTrade(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]���׿���Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_trade = blOn;	
}

void SetNoMail(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]�ʼ�����Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_mail = blOn;	
}

void SetNoAuction(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]��������Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_auction = blOn;	
}

void SetNoFaction(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]���ɿ���Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_faction = blOn;	
}

void SetDoubleMoney(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]˫����Ǯ����Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().double_money = blOn;	
}

void SetDoubleObject(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]˫�����ʿ���Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().double_drop = blOn;	
}


void SetDoubleSP(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]˫��Ԫ�񿪹�Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().double_sp = blOn;	
}

void SetNoSellPoint(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]��ֹ�㿨���׿���Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_cash_trade = blOn;	

}

void SetPVP(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:������%d[tag:%d]PVP����Ϊ(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().pve_mode = !blOn;	
}
}

