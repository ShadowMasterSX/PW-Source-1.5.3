package expr;
import java.io.*;
import java.lang.*;

public class OGParser
{
	private Scanner scanner;
	private BufferedReader buf;
	private int token;

	public OGParser(String fn) throws FileNotFoundException
	{
		buf=new BufferedReader(new FileReader(fn));
		scanner = new Scanner(buf);
	}
	public OGParser(InputStream is)
	{
		buf=new BufferedReader(new InputStreamReader(is));
		scanner = new Scanner(buf);
	}
	private void match(int expected_token) throws OGException
	{
		if (token==expected_token)
		{
			token = scanner.getToken();
		}
		else
		{
			if (expected_token == Token.LT || expected_token == Token.GT)
			{
				throw new OGException("^Brackets unmatched.\n");
			}
			else if (expected_token==Token.ASSIGN) 
			{
				throw new OGException("^Miss assignment.\n");
			}
			else if (expected_token==Token.SEMI)
			{
				throw new OGException("^Miss semicolon.\n");
			}
			else if (expected_token==Token.COMMA) 
			{
				throw new OGException("^Miss comma.\n");
			}
			else if (token == Token.ERROR)
			{
				throw new OGException("^Invalid token.\n");
			}
			else
			{
				throw new OGException("^Unexpected token.\n");			
			}
		}
	}
	private void newStmtSequence(ObjectGraph og) throws OGException
	{
		while (token != Token.END)
		{
			if (token == Token.ID) newStatement(og);
			match(Token.SEMI);
		}
	}
	private void newStatement(ObjectGraph og) throws OGException
	{
		og.AddTypeNode(new String(scanner.getTokenName()));
		match(Token.ID);
		match(Token.ASSIGN);
		newMemberNode(og);
		while (token != Token.SEMI)
		{
			match(Token.COMMA);
			newMemberNode(og);
		}
	}	
	private void newMemberNode(ObjectGraph og) throws OGException
	{

		String member_name;
		match(Token.LT);
		if (token == Token.GT)  //null member node
		{
			match(Token.GT);
			return;
		}
		member_name=new String(scanner.getTokenName());
		match(Token.ID);
		match(Token.COMMA);
		og.AddMemberNode(new String(scanner.getTokenName()),member_name);
		match(Token.ID);
		match(Token.GT);
	}
	public void Parse(ObjectGraph og) throws OGException
	{
		token = scanner.getToken();
		try
		{
			newStmtSequence(og);
		}
		catch (OGException e)
		{
			System.out.println("Error: line "+scanner.getLineno());
			System.out.println("\t"+scanner.getSourceCode());
			int i=scanner.getLinepos();
			System.out.print("\t");
			while (i!=0) { System.out.print(" ");i--; }
			System.out.println(e.getMessage());
			throw e;
		}
	}
	public static void main(String args[])
	{
		try
		{
			OGParser ogparser=new OGParser(args[0]);
			ObjectGraph og=new ObjectGraph();
			ogparser.Parse(og);
			og.Display();
		}
		catch (FileNotFoundException e)
		{
			System.out.println("File "+args[0]+" not found.");
		}
		catch (Exception e)
		{
			System.out.println("catch exception in main.");
			e.printStackTrace();
		}	
	}

}
