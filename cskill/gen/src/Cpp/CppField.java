package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppField implements CppWriter, CppComponent
{
	protected boolean accessor;
	protected String type;
	protected String variable;

	private static String UcFirst(String s)
	{
		return s.substring(0, 1).toUpperCase() + s.substring(1).toLowerCase();
	}

	public CppField( String type, String variable, boolean accessor )
	{
		this.type = type;
		this.variable = variable;
		this.accessor = accessor;
	}

	public CppField( String type, String variable )
	{
		this.type = type;
		this.variable = variable;
		this.accessor = true;
	}

	public CppComponent add(CppComponent component) { return this; }
	public void write(PrintStream ps)
	{
		ps.println ( type + "\t" + variable + ";" );
		if ( accessor )
		{
			ps.println (type + " Get" + UcFirst(variable) + "() { return " + variable + "; }" );
			ps.println ("void Set" + UcFirst(variable) + "(" + type + " _" + variable + ") { " + variable + " = _" + variable + "; }" );
		}
	}
}

