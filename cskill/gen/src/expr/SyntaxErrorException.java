package expr;
import java.lang.*;
public class SyntaxErrorException extends Exception
{
	private int lineno;
	private int linepos;
	private String source;
	public SyntaxErrorException()
	{
	}
	public SyntaxErrorException(String msg,int _lineno,int _linepos,String _src)
	{
		super(msg);
		lineno=_lineno;
		linepos=_linepos;
		source=_src;
	}
	public int getLineno() { return lineno; }
	public int getLinepos() { return linepos; }
	public String getSourceCode() { return source; }
}

