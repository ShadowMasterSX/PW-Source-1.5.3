package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppEvaluate implements CppWriter, CppComponent
{
	protected String variable;
	protected String value;

	public CppEvaluate( String variable, String value, String defaultvalue, boolean isstring )
	{
		this.variable = variable;
		this.value = value;
		if( defaultvalue == null )
			defaultvalue = "";

		if( value == null )
			this.value = defaultvalue;
		if( this.value.length() == 0 && !isstring )
			this.value = defaultvalue;

		if( isstring )
			this.value = "\"" + this.value + "\"";
	}

	public CppComponent add(CppComponent component) { return this; }
	public void write(PrintStream ps)
	{
		ps.println ( variable + " = " + value + ";" );
	}
}

