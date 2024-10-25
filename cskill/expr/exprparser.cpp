#include "exprparser.hpp"
namespace GNET
{
TreeNode * ExpressionParser::stmt_sequence(void)
{
	TreeNode * t = statement();
	TreeNode * p = t;
	if (NULL == t) throw ERR_OUT_OF_MEM;
	while ((token!=Token::ENDFILE) && (token!=Token::END) &&
			(token!=Token::ELSE) && (token!=Token::UNTIL))
	{ 
		TreeNode * q=statement();
		if (NULL == q) throw ERR_OUT_OF_MEM;
		p->sibling = q;
		p = q;
	}
	return t;
}
TreeNode * ExpressionParser::statement(void)
{
	TreeNode * t = NULL;
	switch (token) {
		case Token::IF:
			t = if_stmt(); 
			break;
		case Token::REPEAT:
			t = repeat_stmt(); 
			break;
		case Token::READ:
			t = read_stmt();
			match(Token::SEMI);
			break;
		case Token::WRITE:
			t = write_stmt();
			match(Token::SEMI);
			break;
		case Token::PRINT:
			t = print_stmt();
			match(Token::SEMI);
			break;	
		case Token::ID:
			{
			unsigned int type=og->GetTypeID(scanner->getTokenName());
			if (type!=INVALID_IDENTIFIER)
			{
				t=announce_stmt(type);  //variable announce statement
				match(Token::SEMI);
			}
			else
			{
				t=exp();	
				match(Token::SEMI);
			}
			}
			break;
		case Token::NUM:
		case Token::FLOAT:
		case Token::LPAREN:	
		case Token::MINUS:	
			t=exp();	
			match(Token::SEMI);
			break;
		default : 
			throw ERR_UNEXPECT_TOKEN;		 
			break;
	} /* end case */
	return t;
}
TreeNode * ExpressionParser::if_stmt(void)
{
	TreeNode * t = st->CreateStmtNode(TreeNode::IfK,scanner->getContext(false).lineno);
	if (NULL == t) throw ERR_OUT_OF_MEM;
	match(Token::IF);
	t->child[0] = exp();
	match(Token::THEN);
	t->child[1] = stmt_sequence();
	if (token==Token::ELSE)
   	{
		match(Token::ELSE);
		t->child[2] = stmt_sequence();
	}
	match(Token::END);
	return t;
}
TreeNode * ExpressionParser::repeat_stmt(void)
{
	TreeNode * t = st->CreateStmtNode(TreeNode::RepeatK,scanner->getContext(false).lineno);
	if (NULL == t) throw ERR_OUT_OF_MEM;
	match(Token::REPEAT);
	t->child[0] = stmt_sequence();
	match(Token::UNTIL);
	t->child[1] = exp();
	return t;
}
TreeNode * ExpressionParser::read_stmt(void)
{
	TreeNode * t = st->CreateStmtNode(TreeNode::ReadK,scanner->getContext(false).lineno);
	if (NULL == t) throw ERR_OUT_OF_MEM;
	match(Token::READ);
	if (token==Token::ID)
	{
		t->name = scanner->getTokenName();
		match(Token::ID);
	}
	return t;
}
TreeNode * ExpressionParser::write_stmt(void)
{
	TreeNode * t = st->CreateStmtNode(TreeNode::WriteK,scanner->getContext(false).lineno);
	if (NULL == t) throw ERR_OUT_OF_MEM;
	match(Token::WRITE);
	t->child[0] = exp();
	return t;
}
TreeNode * ExpressionParser::print_stmt(void)
{
	TreeNode * t = st->CreateStmtNode(TreeNode::PrintK,scanner->getContext(false).lineno);
	if (NULL == t) throw ERR_OUT_OF_MEM;
	match(Token::PRINT);
	t->child[0] = exp();
	return t;
}
TreeNode * ExpressionParser::announce_stmt(unsigned int type)
{
	TreeNode * t = st->CreateStmtNode(TreeNode::AnnounceK,scanner->getContext(false).lineno);
	t->type = type;
	match(Token::ID);
	if (token == Token::ID) //variable name
	{
		t->name = scanner->getTokenName();
		t->child[0]=exp();
		return t;
	}
	else
		throw ERR_INVALID_ANNOUNCE;
}
TreeNode * ExpressionParser::exp(void)
{
	TreeNode* t=assign_exp();
	return t;	
}
TreeNode * ExpressionParser::assign_exp(void)
{
	TreeNode* t=select_exp();
	if (token == Token::ASSIGN)
	{
		if (t->nodekind!=TreeNode::ExpK)
			throw ERR_INVALID_ASSIGN;
		else if (t->kind.exp!=TreeNode::IdK && (t->kind.exp!=TreeNode::OpK || t->attr.op!=Token::LSQRBRACKET))
			throw ERR_INVALID_ASSIGN;
			
		TreeNode* p= st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
		if (NULL == p) throw ERR_OUT_OF_MEM;
		p->child[0]=t;
		p->attr.op = token;
		p->name = t->name;
		match(Token::ASSIGN);
		p->child[1]=exp();
		t=p;
	}
	return t;
}
TreeNode * ExpressionParser::select_exp(void)
{
	TreeNode* t=logic_relation_exp();
	//Support "exp ? exp:exp"
	if (token == Token::SELECT)
	{
		TreeNode * p = st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
		if (NULL == p) throw ERR_OUT_OF_MEM;
		p->child[0] = t;
		p->attr.op = token;
		t = p;
		match(Token::SELECT); //get next token
		t->child[1] = logic_relation_exp();
		match(Token::COLON);
		t->child[2] = logic_relation_exp();
	}
	return t;
}
TreeNode * ExpressionParser::logic_relation_exp(void)
{
	TreeNode* t=logic_exp();
	if (token == Token::AND || token == Token::OR)
	{
		TreeNode* p=st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
		if (NULL==p) throw ERR_OUT_OF_MEM;
		p->child[0]=t;
		p->attr.op=token;
		t=p;
		match(token);
		t->child[1]=logic_relation_exp();
	}
	return t;
}
TreeNode * ExpressionParser::logic_exp(void)
{
	TreeNode * t = addminus_exp();
	if ((token==Token::LT)||(token==Token::EQ)||(token==Token::GT)||(token==Token::LTE)||(token==Token::GTE)||(token==Token::NE)) {
		TreeNode * p = st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
		if (NULL == p) throw ERR_OUT_OF_MEM;
		p->child[0] = t;
		p->attr.op = token;
		t = p;
		match(token);
		t->child[1] = addminus_exp();
	}
	return t;
}
TreeNode * ExpressionParser::addminus_exp(void)
{
	TreeNode* t = multi_div_exp();
	while ((token==Token::PLUS)||(token==Token::MINUS))
	{ 
		TreeNode * p = st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
		if (NULL == p) throw ERR_OUT_OF_MEM;
		p->child[0] = t;
		p->attr.op = token;
		t = p;
		match(token);
		t->child[1] = multi_div_exp();
	}
	return t;
}
TreeNode * ExpressionParser::multi_div_exp(void)
{
	TreeNode * t = factor();
	while ((token==Token::TIMES)||(token==Token::OVER)||(token==Token::MOD))
	{ 
		TreeNode * p = st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
		if (NULL == p) throw ERR_OUT_OF_MEM;
		p->child[0] = t;
		p->attr.op = token;
		t = p;
		match(token);
		p->child[1] = factor();
	}
	return t;
}	
TreeNode * ExpressionParser::factor(void)
{
	TreeNode * t = NULL;
	switch (token) {
		case Token::NUM :
			t = st->CreateExprNode(TreeNode::ConstIntK,scanner->getContext(false).lineno);
			if (NULL == t) throw ERR_OUT_OF_MEM;
			if (token==Token::NUM)
				t->value.nval = atoi(scanner->getTokenName().c_str());
			match(Token::NUM);
			break;
		case Token::FLOAT :
			t = st->CreateExprNode(TreeNode::ConstFloatK,scanner->getContext(false).lineno);
			if (NULL == t) throw ERR_OUT_OF_MEM;
			if (token==Token::FLOAT)
				t->value.fval = atof(scanner->getTokenName().c_str());
			match(Token::FLOAT);
			break;
		case Token::ID :
			t = st->CreateExprNode(TreeNode::IdK,scanner->getContext(false).lineno);
			if (NULL == t) throw ERR_OUT_OF_MEM;
			if (token==Token::ID)
				t->name = scanner->getTokenName();
			match(Token::ID);
			if (token == Token::MEMSL)
			{
				TreeNode* p = st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
				if (NULL == p) throw ERR_OUT_OF_MEM;
				p->child[0]=t;
				p->attr.op = token;
				t=p;
				match(Token::MEMSL);

				TreeNode* t1=factor();
				TreeNode* tmp=t1;
				while (tmp->child[0]!=NULL) tmp=tmp->child[0];
				tmp->child[0]=t; //put father object to the most left child
				
				t=t1;
			}
			if (token == Token::LSQRBRACKET)
			{
				TreeNode* p = st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
				if (NULL == p) throw ERR_OUT_OF_MEM;
				p->child[0]=t;
				p->attr.op = token;
				t=p;
				match(Token::LSQRBRACKET);
				t->child[1]=exp();
				match(Token::RSQRBRACKET);

				if (token == Token::MEMSL)
				{
					TreeNode* p = st->CreateExprNode(TreeNode::OpK,scanner->getContext(false).lineno);
					if (NULL == p) throw ERR_OUT_OF_MEM;
					p->child[0]=t;
					p->attr.op = token;
					t=p;
					match(Token::MEMSL);
					TreeNode* t1=factor();
					TreeNode* tmp=t1;
					while (tmp->child[0]!=NULL) tmp=tmp->child[0];
					tmp->child[0]=t; //put father object to the most left child
						
					t=t1;
				}
			}
			break;
		case Token::LPAREN :
			match(Token::LPAREN);
			t = exp();
			match(Token::RPAREN);
			break;
		case Token::LBLOCK:
			{
			bool negative=false;	
			match(Token::LBLOCK);
			t = st->CreateExprNode(TreeNode::ConstArrayK,scanner->getContext(false).lineno);
			if (NULL == t) throw ERR_OUT_OF_MEM;
			t->struct_type=TreeNode::ArrayK;
			TreeNode* tail=t;
			while (token!=Token::RBLOCK)
			{
				if (token==Token::MINUS)
				{
					match(Token::MINUS);
					negative=true;
				}
				if (token==Token::NUM)
				{
					TreeNode* p = st->CreateExprNode(TreeNode::ConstIntK,scanner->getContext(false).lineno);
					if (NULL == p) throw ERR_OUT_OF_MEM;
					p->value.nval=atoi(scanner->getTokenName().c_str());
					if (negative) p->value.nval=-p->value.nval;
					tail->child[0]=p;
					tail=p;
					match(Token::NUM);
				}
				else 
				{
					TreeNode* p = st->CreateExprNode(TreeNode::ConstFloatK,scanner->getContext(false).lineno);
					if (NULL == p) throw ERR_OUT_OF_MEM;
					p->value.fval=atof(scanner->getTokenName().c_str());
					if (negative) p->value.fval=-p->value.fval;
					tail->child[0]=p;
					tail=p;
					match(Token::FLOAT);
				}
				/*
				TreeNode* p=factor();
				tail->sibling=p;
				tail=p;
				*/
				if (token==Token::COMMA) match(Token::COMMA);
				negative=false;
			}
			match(Token::RBLOCK);
			}
			break;	
		case Token::MINUS:
			t = st->CreateExprNode(TreeNode::ConstIntK,scanner->getContext(false).lineno);
			if (NULL == t) throw ERR_OUT_OF_MEM;
			t->value.nval=0;
			break;	
		default:
			throw ERR_UNEXPECT_TOKEN;
			break;
	}
	return t;
}

};
