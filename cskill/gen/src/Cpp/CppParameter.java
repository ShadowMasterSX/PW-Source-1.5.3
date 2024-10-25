package Cpp;

import java.io.PrintStream;

public class CppParameter implements CppWriter, CppComponent
{
	protected String type;
	protected String variable;

	public CppParameter ( String type, String variable )
	{
		this.type = type;
		this.variable = variable;
	}

	public CppComponent add(CppComponent component) { return this; }

	public void write(PrintStream ps)
	{
		ps.print ( type + " " + variable );
	}
}

