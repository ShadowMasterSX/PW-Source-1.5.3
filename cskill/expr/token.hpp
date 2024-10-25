#ifndef _GNET_TOKEN_H
#define _GNET_TOKEN_H
#include <string>
#include <vector>
#define MAXRESERVED 20

namespace GNET
{
class Token
{
public:
	typedef enum 
	/* book-keeping tokens */
	{
		ENDFILE,ERROR,
		/* reserved words */
		IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,PRINT,
		/* multicharacter tokens */
		ID,NUM,FLOAT,ARRAY,
		/* logic relation */
		AND,OR,
		/* special symbols */
		ASSIGN,NE/*'!='*/,EQ,LT,LTE,GT,GTE,PLUS,MINUS,TIMES,OVER,MOD,LPAREN,RPAREN,SEMI,SELECT/*'?'*/,COLON/*':'*/,MEMSL/*'.' select member*/,
		LSQRBRACKET/*'[',Left square bracket*/,RSQRBRACKET/*']',right square bracket*/,COMMA/*',',comma*/,
		ANNOUNCE/*announce var*/,
		LBLOCK/*'{'*/,RBLOCK,/*'}'*/
	} TokenType;
	
	typedef std::vector<Token> TokenList;
private:	
	std::string name;
	TokenType tok;
public:
	/* lookup table of reserved words */
	Token(const std::string& _name,TokenType _tok) : name(_name),tok(_tok) { }
    /* Lookup a symbol in symbolList,uses linear search */
	static TokenType LookUp (TokenList& t_list,std::string& t_name)
	{
		for (size_t i=0;i<t_list.size();i++)
			if (0==t_list[i].name.compare(t_name))
				return t_list[i].tok;
		return ID;
	}
	static void DisplayToken(TokenType token, const char* tokenString="")
	{ 
		switch (token)
		{ 
			case IF:
			case THEN:
			case ELSE:
			case END:
			case REPEAT:
			case UNTIL:
			case WRITE:
			case PRINT:
				fprintf(stdout,"%s\n",tokenString);
				break;
			case READ:	
				fprintf(stdout,"read %s\n",tokenString);	
				break;
			case ANNOUNCE:
				fprintf(stdout,"announce %s\n",tokenString);
				break;
			case ASSIGN: fprintf(stdout,"Op: =\n"); break;
			case LT: fprintf(stdout,"Op: <\n"); break;
			case GT: fprintf(stdout,"Op: >\n"); break;
			case LTE: fprintf(stdout,"Op: <=\n"); break;
			case GTE: fprintf(stdout,"Op: >=\n"); break;
			case EQ: fprintf(stdout,"Op: ==\n"); break;
			case NE: fprintf(stdout,"Op: !=\n"); break;
			case AND: fprintf(stdout,"Op: &&\n"); break;
			case OR: fprintf(stdout,"Op: ||\n"); break;		  
			case LPAREN: fprintf(stdout,"(\n"); break;
			case RPAREN: fprintf(stdout,")\n"); break;
			case SEMI: fprintf(stdout,";\n"); break;
			case PLUS: fprintf(stdout,"Op: +\n"); break;
			case MINUS: fprintf(stdout,"Op: -\n"); break;
			case TIMES: fprintf(stdout,"Op: *\n"); break;
			case OVER: fprintf(stdout,"Op: /\n"); break;
			case MOD: fprintf(stdout,"Op: %\n"); break;
			case SELECT: fprintf(stdout,"SELECT ?\n"); break;
			case COLON: fprintf(stdout,":\n"); break;
			case MEMSL: fprintf(stdout,"Op: .\n"); break;
			case LSQRBRACKET: fprintf(stdout,"Op: [*]\n"); break;
			case RSQRBRACKET: fprintf(stdout,"index *]\n"); break;
			case ENDFILE: fprintf(stdout,"EOF\n"); break;
			case ARRAY:
				fprintf(stdout,"ARRAY:\n");
				break;
			case FLOAT:
				fprintf(stdout,"FLOAT: %s\n",tokenString);
				break;
			case NUM:
				fprintf(stdout,"NUM: %s\n",tokenString);
				break;
			case ID:
				fprintf(stdout,"ID: %s\n",tokenString);
				break;
			case ERROR:
				fprintf(stdout,"ERROR: %s\n",tokenString);
				break;
			default: /* should never happen */
				fprintf(stdout,"Unknown token: %d\n",token);
		}
	}	
};	

};
#endif
