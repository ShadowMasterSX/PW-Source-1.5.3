package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppParameterList implements CppWriter, CppComponent
{
	protected ArrayList list = new ArrayList();

	public CppComponent add(CppComponent component)
	{
		if ( component instanceof CppParameter )
		{
			list.add(component);
		}
		return this;
	}

	public void write(PrintStream ps)
	{
		ps.print ( "(" );
		boolean first = true;
		for (Iterator it = list.iterator(); it.hasNext(); )
		{
			if ( first )
				first = false;
			else
				ps.print(", ");
			((CppParameter)it.next()).write(ps);
		}
		ps.print ( ")" );
	}
}

