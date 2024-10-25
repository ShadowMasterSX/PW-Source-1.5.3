#ifndef _GNET_TREENODE_H
#define _GNET_TREENODE_H
#include "token.hpp"
#include <string>

#define MAXCHILDREN 3
#define NAMESPACE_UNDEFINE	0
#define NAMESPACE_DEFAULT	1	
namespace GNET
{
	struct TreeNode
	{
		typedef enum {StmtK,ExpK} NodeKind;
		typedef enum {IfK,RepeatK,/*AssignK,*/ReadK,WriteK,PrintK,AnnounceK} StmtKind;
		typedef enum {OpK,ConstIntK,ConstFloatK,ConstArrayK,IdK} ExpKind;
		typedef enum {ArrayK,VarK,PointerK,RefK} StructKind;
		
		struct TreeNode * child[MAXCHILDREN];
		struct TreeNode * sibling;
		int lineno;
		NodeKind nodekind;
		union { StmtKind stmt; ExpKind exp;} kind;
		union 
		{ 
			Token::TokenType	op;
			//float 				fval;
			//int   				val;
		} attr;
		std::string 		name; 
		unsigned int		name_space;
		unsigned int type; /* for type checking of exps */
		int			 struct_type; /* struct type, such as array,variable,pointer,reference*/
		union
		{
			bool	blval;
			char	cval;
			int		nval;
			float	fval;
			void*	pval;
		}value;
		void* env_object;

		TreeNode() 
		{
			for (int i=0;i<MAXCHILDREN;i++) child[i]=NULL;
			sibling = NULL;	   
			env_object=NULL;
			name_space=NAMESPACE_UNDEFINE;
			struct_type=VarK;//default struct type is VarK
		}
		
		static void DisplayTreeNode(TreeNode* node,unsigned int indent=0)
		{
			if (node == NULL) return;
			static char val[32];
			int i=indent;
			while (i!=0) { printf(" "); i--; }
			if (node->nodekind == StmtK)
			{
				switch (node->kind.stmt)
				{
				case IfK:	
					Token::DisplayToken(Token::IF,"if");
					break;
				case RepeatK:	
					Token::DisplayToken(Token::REPEAT,"repeat");
					break;
				case AnnounceK:	
					Token::DisplayToken(Token::ANNOUNCE,node->name.c_str());
					break;
				case ReadK:	
					Token::DisplayToken(Token::READ,node->name.c_str());
					break;
				case WriteK:	
					Token::DisplayToken(Token::WRITE,"write");
					break;
				case PrintK:	
					Token::DisplayToken(Token::PRINT,"print");
					break;
				}
			}
			if (node->nodekind == ExpK)
			{
				switch (node->kind.exp)
				{
				case ConstArrayK:
					Token::DisplayToken(Token::ARRAY);
					break;
				case ConstIntK:	
					sprintf(val,"%d",node->value.nval);
					Token::DisplayToken(Token::NUM,val);
					break;
				case ConstFloatK:
					sprintf(val,"%.3f",node->value.fval);
					Token::DisplayToken(Token::FLOAT,val);
					break;
				case IdK:
					Token::DisplayToken(Token::ID,node->name.c_str());
					break;
				case OpK:
					Token::DisplayToken(node->attr.op);
					break;
				}
			}
		}

	};
};
#endif
