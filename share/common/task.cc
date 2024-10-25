
#include "taskgraph.h"

using namespace GNET;

class SimpleTask : public Thread::StatefulRunnable
{
	int value;
public:
	SimpleTask(int _value) : value(_value) { }
	void Run()
	{
		printf ( "%d\n", value);
		FireObjectChange();
	}
	State GetState() const { return SUCCEED; }
};

class RunTG : public Thread::Runnable
{
	Thread::TaskGraph *graph;
	Thread::TaskGraph::Node *start_at;
public:
	RunTG(Thread::TaskGraph *tg, Thread::TaskGraph::Node *node) : graph(tg), start_at(node) { graph->Start(start_at); }
	void Run()
	{
		printf("Finish = %d\n", graph->IsFinish());
		sleep(1);
		if ( graph->IsFinish() )
			graph->Stop();
		else
			Thread::Pool::AddTask(this, true);
	}
};

int main()
{
	Thread::TaskGraph *tg = Thread::TaskGraph::CreateInstance();

	SimpleTask *t0 = new SimpleTask(0);
	SimpleTask *t1 = new SimpleTask(1);
	SimpleTask *t2 = new SimpleTask(2);
	SimpleTask *t3 = new SimpleTask(3);
	SimpleTask *t4 = new SimpleTask(4);
	SimpleTask *t5 = new SimpleTask(5);

	Thread::TaskGraph::Node *n0 = tg->CreateNode( t0 );
	Thread::TaskGraph::Node *n1 = tg->CreateNode( t1 );
	Thread::TaskGraph::Node *n2 = tg->CreateNode( t2 );
	Thread::TaskGraph::Node *n3 = tg->CreateNode( t3 );
	Thread::TaskGraph::Node *n4 = tg->CreateNode( t4 );
	Thread::TaskGraph::Node *n5 = tg->CreateNode( t5 );

	n4->AddChild(n5);
	n3->AddChild(n5);
	n2->AddChild(n4);
	n1->AddChild(n4);
	n0->AddChild(n1);	
	n0->AddChild(n2);	
	n0->AddChild(n3);	

	Thread::Pool::AddTask(new RunTG(tg, n0), true);
	Thread::Pool::Run();
	return 0;
}

