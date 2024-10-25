package expr;
import java.io.*;
import java.lang.*;


public class ExprParser
{
	private Scanner scanner;
	private BufferedReader buf;
	private int token;
	private ObjectGraph og;

	public static final int FILE=1;	
	public static final int STRING=2;	
	public ExprParser(String str,int src_type) throws FileNotFoundException
	{
		if (src_type == FILE)
		{
			buf=new BufferedReader(new FileReader(str));
			scanner = new Scanner(buf);
		}
		else if (src_type == STRING)
		{
			scanner = new Scanner(new String(str));	
		}
	}
	private void match(int expected_token) throws SyntaxErrorException
	{
		if (token == expected_token)
		{
			token = scanner.getToken();
			while (token == Token.STATIC || token == Token.EXTERN || token == Token.CONST) token=scanner.getToken();
		}
		else if (expected_token==Token.RPAREN || expected_token==Token.LPAREN)
		{
			throw new SyntaxErrorException("^Brackets unmatched.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());	
		}
		else if (expected_token==Token.ASSIGN)
		{
			throw new SyntaxErrorException("^miss `='",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
		else if (expected_token==Token.SEMI)
		{
			throw new SyntaxErrorException("^miss `;'",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
		else if (expected_token==Token.COMMA)
		{
			throw new SyntaxErrorException("^miss `,'",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
		else if (expected_token==Token.COLON)
		{
			throw new SyntaxErrorException("^miss `:'",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
		else if (expected_token==Token.SELECT)
		{
			throw new SyntaxErrorException("^miss `?'",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
		else if (token == Token.ERROR)
		{
			throw new SyntaxErrorException("^Invalid symbol.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
		else
		{
			throw new SyntaxErrorException("^Unexpected symbol.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
	}	
	private void throwOutOfMemory() throws SyntaxErrorException
	{
		throw new SyntaxErrorException("Out of memory.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
	}
	private TreeNode stmt_sequence() throws SyntaxErrorException
	{
		TreeNode t = statement();
		TreeNode p = t;
		if (null == t) throwOutOfMemory();
		while ((token!=Token.END) && (token!=Token.ELSE) && (token!=Token.UNTIL))
		{
			TreeNode q=statement();
			if (null == q) throwOutOfMemory();
			p.sibling = q;
			p = q;
		}
		return t;
	}
	private TreeNode statement() throws SyntaxErrorException
	{
		TreeNode t=null;
		switch (token) {
			case Token.IF:
				t = if_stmt();
				break;
			case Token.REPEAT:
				t = repeat_stmt();
				break;
				/*	
					case Token.ID:
					t = assign_stmt();
					match(Token.SEMI);
					break;
				 */	
			case Token.READ:
				t = read_stmt();
				match(Token.SEMI);
				break;
			case Token.WRITE:
				t = write_stmt();
				match(Token.SEMI);
				break;
			case Token.ID:	
				{
					int type=og.GetTypeID(scanner.getTokenName());
					if (type!=Constants.INVALID_IDENTIFIER)
					{
						t=announce_stmt(type);  //variable announce statement
						match(Token.SEMI);
					}
					else
					{
						t=exp();    
						match(Token.SEMI);
					}
				}			
				break;
			case Token.NUM:
			case Token.FLOAT:
			case Token.LPAREN:	
			case Token.MINUS:	
				t = exp();
				match(Token.SEMI);
				break;	
			case Token.FUNCTION:
				//t = function();
				t = exp();
				match(Token.SEMI);
				break;	
			default :
				throw new SyntaxErrorException("^Unexpected symbol.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		} /* end case */
		return t;
	}
	private TreeNode function() throws SyntaxErrorException
	{
		TreeNode  t = SyntaxTree.CreateExprNode(TreeNode.FunctionK,scanner.getLineno());
		if (null == t) throwOutOfMemory();
		t.type = Token.FUNCTION;
		match(Token.FUNCTION);
		match(Token.LPAREN);
		t.child.add(exp());
		match(Token.RPAREN);
		return t;
	}	
	private TreeNode if_stmt() throws SyntaxErrorException
	{
		TreeNode  t = SyntaxTree.CreateStmtNode(TreeNode.IfK,scanner.getLineno());
		if (null == t) throwOutOfMemory();
		match(Token.IF);
		t.child.add(exp());
		match(Token.THEN);
		t.child.add(stmt_sequence());
		if (token==Token.ELSE)
		{
			match(Token.ELSE);
			t.child.add(stmt_sequence());
		}
		match(Token.END);
		return t;
	}	
	private TreeNode repeat_stmt() throws SyntaxErrorException
	{
		TreeNode t = SyntaxTree.CreateStmtNode(TreeNode.RepeatK,scanner.getLineno());
		if (null == t) throwOutOfMemory();
		match(Token.REPEAT);
		t.child.add(stmt_sequence());
		match(Token.UNTIL);
		t.child.add(exp());
		return t;
	}

	private TreeNode read_stmt() throws SyntaxErrorException
	{   
		TreeNode t = SyntaxTree.CreateStmtNode(TreeNode.ReadK,scanner.getLineno());
		if (null == t) throwOutOfMemory();
		match(Token.READ);
		if (token==Token.ID)
		{
			t.name = scanner.getTokenName();
			match(Token.ID);
		}
		return t;
	}  
	private TreeNode write_stmt() throws SyntaxErrorException
	{   
		TreeNode t = SyntaxTree.CreateStmtNode(TreeNode.WriteK,scanner.getLineno());
		if (null == t) throwOutOfMemory();
		match(Token.WRITE);
		t.child.add(exp());
		return t;
	}	
	private TreeNode announce_stmt(int type) throws SyntaxErrorException
	{           
		TreeNode t = SyntaxTree.CreateStmtNode(TreeNode.AnnounceK,scanner.getLineno());
		t.type = type;
		match(Token.ID);
		if (token == Token.ID) //variable name
		{           
			t.name = scanner.getTokenName();
			t.child.add(exp()); //add child[0]
			return t;
		}           
		else        
			throw new SyntaxErrorException("^Invalid announcement.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
	}           	
	private TreeNode exp() throws SyntaxErrorException
	{
		TreeNode t=assign_exp();
		return t;
	}	
	private TreeNode assign_exp() throws SyntaxErrorException
	{
		TreeNode t=select_exp();
		if (token == Token.ASSIGN)
		{
			//Assign Treenode's child[0] must be ID
			if (t.nodekind!=TreeNode.ExpK) 
				throw new SyntaxErrorException("^Invalid assign statement.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
			else if (t.kind!=TreeNode.IdK && (t.kind!=TreeNode.OpK || t.op!=Token.LSQRBRACKET))	
				throw new SyntaxErrorException("^Invalid assign statement.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());

			TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
			if (null == p) throwOutOfMemory(); 
			p.child.add(t); //child[0]
			p.name = t.name;
			p.op = token;
			match(Token.ASSIGN);
			p.child.add(exp()); //child[1]
			t=p;
		}
		return t;
	}
	private TreeNode select_exp() throws SyntaxErrorException
	{
		TreeNode t=logic_relation_exp();
		//Support "exp ? exp:exp"
		if (token == Token.SELECT)
		{
			TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
			if (null == p) throwOutOfMemory();
			p.child.add(t); //add child[0]
			p.op = token;
			t = p;
			match(Token.SELECT); //get next token
			//t.child.add(logic_exp());
			t.child.add(logic_relation_exp());
			match(Token.COLON);
			//t.child.add(logic_exp());
			t.child.add(logic_relation_exp());
		}
		return t;
	}
	private TreeNode logic_relation_exp() throws SyntaxErrorException // '&' '|'
	{
		TreeNode t= logic_exp();
		if (token == Token.AND || token == Token.OR)
		{
			TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
			if (null == p) throwOutOfMemory();
			p.child.add(t);  //add child[0]
			p.op=token;
			t=p;
			match(token);
			t.child.add(logic_relation_exp()); //add child[1]
		}
		return t;
	}
	private TreeNode logic_exp() throws SyntaxErrorException
	{
		TreeNode t = addminus_exp();
		if ((token==Token.LT)||(token==Token.EQ)||(token==Token.GT)||(token==Token.LTE)||(token==Token.GTE)||(token==Token.NE)) {
			TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
			if (null == p) throwOutOfMemory();
			p.child.add(t); //add child[0]
			p.op = token;
			t = p;
			match(token);
			t.child.add(addminus_exp()); //ad child[1]
		}
		return t;
	}	
	private TreeNode addminus_exp() throws SyntaxErrorException
	{
		TreeNode t = multi_div_exp();
		while ((token==Token.PLUS)||(token==Token.MINUS))
		{
			TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
			if (null == p) throwOutOfMemory();
			p.child.add(t);
			p.op = token;
			t = p;
			match(token);
			t.child.add(multi_div_exp());
		}
		return t;
	}	
	private TreeNode multi_div_exp() throws SyntaxErrorException
	{
		TreeNode t = factor();
		while ((token==Token.TIMES)||(token==Token.OVER)||(token==Token.MOD))
		{
			TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
			if (null == p) throwOutOfMemory();
			p.child.add(t); //add child[0]
			p.op = token;
			t = p;
			match(token);
			p.child.add(factor());//add child[2]
		}
		return t;
	}  	
	private TreeNode factor() throws SyntaxErrorException
	{
		TreeNode t = null;
		switch (token) {
			case Token.NUM :
				t = SyntaxTree.CreateExprNode(TreeNode.ConstIntK,scanner.getLineno());
				if (null == t) throwOutOfMemory();
				t.val = Integer.valueOf(scanner.getTokenName()).intValue();
				match(Token.NUM);
				break;
			case Token.FLOAT :
				t = SyntaxTree.CreateExprNode(TreeNode.ConstFloatK,scanner.getLineno());
				if (null == t) throwOutOfMemory();
				t.fval = Float.valueOf(scanner.getTokenName()).floatValue();
				match(Token.FLOAT);
				break;
			case Token.FUNCTION :
				t = function();
				break;
			case Token.ID :
				t = SyntaxTree.CreateExprNode(TreeNode.IdK,scanner.getLineno());
				if (null == t) throwOutOfMemory();
				if (token==Token.ID)
					t.name = new String(scanner.getTokenName());
				match(Token.ID);
				if (token == Token.MEMSL)
				{
					TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
					if (null == p) throwOutOfMemory();
					p.child.add(t); //add child[0]
					p.op = token;
					t=p;
					match(Token.MEMSL);

					//t.child[1]=factor();
					TreeNode t1=factor();
					TreeNode tmp=t1;
					while (tmp.child.size() !=0 ) tmp=(TreeNode)tmp.child.get(0);
					tmp.child.add(t); //put father object to the most left child,add to child[0]

					t=t1;
				}
				if (token == Token.LSQRBRACKET)
				{
					TreeNode p = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
					if (null == p) throwOutOfMemory(); 
					p.child.add(t);		//add child[0]
					p.op = token;
					t=p;
					match(Token.LSQRBRACKET);
					t.child.add(exp()); //add child[1]
					match(Token.RSQRBRACKET);

					if (token == Token.MEMSL)
					{
						TreeNode p1 = SyntaxTree.CreateExprNode(TreeNode.OpK,scanner.getLineno());
						if (null == p1) throwOutOfMemory(); 
						p1.child.add(t);
						p1.op = token;
						t=p1;
						match(Token.MEMSL);
						//t.child[1]=factor();
						TreeNode t1=factor();
						TreeNode tmp=t1;
						while (tmp.child.size() != 0) tmp=(TreeNode)tmp.child.get(0);
						tmp.child.add(t); //put father object to the most left child,add child[0]

						t=t1;
					}
				}
				break;
			case Token.LPAREN :
				match(Token.LPAREN);
				t = exp();
				match(Token.RPAREN);
				break;
			case Token.MINUS:
				t = SyntaxTree.CreateExprNode(TreeNode.ConstIntK,scanner.getLineno());
				if (null == t) throwOutOfMemory();
				break;	
			case Token.LBLOCK : //self-defined array
				{
					boolean negative=false;
					match(Token.LBLOCK);
					t = SyntaxTree.CreateExprNode(TreeNode.ConstArrayK,scanner.getLineno());
					if (null == t) throwOutOfMemory();	
					t.struct_type=TreeNode.ArrayK;
					TreeNode tail=t;
					while (token!=Token.RBLOCK)
					{
						if (token==Token.MINUS)
						{
							match(Token.MINUS);
							negative=true;
						}
						if (token==Token.NUM)
						{
							TreeNode p = SyntaxTree.CreateExprNode(TreeNode.ConstIntK,scanner.getLineno());
							if (null == p) throwOutOfMemory();
							tail.child.add(p);  //add child[0]
							tail=p;
							match(Token.NUM);
						}
						else
						{
							TreeNode p = SyntaxTree.CreateExprNode(TreeNode.ConstFloatK,scanner.getLineno());
							if (null== p) throwOutOfMemory(); 
							tail.child.add(p); //add child[0]
							tail=p;
							match(Token.FLOAT);
						}
						if (token==Token.COMMA) match(Token.COMMA);
						negative=false;
					}
					match(Token.RBLOCK);
				}			
				break;
			default:
				throw new SyntaxErrorException("^Unexpected symbol.",scanner.getLineno(),scanner.getLinepos(),scanner.getSourceCode());
		}
		return t;
	}

	public void Parse(SyntaxTree _st,ObjectGraph _og) throws SyntaxErrorException
	{
		token = scanner.getToken();
		while (token == Token.STATIC || token == Token.EXTERN || token == Token.CONST)
		{
			token=scanner.getToken();
		}
		og=_og;
		try
		{
			_st.SetRoot(stmt_sequence());
		}
		catch (SyntaxErrorException e)
		{
			System.out.println("Error: line "+e.getLineno());
			System.out.println("\t"+e.getSourceCode());
			int i=e.getLinepos();
			System.out.print("\t");
			while (i!=0) { System.out.print(" ");i--; }
			System.out.println(e.getMessage());
			e.printStackTrace();
			throw e;
		}
	}	
	/*
	   public static void main(String args[])
	   {
	   try
	   {
	   ExprParser expr_parser=new ExprParser(args[0],ExprParser.FILE);
	   SyntaxTree st=new SyntaxTree();
	   expr_parser.Parse(st,null);
	   SyntaxTree.DisplayWholeTree(st.GetRoot(),0);
	   }
	   catch (FileNotFoundException e)
	   {
	   System.out.println("File "+args[0]+"not found.");
	   }
	   catch (Exception e)
	   {
	   System.out.println("Catch exception in main.");
	   e.printStackTrace();
	   }
	   }
	 */
}
