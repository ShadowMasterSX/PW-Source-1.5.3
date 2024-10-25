#ifndef __EXPR_H
#define __EXPR_H
#include "ogparser.hpp"
#include "exprparser.hpp"
#include "analyzer.hpp"
#include "executor.hpp"

#include "visitors.h"
#include <ext/hash_map>

#include "../common/thread.h"

#ifndef BUFLEN
	#define	BUFLEN 256
#endif

#define CALCULATE( TYPE ) \
{\
	Thread::RWLock::RDScoped l(locker);\
	SynTree_Map::iterator it=syntree_map.find(key);\
	if (it==syntree_map.end()) {printf("cannot find %s in map.\n",key.c_str());throw 0;}\
	SyntaxTree& st=(*it).second;\
	{\
		Thread::Mutex::Scoped l(st.locker);\
		Executor* e=Executor::NewInstance();\
		e->Execute(st,og,co);\
		e->Release();\
		switch (st.GetRoot()->type)\
		{\
			case BOOL_T:   	return (TYPE)st.GetRoot()->value.blval;\
			case INT_T:     return (TYPE)st.GetRoot()->value.nval;\
			case FLOAT_T:   return (TYPE)st.GetRoot()->value.fval;\
			default: throw 0;\
		}\
	}\
}

namespace GNET
{
	using namespace __gnu_cxx;
	class Expr
	{
		typedef hash_map<std::string,SyntaxTree> SynTree_Map; 
		
		Thread::RWLock	locker;
		
		ObjectGraph		og;
		SynTree_Map		syntree_map;
		Expr() { }

		public:
		~Expr() { syntree_map.clear(); }
		static Expr& GetInstance(const char* filename=NULL)
		{
			static Expr instance;
			if (filename!=NULL && 0==access(filename,R_OK))
			{
				FILE* ogfile=fopen(filename,"r");
				if (ogfile!=NULL) ObjectGraphParser::GetInstance(ogfile).Parse(instance.og);
				//instance.og.Display();
				fclose(ogfile);
			}
			return instance;
		}	

		bool UpdateExpr( const std::string& key, const std::string & expr ,const std::string& env_class)
		{
			try
			{
				Thread::RWLock::WRScoped l(locker);
				SyntaxTree st;
				SynTree_Map::iterator it;
				
				ExpressionParser::GetInstance(expr).Parse(st,og);
				//SyntaxTree::DisplayWholeTree(st.GetRoot());
				Analyzer::GetInstance().Analyze(st,og,env_class);
				if ((it=syntree_map.find(key))!=syntree_map.end()) 
				{ syntree_map.erase(it); }
				syntree_map[key]=st;
				/*
				printf("\n\n");
				SyntaxTree::DisplayWholeTree(st.GetRoot());
				*/
				st.SetRoot(NULL);
				return true;
			}
			catch(...)
			{
				return false;
			}
		}
		bool UpdateExpr( const std::string& filename, const std::string& env_class)
		{
			FILE* file=NULL;
			try
			{
				Thread::RWLock::WRScoped l(locker);
				SyntaxTree st;
				SynTree_Map::iterator it;
				
				file = fopen(filename.c_str(),"r");
				if (file==NULL) return false;
				ExpressionParser::GetInstance(file).Parse(st,og);
				//SyntaxTree::DisplayWholeTree(st.GetRoot());
				
				Analyzer::GetInstance().Analyze(st,og,env_class);
				if ((it=syntree_map.find(filename))!=syntree_map.end()) 
				{ syntree_map.erase(it); }
				syntree_map[filename]=st;
				
				//SyntaxTree::DisplayWholeTree(st.GetRoot());
				st.SetRoot(NULL);
				fclose(file);
				return true;
			}
			catch(...)
			{
				if (file!=NULL) fclose(file);
				return false;
			}
		}
		bool	boolValue(const std::string& key, GNET::CommonObject* co) { CALCULATE(bool) }
		char	charValue(const std::string& key, GNET::CommonObject* co) { CALCULATE(char) }
		int		intValue(const std::string& key, GNET::CommonObject* co)  {	CALCULATE(int) }
		float	floatValue(const std::string& key, GNET::CommonObject* co) { CALCULATE(float) }

		void Transact( const std::string & key, GNET::CommonObject* co )
		{
			{
				Thread::RWLock::RDScoped l(locker);
				SynTree_Map::iterator it=syntree_map.find(key);
				if (it==syntree_map.end()) throw 0;
				SyntaxTree& st=(*it).second;
				{
					Thread::Mutex::Scoped l(st.locker);
					//Executor::GetInstance().Execute(st,og,co);
					Executor* e=Executor::NewInstance();
					e->Execute(st,og,co);
					e->Release();
				}
			}	
		}
		void Release()
		{
			Thread::RWLock::WRScoped l(locker);
			syntree_map.clear();
		}
	};

};

#endif

