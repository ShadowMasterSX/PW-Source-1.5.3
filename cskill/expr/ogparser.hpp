#ifndef _GNET_OGPARSER_H
#define _GNET_OGPARSER_H

#include "scanner.hpp"
#include "objectgraph.hpp"
namespace GNET
{
class ObjectGraphParser //Object Graph
{
	Scanner* scanner;
	Token::TokenType token;
	std::string tokenString;
	Scanner::context_t context;
	
public:
	enum ErrCode
	{
		ERR_UNEXPECT_TOKEN,		//出现不期望的符号
		ERR_INVALID_TOKEN,		//出现非法的符号
		ERR_BRACKET_UNMATCH,	//括号不匹配
		ERR_MISS_ASSIGN,		//缺少等号
		ERR_MISS_SEMI,			//缺少分号
		ERR_MISS_COMMA,			//缺少逗号
	};
private:
	void match(Token::TokenType expected_token)
	{
		if (token != expected_token)
		{
			if (expected_token==Token::LT || expected_token==Token::GT) throw ERR_BRACKET_UNMATCH;
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
	void newStmtSequence(ObjectGraph& og)
	{
		while (token != Token::ENDFILE)
		{
			if (token == Token::ID) newStatement(og);
			match(Token::SEMI);
		}
	}
	void newStatement(ObjectGraph& og)
	{
		og.AddTypeNode(scanner->getTokenName());
		match(Token::ID);
		match(Token::ASSIGN);
		newMemberNode(og);
		while (token != Token::SEMI)
		{
			match(Token::COMMA);
			newMemberNode(og);
		}
	}
	void newMemberNode(ObjectGraph& og)
	{
		std::string member_name;
		match(Token::LT);
		if (token == Token::GT)  //null member node
		{
			match(Token::GT);
			return;
		}
		member_name=scanner->getTokenName();
		match(Token::ID);
		match(Token::COMMA);
		og.AddMemberNode(scanner->getTokenName(),member_name);
		match(Token::ID);
		match(Token::GT);
	}
	ObjectGraphParser(FILE* _s)  { scanner=new Scanner(_s); }
public:
	~ObjectGraphParser()
	{
		delete scanner;
	}
	static ObjectGraphParser& GetInstance(FILE* file=NULL) 
	{ 
		static ObjectGraphParser instance(file); 
		if (file!=NULL) instance.scanner->Reload(file);
		return instance; 
	}
	void Parse(ObjectGraph& og)
	{
		og.Reset();
		token = scanner->getToken();
		try 
		{
			newStmtSequence(og);
		}
		catch(ErrCode errcode)
	   	{ 
			Scanner::context_t context=scanner->getContext();
			char errinfo[256];
			sprintf(errinfo,"OGParser:: ERROR:LINE%4d: %s",context.lineno,context.linebuf);
			printf(errinfo);
			int i=27+context.linepos-scanner->getTokenName().size();
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
			}
			throw errcode;
		}
	}
}; //end of class
}; //end of namespace
#endif
