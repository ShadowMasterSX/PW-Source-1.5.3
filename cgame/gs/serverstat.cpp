#include "world.h"
#include "worldmanager.h"
#include <glog.h>

/*
 *	整个服务器参数的设置
 */

namespace GNET
{
void SetDoubleExp(unsigned char factor)
{
	//factor为经验调整比例的10倍，有效值为10，15，20 ... 100
	if(factor < 10 || factor > 100 || (factor%5) != 0) return;

	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]的双倍经验开关为(%s),经验调整比例是%f",
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
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]交易开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_trade = blOn;	
}

void SetNoMail(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]邮件开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_mail = blOn;	
}

void SetNoAuction(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]拍卖开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_auction = blOn;	
}

void SetNoFaction(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]帮派开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_faction = blOn;	
}

void SetDoubleMoney(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]双倍金钱开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().double_money = blOn;	
}

void SetDoubleObject(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]双倍掉率开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().double_drop = blOn;	
}


void SetDoubleSP(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]双倍元神开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().double_sp = blOn;	
}

void SetNoSellPoint(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]禁止点卡交易开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"Off":"On");
	world_manager::GetWorldParam().forbid_cash_trade = blOn;	

}

void SetPVP(unsigned char blOn)
{
	GLog::log(GLOG_INFO,"GM:服务器%d[tag:%d]PVP开关为(%s)",
			world_manager::GetWorldIndex(),
			world_manager::GetWorldTag(),
			blOn?"On":"Off");
	world_manager::GetWorldParam().pve_mode = !blOn;	
}
}

