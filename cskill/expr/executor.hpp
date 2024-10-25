#ifndef _GNET_EXECUTOR_H
#define _GNET_EXECUTOR_H
#include "objectgraph.hpp"
#include "syntaxtree.hpp"
#include "visitors.h"
#include <vector>
#include <deque>
#include "../common/thread.h"
namespace GNET
{
class Executor
{
	//define runtime enviroment
    ObjectGraph* og;
	CommonObject* current_evn;
	TreeNode* current_treenode;
	SyntaxTree* st;

	//typedef std::vector<Executor*> ExecutorPool;
	typedef std::deque<Executor*> ExecutorPool;
	static ExecutorPool pool;
	static Thread::Mutex locker_pool;
	
	enum State {_STATE_BUSY,_STATE_FREE,};
	State state;
	
	enum ErrCode
	{
		ERR_NONE_VALUE,			//无效值
		ERR_NULL_SYNTREE,       //空的语法树
		ERR_NULL_ENV,			//空环境	
		ERR_ID_NODE,			//错误的ID节点结构
		ERR_SET_MEMBER,			//给成员变量赋值时发生错误，一般多见于类型不匹配
		ERR_GET_MEMBER,			//获取成员变量值时出错，多为成员不匹配，或类型不匹配
		ERR_INVALID_TYPE,		//无效的类型
		ERR_INVALID_NODEKIND,	//无效的操作类型
		ERR_UNDEFINED_VAR,		//无效的变量
		ERR_INVALID_INDEX,		//无效的索引值
		ERR_INIT_ARRAY_FAILED,	//初始化数组失败
	};
	void stmtexec(TreeNode* t);
	//get member value,(exp: player.level)
	void get_member_value(TreeNode* t,int index=-1);
	//set member value
	template <typename _t>
	void set_member_value(TreeNode* t,const _t& value,int index=-1);
	//type converter
	template<typename _t>
	static void to_dstType(TreeNode* t,int dstType,const _t& value);
	static void type_converter(TreeNode* t,int dstType);
	
	Executor() { state=_STATE_BUSY; }	
public:
	~Executor() {}
	static Executor& GetInstance() { static Executor instance; return instance; }
	static Executor* NewInstance() 
	{ 
		Thread::Mutex::Scoped l(locker_pool);
		/* use deque maybe better*/
		/*
		static Executor* e=NULL;
		if (pool.size()==0)
			return new Executor();
		else
		{
			e=pool.front();
			pool.pop_front();
			return e;
		}
		*/
		
		/* use vector */
		ExecutorPool::iterator it;
		for (it=pool.begin();it!=pool.end();it++)
		{
			if ((*it)->state == _STATE_FREE) 
			{
				(*it)->state = _STATE_BUSY;
				return (*it);
			}
		}
		Executor* e=new Executor();
		pool.push_back(e);
		return e;
		
	}
	void Release() 
	{ 
		/*
		Thread::Mutex::Scoped l(locker_pool);
		pool.push_back(this); 
		*/
		Thread::Mutex::Scoped l(locker_pool);
		state = _STATE_FREE;
	}
		
	void Execute(SyntaxTree& _st,ObjectGraph& _og,CommonObject* env_class)
	{
		og=&_og;
		st=&_st;
		current_evn=env_class;
		try
		{
			TreeNode* t=_st.GetRoot();
			if (NULL==t) throw ERR_NULL_SYNTREE;
			while (t!=NULL)
			{
				stmtexec(t);
				t=t->sibling;
			}
		}
		catch (ErrCode errcode)
		{
			printf("Executor:");
			switch (errcode)
			{
				case ERR_NULL_SYNTREE:
					printf("syntax-tree is empty.\n");	
					break;
				case ERR_NONE_VALUE:
					printf("invalid value. Line %d\n",current_treenode->lineno);
					break;
				case ERR_NULL_ENV:
					printf("environment object is empty. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_ID_NODE:
					printf("wrong Identifier-node structure. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_SET_MEMBER:
					printf("assign member-variable error. maybe type mismatch. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_GET_MEMBER:
					printf("get member-variable error. maybe type mismatch. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_INVALID_TYPE:
					printf("invalid type. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_INVALID_NODEKIND:
					printf("invalid operation type. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_UNDEFINED_VAR:
					printf("invalid variable. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_INVALID_INDEX:
					printf("invalid index value. Line %d\n",current_treenode->lineno);	
					break;
				case ERR_INIT_ARRAY_FAILED:
					printf("initialize array failed. Line %d\n",current_treenode->lineno);	
					break;
			}
			if (errcode != ERR_NULL_SYNTREE)
				SyntaxTree::DisplaySolitaryTree(current_treenode,4);
		}
	}
};
};
#endif
