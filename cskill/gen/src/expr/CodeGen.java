package expr;
import java.lang.*;
import java.io.*;
class BoolWrapper
{
	private boolean b;
	BoolWrapper() { b=false; }
	BoolWrapper(boolean _b) { b=_b; }
	public void setValue(boolean _b) { b=_b; }
	public boolean getValue() { return b; }
}
public class CodeGen
{
	private String code;
	private Scanner scanner;
	private String token_name;
	private int token;
	private SyntaxTree st;

	private int last_token = -1;
	private String addon;//additional code
	private boolean blGetNext;
	//private boolean need_right_bracket=false;

	
	private static String ogfilename;
	private static String start_point=new String();
	private static ObjectGraph og=new ObjectGraph();
	private static OGParser ogparser;
	/*
	 *@ constructors
   	 */	 
	public CodeGen()
	{
		code = new String();
	}
	public CodeGen(String _ogfn)
	{
		try
		{
			code = new String();
			ogfilename = new String(_ogfn);
			og = new ObjectGraph();
			ogparser=new OGParser(ogfilename);
			GenObjectGraph();
		}
		catch (FileNotFoundException e)
		{
			System.out.println("File "+ogfilename+" not found.");
		}

	}
	public CodeGen(InputStream _is)
	{
		code = new String();
		ogfilename=null;
		og = new ObjectGraph();
		ogparser=new OGParser(_is);
		GenObjectGraph();
	}
	/*
	 *@ Get Object Graph, Object Graph is read from file "arg[0]", if "-file" option is set
	 */
	private static void GenObjectGraph()
	{
		try
		{
			/*
			OGParser ogparser;
			if (ogfilename!=null) new OGParser(ogfilename);
			if (is!=null) new OGParser(is);
			*/
			ogparser.Parse(og);
			//og.Display();
			System.out.println("Read ObjectGraph successfully.");
		}
		catch (OGException e)
		{
			System.out.println("catch object graph exception in main.");
		}
	}	
	/*
	 *@ Generate Syntax tree
	 */
	private void GenSyntaxTree(String source) throws SyntaxErrorException
	{
		st=new SyntaxTree();
		try
		{
			ExprParser expr_parser;
			expr_parser=new ExprParser(source,ExprParser.STRING);
					
			expr_parser.Parse(st,og);
			//SyntaxTree.DisplayWholeTree(st.GetRoot(),0);
			//System.out.println("Generate syntax tree successfully.");
		}
		catch (SyntaxErrorException e)
		{
			throw e;
		}
		catch (FileNotFoundException e)
		{
		}
	}
	/*
	 *@ Inspect syntax errors by analyzer.
	 */
	private void AnalyzeSyntaxTree() throws AnalyzerException
	{
		try
		{
			Analyzer.Analyze(st,og,start_point);
			//System.out.println("Pass syntax analyzer. No syntax error found.");
		}
		catch (AnalyzerException e)
		{
			throw e;
		}
	}
		
	/*
	 *@Generate code
     */	 
	private String UcFirst(String str)
	{
		return str.substring(0,1).toUpperCase() + str.substring(1).toLowerCase();
	}
	/*
	* 不同的状态下，对于token的处理不同，比如对于自定义变量，不需要调用Get，Set方法，而对于OG中定义的对象则必须用Get或Set   
	* Context Status:
	* 1.Self-Define variable		//当前token是关于自定义变量的计算
	* 2.OG-Define variable			//当前token是关于OG中定义的变量的计算
	*/
	//定义行状态
	private static final int _CONTEXTSTATE_SD_VAR=0;
	private static final int _CONTEXTSTATE_OGD_VAR=1;
	
	private boolean isType(String IDName)
	{
		if (og.GetTypeID(IDName)==Constants.INVALID_IDENTIFIER)
			return false;
		return true;
	}
	private int judgeContext(String IDName)
	{
		if (st.symtab.Lookup(IDName)!=null) return _CONTEXTSTATE_SD_VAR;
		return _CONTEXTSTATE_OGD_VAR;
	}
	private String contextSDHandler(BoolWrapper need_right_bracket)	//self-defined variable context
	{
		String code=new String();
		switch (token)
		{
			case Token.LPAREN:
			{
				code=code.concat("(");
				code=code.concat(HandleClause());
				code=code.concat(")"); 
				last_token=Token.RPAREN;
				/*
				if (need_right_bracket.getValue())
				{
					code=code.concat(")");
					need_right_bracket.setValue(false);//Boolean.FALSE;
				}
				*/
				token = scanner.getToken();
				token_name=scanner.getTokenName();
				blGetNext=false;
				break;
			}	
			case Token.RPAREN:
				//do nothing
				break;
			case Token.LSQRBRACKET:
				code=code.concat("[");	
				code=code.concat(HandleClause());
				code=code.concat("]"); 
				last_token=Token.RSQRBRACKET;
				token = scanner.getToken();
				token_name=scanner.getTokenName();
				blGetNext=false;
				break;
			case Token.SEMI:
				if (need_right_bracket.getValue())
				{
					code=code.concat(")");
					need_right_bracket.setValue(false);//Boolean.FALSE;
				}
				code = code.concat(token_name);
				break;
			case Token.STATIC:
			case Token.EXTERN:
			case Token.CONST:
				code = code.concat(token_name + " ");
				break;	
			default:		
				//all token resembles source code 
				code = code.concat(token_name);	
		}
		//System.out.println("SDHandler: "+code);
		return code;
	}
	private String contextOGHandler(BoolWrapper need_right_bracket) //OG-defined variable context
	{
		String code=new String();
		switch (token)
		{
			case Token.LPAREN:
			{
				code=code.concat("(");
				code=code.concat(HandleClause());
				code=code.concat(")"); 
				last_token=Token.RPAREN;
				/*
				if (need_right_bracket.getValue())
				{
					code=code.concat(")");
					need_right_bracket.setValue(false);//Boolean.FALSE;
				}
				*/
				token = scanner.getToken();
				token_name=scanner.getTokenName();
				blGetNext=false;
				break;
			}	
			case Token.RPAREN:
				//do nothing
				break;
			case Token.ID:
			case Token.FUNCTION:
				if (isType(token_name.toLowerCase())) 
				{
					code=code.concat(token_name);
					if (token == Token.ID)
						code = code.concat(" ");
					break;
				}
				if (last_token!=Token.MEMSL)
					code=code.concat(addon);
				String id_name=UcFirst(new String(token_name));
				//get next token
				token = scanner.getToken();
				token_name=scanner.getTokenName();
				//analyze
				if (token == Token.ASSIGN)
				{
					code = code.concat("Set"+id_name+"(");
					need_right_bracket.setValue(true);//Boolean.TRUE;
					blGetNext=true;
					last_token=token;
				}
				else if (token == Token.LSQRBRACKET)
				{
					code = code.concat("Get"+id_name+"()->");
					//get index
					String index=HandleClause();
					//get next token,the token next to ']'
					token=scanner.getToken();
					token_name=scanner.getTokenName();
					if (token==Token.ASSIGN)
					{
						code=code.concat("SetValue("+index+",");
						need_right_bracket.setValue(true);//Boolean.TRUE;
						blGetNext=true;
						last_token=token;
					}
					else
					{
						code=code.concat("GetValue("+index+")");
						blGetNext=false;
						last_token=Token.RSQRBRACKET;
					}
				}
				//----process parameterized function call
				else if (token == Token.LPAREN)
				{
					last_token=Token.LPAREN;
					code=code.concat("Get"+id_name+"(");
					code=code.concat(HandleClause());
					code=code.concat(")"); 
					last_token=Token.RPAREN;
					token = scanner.getToken();
					token_name=scanner.getTokenName();
					blGetNext=false;
					break;
				}
				else
				{
					code = code.concat("Get"+id_name+"()");
					blGetNext=false;
					last_token=token;
				}
				break;
			case Token.ASSIGN:
			case Token.LSQRBRACKET:
				//do nothing
				break;	
			case Token.RSQRBRACKET:
				//do nothing
				break;
			case Token.MEMSL:
				code = code.concat("->");
				break;
			case Token.SEMI:
				if (need_right_bracket.getValue())
				{
					code=code.concat(")");
					need_right_bracket.setValue(false);//Boolean.FALSE;
				}
				code = code.concat(token_name);
				//code = code.concat("\r\n");
				break;	
			case Token.AND:
				code = code.concat(" && ");
				break;
			case Token.OR:
				code = code.concat(" || ");
				break;  	
			case Token.STATIC:
			case Token.EXTERN:
			case Token.CONST:
				code = code.concat(token_name + " ");
				break;	
			default:
				code = code.concat(token_name);
				break;	
		}
		//System.out.println("OGHandler: "+code);
		return code;
	}
	private String HandleClause() //clause根据[],()符号划分，[...]或者(...)为一个新的子句
	{
		int context_state;
		String code=new String();
		BoolWrapper need_right_bracket=new BoolWrapper(false);//Boolean.FALSE;
		token = scanner.getToken();
		token_name=scanner.getTokenName();
		
		context_state=judgeContext(token_name);
		while (token != Token.END && token != Token.RSQRBRACKET && token != Token.RPAREN)
		{
			blGetNext = true;
			if (context_state == _CONTEXTSTATE_SD_VAR)
				code=code.concat(contextSDHandler(need_right_bracket));
			else if (context_state == _CONTEXTSTATE_OGD_VAR)
				code=code.concat(contextOGHandler(need_right_bracket));
			if (blGetNext)
			{
				last_token=token;
				token = scanner.getToken();
				token_name=scanner.getTokenName();
			}
			//if there is non-member-select token before ID token, do context judgement
			if (token==Token.ID && last_token!=Token.MEMSL)
				context_state=judgeContext(token_name);
		}
		if (need_right_bracket.getValue())
		{
			code=code.concat(")");
			need_right_bracket.setValue(false);//Boolean.FALSE;
		}

		//System.out.println("HandleClause: "+code);
		return code;
	}
	private String GenerateCode(BufferedReader source)
	{
		scanner = new Scanner(source);
		//inital variables
		//need_right_bracket=false;
		last_token = -1;
		return HandleClause();
	}
	private String GenerateCode(String source)
	{

		scanner = new Scanner(source);
		//inital variables
		//need_right_bracket=false;
		last_token = -1;
		return HandleClause();
	}
	/*
	 *@ The Interface to transform a string to code
	 */	 
	public void SyntaxCheck(String source,String env_class) throws SyntaxErrorException,AnalyzerException
	{
		try
		{
			if (env_class == null) env_class=new String("Skill");
			start_point=new String(env_class);
			GenSyntaxTree(source);
			AnalyzeSyntaxTree();
		}
		catch (SyntaxErrorException e)
		{
			throw e;
		}
		catch (AnalyzerException e)
		{
			throw e;
		}
	}	

	public String GenExpr(String source,String env_class,String _addon) throws SyntaxErrorException,AnalyzerException
	{
		try
		{
			start_point=new String(env_class);
			GenSyntaxTree(source);
			AnalyzeSyntaxTree();
			code=""; addon=new String(_addon);
			return GenerateCode(source);
		}
		catch (SyntaxErrorException e)
		{
			throw e;
		}
		catch (AnalyzerException e)
		{
			throw e;
		}
	}

	/*
	 * test codes
	 */	 
	public static void main(String args[])
	{
		if (args.length != 3)
			System.out.println("usage: CodeGen <*.og> <env_class> <source file>");
		try
		{
			CodeGen codegen=new CodeGen(new FileInputStream(new File(args[0])));

			/*BufferedReader reader = new BufferedReader(new FileReader(args[2]));
			String expr = reader.readLine();
			while (expr!=null)
			{
				if (!expr.equals(""))
				{
					System.out.print(expr+":\t");
					System.out.println(codegen.GenExpr(expr,args[1],"skill->"));
				}
				expr = reader.readLine();
			}*/
			System.out.println(codegen.GenExpr(args[2],args[1], "skill->"));
		}
		catch (FileNotFoundException e)
		{
			System.out.println("File "+args[0]+" not found.");
		}
		catch (SyntaxErrorException e)
		{
			System.out.println("catch Syntax error exception.");
		}
		catch (AnalyzerException e)
		{
			System.out.println("catch Analyzer exception.");
		}
		catch (IOException e)
		{
			System.out.println("read expr file exception.");
		}
		return;

	}
}

