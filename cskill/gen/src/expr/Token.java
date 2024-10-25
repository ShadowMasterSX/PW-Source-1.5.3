package expr;
import java.lang.*;
import java.util.*;
public final class Token
{
	//Exception Token
	public final static int END=0;
	public final static int ERROR=1;
	//multicharacter tokens
	public final static int ID=2;
	public final static int NUM=3;
	public final static int FLOAT=4;
	// special symbols 
	public final static int ASSIGN=5;
	public final static int EQ=6;
	public final static int LT=7;
	public final static int LTE=8;
	public final static int GT=9;
	public final static int GTE=10;
	public final static int PLUS=11;
	public final static int MINUS=12;
	public final static int TIMES=13;
	public final static int OVER=14;
	public final static int LPAREN=15;
	public final static int RPAREN=16;
	public final static int SEMI=17;
	
	public final static int SELECT=18/*'?'*/;
	public final static int COLON=19/*':'*/;
	public final static int MEMSL=20/*'.' select member*/;
	public final static int LSQRBRACKET=21/*'[',Left square bracket*/;
	public final static int RSQRBRACKET=22/*']',right square bracket*/;
	public final static int COMMA=23/*',',comma*/;

	public final static int NEWLINE=24; /* '\r\n' */
	// logic expression
	public final static int AND	= 25;	/* '&' */
	public final static int OR	= 26;	/* '|' */
	public final static int NOT	= 27;	/* '!' */
	
	//addition
	public final static int MOD	= 28;	/* '%' */
	public final static int NE	= 29;	/* '!=' */
	public final static int ARRAY = 30;	/* '' */
	public final static int LBLOCK= 31;	/* '{' */
	public final static int RBLOCK= 32;	/* '}' */
	public final static int ANNOUNCE= 33;	/*announce var*/
	
	// statement symbols
	public final static int IF=34;
	public final static int THEN=35;
	public final static int ELSE=36;
	public final static int REPEAT=37;
	public final static int UNTIL=38;
	public final static int READ=39;
	public final static int WRITE=40;
	//temp defined symbols
	public final static int OPERATOR=50;
	public final static int FUNCTION=51;

	// define decorate tokens,such as 'static' , 'const' , 'extern'
	public final static int STATIC=101;
	public final static int CONST=102;
	public final static int EXTERN=103;
	
	private static final Map reserved_keywords = new HashMap();
	static {
		reserved_keywords.put("static",new Integer(STATIC));
		reserved_keywords.put("const",new Integer(CONST));
		reserved_keywords.put("extern",new Integer(EXTERN));
	}
	public static int LookUp(String tokenString)
	{
		Integer token =(Integer) reserved_keywords.get(tokenString);
		if (token == null)  return ID;
		return token.intValue();
	}
	public static void DisplayToken(int token, String tokenString)
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
				System.out.println(tokenString);
				break;
			case READ:	
				System.out.println("read "+tokenString);
				break;
			case ASSIGN: 
				//System.out.println("Assigned to:"+tokenString);
				System.out.println("Op: =");
				break;
			case ARRAY:
				System.out.println("Array:");
				break;	
			case LT: 
				System.out.println("Op: <");
				break;
			case GT: 
				System.out.println("Op: >");
				break;
			case LTE: 
				System.out.println("Op: <=");
				break;
			case GTE: 
				System.out.println("Op: >=");
				break;
			case EQ: 
				System.out.println("Op: ==");
				break;
			case NE: 
				System.out.println("Op: !=");
				break;
			case AND:
				System.out.println("Op: &");
				break;
			case OR:
				System.out.println("Op: |");	
				break;
			case NOT:
				System.out.println("Op: !");	
				break;
			case MOD:
				System.out.println("Op: %");	
				break;
			case LPAREN: 
				System.out.println("(");
				break;
			case RPAREN: 
				System.out.println(")");
				break;
			case SEMI:
				System.out.println(";");
				break;
			case PLUS: 
				System.out.println("Op: +");
				break;
			case MINUS: 
				System.out.println("Op: -");
				break;
			case TIMES: 
				System.out.println("Op: *");
				break;
			case OVER: 
				System.out.println("Op: /");
				break;
			case SELECT:
				System.out.println("SELECT ?");
				break;
			case COLON: 
				System.out.println(":");
				break;
			case MEMSL: 
				System.out.println("Op: .");
				break;
			case LSQRBRACKET:
				System.out.println("Op: [*]");
				break;
			case RSQRBRACKET:
				System.out.println("index *]");
				break;
			case FLOAT:
				System.out.println("FLOAT: "+tokenString);
				break;
			case NUM:
				System.out.println("NUM: "+tokenString);
				break;
			case ID:
				System.out.println("ID: "+tokenString);
				break;
			case FUNCTION:
				System.out.println("FUNCTION: "+tokenString);
				break;
			case ANNOUNCE:
				System.out.println("Announce: "+tokenString);
				break;
			case ERROR:
				System.out.println("ERROR: "+tokenString);
				break;
			default: /* should never happen */
				System.out.println("Unkown token"+token);
		}

	}
}

