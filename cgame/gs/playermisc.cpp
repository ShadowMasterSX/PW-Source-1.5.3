#include "world.h"
#include "player_imp.h"
#include "pathfinding/pathfinding.h"
#include "playermisc.h"

/*原则1：	状态转换限制
	GROUND->其他 允许
	其他->GROUND 允许
	JUMP->FALL	 允许
	JUMP->FLY    允许
	FALL->JUMP 	 根据起跳次数判断，起跳次数为[1,max-1]允许，否则不允许
	FLY->JUMP	不允许
	FALL->FLY	允许
	FLY->FALL   允许
	是否GROUND 使用碰撞来完成，而不是使用传入的状态
	*/

/*原则2：
	JUMP限制
	方向必须向上或几乎向上，Y值必须为正
	从第一个JUMP开始进行跳跃时间和高度累计,同时设置jump_count=1
	跳跃累计时间和高度超过指定阈值,则jump_count++,累计高度和时间减去指定阈值,若小于0则变为0
	jump_count受最大跳跃次数限制
	*/

/*原则3：
	跌落限制
	方向必须向下或几乎向下
	从开始下落开始，进行下落速度统计，下落速度必须不断增大，不能小于1/2重力所造成的影响值
	当jump_count>0 && jump<max时，允许从该状态跳起
	*/

/*原则4：
	最后一次跌落到地面的时，由于传入的时间会偏大，各种判定可能需要重新处理
 	*/
	
	const A3DVECTOR aExts[USER_CLASS_COUNT*2] =
	{
		A3DVECTOR(0.4f, 0.53f, 0.4f),		//	武侠
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f),		//	法师
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f),		//  巫师
		A3DVECTOR(0.3f, 0.5f, 0.3f),	

		A3DVECTOR(0.3f, 0.53f, 0.3f),		//	妖精
		A3DVECTOR(0.3f, 0.5f, 0.3f),	

		A3DVECTOR(0.5f, 0.62f, 0.5f), 	//	妖兽
		A3DVECTOR(0.3f, 0.53f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//  刺客
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	羽芒
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	羽灵
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	剑灵
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	魅灵
		A3DVECTOR(0.3f, 0.5f, 0.3f),
		
		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	夜影
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	月仙
		A3DVECTOR(0.3f, 0.5f, 0.3f),
	};
	const A3DVECTOR aExts2[USER_CLASS_COUNT*2] =
	{
		A3DVECTOR(0.4f, 0.9f, 0.4f),		//	武侠
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f),		//	法师
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f),		//  巫师
		A3DVECTOR(0.3f, 0.85f, 0.3f),	

		A3DVECTOR(0.3f, 0.9f, 0.3f),		//	妖精
		A3DVECTOR(0.3f, 0.85f, 0.3f),	

		A3DVECTOR(0.5f, 1.05f, 0.5f), 	//	妖兽
		A3DVECTOR(0.3f, 0.9f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//  刺客
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	羽芒
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	羽灵
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	剑灵
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	魅灵
		A3DVECTOR(0.3f, 0.85f, 0.3f),
		
		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	夜影
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	月仙
		A3DVECTOR(0.3f, 0.85f, 0.3f),
	};
	static const A3DVECTOR off(0,-1.05f,0);

int 
phase_control::PhaseControl(gplayer_imp * pImp, const A3DVECTOR & target, float theight, int mode, int use_time)
{
	trace_manager2 & man = *(pImp->_plane->GetTraceMan());
	if(!man.Valid()) return 0;

	//确认玩家的目标点是否在空中
	const A3DVECTOR & ext = aExts[(pImp->IsPlayerFemale() ? 2*pImp->GetPlayerClass()+1 : 2*pImp->GetPlayerClass())];
	bool is_ground = true;
	bool is_solid;
	float ratio;
	bool bRst = man.AABBTrace(target, off, ext, is_solid,ratio,&pImp->_plane->w_collision_flags);
	if(bRst && is_solid) return -1;	//目标点嵌入了建筑 直接返回即可

	if(target.y >  theight + 0.2f)	//不在地面上，根据碰撞进行是否悬空的判断
	{
		if(!bRst) 
		{
			is_ground = false;
		}
		else
		{
			is_ground = (ratio * -off.y < 0.2f);
		}
	}

	if(is_ground) 
	{
		jump_count = 0;
		state = STATE_GROUND;
		return 0;
	}

	A3DVECTOR offset = target;
	offset -= pImp->_parent->pos;
	
	float water_height = path_finding::GetWaterHeight(pImp->_plane, target.x,target.z);
	if(water_height-target.y >= 2.0f)
	{
		if(state == STATE_WATER)
		{
			float speed_square = offset.y * offset.y * 1000000.f / use_time / use_time;
			float cur_speed_square = (pImp->_cur_prop.swim_speed + 0.2f)*(pImp->_cur_prop.swim_speed + 0.2f);
			if(speed_square > cur_speed_square)	
			{
				if(speed_square > speed_ctrl_factor)
				{
					__PRINTF("水中超过历史最大速度speed_square=%f speed_ctrl_factor=%f\n",speed_square,speed_ctrl_factor);
					return -1;	
				}
				__PRINTF("水中可疑速度\n");
				return 2;	
			}
			else
			{
				//更新历史最大合法速度
				speed_ctrl_factor = cur_speed_square;	
			}
		}
		jump_count = 0;
		state = STATE_WATER;
		return 0;
	}
	else if(water_height >= target.y)
	{
		jump_count = 0;
		state = STATE_GROUND;
		return 0;
	}

	if( ((gactive_object*)pImp->_parent)->IsFlyMode())
	{
		if(state == STATE_FALL)
		{
			if(offset.y > (pImp->_cur_prop.flight_speed + 0.2f) * use_time * 0.001f)
			{
				__PRINTF("start fall-to-fly y方向向上超速\n");
				return -1;	
			}
			//进入fall-to-fly阶段,过渡时间设置为3s
			fall_to_fly_time = 3000;
		}
		else if(state == STATE_FLY)
		{
			float speed_square = offset.y * offset.y * 1000000.f / use_time / use_time;
			float cur_speed_square = (pImp->_cur_prop.flight_speed + 0.2f)*(pImp->_cur_prop.flight_speed + 0.2f);
			if(speed_square > cur_speed_square)	
			{
				if(fall_to_fly_time > 0)
				{
					//在fall-to-fly阶段，只限制y方向向上的速度
					if(offset.y > 0)
					{
						__PRINTF("in fall-to-fly y方向向上超速\n");
						return -1;	
					}
					fall_to_fly_time -= use_time;	
					return 2;
				}
				else
				{
					//正常fly阶段要比较历史最大速度
					if(speed_square > speed_ctrl_factor)
					{
						__PRINTF("空中超过历史最大速度speed_square=%f speed_ctrl_factor=%f\n",speed_square,speed_ctrl_factor);
						return -1;	
					}
					__PRINTF("空中可疑速度\n");
					return 2;	
				}
			}
			else
			{
				//更新历史最大合法速度
				speed_ctrl_factor = cur_speed_square;	
			}
			if(fall_to_fly_time > 0) fall_to_fly_time -= use_time;	
		}
		else
		{
			fall_to_fly_time = 0;	
		}
		jump_count = 0;
		state = STATE_FLY;
		return 0;
	}

	if(offset.y > 0)
	{
		if(state == STATE_GROUND) 
		{
			//刚开始跳跃,起跳时难以检查跳跃高度
			state = STATE_JUMP;
			jump_distance = offset.y;
			jump_time = use_time;
			jump_count = 1;
		}
		else if(state == STATE_JUMP)
		{
			//接着跳跃
			float ndis  = jump_distance +  offset.y;
			int ntime = jump_time + use_time;
			if(ndis > max_jump_distance)
			{
				if(ndis > max_jump_distance*2.0f || ++jump_count > MAX_JUMP_COUNT)
				{
					__PRINTF("跳跃距离超高%f", ndis);
					return -1;
				}
				else
				{
					__PRINTF("跳跃过高:进入二次跳\n");
					ndis -= max_jump_distance;
					ntime = (ntime>max_jump_time ? ntime-max_jump_time : 0);
				}
			}
			if(ntime > max_jump_time)
			{
				if(++jump_count > MAX_JUMP_COUNT)
				{
					__PRINTF("跳跃时间超长%d", ntime);
					return -1;
				}
				else
				{
					__PRINTF("跳跃时间过长:进入二次跳\n");
					ndis = (ndis>max_jump_distance ? ndis-max_jump_distance : 0);
					ntime -= max_jump_time;
				}
			}
			jump_distance = ndis;
			jump_time = ntime;
		}
		else if(state == STATE_FLY)
		{
			//上一时刻是飞行，则允许向上飞一点
			if(offset.y > (pImp->_cur_prop.flight_speed + 0.2f) * use_time * 0.001f)
			{
				__PRINTF("start fly-to-fall y方向向上超速\n");
				return -1;	
			}
			//进入fly-to-fall阶段,过渡时间设置为3s
			fly_to_fall_time = 3000;
			state = STATE_FALL;
			drop_speed = 0;
			return 2;
		}
		else	//STATE_FALL 很小可能是STATE_WATER
		{
			if(jump_count <= 0 || jump_count >= MAX_JUMP_COUNT)
			{	
				if(fly_to_fall_time > 0)
				{
					//在fall-to-fly阶段，允许向上飞	
					if(offset.y > (pImp->_cur_prop.flight_speed + 0.2f) * use_time * 0.001f)
					{
						__PRINTF("in fly-to-fall y方向向上超速\n");
						return -1;	
					}
					fly_to_fall_time -= use_time;
					return 2;				
				}
				__PRINTF("不是跳起后的跌落或连跳的次数过多，不能再往上跳%d\n", jump_count);
				return -1;
			}
			else
			{
				if(offset.y > max_jump_distance)
				{
					__PRINTF("跳跃距离超高%f", offset.y);
					return -1;
				}
				state = STATE_JUMP;
				++ jump_count;
				jump_distance = offset.y;
				jump_time = use_time;
				__PRINTF("下落后跳起:进入二次跳\n");
			}
		}
		return 0;
	}
	else
	{
		//这是跌落
		if(state != STATE_FALL)
		{
			//原来不是跌落
			//开始进行跌落控制
			fly_to_fall_time = 0;
			state = STATE_FALL;
			drop_speed = 0;
		}
		else
		{
			float WORLD_GRAVITY_ACC = (9.8f/2);
			float dis = - offset.y;
			if(dis < drop_speed * (use_time *0.001f))
			{
				__PRINTF("跌落速度太慢太慢\n");
				drop_speed = 0.f; //这样是不是有漏洞?? 不过实际时可能出现的
				return -1;
			}
			drop_speed += WORLD_GRAVITY_ACC * (use_time *0.001f);
			if(drop_speed > 50) drop_speed = 50;
			if(fly_to_fall_time > 0) fly_to_fall_time -= use_time;
		}
	}
	return 0;
}

void phase_control::Initialize(gplayer_imp * pImp)
{
	trace_manager2 & man = *(pImp->_plane->GetTraceMan());
	if(!man.Valid()) 
	{
		state = STATE_GROUND;
		return;
	}
	
	jump_distance = 0;
	jump_time = 0;
	jump_count = 0;
	drop_speed = 0;
	speed_ctrl_factor = 0.f;
	fall_to_fly_time = 0;
	fly_to_fall_time = 0;
	if(world_manager::GetWorldLimit().lowjump)
	{
		max_jump_distance = 2.0f;	//一次跳实际测试最高1.2 
		max_jump_time = 1500;		//时间不准确不做严格限制	
	}
	else
	{
		max_jump_distance = 7.0f;	//一次跳实际测试最高5.5 
		max_jump_time = 2000;		//一次跳实际测试最长1550 
	}

	if( ((gactive_object*)pImp->_parent)->IsFlyMode())
	{
		state = STATE_FLY;
		return;	
	}
	
	if(pImp->IsUnderWater(2.0f))
	{
		state = STATE_WATER;
		return;
	}
	else if(pImp->IsUnderWater(0.0f))
	{
		state = STATE_GROUND;
		return;	
	}
	
	A3DVECTOR pos(pImp->_parent->pos);
	float height = pImp->_plane->GetHeightAt(pos.x,pos.z);
	if(pos.y < height + 0.2) 
	{
		state = STATE_GROUND;
		return;
	}

	const A3DVECTOR & ext = aExts[(pImp->IsPlayerFemale() ? 2*pImp->GetPlayerClass()+1 : 2*pImp->GetPlayerClass())];
	bool is_solid;
	float ratio;
	bool bRst = man.AABBTrace(pos, off, ext, is_solid,ratio,&pImp->_plane->w_collision_flags);
	if(bRst && is_solid) 
	{
		state = STATE_GROUND;
		return;
	}
	
	if(!bRst) 
	{
		state = STATE_FALL;
	}
	else
	{
		state = (ratio * -off.y < 0.2f)?STATE_GROUND:STATE_FALL;
	}
}

void phase_control::Load(archive & ar)
{
	ar >> state >> jump_distance >> jump_time >> drop_speed >> jump_count >> max_jump_distance >> max_jump_time >> speed_ctrl_factor >> fall_to_fly_time >> fly_to_fall_time;	
}

void phase_control::Save(archive & ar)
{
	ar << state << jump_distance << jump_time << drop_speed << jump_count << max_jump_distance << max_jump_time << speed_ctrl_factor << fall_to_fly_time << fly_to_fall_time;	
}
		
