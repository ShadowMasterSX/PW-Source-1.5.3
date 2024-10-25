/********************************************************************
  created:	   20/12/2005  
  filename:	   Pf2DBfs.h
  author:      Wangkuiwu  
  description: path finding 2d using best first searching
  Copyright (c) , All Rights Reserved.
*********************************************************************/
#ifndef   _PF_2D_BFS_H_
#define   _PF_2D_BFS_H_

#include "PathFinding2D.h"

#include <hashtab.h>

class Pf2DNode
{
public:
   short x, z;   //pos
   short prv_x, prv_z;   //prv node
   float cost;   //h 
   
};

/*
 * open list, currently use array
 * @desc :  
 * @todo:   
 * @author: kuiwu [20/12/2005]
 * @ref:
 */
class Pf2DOpen
{
public:
	Pf2DOpen();
	~Pf2DOpen();
	void Init(int nMaxSize = 30);
	void Clear();
	inline void Push(const Pf2DNode& node);
	bool Empty() const
	{
		return (m_List.empty());
	}
	inline void PopMinCost(Pf2DNode& node);
	/*
	 *
	 * @desc :
	 * @param :     
	 * @return :  size of the list if not found
	 * @note:
	 * @todo:   
	 * @author: kuiwu [21/12/2005]
	 * @ref:
	 */
	inline int Find(short x, short z);
	inline int GetSize()
	{
		return m_List.size();
	}
private:
	vector<Pf2DNode> m_List;
	int m_nMaxSize;


};

/*
 * close list, currently use hash table
 * @desc :
 * @todo:   
 * @author: kuiwu [21/12/2005]
 * @ref:
 */
class Pf2DClose
{
	typedef hashtab<unsigned int, unsigned int, abase::_hash_function>  CloseList;
public:
	Pf2DClose()
		:m_List(256)
	{
	}
	~Pf2DClose()
	{
		Clear();
	}
	inline bool Find(short x, short z);
	inline void Push(const Pf2DNode& node);
	inline void GetPrv(short x, short z, short& prv_x, short& prv_z);
	void        Clear()
	{
		m_List.clear();
	}
private:
	CloseList    m_List;	
};

class CPf2DBfs : public CPathFinding2D  
{
public:
	CPf2DBfs();
	virtual ~CPf2DBfs();
	virtual void Init(CMap * pMap, const POS2D& posStart, const POS2D& posGoal, float fRange = 0.01f );
	virtual void StepSearch(int nSteps);
	virtual void GeneratePath(vector<POS2D>& path);
	int          GetSumSearchSteps() const 
	{
		return m_nSumSearchSteps;
	}
	void         Release();
protected:
	Pf2DOpen   m_Open;
	Pf2DClose  m_Close;
	Pf2DNode  m_CurNode;
	int       m_nSumSearchSteps;
};

#endif
