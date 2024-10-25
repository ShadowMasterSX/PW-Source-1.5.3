/*
 * FILE: cpxtab.h
 *
 * DESCRIPTION: 支持多种快速查询方式的数据结构
 *
 * CREATED BY: Cui Ming 2002-9-13
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
*/

#ifndef __CM_CPX_TABLE_H__
#define __CM_CPX_TABLE_H__

#include <stdlib.h>
#include <string.h>

#include "spinlock.h"
#include "hashtab.h"
#include "allocator.h"

namespace abase {

//注意，表的容量是固定的，不能扩张。
template <class __data,class __seckey,class __hf = _hash_function,class __allocator=default_alloc>
class cpxtab
{
public:
	typedef __data 	DataType;
	typedef __data*	LPDATA;
	typedef __seckey KeyType;
	typedef pair<DataType,const KeyType> DataPack;
	typedef DataPack * LPDATAPACK;
	typedef struct __DataNode
	{
		DataPack*	data;
		int		spinlock;
		__DataNode*	prev;
		__DataNode*	next;
	}DataNode;
	typedef DataNode * LPDATANODE;
	typedef __allocator HashAlloc;
	typedef hashtab<LPDATANODE,__seckey,__hf,HashAlloc> SecTable;
	typedef struct __DataTab
	{
		DataPack * 	data;
		int		spinlock;
		__DataTab *	prev;
		__DataTab * 	next;
	}DataTab;
	typedef struct __DataChain
	{
		LPDATANODE  head;
		LPDATANODE  tail;
		
	}DataChain;

private:
	SecTable 	_sec_tab;
	DataTab*	_fst_tab;
	DataTab*	_databuckets;
	DataChain 	_seq_chain;
	
	int 		_element_count;
	int		_max_size;
	int		_spinlock;

public:
	cpxtab(int table_size);
	~cpxtab();
	
	int get(int index,DataType ** data,const KeyType **key);
	int get(const KeyType &, DataType **data,int *index);
	int put(const KeyType & key, const DataType & data);
	int remove(const KeyType &);
	int remove(int index);
	template <class callback> int heartbeat(callback &);
	template <class callback> int heartbeat2(callback &);

	void lock_tab() { mutex_spinlock(&_spinlock);}
	void unlock_tab() { mutex_spinunlock(&_spinlock);}

	int lock_record(int index)
	{
		if(index < 0 || index>=_max_size) return OUT_OF_RANGE;
		mutex_spinlock(&_fst_tab[index].spinlock);
		return 0;
	}

	int unlock_record(int index)
	{
		if(index < 0 || index>=_max_size) return OUT_OF_RANGE;
		mutex_spinunlock(&_fst_tab[index].spinlock);
		return 0;
	}
	
public:
	enum {	ALLOC_MEM_FAILED	=-1,
		NOT_ENOUGH_SLOT		=-2,
		SECKEY_ALREADY_EXIST	=-3,
		OUT_OF_RANGE		=-4,
		INVALID_INDEX		=-5,
		SECKEY_NOT_FOUND	=-6
	};
	
};

template <class __data,class __seckey,class __hf,class __allocator> 
	cpxtab<__data,__seckey,__hf,__allocator>::
	cpxtab(int table_size):_sec_tab(table_size),_element_count(0),_max_size(table_size),_spinlock(0)
{
	_fst_tab = new DataTab[table_size];
	memset(_fst_tab,0,sizeof(DataTab)*table_size);
	for(int i = 0; i<_max_size;i++)
	{
		_fst_tab[i].data = NULL;
		_fst_tab[i].next = _fst_tab + i + 1;
		_fst_tab[i].prev = NULL;
	}
	_fst_tab[_max_size - 1].next = NULL;
	_seq_chain.head = _seq_chain.tail = NULL;
	_databuckets = _fst_tab;
}

template <class __data,class __seckey,class __hf,class __allocator>
	cpxtab<__data,__seckey,__hf,__allocator>::
	~cpxtab()
{
	//$$$$
}

template <class __data,class __seckey,class __hf,class __allocator>
int 	cpxtab<__data,__seckey,__hf,__allocator>::
put(const KeyType &key, const DataType & data)
{
	if(!_databuckets) return NOT_ENOUGH_SLOT;
	
	pair<DataNode **, bool> hp = _sec_tab.get(key);
	if(hp.second) return SECKEY_ALREADY_EXIST;
	
	DataTab * tmptab= _databuckets;
	DataNode * tmp = (DataNode *)tmptab;
	_databuckets = _databuckets->next;
	
	tmp->data = new(__allocator::allocate(sizeof(DataPack))) DataPack(data,key);

	if(_seq_chain.tail == NULL)
	{
		_seq_chain.head = _seq_chain.tail = tmp;
		tmp->next = tmp->prev = NULL;
	}
	else
	{
		_seq_chain.tail->next = tmp;
		tmp->prev = _seq_chain.tail; 
		tmp->next = NULL;
		_seq_chain.tail = tmp;
	}
	_sec_tab.put(key,tmp);
	_element_count ++;
	return tmptab - _fst_tab;
}

template <class __data,class __seckey,class __hf,class __allocator>
int 	cpxtab<__data,__seckey,__hf,__allocator>::
get(int index, __data ** data,const __seckey ** key)
{
	if(index < 0 || index >= _max_size) return OUT_OF_RANGE;
	if(_fst_tab[index].data == NULL) return INVALID_INDEX;
	*data 	= &(_fst_tab[index].data->first);
	if(key) *key = &(_fst_tab[index].data->second);
	return 0;
}

template <class __data,class __seckey,class __hf,class __allocator>
int 	cpxtab<__data,__seckey,__hf,__allocator>::
get(const __seckey &key, __data ** data,int * index)
{
	pair<DataNode **, bool> hp = _sec_tab.get(key);
	if(hp.second) {
		*data 	= &((*hp.first)->data->first);
		if(index) *index =(DataTab *)((*hp.first)->data) - _fst_tab;
		return 0;
	}
	else
	{
		return SECKEY_NOT_FOUND;
	}
}

template <class __data,class __seckey,class __hf,class __allocator>
int 	cpxtab<__data,__seckey,__hf,__allocator>::
remove(int index)
{
	if(index < 0 || index >= _max_size) return OUT_OF_RANGE;
	if(_fst_tab[index].data == NULL) return INVALID_INDEX;
	DataPack *data = _fst_tab[index].data;
	DataNode *node = (DataNode *)(_fst_tab + index);
	if(node->prev == NULL) {
		_seq_chain.head = _seq_chain.head->next;
		if(_seq_chain.head) _seq_chain.head->prev = NULL;
	}
	else
	{
		node->prev->next = node->next;
	}
	if(node->next == NULL){
		_seq_chain.tail = _seq_chain.tail->prev;
		if(_seq_chain.tail) _seq_chain.tail->next = NULL;
	}
	else
	{
		node->next->prev = node->prev;
	}
	node->next = node->prev = NULL;
	node->data = NULL;

	_sec_tab.erase(data->second);

	data->~DataPack();
	__allocator::deallocate(data,sizeof(DataPack));
	
	_fst_tab[index].next = _databuckets;
	_databuckets = _fst_tab + index;

	_element_count --;
	return 0;
}

template <class __data,class __seckey,class __hf,class __allocator>
int 	cpxtab<__data,__seckey,__hf,__allocator>::
remove(const __seckey & key)
{
	pair<DataNode **, bool> hp = _sec_tab.get(key);
	if(hp.second) {
		int index = (DataTab *)(*hp.first) - _fst_tab; 
		return remove(index);
	}
	else
	{
		return SECKEY_NOT_FOUND;
	}
}

template <class __data,class __seckey,class __hf,class __allocator>
template <class callback> 
int cpxtab<__data,__seckey,__hf,__allocator>::
heartbeat(callback & cb)
{
	DataNode * tmp = _seq_chain.head;
	while(tmp)
	{
		int index = (DataTab *)tmp - _fst_tab;
		cb(index,tmp->data->first,tmp->data->second);
		tmp = tmp->next;
	}
	return 0;
}

template <class __data,class __seckey,class __hf,class __allocator>
template <class callback> 
int cpxtab<__data,__seckey,__hf,__allocator>::
heartbeat2(callback & cb)
{
	for(int  i = 0; i< _max_size;i++)
	{
		if(_fst_tab[i].data){
			cb(i,_fst_tab[i].data->first,_fst_tab[i].data->second);
		}
	}
	return 0;
}
}

#endif

