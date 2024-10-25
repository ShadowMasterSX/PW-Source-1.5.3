#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "ASSERT.h"
#include <string.h>

#include "vector.h"
#include "spinlock.h"
#include "interlocked.h"
#include "timer.h"

//用于策划改时间用
int abase::g_delta_time = 0;

//空的命名空间，只准内部使用
namespace {
class gtimer
{
	struct timer_node
	{
		int spinlock;
		struct timer_node * prev;
		struct timer_node * next;
		int interval;
		int times;
		unsigned int nexttime;
		int tab_idx;
		void * object;
		abase::timer_callback  callback;
		bool exec_flag;
	};

	struct node_LL{
		timer_node * head;
		int lock;
		node_LL():head(NULL),lock(0){}
		inline void do_lock()
		{
			mutex_spinlock(&lock);
		}
		inline void do_unlock()
		{
			mutex_spinunlock(&lock);
		}
		
		inline void push(timer_node * node)
		{
			node->next = head;
			head = node;
		}

		inline void push_list(timer_node * first,timer_node * last)
		{
			last->next = head;
			head = first; 
		}

		inline timer_node * pop()
		{
			timer_node * tmp = head;
			if(tmp)	head = tmp->next;
			return tmp;
		}
	};

	struct remove_entry{
		int _index;
		void * _object;
		remove_entry(int index,void *object):_index(index),_object(object){}
	};

	unsigned int	_tickcount;			//现在的时间
	abase::vector<timer_node *>_timerlist;		//这个list里保存的是双向循环链表 Double circular linked list
	abase::vector<int>	   _timercount;		//这个list里保存的是上表中每个链表的元素数目
	int		_timer_offset;			//当前时间条目的便宜量
	timer_node *	_hinterval_list;		//保存大于idx_size个tick的timer，这也是一个双向循环链表
	int		_hinterval_count;
	timer_node *	_node_pool;			//这是内存池，用以定位定时器节点的位置
	node_LL		_incoming_list;			//保存新加入的对象列表
	node_LL 	_bucket;			//当前的内存池，从这里分配节点	单链表
	bool		_thread_flag;			//线程的标志
	int		_thread_state;
	int		_idx_size;
	int		_max_count;
	abase::vector<remove_entry> _remove_list;	//要删除的定时器列表
	abase::vector<remove_entry> _remove_list_aux;	//要删除的定时器列表缓冲
	int		_remove_lock;			//删除用的锁
	int 		_ticktime;
	int 		_mintime;
	int		_systime;			//以秒为单位的系统时间
	timeval		_systime_tv;			//高精度一些的系统时间
	abase::vector<timer_node  *> _interval_update_list;	//更新时间间隔的列表
	int		_interval_update_lock;
	int		_free_timer_count;		//当前有多少个timer在空闲
	int		_timer_total_alloc;		//总共分配过多少个timer

private:
	void timer_thread();
	void handle_incoming_list(int offset);
	void handle_remove_list(int offset);
	void handle_current_list(int offset,timer_node*);
	void handle_update_interval_list(int offset);
	void regroup_all_timer();

	inline void DCLL_remove(timer_node*& head,timer_node *node)
	{
		if(head == node)
		{
			if(node->prev == node)
			{
				head = NULL;
				return ;
			}	
			head = node->next;
		}
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	inline void DCLL_push_front(timer_node*& head,timer_node *node)
	{
		if(head == NULL)
		{
			head = node;
			node->prev = node->next = node;
		}
		else
		{
			timer_node * tmp = head->prev;
			node->prev = tmp;
			tmp->next = node;
			head->prev = node;
			node->next = head;
			head = node;
		}
	}

	inline void push_to_bucket(timer_node * node)
	{
		interlocked_increment(&_free_timer_count);
		spin_autolock alock(_bucket.lock);
		_bucket.push(node);
	}

	inline void push_list_to_bucket(timer_node * first,timer_node * last)
	{
		spin_autolock alock(_bucket.lock);
		_bucket.push_list(first,last);
	}
/*
	inline timer_node * get_from_bucket()
	{
		interlocked_decrement(&_free_timer_count);
		spin_autolock alock(_bucket.lock);
		return _bucket.pop();
	}*/

	inline void remove_from_list(int offset,timer_node * node)
	{
		DCLL_remove(_timerlist[offset],node);
		_timercount[offset]--;		
	}

	inline void insert_to_list(int offset,timer_node * node)
	{
		DCLL_push_front(_timerlist[offset],node);
		_timercount[offset] ++;	
		node->tab_idx = offset;
	}

	inline void remove_from_hi_list(timer_node * node)
	{
		DCLL_remove(_hinterval_list,node);
		_hinterval_count --;
	}

	inline void insert_to_hi_list(timer_node * node)
	{
		DCLL_push_front(_hinterval_list,node);
		_hinterval_count ++;
		node->tab_idx = -1;
	}
public:
	gtimer(int idx_size,int max_timer_count);
	~gtimer();

	enum 
	{
		THREAD_STOP = 0,
		THREAD_RUN = 1,
		THREAD_PAUSE = 2,
	};
	enum 
	{
		TICKTIME = 50000,
		MINTIME = 10000
	};

	int set_timer(int interval,int start_time,int times,abase::timer_callback routine, void * obj);
	int remove_timer(int index,void *object);
	int callback_remove_self(int index);
	inline unsigned int get_tick() { return _tickcount;}
	inline long sys_time() { return _systime + abase::g_delta_time;}
	inline void sys_time(timeval & tv) { tv = _systime_tv; tv.tv_sec += abase::g_delta_time; }
	inline int get_free_timer_count(){ return _free_timer_count;}
	inline int get_timer_total_alloced(){ return _timer_total_alloc;}
	int get_next_interval(int index, void * obj,int * interval, int * rtimes);
	int change_interval(int index ,void * obj, int interval,bool nolock);
	int change_interval_at_once(int index ,void * obj, int interval,bool nolock);

	void timer_thread(int ticktime,int mintime) 
	{
		//_ticktime = TICKTIME;
		//_mintime = MINTIME;
		_ticktime = ticktime;
		_mintime = mintime;
		if(_thread_flag == false) 
		{
			timer_thread();
		}
		else
			ASSERT(false && "重复调用定时器线程");
	}
	void timer_tick();

	void stop_thread()	//stop之后不要立刻重新开始
	{
		if(_thread_flag == false) return;
		_thread_state = THREAD_STOP;
	}
	void pause_thread()
	{
		_thread_state = THREAD_PAUSE;
	}
	void resume_thread()
	{
		_thread_state = THREAD_RUN;
	}
	void reset();
	
};

gtimer::gtimer(int idx_size,int max_timer_count):_idx_size(idx_size),_max_count(max_timer_count)
{
	_timerlist.insert(_timerlist.end(),idx_size,NULL);
	_timercount.insert(_timercount.end(),idx_size,0);
	_node_pool = new timer_node[max_timer_count];
	memset(_node_pool,0,sizeof(timer_node)* max_timer_count);
	int i;
	for(i = 0; i < max_timer_count - 1; i++)
	{
		_node_pool[i].prev = NULL;
		_node_pool[i].next = _node_pool + (i + 1);
		_node_pool[i].callback = NULL;
	}
	_node_pool[i].prev = NULL;
	_node_pool[i].next = NULL;
	_node_pool[i].interval = -1;
	_bucket.head = _node_pool;
	_thread_flag = false;
	_thread_state = 0;
	_tickcount = 0;
	_hinterval_count = 0;
	_hinterval_list = NULL;
	_remove_lock = 0;
	_ticktime  = 0;
	_mintime = 0;
	_timer_offset = 0;
	_interval_update_lock = 0;
	_free_timer_count = _max_count;
	_timer_total_alloc = 0;

	_systime = time(NULL);
	gettimeofday(&_systime_tv, NULL);
}

gtimer::~gtimer()
{
	delete [] _node_pool;
}

void gtimer::reset()
{
	_timerlist.clear();
	_timercount.clear();
	_timerlist.insert(_timerlist.end(),_idx_size,NULL);
	_timercount.insert(_timercount.end(),_idx_size,0);
	_bucket.do_lock();
	_incoming_list.do_lock();
	int i;
	for(i = 0; i < _max_count - 1; i++)
	{
		_node_pool[i].prev = NULL;
		_node_pool[i].next = _node_pool + (i + 1);
		_node_pool[i].callback = NULL;
	}
	_node_pool[i].prev = NULL;
	_node_pool[i].next = NULL;
	_node_pool[i].interval = -1;
	_bucket.head = _node_pool;

	_incoming_list.head = NULL;
	_incoming_list.do_unlock();
	_bucket.do_unlock();

	mutex_spinlock(&_remove_lock);
	_remove_list.clear();
	mutex_spinunlock(&_remove_lock);
	
	_tickcount = 0;
	_hinterval_count = 0;
	_hinterval_list = NULL;
	_timer_offset = 0;
	_free_timer_count = _max_count;
	_timer_total_alloc = 0;
}

int gtimer::get_next_interval(int index, void * obj, int * interval , int * rtimes)
{
	if(index < 0 || index >= _max_count) return -1;
	timer_node & node = _node_pool[index];
	spin_autolock alock(node.spinlock);
	if(node.callback == NULL || node.object != obj) return -1;
	*interval = node.interval;
	*rtimes = node.times;
	int t = (int)(node.nexttime - _tickcount);
	return t;
}

int gtimer::change_interval(int index, void * obj, int interval,bool nolock)
{
	if(index < 0 || index >= _max_count) return -1;
	timer_node & node = _node_pool[index];
	if(nolock) 
	{
		node.interval = interval;
		return 0;
	}
	spin_autolock alock(node.spinlock);
	if(node.callback == NULL || node.object != obj) return -1;
	node.interval = interval;
	return 0;
}

int 
gtimer::change_interval_at_once(int index ,void * obj, int interval,bool nolock)
{
	if(index < 0 || index >= _max_count) return -1;
	timer_node & node = _node_pool[index];
	if(node.callback == NULL || node.object != obj) return -1;
	spin_autolock alock(node.spinlock);
	if(node.callback == NULL || node.object != obj) return -1;
	node.interval = interval;
	//加入到待更新列表中
	spin_autolock alock2(_interval_update_lock);
	_interval_update_list.push_back(&node);

	return 0;
}

int gtimer::remove_timer(int index,void *object)
{
	if(index < 0 || index >= _max_count) return -1;
	timer_node & node = _node_pool[index];
	/* 死锁检测会发现这一点
	if(node.exec_flag)
	{
		//虽然这种情况是可能的，但是更多的情况是错误的调用（在callback里调用了）
		ASSERT(false);
	}*/
	spin_autolock alock2(node.spinlock);
	if(node.callback == NULL || node.object != object) return -2;
	node.callback = NULL;
	node.times = 0;  //这样防止错误的删除
	spin_autolock alock(_remove_lock);
	if(node.next && node.prev)
	{
		//如果next或者prev不完整，可能尚未准备好，所以不进行如此的删除方法，而是由它自己到实践自动删除
		_remove_list.push_back(remove_entry(index,object));
	}
	return 0;
}


int gtimer::set_timer(int interval,int start_time,int times,abase::timer_callback routine, void * obj)
{
	ASSERT(routine);
	timer_node * node;
	{
		spin_autolock alock(_bucket.lock);
		node = _bucket.pop();
		if(node == NULL) return -1;
		interlocked_decrement(&_free_timer_count);
		_timer_total_alloc ++;
	}
	{
		spin_autolock alock3(node->spinlock);
		node->interval = interval;
		node->times = times;
		node->callback = routine;
		node->object = obj;
		node->next = node->prev = NULL;
		node->tab_idx = -1;
		if(start_time <= 0) start_time = interval;
		node->nexttime = _tickcount + (unsigned int) start_time;
		node->exec_flag = false;
	}
	spin_autolock alock2(_incoming_list.lock);
	_incoming_list.push(node);
	return node - _node_pool;
}


void gtimer::handle_remove_list(int offset)
{
	mutex_spinlock(&_remove_lock);
	_remove_list.swap(_remove_list_aux);
	mutex_spinunlock(&_remove_lock);
	timer_node * head = NULL;
	timer_node * tail = NULL;
	int count = 0;
	for(int i = 0; i < (int)_remove_list_aux.size(); i++)
	{
		const remove_entry & entry = _remove_list_aux[i];
		timer_node & node = _node_pool[entry._index];
		if(node.callback != NULL || node.object != entry._object ) continue;
		if(node.tab_idx == -1)
		{
			remove_from_hi_list(&node);
		}
		else
		{
			ASSERT(node.tab_idx >=0 && node.tab_idx < _idx_size);
			remove_from_list(node.tab_idx,&node);
		}
		node.callback = NULL;
		if(head == NULL)
		{
			head = tail = &node;
			//这里不需要赋值node->next，因为以后会改
		}
		else
		{
			node.next = head;
			head = &node;
		}
		count ++;
	}
	interlocked_add(&_free_timer_count,count);
	if(head) push_list_to_bucket(head,tail);
	_remove_list_aux.clear();
}

void gtimer::handle_incoming_list(int offset)
{
	if(_incoming_list.head == NULL) return;
	mutex_spinlock(&_incoming_list.lock);
	node_LL tmpLL = _incoming_list;
	_incoming_list.head = NULL;
	mutex_spinunlock(&_incoming_list.lock);
	timer_node * node;
	while((node = tmpLL.pop()))
	{
		spin_autolock alock(node->spinlock);
		if(node -> callback == NULL)
		{
			//表示是需要删除的对象
			node->callback = NULL;
			node->object = NULL;
			push_to_bucket(node);
			continue;
		}
		unsigned int time_off = node->nexttime - _tickcount;
		if(time_off < (unsigned int)_idx_size)
		{
			int dest = offset + time_off;
			if(dest >= _idx_size) dest -= _idx_size;
			insert_to_list(dest,node);			
		}
		else
		{
			insert_to_hi_list(node);
		}
	}
}

void gtimer:: handle_update_interval_list(int offset)
{
	spin_autolock iulock(_interval_update_lock);
	if(_interval_update_list.empty()) return;
	abase::vector<timer_node*> tmp;
	tmp.swap(_interval_update_list);
	iulock.detach();
	for(size_t i = 0;i < tmp.size(); i ++)
	{
		timer_node * node = tmp[i];
		spin_autolock alock(node->spinlock);
		if(node->callback == NULL) continue;
		node->nexttime = node->interval + _tickcount ;
		if(node->interval < _idx_size)
		{
			int dest = offset + node->interval;
			if(dest >= _idx_size) dest -= _idx_size;
			insert_to_list(dest,node);			
		}
		else
		{
			insert_to_hi_list(node);
		}
	}
}


int gtimer::callback_remove_self(int index)
{
	if(index < 0 || index >= _max_count) return -1;
	timer_node & node = _node_pool[index];
	if(node.exec_flag == 1 && node.callback)
	{
		node.times = 1;
	}
	else
	{
		ASSERT(node.exec_flag == 1 && node.callback && "试图删除错误的条目");
	}
	return 0;
}

void gtimer::handle_current_list(int offset, timer_node * node)
{
	int count = _timercount[offset];
	timer_node * tmp;
	for(int i = 0; i < count; i++,node = tmp)
	{
		tmp = node->next;
		ASSERT(node->nexttime == _tickcount);
		spin_autolock alock(node->spinlock);
		node->exec_flag = 1;
		if(node->callback)
		{
			(*(node->callback))(node - _node_pool, node->object,node->times-1);
		}
		else
		{
		//callback 是NULL 表示处于被删除状态，所以不再调用onTimer
		}
		node->exec_flag = 0;
		node->nexttime = _tickcount + node->interval;
		remove_from_list(offset,node);
		if(node->times) 
		{
			node->times --;
			if(!node->times)
			{
				node->callback = NULL;
				push_to_bucket(node);
				continue;
			}
		}
		if(node->interval < _idx_size)
		{
			int dest = offset + node->interval;
			if(dest >= _idx_size) dest -= _idx_size;
			insert_to_list(dest,node);
		}
		else
		{
			insert_to_hi_list(node);
		}
	}
}

void gtimer::regroup_all_timer()
{
	int count = _hinterval_count;
	timer_node * node = _hinterval_list;
	for(int i = 0; i < count; i ++)
	{
		timer_node * tmp = node->next;
		unsigned int offset = node->nexttime - _tickcount;
		if(offset < (unsigned int)_idx_size)
		{
			remove_from_hi_list(node);
			insert_to_list(offset,node);
		}
		node = tmp;
	}
}

void gtimer::timer_tick()
{
	int offset = _timer_offset;

	handle_incoming_list(offset);
	handle_remove_list(offset);
	handle_update_interval_list(offset);
	timer_node * tmpnode = _timerlist[offset];
	if(tmpnode){
		handle_current_list(offset,tmpnode);
		_timerlist[offset] = NULL;
	}
	offset ++;
	_tickcount ++;
	if(offset >= _idx_size)
	{
		offset = 0;
		regroup_all_timer();
	}
	

	_timer_offset = offset;
	_systime = time(NULL);
	gettimeofday(&_systime_tv, NULL);
}

void gtimer::timer_thread()
{
	int exec_time = _ticktime - _mintime;
	_thread_flag = true;
	_timer_offset = 0;
	timeval tv,tv2;
	_thread_state  = THREAD_RUN;
	gettimeofday(&tv,NULL);
	while(_thread_state != THREAD_STOP)
	{
		if(_thread_state == THREAD_PAUSE)
		{
			usleep(_mintime);
			continue;
		}
		timer_tick();
		gettimeofday(&tv2,NULL);
		int delta = tv2.tv_usec - tv.tv_usec;
		if(tv2.tv_sec - tv.tv_sec > 0) delta += 1000000;
		if(delta < exec_time) delta = _ticktime - delta; else delta = _mintime;
		if(_thread_state == THREAD_STOP) break;
		usleep(delta);
		tv.tv_usec += _ticktime;
		if(tv.tv_usec > 1000000)
		{
			tv.tv_usec -= 1000000;
			tv.tv_sec ++;
		}
	}
	
	reset();
}
}

/*
 *		上面是定时器的具体实现，下面是定时器接口
 */
#define IMP_TIMER	((gtimer*)__imp)
abase::timer::timer(int idx_size,int max_timer_count)
{
	__imp = new gtimer(idx_size,max_timer_count);
}

abase::timer::~timer()
{
	delete IMP_TIMER;
}

int abase::timer::set_timer(int interval,int start_time,int times,abase::timer_callback routine, void * obj)
{
	return IMP_TIMER->set_timer(interval,start_time,times,routine,obj);
}

int abase::timer::remove_timer(int index,void *object)
{
	return IMP_TIMER->remove_timer(index,object);
}

unsigned int abase::timer::get_tick()
{
	return IMP_TIMER->get_tick();
}

long abase::timer::get_systime()
{
	return IMP_TIMER->sys_time();
}

void  abase::timer::get_systime(timeval & tv)
{
	return IMP_TIMER->sys_time(tv);
}

int abase::timer::get_free_timer_count()
{
	return IMP_TIMER->get_free_timer_count();
}

int abase::timer::get_timer_total_alloced()
{
	return IMP_TIMER->get_timer_total_alloced();
}


void abase::timer::timer_thread(int ticktime,int mintime)
{
	return IMP_TIMER->timer_thread(ticktime,mintime);
}

void abase::timer::stop_thread()
{
	return IMP_TIMER->stop_thread();
}

void abase::timer::pause_thread()
{
	return IMP_TIMER->pause_thread();
}

void abase::timer::resume_thread()
{
	return IMP_TIMER->resume_thread();
}

void abase::timer::reset()
{
	return IMP_TIMER->reset();
}

void abase::timer::timer_tick()
{
	return IMP_TIMER->timer_tick();
}



int abase::timer::callback_remove_self(int index)
{
	return IMP_TIMER->callback_remove_self(index);
}

int abase::timer::get_next_interval(int index, void * obj,int * interval, int *rtimes)
{
	return IMP_TIMER->get_next_interval(index,obj, interval, rtimes);
}

int abase::timer::change_interval(int index ,void * obj, int interval,bool nolock)
{
	return IMP_TIMER->change_interval(index ,obj, interval,nolock);

}

