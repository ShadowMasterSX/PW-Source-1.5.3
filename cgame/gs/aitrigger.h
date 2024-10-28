#ifndef __ONLINE_GAME_GS_AI_TRIGGER_H__
#define __ONLINE_GAME_GS_AI_TRIGGER_H__

#include "substance.h"
#include <common/types.h>
#include <amemory.h>
#include <arandomgen.h>
#include <ASSERT.h>
#include <string.h>

class ai_object;
class ai_policy;
namespace ai_trigger
{
	class policy;
	class condition : public substance
	{
	public:
		enum 
		{
			TYPE_INVALID,
			TYPE_HEARTBEAT,
			TYPE_TIMER,
			TYPE_KILL_TARGET,
			TYPE_START_COMBAT,
			TYPE_START_DEATH,
			TYPE_ON_DAMAGE,
			TYPE_PATH_END,
			TYPE_END_COMBAT,
		};
	public:
		virtual condition * Clone() const = 0;
		virtual ~condition() {}
		virtual bool Check(policy * self, int param) = 0;	
		virtual int GetConditionType() = 0;		//条件类型， 心跳类型和触发类型 
		virtual bool IsAutoDisable() = 0;
		virtual void Reset() {}
	};

	class expr
	{
		public:
			enum {
				EXPR_CONSTANT,
				EXPR_COMMON_DATA,
				EXPR_PLUS,
				EXPR_MINUS,
				EXPR_MULTIPLY,
				EXPR_DIVIDE,
				EXPR_HISTORY_VALUE,
				EXPR_LOCAL_VALUE,
				EXPR_ROOM_INDEX,
			};
			expr (){}
			virtual expr* Clone() const = 0;
			virtual ~expr () {}
			virtual int GetValue(policy *self) =0;
			virtual int GetExprType() =0;
			class Exception {};     //异常，除零错误
	};

	class target : public substance
	{
	public:
		virtual target * Clone() const = 0;
		virtual ~target() {}
		virtual void GetTarget(policy * self, XID & target) = 0;	
	};

	class operation : public substance
	{
	protected:
		target * 	_target;	
		operation * 	_next;
	public:
		virtual operation * Clone() const = 0;
		operation():_target(0),_next(0) {}
		operation(const operation & rhs)
		{
			if(rhs._target)
			{
				_target = rhs._target->Clone();
			}
			else
			{
				_target = 0;
			}
			_next = 0;
		}

		virtual ~operation() 
		{
			if(_target) delete _target;
		}

		void SetTarget(target * __tar) 
		{
			_target = __tar;
		}

		void SetNext(operation * next)
		{
			ASSERT(next == 0 || _next == 0);
			_next = next;
		}
		operation * GetNext() 
		{
			return _next;
		}
	public:
		virtual bool DoSomething(policy * self) = 0;
		virtual void Reset() {}
		virtual bool RequireTarget() = 0;
	};

	class trigger : public abase::ASmallObject
	{
	protected:
		int 		_id;
		condition * 	_cond;
		operation * 	_ops;
		bool 		_enable;
		bool		_default_enable;
		bool		_battle_trigger;
	public:
		trigger():_id(-1),_cond(0),_ops(0),_enable(false),_default_enable(false),_battle_trigger(true)
		{}
		~trigger()
		{
			delete _cond;
			operation * tmp = _ops;
			while(tmp)
			{
				operation *tmp2 = tmp->GetNext();
				delete tmp;
				tmp = tmp2;
			}
		}
		
		trigger(const trigger &rhs)
		{
			_enable = rhs._enable;
			_default_enable = rhs._default_enable;
			_battle_trigger = rhs._battle_trigger;
			_id = rhs._id;
			_cond = rhs._cond->Clone();
			if(rhs._ops)
			{
				operation *tmp = rhs._ops;
				operation *tmp2 = rhs._ops;
				_ops = tmp->Clone();
				tmp2 = _ops;
				while(tmp->GetNext())
				{
					tmp2->SetNext(0);
					tmp2->SetNext(tmp->GetNext()->Clone());
					tmp2= tmp2->GetNext();
					tmp = tmp->GetNext();
				}
				tmp2->SetNext(0);
			}
			else
			{
				_ops = 0;
			}
		}
		
		inline void SetData(int id , condition * cond)
		{
			_id = id;
			_cond = cond;
		}

		inline void AddOp(operation * op)
		{
			op->SetNext(0);
			if(_ops)
			{
				operation * tmp = _ops;
				while(tmp->GetNext())
				{
					tmp = tmp->GetNext();
				}
				tmp->SetNext(op);
				
			}
			else
			{
				_ops = op;
			}
		}
		
		inline bool IsEnable() 
		{
			return _enable;
		}

		inline bool IsBattleTrigger()
		{
			return _battle_trigger;
		}

		inline void EnableTrigger(bool enable)
		{
			_enable = enable;
		}

		inline void SetBattleEnable(bool battle_trigger)
		{
			_battle_trigger = battle_trigger;
		}

		inline void SetDefaultEnable(bool enable)
		{
			_default_enable = enable;
			_enable = enable;
		}

		inline int GetTriggerID()
		{
			return _id;
		}
		inline int GetCondType()
		{
			return _cond->GetConditionType();
		}
	public:
		inline bool TestTrigger(policy * self, int param = 0)
		{
			try {
				if(_cond->Check(self, param))
				{
					bool bRst = true;
					operation * tmp = _ops;
					while(tmp)
					{
						if(!tmp->DoSomething(self)) 
						{
							bRst = false;
							break;
						}
						tmp = tmp->GetNext();
					}
					if(_cond->IsAutoDisable())
					{
						EnableTrigger(false);
					}
					return bRst;
				}
			}
			catch (...)
			{
				//do nothing
			}
			return true;
		}

		inline void Reset()
		{
			_cond->Reset();
			_ops->Reset();
			_enable = _default_enable;
		}
	};

	class policy : public abase::ASmallObject
	{
		struct timer
		{
			int id;
			int timeout;
			int period;
			int times;
			timer(int id, int to , int p , int ts):id(id),timeout(to),period(p),times(ts)
			{}
		};
		typedef abase::vector<trigger *,abase::fast_alloc<> > TRIGGER_LIST;	
		TRIGGER_LIST _trigger_list;	//触发器总表
		TRIGGER_LIST _hb_tri_list;	//心跳触发器（血量判断等）
		TRIGGER_LIST _tm_tri_list;	//定时器触发器
		TRIGGER_LIST _st_bat_list;	//开始战斗触发器
		TRIGGER_LIST _kl_ply_list;	//杀死对手
		TRIGGER_LIST _death_list;	//杀死对手
		TRIGGER_LIST _on_damage_list;//受到伤害触发器
		TRIGGER_LIST _path_list;	//路径相关触发器
		TRIGGER_LIST _ed_bat_list;	//结束战斗触发器

		abase::vector<timer>				_timer_list;
		int _timer_flag;	//本次操作的定时器状态	 0 ,无操作 0x01有定时器触发,0x02,有定时器要被删除
		ai_object * _self;	//自身指针对象
		ai_policy * _aip;	//ai 控制
		int _id;		//策略id
		int _peace_trigger_count;
	public:
		void AddTrigger(trigger * pTrigger)
		{
			int t = pTrigger->GetCondType();
			switch(t)
			{
				case condition::TYPE_HEARTBEAT:
				_hb_tri_list.push_back(pTrigger);
				if(!pTrigger->IsBattleTrigger()) 
				{
					_peace_trigger_count ++;
				}
				break;
				case condition::TYPE_TIMER:
				_tm_tri_list.push_back(pTrigger);
				break;
				case condition::TYPE_KILL_TARGET:
				_kl_ply_list.push_back(pTrigger);
				break;
				case condition::TYPE_START_COMBAT:
				pTrigger->SetDefaultEnable(true);
				_st_bat_list.push_back(pTrigger);
				break;
				case condition::TYPE_START_DEATH:
				pTrigger->SetDefaultEnable(true);
				_death_list.push_back(pTrigger);
				break;
				case condition::TYPE_ON_DAMAGE:
				_on_damage_list.push_back(pTrigger);
				break;
				case condition::TYPE_PATH_END:
				_path_list.push_back(pTrigger);
				break;
				case condition::TYPE_END_COMBAT:
				pTrigger->SetDefaultEnable(true);
				_ed_bat_list.push_back(pTrigger);
				break;
				default:
				ASSERT(false);
				break;
			}
			_trigger_list.push_back(pTrigger);
		}
	public:
		policy(int id):_timer_flag(0),_self(NULL),_aip(NULL),_id(id),_peace_trigger_count(0)
		{}

		policy(const policy & rhs):_timer_flag(rhs._timer_flag),_self(NULL),_aip(NULL),_id(rhs._id)
		{
			_peace_trigger_count = rhs._peace_trigger_count;
			size_t count = rhs._trigger_list.size();
			for(size_t i = 0; i < count ; i ++)
			{
				trigger * pTri = new trigger(*rhs._trigger_list[i]);
				AddTrigger(pTri);
			}
		}

		~policy()
		{
			size_t count = _trigger_list.size();
			for(size_t i = 0; i < count ; i ++)
			{
				delete _trigger_list[i];
			}
		}
		
		void Reset()
		{
			size_t count = _trigger_list.size();
			for(size_t i = 0; i < count ; i ++)
			{
				if (_trigger_list[i]->IsBattleTrigger()) _trigger_list[i]->Reset();
			}

			_timer_flag = 0;
			_timer_list.clear();
		}

		void ResetAll()
		{
			size_t count = _trigger_list.size();
			for(size_t i = 0; i < count ; i ++)
			{
				_trigger_list[i]->Reset();
			}

			_timer_flag = 0;
			_timer_list.clear();
		}

		void SetParent(ai_object * self, ai_policy * aip)
		{
			_self = self;
			_aip = aip;
		}

		int GetID()
		{
			return _id;
		}
		

		ai_object * GetAIObject()
		{
			return _self;
		}

		ai_policy * GetAIPolicy()
		{
			return _aip;
		}

		void EnableTrigger(int id, bool enable)
		{
			for(size_t i = 0; i < _trigger_list.size(); i ++)
			{
				if(_trigger_list[i]->GetTriggerID() == id)
				{
					_trigger_list[i]->EnableTrigger(enable);
				}
			}
		}
		
		void CreateTimer(int id, int period, int times)
		{
			for(size_t i = 0; i < _timer_list.size(); i ++)
			{
				if(_timer_list[i].id == id)
				{
					_timer_list[i].id = id;
					_timer_list[i].timeout = period;
					_timer_list[i].period = period;
					_timer_list[i].times = times;
					return;
				}
			}
			_timer_list.push_back(timer(id,period,period,times));
		}

		void RefreshTimer()
		{
			_timer_flag = 0;
			for(size_t i = 0; i < _timer_list.size(); i ++)
			{
				timer & t = _timer_list[i];
				if(!t.timeout) t.timeout = t.period;
				t.timeout --;
				if(t.timeout == 0)
				{
					_timer_flag |= 0x01;
					if(t.times > 0)
					{
						t.times --;
						if(!t.times)
						{
							t.times = -1;
							_timer_flag |= 0x02;
						}
					}
				}
			}
		}

		void RemoveTimer(int id)
		{
			for(size_t i = 0; i < _timer_list.size(); i ++)
			{
				if(_timer_list[i].id == id)
				{
					_timer_list.erase(_timer_list.begin() + i);
					return;
				}
			}
		}

		void RemoveEmptyTimer()
		{
			_timer_flag = 0;
			for(size_t i = 0; i < _timer_list.size(); i ++)
			{
				timer & t = _timer_list[i];
				if(t.times < 0)
				{
					_timer_list.erase(_timer_list.begin() + i);
					i --;
				}
			}
		}
		
		bool CheckTimer(int id)
		{
			if(_timer_flag == 0) return false;
			for(size_t i = 0; i < _timer_list.size(); i ++)
			{
				timer & t = _timer_list[i];
				if(t.id != id) continue;
				return (t.timeout == 0);
			}
			return false;
		}

		void CheckTriggers(abase::vector<trigger *,abase::fast_alloc<> > & list, int param = 0)
		{
			for(size_t i = 0; i < list.size(); i ++)
			{
				if (!list[i]->IsEnable()) continue;
				if(!list[i]->TestTrigger(this, param))
				{
					return ;
				}
			}
		}

		void CheckPeaceTriggers(abase::vector<trigger *,abase::fast_alloc<> > & list)
		{
			for(size_t i = 0; i < list.size(); i ++)
			{
				if (list[i]->IsBattleTrigger()) continue;
				if (!list[i]->IsEnable()) continue;
				if(!list[i]->TestTrigger(this))
				{
					return ;
				}
			}
		}

		void CheckTriggersNoBreak(abase::vector<trigger *,abase::fast_alloc<> > & list)
		{
			for(size_t i = 0; i < list.size(); i ++)
			{
				if (!list[i]->IsEnable()) continue;
				list[i]->TestTrigger(this);
			}
		}

		void CheckTriggersNoTest(abase::vector<trigger *,abase::fast_alloc<> > & list)
		{
			for(size_t i = 0; i < list.size(); i ++)
			{
				if(!list[i]->TestTrigger(this))
				{
					return ;
				}
			}
		}

		void OnHeartbeat()
		{
			RefreshTimer();
			if(_timer_flag & 0x01)
			{
				CheckTriggersNoBreak(_tm_tri_list);
			}

			if(_timer_flag & 0x02)
			{
				RemoveEmptyTimer();
			}

			CheckTriggers(_hb_tri_list);
		}

		void OnPeaceHeartbeat()
		{
			RefreshTimer();
			if(_timer_flag & 0x01)
			{
				CheckTriggersNoBreak(_tm_tri_list);
			}

			if(_timer_flag & 0x02)
			{
				RemoveEmptyTimer();
			}

			if(!_peace_trigger_count) return;
			CheckPeaceTriggers(_hb_tri_list);
		}

		void OnPeaceHeartbeatInCombat()
		{
			if(!_peace_trigger_count) return;
			CheckPeaceTriggers(_hb_tri_list);
		}

		void StartCombat()
		{
			CheckTriggersNoTest(_st_bat_list);
		}

		void KillTarget(const XID & target)
		{
			//这里还需要完成设置一些东西
			CheckTriggers(_kl_ply_list);
		}

		void OnDeath()
		{
			CheckTriggers(_death_list);
		}

		void OnDamage()
		{
			CheckTriggers(_on_damage_list);
		}

		void PathEnd(int path_id)
		{
			CheckTriggers(_path_list, path_id);
		}
		
		void EndCombat()
		{
			CheckTriggersNoTest(_ed_bat_list);
		}
	};

//后面是定义的条件,目标选择和操作
//-------------首先是条件操作符 -------- 与或非
	class cond_unary : public condition
	{
	protected:
		condition * _cond;
	public:
		
		cond_unary(condition * cond):_cond(cond)
		{
			ASSERT(_cond);
		}

		cond_unary(const cond_unary & rhs)
		{
			_cond = rhs._cond->Clone();
		}

		~cond_unary()
		{
			delete _cond;
		}
		
		virtual bool Check(policy * self, int p)
		{
			return !_cond->Check(self,p);
		}
		virtual int GetConditionType()
		{
			return _cond->GetConditionType();
		}
	};

	class cond_not : public cond_unary
	{
	public:
		
		virtual condition * Clone() const
		{
			return new cond_not(*this);
		}

		cond_not(condition * cond):cond_unary(cond)
		{
		}

		virtual bool Check(policy * self, int p)
		{
			return !_cond->Check(self,p);
		}
		virtual bool IsAutoDisable()
		{
			return _cond->IsAutoDisable();
		}
	};

	class cond_binary : public condition
	{
	protected:
		condition * _left;
		condition * _right;
	public:

		cond_binary(condition * left,condition *right):_left(left),_right(right)
		{
			ASSERT(_left && _right);
		}

		cond_binary(const cond_binary & rhs)
		{
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}

		~cond_binary()
		{
			delete _left;
			delete _right;
		}
		
		virtual int GetConditionType()
		{
			return _left->GetConditionType();
		}
	};

	class cond_and : public cond_binary
	{
	public:
		
		virtual condition * Clone() const
		{
			return new cond_and(*this);
		}

		cond_and(condition * left,condition *right):cond_binary(left,right)
		{
		}

		virtual bool Check(policy * self, int p)
		{
			return _left->Check(self,p) && _right->Check(self,p);
		}
		virtual bool IsAutoDisable()
		{
			return _left->IsAutoDisable() && _right->IsAutoDisable();
		}

	};

	class cond_or : public cond_binary
	{
	public:
		
		virtual condition * Clone() const
		{
			return new cond_or(*this);
		}

		cond_or(condition * left,condition *right):cond_binary(left,right)
		{
		}

		virtual bool Check(policy * self, int p)
		{
			return _left->Check(self,p) || _right->Check(self,p);
		}
		virtual bool IsAutoDisable()
		{
			return _left->IsAutoDisable() || _right->IsAutoDisable();
		}

	};

//后面是基础元素
	class cond_hp_less : public condition
	{
		float _ratio;
	public:
		virtual condition * Clone() const
		{
			return new cond_hp_less(*this);
		}
		
		cond_hp_less(float ratio):_ratio(ratio)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return true;
		}
	};

	class cond_timer : public condition
	{
		int _timer_id;
	public:
		virtual condition * Clone() const
		{
			return new cond_timer(*this);
		}
		
		cond_timer(int id ):_timer_id(id)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_TIMER;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};
	
	class cond_start_combat : public condition
	{
	public:
		virtual condition * Clone() const
		{
			return new cond_start_combat(*this);
		}
		
		virtual bool Check(policy * self, int)
		{
			return true;
		}
		virtual int GetConditionType()
		{
			return TYPE_START_COMBAT;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_on_death : public condition
	{
	public:
		virtual condition * Clone() const
		{
			return new cond_on_death(*this);
		}
		
		virtual bool Check(policy * self, int)
		{
			return true;
		}
		virtual int GetConditionType()
		{
			return TYPE_START_DEATH;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_random : public condition
	{
		float _rate;
	public:
		virtual condition * Clone() const
		{
			return new cond_random(*this);
		}
		
		cond_random(float rate):_rate(rate)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_kill_target : public condition
	{
	public:
		virtual condition * Clone() const
		{
			return new cond_kill_target(*this);
		}
		
		virtual bool Check(policy * self, int)
		{
			return true;
		}
		virtual int GetConditionType()
		{
			return TYPE_KILL_TARGET;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_expr : public condition    
	{       
		expr * _e;
	public:         
		cond_expr(expr *e):_e(e) {}

		virtual condition * Clone() const
		{       
			return new cond_expr(*this);
		}
		cond_expr (const cond_expr& rhs)
		{
			_e = rhs._e->Clone();
		}
		~cond_expr ()
		{
			delete _e;
		}
		virtual bool Check(policy * self, int)
		{
			return _e->GetValue(self);
		}

		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	}; 

	class cond_compare_less : public condition      //小于
	{
		expr * _left;
		expr * _right;
		public:
		cond_compare_less (expr* l, expr* r):_left(l), _right(r) {}
		virtual condition * Clone() const
		{
			return new cond_compare_less(*this);
		}
		cond_compare_less (const cond_compare_less& rhs)
		{
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}
		~cond_compare_less()
		{
			delete _left;
			delete _right;
		}
		virtual bool Check(policy * self, int)
		{
			return _left->GetValue(self) < _right->GetValue(self);
		}

		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_compare_greater : public condition   //大于
	{
		expr * _left;
		expr * _right;
		public:
		cond_compare_greater (expr* l, expr* r):_left(l), _right(r) {}

		virtual condition * Clone() const
		{
			return new cond_compare_greater(*this);
		}
		cond_compare_greater (const cond_compare_greater& rhs)
		{
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}
		~cond_compare_greater()
		{
			delete _left;
			delete _right;
		}
		virtual bool Check(policy * self, int)
		{
			return _left->GetValue(self) > _right->GetValue(self);
		}

		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};
	class cond_compare_equal : public condition     //等于
	{
		expr * _left;
		expr * _right;
		public:
		cond_compare_equal (expr* l, expr* r):_left(l), _right(r) {}

		virtual condition * Clone() const
		{
			return new cond_compare_equal(*this);
		}
		cond_compare_equal (const cond_compare_equal& rhs)
		{
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}
		~cond_compare_equal()
		{
			delete _left;
			delete _right;
		}
		virtual bool Check(policy * self, int)
		{
			return _left->GetValue(self) == _right->GetValue(self);
		}

		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};
	
	class cond_time_point : public condition	//lgc
	{
		int _hour;
		int _minute;
	public:
		virtual condition * Clone() const
		{
			return new cond_time_point(*this);
		}
		
		cond_time_point(int h, int m):_hour(h),_minute(m)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return true;
		}
	};

	class cond_on_damage : public condition
	{
		int _min_damage;
		int _max_damage;
	public:
		virtual condition * Clone() const
		{
			return new cond_on_damage(*this);
		}
		
		cond_on_damage(int min_dmg, int max_dmg):_min_damage(min_dmg),_max_damage(max_dmg)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_ON_DAMAGE;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_path_end : public condition
	{
		int _global_path_id;
	public:
		virtual condition * Clone() const
		{
			return new cond_path_end(*this);
		}
		
		cond_path_end(int global_path_id):_global_path_id(global_path_id)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_PATH_END;
		}
		virtual bool IsAutoDisable()
		{
			return true;
		}
	};
	
	class cond_path_end_2 : public condition
	{
		int _index;
		int _type;
	public:
		virtual condition * Clone() const
		{
			return new cond_path_end_2(*this);
		}
		
		cond_path_end_2(int id,int type):_index(id),_type(type)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_PATH_END;
		}
		virtual bool IsAutoDisable()
		{
			return true;
		}
	};

	class cond_at_history_stage : public condition
	{
		int _history_stage_id;
	public:
		virtual condition * Clone() const
		{
			return new cond_at_history_stage(*this);
		}
		
		cond_at_history_stage(int hid):_history_stage_id(hid)
		{}
		
		virtual bool Check(policy * self, int);
		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_end_combat : public condition
	{
	public:
		virtual condition * Clone() const
		{
			return new cond_end_combat(*this);
		}

		virtual bool Check(policy *self,int)
		{
			return true;
		}

		virtual int GetConditionType()
		{
			return TYPE_END_COMBAT;
		}

		virtual bool IsAutoDisable()
		{
			return false;
		}
	};

	class cond_spec_filter : public condition
	{
		int spec_filter_id;
	public:
		cond_spec_filter(int sfi) : spec_filter_id(sfi) {}

		virtual condition * Clone() const
		{
			return new cond_spec_filter(*this);
		}

		virtual bool Check(policy *self,int);
		
		virtual int GetConditionType()
		{
			return TYPE_HEARTBEAT;
		}
		
		virtual bool IsAutoDisable()
		{
			return false;
		}

	};

	//大于等于、小于等于和不等于可以用cond_not组合出来。

	//下边是两种基础表达式
	class expr_constant: public expr        //常数
	{
		int _value;
		public:
		expr_constant(const int value):_value(value) {}
		virtual expr* Clone () const
		{
			return new expr_constant(*this);
		}
		expr_constant (const expr_constant& rhs)
		{
			_value = rhs._value;
		}
		virtual int GetValue(policy*)
		{
			return _value;
		}
		virtual int GetExprType()
		{
			return EXPR_CONSTANT;
		}
	};

	class expr_common_data:public expr      //从全局数据中取
	{
		int _key;
		public:
		expr_common_data (const int key):_key(key) {}
		virtual expr* Clone () const
		{
			return new expr_common_data(*this);
		}
		expr_common_data (const expr_common_data& rhs)
		{
			_key = rhs._key;
		}
		virtual int GetValue(policy*);
		virtual int GetExprType()
		{
			return EXPR_COMMON_DATA;
		}
	};

	class expr_history_value:public expr      //取历史变量数据
	{
		int _key;
		public:
		expr_history_value (const int key):_key(key) {}
		virtual expr* Clone () const
		{
			return new expr_history_value(*this);
		}
		expr_history_value (const expr_history_value& rhs)
		{
			_key = rhs._key;
		}
		virtual int GetValue(policy*);
		virtual int GetExprType()
		{
			return EXPR_HISTORY_VALUE;
		}
	};

	class expr_local_value:public expr      //取npc特征变量数据
	{
		int _key;
		public:
		expr_local_value (const int key):_key(key) {}
		virtual expr* Clone () const
		{
			return new expr_local_value(*this);
		}
		expr_local_value (const expr_local_value& rhs)
		{
			_key = rhs._key;
		}
		virtual int GetValue(policy*);
		virtual int GetExprType()
		{
			return EXPR_LOCAL_VALUE;
		}
	};

	class expr_room_index:public expr      //取当前迷宫房间主线序号
	{
		public:
		expr_room_index () {}
		virtual expr* Clone () const
		{
			return new expr_room_index(*this);
		}
		expr_room_index ( const expr_room_index& rhs)
		{		
		}
		virtual int GetValue(policy*);
		virtual int GetExprType()
		{
			return EXPR_ROOM_INDEX;
		}
	};

	//下边是基础表达式的四则运算
	class expr_plus : public expr   //加法运算
	{       
		expr *_left;
		expr *_right;
		public:
		expr_plus (expr *a1, expr*a2): _left(a1), _right(a2) {}
		virtual expr* Clone () const
		{       
			return new expr_plus(*this);
		}
		expr_plus (const expr_plus& rhs)
		{       
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}
		~expr_plus ()
		{
			delete _left;
			delete _right;
		}

		virtual int GetValue(policy *self)
		{
			return _left->GetValue(self) + _right->GetValue(self);
		}
		virtual int GetExprType()
		{       
			return EXPR_PLUS;
		}
	};      
	class expr_minus : public expr  //减法运算
	{       
		expr *_left;
		expr *_right;
		public:         
		expr_minus (expr *a1, expr*a2): _left(a1),_right(a2) {}
		virtual expr* Clone () const
		{
			return new expr_minus(*this);
		}
		expr_minus (const expr_minus& rhs)
		{
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}
		~expr_minus ()
		{
			delete _left;
			delete _right;
		}       

		virtual int GetValue(policy *self) 
		{
			return _left->GetValue(self) - _right->GetValue(self);
		}
		virtual int GetExprType()
		{
			return EXPR_MINUS;
		}       
	};      

	class expr_multiply : public expr       //乘法运算
	{       
		expr *_left;
		expr *_right;
		public: 
		expr_multiply (expr *a1, expr*a2): _left(a1),_right(a2) {}
		virtual expr* Clone () const
		{       
			return new expr_multiply(*this);
		}
		expr_multiply (const expr_multiply& rhs)
		{       
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}
		~expr_multiply ()
		{       
			delete _left;
			delete _right;
		}

		virtual int GetValue(policy *self)
		{       
			return _left->GetValue(self) * _right->GetValue(self);
		}
		virtual int GetExprType()
		{       
			return EXPR_MULTIPLY;
		}
	};
	class expr_divide : public expr //除法运算
	{       
		expr *_left;
		expr *_right;
		public: 
		expr_divide (expr *a1, expr*a2): _left(a1),_right(a2) {}
		virtual expr* Clone () const
		{       
			return new expr_divide(*this);
		}
		expr_divide (const expr_divide& rhs)
		{       
			_left = rhs._left->Clone();
			_right = rhs._right->Clone();
		}
		~expr_divide ()
		{       
			delete _left;
			delete _right;
		}

		virtual int GetValue(policy *self)
		{       
			int value = _right->GetValue(self);
			if (value ==0)
			{       
				throw expr::Exception();
			}
			//return _left->GetValue(self) / _right->GetValue(self);
			return _left->GetValue(self) / value;
		}
		virtual int GetExprType()
		{       
			return EXPR_DIVIDE;
		}
	};

//------ 下面是选择逻辑 --------
		
	class target_self : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_self(*this);
		}
		virtual void GetTarget(policy * self, XID & target);

	};

	class target_aggro_first : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_aggro_first(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_aggro_second : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_aggro_second(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_aggro_second_rand : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_aggro_second_rand(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_least_hp : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_least_hp(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_most_hp : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_most_hp(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_most_mp : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_most_mp(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_class_combo : public target
	{
		int _combo_state;
	public:
		target_class_combo(int combo):_combo_state(combo)
		{}

		virtual target * Clone() const
		{
			return new target_class_combo(*this);
		}
		virtual void GetTarget(policy * self, XID & target);

	};

	class target_monster_killer : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_monster_killer(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_monster_birthplace_faction : public target
	{
	public:
		virtual target * Clone() const
		{
			return new target_monster_birthplace_faction(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

	class target_aggro_special : public target
	{
		int _type;
		public:
		enum{
			ATAS_RAND,
			ATAS_NEAR,
			ATAS_FAR,
		};

		target_aggro_special(int type) : _type(type) {}

		virtual target * Clone() const
		{
			return new target_aggro_special(*this);
		}
		virtual void GetTarget(policy * self, XID & target);	
	};


	class target_aggro_first_redirected : public target//策略目标为仇恨排名第一（宠物）时，将第一仇恨目标选定为宠物的主人
	{
	public:
		virtual target * Clone() const
		{
			return new target_aggro_first_redirected(*this);
		}
		virtual void GetTarget(policy * self, XID & target);
	};

//-----------后面是有的操作-----------------------	
	class op_attack : public operation
	{
		int _attack_strategy;	//肉搏 ，近身，技能， 其他
	public:
		op_attack(int strategy):_attack_strategy(strategy)
		{}

		virtual operation * Clone() const
		{
			return new op_attack(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
		
	};

	class op_skill : public operation
	{
		int _skill_id;
		int _skill_lvl;
	public:
		op_skill(int skill , int level):_skill_id(skill),_skill_lvl(level)
		{}

		virtual operation * Clone() const
		{
			return new op_skill(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
		
	};
	
	class op_skill_2 : public operation
	{
		int _skill_id;
		int _skill_id_type;
		int _skill_lvl;
		int _skill_lvl_type;
	public:
		op_skill_2(int skill ,int st, int level, int lt):
			_skill_id(skill),_skill_id_type(st),_skill_lvl(level),_skill_lvl_type(lt)
		{}

		virtual operation * Clone() const
		{
			return new op_skill_2(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
	};

	class op_flee : public operation
	{
	public:
		op_flee()
		{}

		virtual operation * Clone() const
		{
			return new op_flee(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
		
	};
	
	class op_create_timer : public operation
	{	
		int _timerid;
		int _interval;
		int _count;
	public:
		op_create_timer(int id, int interval, int count):_timerid(id),_interval(interval),_count(count)
		{}

		virtual operation * Clone() const
		{
			return new op_create_timer(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return false;
		}
		
	};

	class op_remove_timer : public operation
	{	
		int _timerid;
	public:
		op_remove_timer(int id):_timerid(id)
		{}

		virtual operation * Clone() const
		{
			return new op_remove_timer(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return false;
		}
	};

	class op_enable_trigger : public operation
	{	
		int 	_trigger_id;
		bool 	_is_enable;
	public:
		op_enable_trigger(int id,bool is_enable):_trigger_id(id),_is_enable(is_enable)
		{}

		virtual operation * Clone() const
		{
			return new op_enable_trigger(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return false;
		}
	};

	class op_exec_trigger : public operation
	{
		trigger * _trigger;
	public:
		op_exec_trigger(trigger * ptri):_trigger(ptri)
		{
		}

		op_exec_trigger(const op_exec_trigger &  rhs)
		{
			_trigger = new trigger(*rhs._trigger);
		}

		~op_exec_trigger()
		{
			if(_trigger) delete _trigger;
		}

		virtual operation * Clone() const
		{
			return new op_exec_trigger(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return false;
		}
	};

	class op_say : public operation
	{
		void * _msg;
		size_t _size;

	public:
		op_say(const char * str)
		{	
			size_t len = strlen(str);
			_size = len*2;
			_msg = abase::fastalloc(_size);
			for(size_t i = 0; i < len; i ++)
			{
				((char*)_msg)[i*2] = str[i];
				((char*)_msg)[i*2+1] = 0;
			}
		}

		op_say(const void * msg, size_t size)
		{
			_msg = abase::fastalloc(size);
			_size = size;
			memcpy(_msg,msg,size);
		}

		~op_say()
		{
			abase::fastfree(_msg,_size);
		}

		op_say(const op_say &  rhs)
		{
			_size = rhs._size;
			_msg = abase::fastalloc(_size);
			memcpy(_msg,rhs._msg,_size);
		}

		virtual operation * Clone() const
		{
			return new op_say(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
	};

	class op_say_2 : public operation
	{
		void * _msg;
		size_t _size;
		unsigned int   _mask;

	public:
		op_say_2(const void * msg, size_t size,unsigned int mask)
		{
			_msg = abase::fastalloc(size);
			_size = size;
			_mask = mask;
			memcpy(_msg,msg,size);
		}

		~op_say_2()
		{
			abase::fastfree(_msg,_size);
		}

		op_say_2(const op_say_2 &  rhs) : operation(rhs)
		{
			_size = rhs._size;
			_mask = rhs._mask;
			_msg = abase::fastalloc(_size);
			memcpy(_msg,rhs._msg,_size);
		}

		virtual operation * Clone() const
		{
			return new op_say_2(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
	};

	class op_reset_aggro : public operation
	{	
	public:
		op_reset_aggro()
		{}

		virtual operation * Clone() const
		{
			return new op_reset_aggro(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return false;
		}
	};

	class op_swap_aggro : public operation
	{	
		size_t _index1;
		size_t _index2;
	public:
		op_swap_aggro(size_t index1,size_t index2):_index1(index1),_index2(index2)
		{}

		virtual operation * Clone() const
		{
			return new op_swap_aggro(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return false;
		}
	};

	class op_be_taunted : public operation
	{
	public:
		op_be_taunted()
		{}

		virtual operation * Clone() const
		{
			return new op_be_taunted(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
	};

	class op_fade_target : public operation
	{
	public:
		op_fade_target()
		{}

		virtual operation * Clone() const
		{
			return new op_fade_target(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return true;
		}
	};

	class op_aggro_fade: public operation
	{
	public:
		op_aggro_fade()
		{}

		virtual operation * Clone() const
		{
			return new op_aggro_fade(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget()
		{
			return false;
		}
	};

	class op_break : public operation
	{
	public:
		op_break()
		{}

		virtual operation * Clone() const
		{
			return new op_break(*this);
		}
		virtual bool DoSomething(policy * self)
		{
			return false;
		}
		virtual bool RequireTarget()
		{
			return false;
		}
	};

	class op_active_spawner : public operation
	{
		bool _is_active_spawner;
		int _ctrl_id;
	public:
		op_active_spawner(int id, bool bStop)
		{
			_ctrl_id = id;
			_is_active_spawner = !bStop;
		}

		virtual operation * Clone() const
		{
			return new op_active_spawner(*this);
		}
		virtual bool RequireTarget()
		{
			return false;
		}
		virtual bool DoSomething(policy * self);
	};

	class op_active_spawner_2 : public operation
	{
		bool _is_active_spawner;
		int _ctrl_id;
		int _ctrl_id_type;
	public:
		op_active_spawner_2(int id, int itype, bool bStop)
		{
			_ctrl_id = id;
			_ctrl_id_type = itype;
			_is_active_spawner = !bStop;
		}

		virtual operation * Clone() const
		{
			return new op_active_spawner_2(*this);
		}
		virtual bool RequireTarget()
		{
			return false;
		}
		virtual bool DoSomething(policy * self);
	};

	class op_set_common_data : public operation
	{
		int _key;
		int _set_value;
		bool _is_value;	//true: 将key对应的全局变量设为set_value false: 将key对应的全局变量设置为全局变量(id=_set_value)的值
		public:
		op_set_common_data (int key, int set_value, bool is_value):
			_key (key), _set_value(set_value), _is_value(is_value) {}
		virtual operation * Clone() const
		{
			return new op_set_common_data(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_add_common_data : public operation
	{
		int _key;
		int _add_value;
		public:
		op_add_common_data (int key, int add_value):
			_key (key), _add_value(add_value) {}
		virtual operation * Clone() const
		{
			return new op_add_common_data(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_summon_monster : public operation
	{
		int _mob_id;
		int _count;
		int _target_distance;//与目标的距离
		int _remain_time;
		char _die_with_who;	//mask, 0x01是随leader死亡，0x02是随target死亡
		int _path_id;
		public:
		op_summon_monster(int mob_id,int count,int target_distance,int remain_time,char die_with_who,int path_id):
		_mob_id(mob_id),_count(count),_target_distance(target_distance),_remain_time(remain_time),_die_with_who(die_with_who),_path_id(path_id){}
		virtual operation * Clone() const
		{
			return new op_summon_monster(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return true;}
	};
	
	class op_summon_monster_2 : public operation
	{
		int _mob_id;
		int _mob_id_type;
		int _count;
		int _count_type;
		int _target_distance;//与目标的距离
		int _remain_time;
		char _die_with_who;	//mask, 0x01是随leader死亡，0x02是随target死亡
		int _path_id;
		int _path_id_type;
		public:
		op_summon_monster_2(int mob_id,int mt,int count,int ct,int target_distance,int remain_time,char die_with_who,int path_id, int pt):
		_mob_id(mob_id),_mob_id_type(mt),_count(count),_count_type(ct),_target_distance(target_distance),_remain_time(remain_time),_die_with_who(die_with_who),_path_id(path_id),_path_id_type(pt){}
		virtual operation * Clone() const
		{
			return new op_summon_monster_2(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return true;}
	};

	class op_summon_npc : public operation
	{
		int _npc_id;
		int _npc_id_type;
		int _count;
		int _count_type;
		int _target_distance;//与目标的距离
		int _remain_time;
		int _remain_time_type;
		int _path_id;
		int _path_id_type;
		public:
		op_summon_npc(int npc_id,int nt,int count,int ct,int target_distance,int remain_time,int rt,int path_id, int pt):
		_npc_id(npc_id),_npc_id_type(nt),_count(count),_count_type(ct),_target_distance(target_distance),_remain_time(remain_time),_remain_time_type(rt),_path_id(path_id),_path_id_type(pt){}
		virtual operation * Clone() const
		{
			return new op_summon_npc(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return true;}
	};

	class op_summon_mine : public operation
	{
		int _mine_id;
		int _mine_id_type;
		int _count;
		int _count_type;
		int _remain_time;
		int _remain_time_type;
		int _target_distance;//与目标的距离
		public:
		op_summon_mine(int mine_id,int mt,int count,int ct,int remain_time,int rt,int target_distance):
		_mine_id(mine_id),_mine_id_type(mt),_count(count),_count_type(ct),_remain_time(remain_time),_remain_time_type(rt),_target_distance(target_distance){}
		virtual operation * Clone() const
		{
			return new op_summon_mine(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return true;}
	};

	class op_change_path : public operation
	{
		int _world_tag;
		int _global_path_id;
		int _path_type;
		char _speed_flag;
		public:
		op_change_path(int world_tag,int global_path_id,int path_type,char speed_flag):
			_world_tag(world_tag),_global_path_id(global_path_id),_path_type(path_type),_speed_flag((speed_flag==2)?1:0) {}

		virtual operation * Clone() const
		{
			return new op_change_path(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_change_path_2 : public operation
	{
		int _world_tag;
		int _global_path_id;
		int _global_path_id_type;
		int _path_type;
		char _speed_flag;
		public:
		op_change_path_2(int world_tag,int global_path_id,int gptype,int path_type,char speed_flag):
			_world_tag(world_tag),_global_path_id(global_path_id),_global_path_id_type(gptype),_path_type(path_type),_speed_flag((speed_flag==2)?1:0) {}

		virtual operation * Clone() const
		{
			return new op_change_path_2(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_play_action : public operation
	{
		char _action_name[128];
		int _play_times;
		int _action_last_time;
		int _interval_time;
		public:
		op_play_action (char action_name[128], int play_times, int action_last_time, int interval_time):
			_play_times(play_times),_action_last_time(action_last_time),_interval_time(interval_time)
		{
			memcpy(_action_name,action_name,sizeof(_action_name));
		}
		virtual operation * Clone() const
		{
			return new op_play_action(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_revise_history : public operation
	{
		int _key;
		int _value;
		public:
		op_revise_history(int key,int val) : _key(key),_value(val) {}
		virtual operation * Clone() const
		{
			return new op_revise_history(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};
	
	class op_set_history : public operation
	{
		int _key;
		int _value;
		bool _flag;
		public:
		op_set_history(int key,int val,bool flag) : _key(key),_value(val),_flag(flag) {}
		virtual operation * Clone() const
		{
			return new op_set_history(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_deliver_faction_pvp_points : public operation
	{
		int _type;
		public:
		op_deliver_faction_pvp_points(int type) : _type(type) {}
		virtual operation * Clone() const
		{
			return new op_deliver_faction_pvp_points(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return true;}
	};
	
	class op_deliver_task : public operation
	{
		int _taskid;
		int _taskidtype;
		public:
		op_deliver_task(int task,int ttype) : _taskid(task),_taskidtype(ttype) {}
		virtual operation * Clone() const
		{
			return new op_deliver_task(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return true;}
	};

	class op_calc_var : public operation
	{
		int _dest;
		int _dtype;
		int _src1;
		int _stype1;
		int _src2;
		int _stype2;
		int _op;

		public:
		op_calc_var(int d,int dt,int s1,int st1,int s2,int st2,int op) : 
			_dest(d),_dtype(dt),_src1(s1),_stype1(st1),_src2(s2),_stype2(st2),_op(op){}
		virtual operation * Clone() const
		{
			return new op_calc_var(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_deliver_random_task_in_region : public operation
	{
		int  _task_storageid;
		rect _rect;
		public:
		op_deliver_random_task_in_region(int tsid,float l,float t,float r, float b):
			_task_storageid(tsid),_rect(l,t,r,b) {}
		virtual operation * Clone() const
		{
			return new op_deliver_random_task_in_region(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};

	class op_deliver_task_in_dmglist : public operation
	{
		int _taskid;
		int _ttype;
		int _distance;
		int _count;
		public:
		op_deliver_task_in_dmglist(int tid, int ttype, int dis, int count):
			_taskid(tid),_ttype(ttype),_distance(dis),_count(count) {}
		virtual operation * Clone() const
		{
			return new op_deliver_task_in_dmglist(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};
	
	class op_clear_tower_task_in_region: public operation
	{
		rect _rect;
		public:
		op_clear_tower_task_in_region(float l,float t,float r, float b):_rect(l,t,r,b) {}
		virtual operation * Clone() const
		{
			return new op_clear_tower_task_in_region(*this);
		}
		virtual bool DoSomething(policy * self);
		virtual bool RequireTarget() {return false;}
	};
}

#endif

