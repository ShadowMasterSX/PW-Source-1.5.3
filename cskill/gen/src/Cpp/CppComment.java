package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppComment implements CppWriter, CppComponent
{
	protected String comment;

	public CppComment( String comment)
	{
		this.comment = comment;
	}

	public CppComponent add(CppComponent component) { return this; }

	public void write(PrintStream ps)
	{
		ps.println ( comment );
	}
}

