#include "world.h"
#include "player_imp.h"
#include "pathfinding/pathfinding.h"
#include "playermisc.h"

/*ԭ��1��	״̬ת������
	GROUND->���� ����
	����->GROUND ����
	JUMP->FALL	 ����
	JUMP->FLY    ����
	FALL->JUMP 	 �������������жϣ���������Ϊ[1,max-1]������������
	FLY->JUMP	������
	FALL->FLY	����
	FLY->FALL   ����
	�Ƿ�GROUND ʹ����ײ����ɣ�������ʹ�ô����״̬
	*/

/*ԭ��2��
	JUMP����
	����������ϻ򼸺����ϣ�Yֵ����Ϊ��
	�ӵ�һ��JUMP��ʼ������Ծʱ��͸߶��ۼ�,ͬʱ����jump_count=1
	��Ծ�ۼ�ʱ��͸߶ȳ���ָ����ֵ,��jump_count++,�ۼƸ߶Ⱥ�ʱ���ȥָ����ֵ,��С��0���Ϊ0
	jump_count�������Ծ��������
	*/

/*ԭ��3��
	��������
	����������»򼸺�����
	�ӿ�ʼ���俪ʼ�����������ٶ�ͳ�ƣ������ٶȱ��벻�����󣬲���С��1/2��������ɵ�Ӱ��ֵ
	��jump_count>0 && jump<maxʱ������Ӹ�״̬����
	*/

/*ԭ��4��
	���һ�ε��䵽�����ʱ�����ڴ����ʱ���ƫ�󣬸����ж�������Ҫ���´���
 	*/
	
	const A3DVECTOR aExts[USER_CLASS_COUNT*2] =
	{
		A3DVECTOR(0.4f, 0.53f, 0.4f),		//	����
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f),		//	��ʦ
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f),		//  ��ʦ
		A3DVECTOR(0.3f, 0.5f, 0.3f),	

		A3DVECTOR(0.3f, 0.53f, 0.3f),		//	����
		A3DVECTOR(0.3f, 0.5f, 0.3f),	

		A3DVECTOR(0.5f, 0.62f, 0.5f), 	//	����
		A3DVECTOR(0.3f, 0.53f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//  �̿�
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	��â
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.5f, 0.3f),
		
		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	ҹӰ
		A3DVECTOR(0.3f, 0.5f, 0.3f),

		A3DVECTOR(0.3f, 0.53f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.5f, 0.3f),
	};
	const A3DVECTOR aExts2[USER_CLASS_COUNT*2] =
	{
		A3DVECTOR(0.4f, 0.9f, 0.4f),		//	����
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f),		//	��ʦ
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f),		//  ��ʦ
		A3DVECTOR(0.3f, 0.85f, 0.3f),	

		A3DVECTOR(0.3f, 0.9f, 0.3f),		//	����
		A3DVECTOR(0.3f, 0.85f, 0.3f),	

		A3DVECTOR(0.5f, 1.05f, 0.5f), 	//	����
		A3DVECTOR(0.3f, 0.9f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//  �̿�
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	��â
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.85f, 0.3f),
		
		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	ҹӰ
		A3DVECTOR(0.3f, 0.85f, 0.3f),

		A3DVECTOR(0.3f, 0.9f, 0.3f), 	//	����
		A3DVECTOR(0.3f, 0.85f, 0.3f),
	};
	static const A3DVECTOR off(0,-1.05f,0);

int 
phase_control::PhaseControl(gplayer_imp * pImp, const A3DVECTOR & target, float theight, int mode, int use_time)
{
	trace_manager2 & man = *(pImp->_plane->GetTraceMan());
	if(!man.Valid()) return 0;

	//ȷ����ҵ�Ŀ����Ƿ��ڿ���
	const A3DVECTOR & ext = aExts[(pImp->IsPlayerFemale() ? 2*pImp->GetPlayerClass()+1 : 2*pImp->GetPlayerClass())];
	bool is_ground = true;
	bool is_solid;
	float ratio;
	bool bRst = man.AABBTrace(target, off, ext, is_solid,ratio,&pImp->_plane->w_collision_flags);
	if(bRst && is_solid) return -1;	//Ŀ���Ƕ���˽��� ֱ�ӷ��ؼ���

	if(target.y >  theight + 0.2f)	//���ڵ����ϣ�������ײ�����Ƿ����յ��ж�
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
					__PRINTF("ˮ�г�����ʷ����ٶ�speed_square=%f speed_ctrl_factor=%f\n",speed_square,speed_ctrl_factor);
					return -1;	
				}
				__PRINTF("ˮ�п����ٶ�\n");
				return 2;	
			}
			else
			{
				//������ʷ���Ϸ��ٶ�
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
				__PRINTF("start fall-to-fly y�������ϳ���\n");
				return -1;	
			}
			//����fall-to-fly�׶�,����ʱ������Ϊ3s
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
					//��fall-to-fly�׶Σ�ֻ����y�������ϵ��ٶ�
					if(offset.y > 0)
					{
						__PRINTF("in fall-to-fly y�������ϳ���\n");
						return -1;	
					}
					fall_to_fly_time -= use_time;	
					return 2;
				}
				else
				{
					//����fly�׶�Ҫ�Ƚ���ʷ����ٶ�
					if(speed_square > speed_ctrl_factor)
					{
						__PRINTF("���г�����ʷ����ٶ�speed_square=%f speed_ctrl_factor=%f\n",speed_square,speed_ctrl_factor);
						return -1;	
					}
					__PRINTF("���п����ٶ�\n");
					return 2;	
				}
			}
			else
			{
				//������ʷ���Ϸ��ٶ�
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
			//�տ�ʼ��Ծ,����ʱ���Լ����Ծ�߶�
			state = STATE_JUMP;
			jump_distance = offset.y;
			jump_time = use_time;
			jump_count = 1;
		}
		else if(state == STATE_JUMP)
		{
			//������Ծ
			float ndis  = jump_distance +  offset.y;
			int ntime = jump_time + use_time;
			if(ndis > max_jump_distance)
			{
				if(ndis > max_jump_distance*2.0f || ++jump_count > MAX_JUMP_COUNT)
				{
					__PRINTF("��Ծ���볬��%f", ndis);
					return -1;
				}
				else
				{
					__PRINTF("��Ծ����:���������\n");
					ndis -= max_jump_distance;
					ntime = (ntime>max_jump_time ? ntime-max_jump_time : 0);
				}
			}
			if(ntime > max_jump_time)
			{
				if(++jump_count > MAX_JUMP_COUNT)
				{
					__PRINTF("��Ծʱ�䳬��%d", ntime);
					return -1;
				}
				else
				{
					__PRINTF("��Ծʱ�����:���������\n");
					ndis = (ndis>max_jump_distance ? ndis-max_jump_distance : 0);
					ntime -= max_jump_time;
				}
			}
			jump_distance = ndis;
			jump_time = ntime;
		}
		else if(state == STATE_FLY)
		{
			//��һʱ���Ƿ��У����������Ϸ�һ��
			if(offset.y > (pImp->_cur_prop.flight_speed + 0.2f) * use_time * 0.001f)
			{
				__PRINTF("start fly-to-fall y�������ϳ���\n");
				return -1;	
			}
			//����fly-to-fall�׶�,����ʱ������Ϊ3s
			fly_to_fall_time = 3000;
			state = STATE_FALL;
			drop_speed = 0;
			return 2;
		}
		else	//STATE_FALL ��С������STATE_WATER
		{
			if(jump_count <= 0 || jump_count >= MAX_JUMP_COUNT)
			{	
				if(fly_to_fall_time > 0)
				{
					//��fall-to-fly�׶Σ��������Ϸ�	
					if(offset.y > (pImp->_cur_prop.flight_speed + 0.2f) * use_time * 0.001f)
					{
						__PRINTF("in fly-to-fall y�������ϳ���\n");
						return -1;	
					}
					fly_to_fall_time -= use_time;
					return 2;				
				}
				__PRINTF("���������ĵ���������Ĵ������࣬������������%d\n", jump_count);
				return -1;
			}
			else
			{
				if(offset.y > max_jump_distance)
				{
					__PRINTF("��Ծ���볬��%f", offset.y);
					return -1;
				}
				state = STATE_JUMP;
				++ jump_count;
				jump_distance = offset.y;
				jump_time = use_time;
				__PRINTF("���������:���������\n");
			}
		}
		return 0;
	}
	else
	{
		//���ǵ���
		if(state != STATE_FALL)
		{
			//ԭ�����ǵ���
			//��ʼ���е������
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
				__PRINTF("�����ٶ�̫��̫��\n");
				drop_speed = 0.f; //�����ǲ�����©��?? ����ʵ��ʱ���ܳ��ֵ�
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
		max_jump_distance = 2.0f;	//һ����ʵ�ʲ������1.2 
		max_jump_time = 1500;		//ʱ�䲻׼ȷ�����ϸ�����	
	}
	else
	{
		max_jump_distance = 7.0f;	//һ����ʵ�ʲ������5.5 
		max_jump_time = 2000;		//һ����ʵ�ʲ����1550 
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
		
