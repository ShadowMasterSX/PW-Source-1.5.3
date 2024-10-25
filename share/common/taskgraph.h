#ifndef __GNET_TASKGRAPH_H
#define __GNET_TASKGRAPH_H

#include "thread.h"
#include "objectchangesupport.h"
#include "reference.h"
#include <set>
#include <algorithm>
#include <functional>

namespace GNET
{
namespace Thread
{

#define STATE_ENUM enum { INIT, RUNNING, STOPPING, STOPPED, FAIL, SUCCEED, USERDEFINE };

class TaskContext
{
public:
	Thread::Mutex locker;
	virtual ~TaskContext() { }
};

class StatefulRunnable : public ObjectChangeSupport, public ObjectChangeListener
{
	friend class TaskGraph;
	TaskGraph *graph;
protected:
	template<typename T>
	HardReference<Thread::Mutex> GetContext(T*&);
	TaskContext* GetContext();
public:
	STATE_ENUM
	virtual ~StatefulRunnable() { }
	virtual void Destroy() { delete this; }
	virtual void Init() { }
	virtual void ObjectChange(const ObjectChangeSupport *o) { }
	virtual int GetState() const = 0;
	virtual void Run() = 0;
};

class TaskGraph : public Runnable
{
	friend class StatefulRunnable;
	friend class TaskContext;
public:
	STATE_ENUM
	class Node;
	typedef std::set<Node *> NodeSet;
	typedef std::set<StatefulRunnable *> RunnableSet;
	class Node : public Runnable, public ObjectChangeListener
	{
		friend class TaskGraph;
		STATE_ENUM
		NodeSet prev;
		NodeSet next;
		StatefulRunnable  *task;
		TaskGraph *graph;
		int state;

		void TurnStopping()
		{
			if ( state == RUNNING )   state = STOPPING;
			else if ( state == INIT ) state = STOPPED;
		}

		void SetInit()
		{
			state = INIT;
			if ( task )
				task->Init();
		}

		void TurnRunning()
		{
			if ( state == INIT || state >= FAIL )
			{
				Thread::Pool::AddTask(this, true);
				state = RUNNING;
			}
		}

		void Destroy()
		{
			delete this;
		}

		Node(StatefulRunnable *_task, TaskGraph *_graph) : task(_task), graph(_graph), state(INIT)
		{
			if ( task )
			{
				task->Init();
				task->AddObjectChangeListener(this);
			}
		}

		void ObjectChange(const ObjectChangeSupport *o)
		{
			{
			Mutex::Scoped l(graph->locker);
			if ( graph->IsStopping() )
				return;
			state = task->GetState();
			}
			graph->RunnableChangeState( this );
			if ( state >= FAIL )
			{
				Mutex::Scoped l(graph->locker);
				std::for_each(next.begin(), next.end(), std::mem_fun(&Node::TurnRunning));
			}
			else if ( state == RUNNING )
			{
				Thread::Pool::AddTask(this, true);
			}
		}

		bool IsFinish()  const { return state >= FAIL || state == STOPPED; }
	public:
		int GetState() const { return state; }
	
		void Run()
		{
			if ( state == STOPPING )
			{
				state = STOPPED; 
				return;
			}
			if ( state != RUNNING )
				return;
			{
			Mutex::Scoped l(graph->locker);
			if ( std::find_if(prev.begin(), prev.end(), std::not1(std::mem_fun(&Node::IsFinish))) != prev.end() )
			{
				state = INIT;
				return;
			}
			}
			if ( task )
			{
				task->Run();
				if ( state == RUNNING || state == STOPPING )
					Thread::Pool::AddTask(this, true);
			}
			else
			{
				state = STOPPED;
				graph->Stop();
			}
		}

		void AddChild(Node *node)
		{
			Mutex::Scoped l(graph->locker);
			next.insert(node);
			node->prev.insert(this);
		}
	};
private:
	Mutex		locker;
	RunnableSet	runners;
	NodeSet		nodes;
	bool		restart; 
	bool		stopping;
	Node		*root;
	TaskContext	*context;
	void Run()
	{
		if ( ! IsFinish() )
		{
			Thread::Pool::AddTask(this, true);
			stopping = false;
		}
		else if ( restart )
		{
			std::for_each(nodes.begin(), nodes.end(), std::mem_fun(&Node::SetInit));
			root->TurnRunning();
			restart  = false;
			stopping = false;
		}
		else
		{
			std::for_each(runners.begin(), runners.end(), std::mem_fun(&StatefulRunnable::Destroy));
			std::for_each(nodes.begin(), nodes.end(), std::mem_fun(&Node::Destroy));
			delete context;
			delete this;
		}
	}
protected:
	TaskGraph(TaskContext *ctx) : restart(false), stopping(false), root(NULL), context(ctx) { }
public:
	bool IsFinish()
	{
		Mutex::Scoped l(locker);
		return std::find_if(nodes.begin(), nodes.end(), std::not1(std::mem_fun(&Node::IsFinish))) == nodes.end();
	}

	bool IsStopping() const { return stopping; }

	virtual void Start(Node *init_node)
	{
		(root = init_node)->TurnRunning();
	}

	virtual void Restart(Node *init_node)
	{
		root = init_node;
		restart = true;
		Stop();
	}

	virtual void Stop()
	{
		Mutex::Scoped l(locker);
		std::for_each(nodes.begin(), nodes.end(), std::mem_fun(&Node::TurnStopping));
		stopping = true;
		Pool::AddTask(this, true);
	}

	Node* CreateNode(StatefulRunnable *r)
	{
		Node* node = new Node(r, this);
		r->graph = this;
		runners.insert(r);
		nodes.insert(node);
		return node;
	}

	Node* CreateStopNode()
	{
		Node* node = new Node(NULL, this);
		nodes.insert(node);
		return node;
	}

	TaskContext* GetContext() { return context; }

	virtual void RunnableChangeState(Node *n)
	{
		if ( n->GetState() == FAIL )
			Stop();
	}
};

inline TaskContext* StatefulRunnable::GetContext() { return graph->context; }

template<typename T> HardReference<Thread::Mutex> StatefulRunnable::GetContext(T*& r)
{
	TaskContext *ctx = graph->context;
	r = dynamic_cast<T*>(ctx);
	ctx->locker.Lock();
	return HardReference<Thread::Mutex>(&ctx->locker, std::mem_fun(&Thread::Mutex::UNLock));
}


};
};

#endif


