#include "analyzer.hpp"
#include "visitors.h"
#define TYPECHECK_LOGIC \
if (t->child[0] == NULL || t->child[1] == NULL)\
	throw ERR_INVALID_OPERATOR;\
TypeCheck(t->child[0]->type,t->child[1]->type,false);\
t->type=og->GetIdentifier("bool");

#define TYPECHECK_CALC \
if (t->child[0] == NULL || t->child[1] == NULL)\
	throw ERR_INVALID_OPERATOR;\
TypeCheck(t->child[0]->type,t->child[1]->type,false);\
t->type=std::max(t->child[0]->type,t->child[1]->type);

#define TYPECHECK_MOD \
if (t->child[0] == NULL || t->child[1] == NULL)\
	throw ERR_INVALID_OPERATOR;\
TypeCheck(t->child[0]->type,INT_T);\
TypeCheck(t->child[1]->type,INT_T);\
t->type=t->child[0]->type;

#define OPERAITON_LOGIC

#define CALCULATE( op ) \
if (t->child[0]->nodekind == TreeNode::ExpK && t->child[1]->nodekind == TreeNode::ExpK)\
{\
if ((t->child[0]->kind.exp == TreeNode::ConstIntK || t->child[0]->kind.exp == TreeNode::ConstFloatK) && (t->child[1]->kind.exp == TreeNode::ConstIntK || t->child[1]->kind.exp == TreeNode::ConstFloatK))\
{\
	if (t->child[0]->type == t->child[1]->type)\
	{\
		if (t->child[0]->type==INT_T) { t->value.nval=t->child[0]->value.nval op t->child[1]->value.nval; }\
		if (t->child[0]->type==FLOAT_T) { t->value.fval=t->child[0]->value.fval op t->child[1]->value.fval; }\
	}\
	else\
	{\
		if (t->child[0]->type > t->child[1]->type)\
			t->value.fval = t->child[0]->value.fval op (float)t->child[1]->value.nval;\
		else\
			t->value.fval = (float)t->child[0]->value.nval op t->child[1]->value.fval;\
	}\
	if (t->type==INT_T)  t->kind.exp=TreeNode::ConstIntK; \
	if (t->type==FLOAT_T) t->kind.exp=TreeNode::ConstFloatK;\
	delete t->child[0];\
	delete t->child[1];\
	t->child[0]=NULL;\
	t->child[1]=NULL;\
}\
}

namespace GNET
{
ObjectGraph*			Analyzer::og=NULL;
SyntaxTree*				Analyzer::st=NULL;
unsigned int			Analyzer::current_type=0;
TreeNode*				Analyzer::current_treenode=NULL;
Analyzer::Context_State	Analyzer::context_state;
	void Analyzer::TypeCheck(unsigned int type1,unsigned int type2,bool strict/*true*/)
	{
		if (strict)
		{
			if (type1 == type2) 
				return;
			else
				throw ERR_TYPE_MISMATCH;
		}
		else
		{
			if (type1 == type2) return;
			if (ObjectGraph::IsReservedType(type1) && ObjectGraph::IsReservedType(type2))
			{
#ifdef _DEBUG				
				if (type1<type2 && current_treenode->attr.op==Token::ASSIGN) 
					printf("Warning: assignment may lose accuration. Line %d\n",current_treenode->lineno);
#endif
				return;
			}
			else
				throw ERR_TYPE_MISMATCH;
		}		
	}
	unsigned int Analyzer::OGCheck(unsigned int parent_type,unsigned int child_name)
	{
		return og->FindMemberNode(parent_type,child_name);
	}
	void Analyzer::StmtCheck(TreeNode* t)
	{
		if (t==NULL) return;
		for (int i=0;i<MAXCHILDREN;i++)
			if (t->child[i]!=NULL) StmtCheck(t->child[i]);
		current_treenode=t; //reassign current_treenode,because it is changed in recursive calls.
		if (t->nodekind == TreeNode::StmtK)
		{
			switch(t->kind.stmt)
			{
				case TreeNode::IfK:
				case TreeNode::RepeatK:
				case TreeNode::ReadK:
				case TreeNode::WriteK:
					break;
				case TreeNode::PrintK:
					if (t->child[0]==NULL) throw ERR_INVALID_PRINT;
					break;
				case TreeNode::AnnounceK:
					if (t->child[0]==NULL) throw ERR_INVALID_ANNOUCE;
					/* create self-defined object */
					if (t->type > MAX_RESERVED_TYPE)
					{
						t->value.pval=(void*) OperationVisitor::CreateCO(t->type,*og);	
						//printf("create: %p, node type=%d, node name=%s\n",t->value.pval,t->type,t->name.c_str());
					}
					else if (t->child[0]->nodekind==TreeNode::ExpK && t->child[0]->kind.exp==TreeNode::OpK && t->child[0]->attr.op==Token::ASSIGN)
					{
						TreeNode* assign_node=t->child[0];
						/* initial self-defined array */
						if (assign_node->child[1]->kind.exp==TreeNode::ConstArrayK)
						{
							TreeNode* var_node=st->symtab.lookup(assign_node->child[0]->child[0]->name);
							int size=assign_node->child[0]->child[1]->value.nval;
							TreeNode* value_node=assign_node->child[1]->child[0];
							int i=0;
							OperationVisitor opvisitor;
							while (value_node!=NULL)
							{
								if (i+1>size) throw ERR_ARRAY_OVERBOUND;
								if (var_node->type != value_node->type) throw ERR_TYPE_MISMATCH;
								((CommonObject*)var_node->value.pval)->Accept(&opvisitor);
								switch (var_node->type)
								{
								case  INT_T:
									if (!opvisitor.Set(i,value_node->value.nval)) throw ERR_INIT_ARRAY_FAILED;
									break;
								case FLOAT_T:
									if (!opvisitor.Set(i,value_node->value.fval)) throw ERR_INIT_ARRAY_FAILED;
									break;
								default:
									throw ERR_INVALID_ARRAY;
									break;
								}	
								value_node=value_node->child[0];
								i++;
							}
							/* 为了保证表达式多次被执行时，数组初值不丢失，不去掉初值结点*/
							t->child[0]=assign_node->child[1];
							assign_node->child[1]=NULL;
							//remove assign branch
							st->ReleaseBranch(assign_node);
						}
					}
					else
					{
						//release varialbe define node
						st->ReleaseBranch(t->child[0]);
						t->child[0]=NULL;
					}
					break;	
			}
		}
		else if (t->nodekind == TreeNode::ExpK)
		{
			switch (t->kind.exp)
			{
				case TreeNode::ConstIntK:
					if ((t->type = og->GetIdentifier("int")) == INVALID_IDENTIFIER)
						throw ERR_INVALID_TYPE;
					break;
				case TreeNode::ConstFloatK:
					if ((t->type = og->GetIdentifier("float")) == INVALID_IDENTIFIER)
						throw ERR_INVALID_TYPE;
					break;
				case TreeNode::ConstArrayK:
					if (context_state == _STATE_STMT)
						throw ERR_ARRAY_CONTEXT;
					if (t->child[0]==NULL)
						throw ERR_INVALID_ARRAY;
					t->type=t->child[0]->type;
					break;	
				case TreeNode::IdK:
					if (t->child[0]!=NULL) //check by ObjectGraph
					{
						t->type=OGCheck(t->child[0]->type,og->GetIdentifier(t->name));
						t->name_space = t->child[0]->type;
					}
					else
					{	
						t->type=OGCheck(current_type,og->GetIdentifier(t->name));
						if (t->type == INVALID_IDENTIFIER)
						{
							TreeNode* var_node=st->symtab.lookup(t->name);
							if (var_node==NULL)
								throw ERR_MEMBER_ERROR;
							else
							{
								t->name_space=NAMESPACE_DEFAULT;
								t->type=var_node->type;
								t->struct_type=var_node->struct_type;
								t->env_object = NULL;
							}
						}
						else
							t->name_space=current_type;
					}
					break;
				case TreeNode::OpK:
					switch (t->attr.op)
					{
						case Token::ASSIGN:
							if (t->child[0]==NULL || t->child[1]==NULL) throw ERR_ASSIGN_CHILD;
							TypeCheck(t->child[0]->type,t->child[1]->type,false);	
							t->type=t->child[0]->type;
							if (context_state==_STATE_VAR_ANNC)
							{
								if (t->child[0]->kind.exp==TreeNode::OpK && t->child[0]->attr.op==Token::LSQRBRACKET)
								{
									if (t->child[1]->kind.exp!=TreeNode::ConstArrayK) throw ERR_INVALID_ARRAY;
								}
							}
							break;
						case Token::SELECT: // ? : 
							if (t->child[0] == NULL || t->child[1] == NULL || t->child[2]==NULL)
								throw ERR_INVALID_OPERATOR;
							TypeCheck(t->child[0]->type,BOOL_T);
							TypeCheck(t->child[1]->type,t->child[2]->type, false);
							t->type=std::max(t->child[1]->type,t->child[2]->type);
							break;
						case Token::MEMSL: //parent.child
							if (t->child[0]==NULL) throw ERR_INVALID_OPERATOR;
							t->type = t->child[0]->type;
							break;
						case Token::LSQRBRACKET: //index
							{
								// []的含义与上下文有关，因此应根据上下文状态来分析
								if (t->child[0] == NULL || t->child[1] == NULL) 
									throw ERR_INVALID_OPERATOR;
								TypeCheck(t->child[1]->type,og->GetIdentifier("int"));
								TreeNode* var_node=st->symtab.lookup(t->child[0]->name);
								//array defined in object_graph
								if (var_node==NULL)
								{	
									t->type=og->FindMemberNode(t->child[0]->type,og->GetIdentifier("value"));
									if (t->type == INVALID_IDENTIFIER)
										throw ERR_INVALID_ARRAY;
									if (context_state == _STATE_VAR_ANNC) //if context is announcement,allocate memory
									{
										/* create objects that defines in OG, now this function is not support */
										throw ERR_ANNOUNCE_OGARRAY;
									}

								}
								else //array defined by self
								{
									t->type=var_node->type;
									if (context_state == _STATE_VAR_ANNC) //if context is announcement,allocate memory
									{
										if (t->type > MAX_RESERVED_TYPE)
											throw ERR_ANNOUNCE_OGARRAY; //cannnot create array list that which class is defined in OG file
										if (!(t->child[1]->nodekind==TreeNode::ExpK && t->child[1]->kind.exp==TreeNode::ConstIntK))
											throw ERR_VAR_ARRAY_UNINIT;
										var_node->value.pval=(void*) OperationVisitor::CreateArray(t->child[1]->value.nval,var_node->type);
										var_node->struct_type=TreeNode::ArrayK;
									}
									else if (context_state==_STATE_STMT)
									{
										if (t->child[0]->struct_type != TreeNode::ArrayK)
											throw ERR_OPERATE_NONARRAY;
									}
								}
							}
							break;
						case Token::EQ:
						case Token::NE:
						case Token::LT:
						case Token::GT:
						case Token::LTE:
						case Token::GTE:
						case Token::AND:
						case Token::OR:
							TYPECHECK_LOGIC
							break;	
						case Token::PLUS:
							TYPECHECK_CALC;
							CALCULATE(+);	
							break;	
						case Token::MINUS: 
							TYPECHECK_CALC;
							CALCULATE(-);	
							break;	
						case Token::TIMES: 
							TYPECHECK_CALC;
							CALCULATE(*);	
							break;	
						case Token::OVER:
							TYPECHECK_CALC;
							CALCULATE(/);	
							break;	
						case Token::MOD:
							TYPECHECK_MOD;
							if ((t->child[0]->kind.exp == TreeNode::ConstIntK) && (t->child[1]->kind.exp == TreeNode::ConstIntK))
							{
								t->value.nval = t->child[0]->value.nval % t->child[1]->value.nval;
								t->kind.exp = TreeNode::ConstIntK;
								t->child[0]=NULL;
								t->child[1]=NULL;
								delete t->child[0];
								delete t->child[1];
							}	
							break;	
						default	:
							throw ERR_INVALID_NODEKIND;
							break;
					}
					break;	
			}
		}
		else
		{ throw ERR_INVALID_NODEKIND; }
	}
};
