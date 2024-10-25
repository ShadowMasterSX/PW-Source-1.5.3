package expr;

import java.io.*;
import java.lang.*;
class SourceType
{
	public final static int FILE=1;
	public final static int STRING=2;
}
public class Scanner
{

	private int lineno;
	private int linepos;
	private String tokenName;
	private int current_token;
	private String source;
	private int state;
	private boolean end_flag;
	private int src_type;
	private BufferedReader buffer;

	private char getNextChar()
	{
		if (source == null) { end_flag=true; return 0; }
		if (linepos >= source.length())
		{
			if (src_type == SourceType.STRING)
			{
				end_flag=true; return 0; 
			}
			else if (src_type == SourceType.FILE)
			{
				try
				{
					do 
					{
						source = buffer.readLine();
						if (source == null) { end_flag=true; return 0; }
						lineno++;
					} while (source.length() == 0);
					linepos=0;
				}
				catch (IOException e) { e.printStackTrace();}
			}
		}
		//System.out.println("linepos="+linepos+" lineno="+lineno+" source.length="+source.length());
		char c=source.charAt(linepos);
		linepos ++;
		return c;
	}

	private void unGetNextChar()
	{
		if (!end_flag) linepos--;
	}
	public Scanner(String _source)
	{
		src_type = SourceType.STRING;
		source = new String(_source);
		tokenName=new String();
		lineno = 1;
		linepos = 0;
		end_flag = false;
	}
	public Scanner(BufferedReader file)
	{
		src_type = SourceType.FILE;
		buffer = file;
		try
		{
			source = buffer.readLine();
			tokenName=new String();
			lineno = 1;
			linepos = 0;
			end_flag = false;
		}
		catch (IOException e) { e.printStackTrace(); }
	}
	public int getLineno()
	{
		return lineno;
	}
	public int getLinepos()
	{
		return linepos;
	}
	public String getSourceCode()
	{
		return source;
	}	
	public String getTokenName()
	{
		return tokenName;
	}
	public int getToken()
	{
		state = State.START;
		char c;
		char tmp;
		boolean save;

		tokenName="";
		while (state != State.DONE)
		{
			c=getNextChar();
			save=true;	
			switch	(state)
			{
				case State.START:
					if (Character.isDigit(c))
					{
						state=State.INNUM;
					}
					else if (Character.isLetter(c))
					{
						state=State.INID;
					}
					else if (c=='=')
					{
						state=State.INASSIGN;
					}
					else if (c=='>')
					{
						state=State.INGT;
					}
					else if (c=='<')
					{
						state=State.INLT;
					}
					else if (c=='!')
					{
						state=State.INNE;
					}
					else if (c==' ' || c=='\t' || c=='\n' || c=='@')
					{
						save = false;
					}
					else if (c=='/')
					{
						state = State.RDYINCMNT;
						save =false;
					}
					else
					{
						state = State.DONE;
						switch (c)
						{
							case 0:
								current_token = Token.END;
								break;
							case '.':
								current_token = Token.MEMSL;	
								break;
							case ',':
								current_token = Token.COMMA;
								break;				
							case '+':
								current_token = Token.PLUS;
								break;
							case '-':
								current_token = Token.MINUS;
								break;
							case '*':
								current_token = Token.TIMES;
								break;
							case '&':
								current_token = Token.AND;
								break;
							case '|':
								current_token = Token.OR;
								break;
							case '!':
								current_token = Token.NOT;	
							case '(':
								current_token = Token.LPAREN;
								break;
							case ')':
								current_token = Token.RPAREN;
								break;
							case '[':
								current_token = Token.LSQRBRACKET;
								break;
							case ']':
								current_token = Token.RSQRBRACKET;
								break;
							case '{':
								current_token = Token.LBLOCK;
								break;
							case '}':
								current_token = Token.RBLOCK;
								break;
							case ';':
								current_token = Token.SEMI;
								break;
							case ':':
								current_token = Token.COLON;
								break;
							case '?':
								current_token = Token.SELECT;
								break;								
							case '%':
								current_token = Token.MOD;
								break;	
							default:
								current_token = Token.ERROR;
								break;	
						}
					}	
					break;
				case State.INNUM:
					if (!Character.isDigit(c))
					{
						if (c=='.')
						{
							state = State.INFLOAT;
						}
						else
						{
							state = State.DONE;
							current_token = Token.NUM;
							unGetNextChar();
							save = false;
						}
					}
					break;
				case State.INFLOAT:
					if (! Character.isDigit(c))
					{
						state = State.DONE;
						current_token = Token.FLOAT;
						unGetNextChar();
						save = false;
					}
					break;
				case State.INID:
					if(c=='(')
					{
						state = State.DONE;
						current_token = Token.FUNCTION;
						unGetNextChar();
						save = false;
					}
					else if (!Character.isDigit(c) && !Character.isLetter(c))
					{
						state = State.DONE;
						current_token = Token.ID;
						unGetNextChar();
						save = false;
					}
					break;
				case State.INASSIGN:
					if (c=='=')
					{
						state = State.DONE;
						current_token = Token.EQ;
					}
					else
					{
						state = State.DONE;
						current_token = Token.ASSIGN;
						unGetNextChar();
						save = false;
					}
					break;
				case State.INNE:
					state = State.DONE;
					if (c=='=')
					{
						current_token = Token.NE;
					}
					else
					{
						current_token = Token.ERROR;
						save = false;
						unGetNextChar();
					}
					break;	
				case State.INLT:
					if (c=='=')
					{
						state = State.DONE;
						current_token = Token.LTE;
					}
					else 
					{
						state = State.DONE;
						current_token = Token.LT;
						unGetNextChar();
						save = false;
					}
					break;
				case State.INGT:
					if (c=='=')
					{
						state = State.DONE;
						current_token = Token.GTE;
					}
					else 
					{
						state = State.DONE;
						current_token = Token.GT;
						unGetNextChar();
						save = false;
					}
					break;	
				case State.RDYINCMNT:
					if (c=='*')
					{
						save = false;
						state = State.INCOMMENT;
					}
					else
					{
						c='/';
						state = State.DONE;
						current_token = Token.OVER;
						unGetNextChar();
					}
					break;	
				case State.INCOMMENT:
					save = false;
					if (c == 0)
					{ 
						state = State.DONE;
						current_token = Token.END;
					}
					else if (c == '*') state = State.RDYOUTCMNT;
					break;	
				case State.RDYOUTCMNT:
					save = false;
					if (c == 0)
					{
						state = State.DONE;
						current_token = Token.END;
					}
					else if (c == '/')
					{
						state = State.START;
					}
					else
						state = State.INCOMMENT;		
					break;	
			}//end switch
			if (save)
			{
				tokenName=tokenName.concat(String.valueOf(c)); 
				//System.out.println("tokenName = "+tokenName); 
			}
		}//end while
		if (current_token == Token.ID)
		{
			current_token = Token.LookUp(tokenName);
		}
		return current_token;
	}
}
