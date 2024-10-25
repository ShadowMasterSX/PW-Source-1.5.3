#ifndef _GNET_EXPR_PARSER_H
#define _GNET_EXPR_PARSER_H
#include "syntaxtree.hpp"
#include "scanner.hpp"
#include "objectgraph.hpp"
namespace GNET
{
class ExpressionParser
{
    Scanner* scanner;
    Token::TokenType token;
    std::string tokenString;
    Scanner::context_t context;

	ObjectGraph* og;
	SyntaxTree* st;
	
	TreeNode* root;
public:	
	enum ErrCode
	{   
		ERR_UNEXPECT_TOKEN,     //出现不期望的符号
		ERR_INVALID_TOKEN,      //出现非法的符号
		ERR_BRACKET_UNMATCH,    //括号不匹配
		ERR_MISS_ASSIGN,        //缺少等号
		ERR_MISS_SEMI,          //缺少分号
		ERR_MISS_COMMA,         //缺少逗号
		ERR_OUT_OF_MEM,			//内存溢出
		ERR_INVALID_ASSIGN,		//无效的赋值语句
		ERR_INVALID_ANNOUNCE,	//无效的变量声明
	};	
private:
	void match(Token::TokenType expected_token)
	{   
		if (token != expected_token)
		{
			if (expected_token==Token::LPAREN || expected_token==Token::RPAREN) throw ERR_BRACKET_UNMATCH;
			if (expected_token==Token::ASSIGN) throw ERR_MISS_ASSIGN;
			if (expected_token==Token::SEMI) throw ERR_MISS_SEMI;
			if (expected_token==Token::COMMA) throw ERR_MISS_COMMA;
			if (token == Token::ERROR) throw ERR_INVALID_TOKEN;
			throw ERR_UNEXPECT_TOKEN;
		}
		else
		{
			token = scanner->getToken();
		}
	}	
	TreeNode * stmt_sequence(void);
	TreeNode * statement(void);
	TreeNode * if_stmt(void);
	TreeNode * repeat_stmt(void);
	TreeNode * read_stmt(void);
	TreeNode * write_stmt(void); 
	TreeNode * ExpressionParser::print_stmt(void);
	TreeNode * announce_stmt(unsigned int type);
	TreeNode * exp(void);
	TreeNode * assign_exp(void);
	TreeNode * select_exp(void);
	TreeNode * logic_relation_exp(void);
	TreeNode * logic_exp(void);
	TreeNode * addminus_exp(void);
	TreeNode * multi_div_exp(void);
	TreeNode * factor(void);

	ExpressionParser(FILE* _s) { scanner=new Scanner(_s);}
	ExpressionParser(const std::string& _s) { scanner=new Scanner(_s); }
public:
	~ExpressionParser() { delete scanner; }
	static ExpressionParser& GetInstance(FILE* file=NULL)
	{
		static ExpressionParser instance(file);
		if (file!=NULL) instance.scanner->Reload(file);
		return instance;
	}
	static ExpressionParser& GetInstance(const std::string& src)
	{
		static ExpressionParser instance(src);
		instance.scanner->Reload(src);
		return instance;
	}

	void Parse(SyntaxTree& _st,ObjectGraph& _og)
	{
		og=&_og;
		st=&_st;
		token = scanner->getToken();
		try
		{
			root=stmt_sequence();
			st->SetRoot(root);
		}
		catch (ErrCode errcode)
		{
			st->CleanDust(); //release allocate memory
			Scanner::context_t context=scanner->getContext();
			char errinfo[256];
			sprintf(errinfo,"ExprParser:: ERROR:LINE%4d: %s",context.lineno,context.linebuf);
			printf(errinfo);
			int i=29+context.linepos-scanner->getTokenName().size();
			while (i) { printf(" "); i--; }
			switch (errcode)
			{
				case ERR_UNEXPECT_TOKEN:
					printf("^error:unexpected token.\n");
					break;
				case ERR_INVALID_TOKEN:
					printf("^error:invalid token.\n");
					break;
				case ERR_BRACKET_UNMATCH:
					printf("^error:brackets unmatched.\n");
					break;
				case ERR_MISS_ASSIGN:
					printf("^error:miss `='\n");
					break;
				case ERR_MISS_SEMI:
					printf("^error:miss `;'\n");
					break;
				case ERR_MISS_COMMA:
					printf("^error:miss `,'\n");
					break;
				case ERR_OUT_OF_MEM:
					printf("^error:out of memory.\n");	
					break;
				case ERR_INVALID_ASSIGN:
					printf("^error:invalid assignment.\n");	
					break;
				case ERR_INVALID_ANNOUNCE:
					printf("^error:invalid variable announcement.\n");	
					break;
			}
			throw errcode;			
		}
	}
};//end of class	
};//end of namespace
#endif
