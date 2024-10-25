package expr;
import java.lang.*;

public class AnalyzerException extends Exception
{
	private	int 	lineno;
	public AnalyzerException()
	{
	}
	public AnalyzerException(String msg,int _lineno)
	{
		super(msg);
		lineno=_lineno;
	}
	public int getLineno() { return lineno; }
}

