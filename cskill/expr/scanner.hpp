/****************************************************/
/* File: scan.h                                     */
/* The scanner interface for the TINY compiler      */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SCAN_H_
#define _SCAN_H_

#include <string>
#include "token.hpp"

/* MAXTOKENLEN is the maximum size of a token */
#define MAXTOKENLEN 64
#define BUFLEN		256

#define MAXRESERVED 20

#define EchoSource	false
#define TraceScan	false	
namespace GNET
{
class Scanner
{
	/* states in scanner DFA */
	typedef enum
	{ START,INASSIGN,RDYINCMNT,INCOMMENT,RDYOUTCMNT,INNE,INLT,INGT,INNUM,INFLOAT,INID,DONE }
	StateType;
	
	enum { 
		SRC_FILE,SRC_STRING
	}src_type;
	
	int lineno; /*current line number of source file*/
	int linepos; /* current position in LineBuf */
	int bufsize; /* current size of buffer string */
	int EOF_flag; /* corrects ungetNextChar behavior on EOF */
	char lineBuf[BUFLEN]; /* holds the current line */
	
	FILE* source;
	std::string str_source;
	
	std::string tokenString;
	static Token::TokenList reservedWords;
public:
	struct context_t
	{
		int lineno;
		int linepos;
		char linebuf[BUFLEN];
		context_t() { }
	};
private:	
	/* getNextChar fetches the next non-blank character
	   from lineBuf, reading in a new line if lineBuf is
	   exhausted */
	int getNextChar(void)
	{ 
		if (src_type == SRC_FILE)
		{
			if (!(linepos < bufsize))
			{ 
				lineno++;
				if (fgets(lineBuf,BUFLEN-1,source))
				{ 
					if (EchoSource) fprintf(stdout,"%4d: %s",lineno,lineBuf);
					bufsize = strlen(lineBuf);
					linepos = 0;
					return lineBuf[linepos++];
				}
				else
				{ 
					EOF_flag = true;
					return EOF;
				}
			}
			else return lineBuf[linepos++];
		}
		if (src_type == SRC_STRING)
		{
			if (linepos < (int)str_source.size())
			{
				lineno=1;
				return str_source.at(linepos++);
			}
			else
			{
				EOF_flag = true;
				return EOF;
			}
		}
		return EOF;
	}
	/* ungetNextChar backtracks one character
	   in lineBuf */
	void ungetNextChar(void)
	{ if (!EOF_flag) linepos-- ;}

	/* lookup an identifier to see if it is a reserved word */
	/* uses linear search */
	Token::TokenType reservedLookup (std::string& s)
	{ 
		return Token::LookUp(Scanner::reservedWords,s);
	}

public:
	explicit Scanner(FILE* _file) : lineno(0),linepos(0),bufsize(0),EOF_flag(false) 
	{
		if (reservedWords.size() == 0)
		{
			reservedWords.reserve(MAXRESERVED);
			reservedWords.push_back(Token("if",Token::IF));
			reservedWords.push_back(Token("then",Token::THEN));
			reservedWords.push_back(Token("else",Token::ELSE));
			reservedWords.push_back(Token("end",Token::END));
			reservedWords.push_back(Token("repeat",Token::REPEAT));
			reservedWords.push_back(Token("until",Token::UNTIL));
			reservedWords.push_back(Token("read",Token::READ));
			reservedWords.push_back(Token("write",Token::WRITE));
			reservedWords.push_back(Token("print",Token::PRINT));
		}
		source=_file; 
		src_type = SRC_FILE;
	}
	explicit Scanner(const std::string& src):lineno(0),linepos(0),bufsize(0),EOF_flag(false)
	{
		if (reservedWords.size() == 0)
		{
			reservedWords.reserve(MAXRESERVED);
			reservedWords.push_back(Token("if",Token::IF));
			reservedWords.push_back(Token("then",Token::THEN));
			reservedWords.push_back(Token("else",Token::ELSE));
			reservedWords.push_back(Token("end",Token::END));
			reservedWords.push_back(Token("repeat",Token::REPEAT));
			reservedWords.push_back(Token("until",Token::UNTIL));
			reservedWords.push_back(Token("read",Token::READ));
			reservedWords.push_back(Token("write",Token::WRITE));
			reservedWords.push_back(Token("print",Token::PRINT));
		}
		str_source=src;
		src_type = SRC_STRING;
	}
	void Reload(FILE* _file)
	{
		lineno=0; linepos=0; bufsize=0; EOF_flag=false;
		source=_file;
		src_type = SRC_FILE;
	}
	void Reload(const std::string& src)
	{
		lineno=0; linepos=0; bufsize=0; EOF_flag=false;
		str_source=src;
		src_type = SRC_STRING;
	}
	context_t& getContext(bool blGetName=true) const
	{
		static context_t con;
		con.lineno=lineno;
		con.linepos=linepos;
		if (blGetName)
		{	
			if (src_type == SRC_FILE)
				strncpy(con.linebuf,lineBuf,BUFLEN);
			else if (src_type == SRC_STRING)
				strncpy(con.linebuf,str_source.c_str(),std::min(BUFLEN,(int)str_source.size()));
		}
		return con;
	}
	/****************************************/
	/* the primary function of the scanner  */
	/****************************************/
	/* function getToken returns the 
	 * next token in source file
	 */
	std::string& getTokenName(void)
	{
		return tokenString; 
	}	
	Token::TokenType getToken(void)
	{ 
		/* index for storing into tokenString */
		//int tokenStringIndex = 0;
		tokenString.clear();
		/* holds current token to be returned */
		Token::TokenType currentToken=Token::ERROR;
		/* current state - always begins at START */
		StateType state = START;
		/* flag to indicate save to tokenString */
		int save;
		int c;
		while (state != DONE)
		{ 
			c = getNextChar();
			save = true;
			switch (state)
			{ 
				case START:
					if (isdigit(c))
						state = INNUM;
					else if (isalpha(c))
						state = INID;
					else if (c == '=')
						state = INASSIGN;
					else if (c == '>')
						state = INGT;
					else if (c=='<')
						state = INLT;
					else if (c=='!')
						state = INNE;
					else if ((c == ' ') || (c == '\t') || (c == '\n') || (c=='@'))
						save = false;
					/*else if (c == '{')
					  { 
					  save = false;
					  state = INCOMMENT;
					  }*/
					else
					{ 
						state = DONE;
						switch (c)
						{ 
							case EOF:
								save = false;
								currentToken = Token::ENDFILE;
								break;
							case '+':
								currentToken = Token::PLUS;
								break;
							case '-':
								currentToken = Token::MINUS;
								break;
							case '*':
								currentToken = Token::TIMES;
								break;
							case '/':
								save = false;
								state = RDYINCMNT;
								break;
							case '%':
								currentToken = Token::MOD;
								break;
							case '&':
								currentToken = Token::AND;
								break;	
							case '|':
								currentToken = Token::OR;
								break;	
							case '(':
								currentToken = Token::LPAREN;
								break;
							case ')':
								currentToken = Token::RPAREN;
								break;
							case '{':
								currentToken = Token::LBLOCK;
								break;
							case '}':
								currentToken = Token::RBLOCK;
								break;
							case '[':
								currentToken = Token::LSQRBRACKET;
								break;
							case ']':
								currentToken = Token::RSQRBRACKET;
								break;
							case ';':
								currentToken = Token::SEMI;
								break;
							case ':':
								currentToken = Token::COLON;
								break;
							case '?':
								currentToken = Token::SELECT;
								break;
							case '.':
								currentToken = Token::MEMSL;
								break;
							case ',':
								currentToken = Token::COMMA;
								break;
							default:
								currentToken = Token::ERROR;
								break;
						}
					}
					break;
				case RDYINCMNT:
					if (c == '*')
					{
						state = INCOMMENT;
						save = false;
					}
					else
					{
						c='/';
						state = DONE;	
						currentToken = Token::OVER;
						ungetNextChar();
					}
				case INCOMMENT:
					save = false;
					if (c == EOF)
					{ 	
						state = DONE;
						currentToken = Token::ENDFILE;
					}
					else if (c == '*')
					{
						state = RDYOUTCMNT;
					}
					break;
				case RDYOUTCMNT:
					save = false;
					if (c=='/')
						state = START;
					else if (c == EOF)
					{
						state = DONE;
						currentToken = Token::ENDFILE;
					}
					else
						state = INCOMMENT;
					break;
				case INASSIGN:
					state = DONE;
					if (c == '=')
						currentToken = Token::EQ;
					else
					{ /* backup in the input */
						ungetNextChar();
						save = false;
						currentToken = Token::ASSIGN;
					}
					break;
				case INNE:
					state = DONE;
					if (c=='=')
						currentToken = Token::NE;
					else
					{
						currentToken = Token::ERROR;
						save=false;
						ungetNextChar();
					}
					break;	
				case INLT:
					state = DONE;
					if (c == '=')
						currentToken = Token::LTE;
					else
					{
						ungetNextChar();
						save = false;
						currentToken = Token::LT;
					}	
					break;
				case INGT:
					state = DONE;
					if (c == '=')
						currentToken=Token::GTE;
					else
					{
						ungetNextChar();
						save = false;
						currentToken = Token::GT;
					}	
					break;
				case INNUM:
					if (!isdigit(c))
					{
						if (c=='.')
							state = INFLOAT;
						else
						{	
							/* backup in the input */
							ungetNextChar();
							save = false;
							state = DONE;
							currentToken = Token::NUM;
						}
					}
					break;
				case INFLOAT:
					if (!isdigit(c))
					{
						/* backup in the input */
						ungetNextChar();
						save = false;
						state = DONE;
						currentToken = Token::FLOAT;
					}
					break;
				case INID:
					if (!isalpha(c) && !isdigit(c))
					{ /* backup in the input */
						ungetNextChar();
						save = false;
						state = DONE;
						currentToken = Token::ID;
					}
					break;
				default: /* should never happen */
					fprintf(stdout,"Scanner Bug: state= %d\n",state);
					state = DONE;
					currentToken = Token::ERROR;
					break;
			}
			//if ((save) && (tokenStringIndex <= MAXTOKENLEN))
			if (save)
			{
				//tokenStringIndex++;
				tokenString += (char) c;
			}
			if (state == DONE)
			{ 
				//tokenString[tokenStringIndex] = '\0';
				if (currentToken == Token::ID)
					currentToken = reservedLookup(tokenString);
			}
		}
		if (TraceScan) {
			fprintf(stdout,"\t%d: ",lineno);
			//printToken(currentToken,tokenString);
			printf("%s\n",tokenString.c_str());
		}
		return currentToken;
	} /* end getToken */
};
};
#endif
