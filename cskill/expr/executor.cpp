#include "executor.hpp"

#define LOGIC_OPERATION(op) \
if (t->child[0]->kind.exp==TreeNode::IdK) get_member_value(t->child[0]);\
if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);\
type1=t->child[0]->type;\
type2=t->child[1]->type;\
if (type1>type2) type_converter(t->child[1],type1);\
else if (type1<type2) type_converter(t->child[0],type2);\
switch(std::max(t->child[0]->type,t->child[1]->type))\
{\
	case BOOL_T: t->value.blval=(t->child[0]->value.blval op t->child[1]->value.blval); break;\
	case CHAR_T: t->value.blval=(t->child[0]->value.cval op t->child[1]->value.cval); break;\
	case INT_T: t->value.blval=(t->child[0]->value.nval op t->child[1]->value.nval); break;\
	case FLOAT_T: t->value.blval=(t->child[0]->value.fval op t->child[1]->value.fval); break;\
	default: t->value.blval=(t->child[0]->value.pval op t->child[1]->value.pval); break;\
}

#define LOGIC_RELATION_OPERATION(op) \
if (t->child[0]->kind.exp==TreeNode::IdK) get_member_value(t->child[0]);\
if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);\
if (t->child[0]->type != BOOL_T) type_converter(t->child[0],BOOL_T);\
if (t->child[1]->type != BOOL_T) type_converter(t->child[1],BOOL_T);\
t->value.blval=t->child[0]->value.blval op t->child[1]->value.blval;
	
#define CALC_OPERATION(op) \
if (t->child[0]->kind.exp==TreeNode::IdK) get_member_value(t->child[0]);\
if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);\
type1=t->child[0]->type;\
type2=t->child[1]->type;\
if (type1>type2) type_converter(t->child[1],type1);\
else if (type1<type2) type_converter(t->child[0],type2);\
switch(std::max(t->child[0]->type,t->child[1]->type))\
{\
	case BOOL_T: t->value.blval=t->child[0]->value.blval op t->child[1]->value.blval; break;\
	case CHAR_T: t->value.cval=t->child[0]->value.cval op t->child[1]->value.cval; break;\
	case INT_T: t->value.nval=t->child[0]->value.nval op t->child[1]->value.nval; break;\
	case FLOAT_T: t->value.fval=t->child[0]->value.fval op t->child[1]->value.fval; break;\
	default: throw ERR_INVALID_TYPE;\
}


namespace GNET
{
//using namespace EXEC;
//ObjectGraph* 			Executor::og;
//GNET::CommonObject* 	Executor::current_evn;
//TreeNode* 	Executor::current_treenode;	
	Executor::ExecutorPool	Executor::pool;
	Thread::Mutex			Executor::locker_pool;
	//get member value
	void Executor::get_member_value(TreeNode* t,int index)
	{
		if (NULL==t) return;
		OperationVisitor opvisitor;
		if (t->name_space == NAMESPACE_DEFAULT)
		{
			TreeNode* var_node;
			var_node=st->symtab.lookup(t->name);
			if (var_node == NULL)
			   	throw ERR_UNDEFINED_VAR;
			else
				t->value = var_node->value;
			return;
		}
		if (t->env_object == NULL) throw ERR_NULL_ENV;
		((CommonObject*)t->env_object)->Accept(&opvisitor);
		bool result;
		/* no array object defined */
		/*
		switch (t->type)
		{
			case BOOL_T:
				result=opvisitor.Get(t->name,t->value.blval,index);
				break;	
			case CHAR_T:
				result=opvisitor.Get(t->name,t->value.cval,index);
				break;	
			case INT_T:
				result=opvisitor.Get(t->name,t->value.nval,index);
				break;	
			case FLOAT_T:
				result=opvisitor.Get(t->name,t->value.fval,index);
				break;	
			default:
				result=opvisitor.Get(t->name,(CommonObject*&) t->value.pval,index);
				break;	
		}
		*/

		/* define array object */
		switch (t->type)
		{
			case BOOL_T:
				if (index==-1)
					result=opvisitor.Get(t->name,t->value.blval);
				else
					result=opvisitor.Get(index,t->value.blval);
				break;	
			case CHAR_T:
				if (index==-1)
					result=opvisitor.Get(t->name,t->value.cval);
				else
					result=opvisitor.Get(index,t->value.cval);
				break;	
			case INT_T:
				if (index==-1)
					result=opvisitor.Get(t->name,t->value.nval);
				else
					result=opvisitor.Get(index,t->value.nval);
				break;	
			case FLOAT_T:
				if (index==-1)
					result=opvisitor.Get(t->name,t->value.fval);	
				else
					result=opvisitor.Get(index,t->value.fval);
				break;	
			default:
				if (index==-1)
					result=opvisitor.Get(t->name,(CommonObject*&) t->value.pval);
				else
					result=opvisitor.Get(index,(CommonObject*&)t->value.pval);
				break;	
		}
		/////////////////////////////////////
		if (!result) throw ERR_GET_MEMBER;
	}
	//set member value
	template<typename _t>
		void Executor::set_member_value(TreeNode* t, const _t& value,int index)
		{
			if (NULL==t) return;
			OperationVisitor opvisitor;
			if (t->env_object == NULL) { throw ERR_NULL_ENV; }
			((CommonObject*)t->env_object)->Accept(&opvisitor);
			/* no array object define */
			/*
			if (!opvisitor.Set(t->name,value,index)) 
				throw ERR_SET_MEMBER;
			*/
			
			/* array object defined */
			if (index==-1)
			{ if (!opvisitor.Set(t->name,value)) throw ERR_SET_MEMBER; }
			else if (index>0)
			{ if (!opvisitor.Set(index,value)) throw ERR_SET_MEMBER; }
			else
				throw ERR_INVALID_INDEX;
		}
	//type converter,srcType and dstType must be int(bool,float,char)
	template<typename _t>
		void Executor::to_dstType(TreeNode* t,int dstType,const _t& value)
		{
			switch (dstType)
			{
				case BOOL_T:
					t->value.blval=(bool)value;
					break;
				case CHAR_T:
					t->value.cval=(char)value;
					break;
				case INT_T:
					t->value.nval=(int)value;
					break;
				case FLOAT_T:
					t->value.fval=(float)value;
					break;
				default:
					/*error*/
					throw ERR_INVALID_TYPE;
					break;
			}
			if (t->nodekind==TreeNode::ExpK && (t->kind.exp == TreeNode::ConstFloatK ||t->kind.exp == TreeNode::ConstIntK))
			{
				t->type=dstType;
				if (t->type == INT_T) t->kind.exp= TreeNode::ConstIntK;
				if (t->type == FLOAT_T) t->kind.exp= TreeNode::ConstFloatK;
			}
		}	
	void Executor::type_converter(TreeNode* t,int dstType)
	{
		if (NULL==t) return;
		switch (t->type)
		{
			case BOOL_T:
				{
					bool tmp=t->value.blval;
					to_dstType(t,dstType,tmp);
					break;
				}
			case CHAR_T:
				{
					char tmp=t->value.cval;
					to_dstType(t,dstType,tmp);
					break;
				}
			case INT_T:
				{
					int tmp=t->value.nval;
					to_dstType(t,dstType,tmp);
					break;
				}
			case FLOAT_T:
				{
					float tmp=t->value.fval;
					to_dstType(t,dstType,tmp);
					break;
				}
			default:
				/*error*/
				throw ERR_INVALID_TYPE;
				break;	
		}
	}

	void Executor::stmtexec(TreeNode* t)
	{
		static unsigned int type1;
		static unsigned int type2;
		if (NULL==t) return;
		current_treenode = t;
		for (int i=0;i<MAXCHILDREN;i++)
			if (t->child[i]!=NULL) stmtexec(t->child[i]);
		current_treenode = t; //reassign current_treenode,because it is changed in recursive calls.
		
		if (t->nodekind==TreeNode::StmtK)
		{
			switch(t->kind.stmt)
			{
				case TreeNode::IfK:
				case TreeNode::RepeatK:
				case TreeNode::ReadK:
				case TreeNode::WriteK:
				case TreeNode::AnnounceK:
					//reassign array data
					if (t->child[0]!=NULL)
					{
						TreeNode* value_node=t->child[0]->child[0];
						OperationVisitor opvisitor;
						((CommonObject*)t->value.pval)->Accept(&opvisitor);
						int i=0;
						while (value_node!=NULL)
						{
							switch (t->type)
							{
								case  INT_T:
									if (!opvisitor.Set(i,value_node->value.nval)) throw ERR_INIT_ARRAY_FAILED;
									break;
								case FLOAT_T:
									if (!opvisitor.Set(i,value_node->value.fval)) throw ERR_INIT_ARRAY_FAILED;
									break;
								default:
									break;
							}
							value_node=value_node->child[0];						
							i++;
						}
					}
					break;
				case TreeNode::PrintK:
					switch (t->child[0]->type)
					{
						case BOOL_T:
							printf("$print= %d\n",t->child[0]->value.blval);
							break;
						case CHAR_T:
							printf("$print= %c\n",t->child[0]->value.cval);
							break;	
						case INT_T:
							printf("$print= %d\n",t->child[0]->value.nval);
							break;
						case FLOAT_T:
							printf("$print= %f\n",t->child[0]->value.fval);
							break;
						default:
							printf("$print= (OBJECT)%p\n",t->child[0]->value.pval);
							break;	
					}
					break;	
			}
		}	
		else if (t->nodekind == TreeNode::ExpK)
		{
			switch (t->kind.exp)
			{
				case TreeNode::ConstIntK:
					//t->value.nval = t->attr.val;
					break;
				case TreeNode::ConstFloatK:
					//t->value.fval = t->attr.fval;
					break;
				case TreeNode::ConstArrayK:
					break;	
				case TreeNode::IdK:
					if (t->child[0] == NULL) //env-member
					{
						TreeNode* var_node=st->symtab.lookup(t->name);
						if (var_node==NULL)
							t->env_object = (void *)current_evn;
						else
							t->value=var_node->value;
					}
					else if (t->child[0]->nodekind==TreeNode::ExpK && t->child[0]->kind.exp==TreeNode::OpK && t->child[0]->attr.op==Token::MEMSL)
					{
						t->env_object = (void *) t->child[0]->value.pval;
					}
					else
					{
						throw ERR_ID_NODE;	//wrong Identity node.
					}
					break;	
				case TreeNode::OpK:
					switch (t->attr.op)
					{
						case Token::ASSIGN:
							{
								if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);
								type1 = t->child[0]->type;
								type2 = t->child[1]->type;
								if (type1!=type2) type_converter(t->child[1],type1);
								
								TreeNode* assign_node;
								int index=-1;
								
								if (t->child[0]->nodekind==TreeNode::ExpK && t->child[0]->kind.exp==TreeNode::OpK &&
										t->child[0]->attr.op==Token::LSQRBRACKET)
								{
									/* no array object define */
									//assign_node=t->child[0]->child[0];

									/* define array object */
									assign_node=t->child[0];
									//////////////////////////////////////
									index=t->child[0]->child[1]->value.nval;
								}
								else
									assign_node=t->child[0];
								//assign node is a temporary variable
								if (assign_node->name_space == NAMESPACE_DEFAULT)
								{
									TreeNode* var_node=st->symtab.lookup(assign_node->name);
									if (var_node == NULL) throw ERR_NULL_ENV;
									var_node->value = t->child[1]->value;
									assign_node->value=t->child[1]->value;	
									break;
								}
								switch(type1)
								{
									case BOOL_T:
										set_member_value(assign_node/*t->child[0]*/,t->child[1]->value.blval,index);
										t->value.blval=t->child[1]->value.blval;
										break;
									case CHAR_T:
										set_member_value(assign_node/*t->child[0]*/,t->child[1]->value.cval,index);
										t->value.cval=t->child[1]->value.cval;
										break;
									case INT_T:
										set_member_value(assign_node/*t->child[0]*/,t->child[1]->value.nval,index);
										t->value.nval=t->child[1]->value.nval;
										break;
									case FLOAT_T:
										set_member_value(assign_node/*t->child[0]*/,t->child[1]->value.fval,index);
										t->value.fval=t->child[1]->value.fval;
										break;
									default:
										if (!ObjectGraph::IsReservedType(type1))
										{
											set_member_value(assign_node/*t->child[0]*/,(CommonObject*)t->child[1]->value.pval,index);
											t->value.pval=t->child[1]->value.pval;
										}
										else
										{
											throw ERR_INVALID_TYPE;
										}
										break;	
								}
							}
							break;
						case Token::SELECT:
							if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);
							if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);
							if (t->child[2]->kind.exp==TreeNode::IdK) get_member_value(t->child[2]);
							if (t->child[0]->type != BOOL_T) type_converter(t->child[0],BOOL_T);
							if (t->child[0]->value.blval)
							{
								t->value=t->child[1]->value;
								t->type=t->child[1]->type;
							}
							else
							{
								t->value=t->child[2]->value;
								t->type=t->child[2]->type;
							}
							break;
						case Token::MEMSL:
							get_member_value(t->child[0]);
							t->value=t->child[0]->value;
							break;
						case Token::LSQRBRACKET:
							if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);
							/* no array object define */
							//get_member_value(t->child[0],t->child[1]->value.nval);
							//t->value=t->child[0]->value;

							/* define array object */
							get_member_value(t->child[0]);
							t->env_object = t->child[0]->value.pval;
							get_member_value(t,t->child[1]->value.nval); //get the value of '[]' operator
							break;
						case Token::EQ:
							LOGIC_OPERATION(==);
							break;
						case Token::NE:
							LOGIC_OPERATION(!=);
							break;
						case Token::LT:
							LOGIC_OPERATION(<);
							break;
						case Token::GT:
							LOGIC_OPERATION(>);
							break;
						case Token::LTE:
							LOGIC_OPERATION(<=);
							break;
						case Token::GTE:
							LOGIC_OPERATION(>=);
							break;
						case Token::AND:
							LOGIC_RELATION_OPERATION(&&);
							break;
						case Token::OR:
							LOGIC_RELATION_OPERATION(||);
							break;
						case Token::PLUS:
							CALC_OPERATION(+);
							break;
						case Token::MINUS:
							CALC_OPERATION(-);
							break;
						case Token::TIMES:
							CALC_OPERATION(*);
							break;
						case Token::OVER:
							CALC_OPERATION(/);
							break;
						case Token::MOD:
							if (t->child[0]->kind.exp==TreeNode::IdK) get_member_value(t->child[0]);
							if (t->child[1]->kind.exp==TreeNode::IdK) get_member_value(t->child[1]);
							t->value.nval = t->child[0]->value.nval % t->child[1]->value.nval;	
							break;
						default:
							throw ERR_INVALID_NODEKIND;
							break;	
					}
					break; //end of "Case TreeNode::OpK"
			}
		}
	}
};
