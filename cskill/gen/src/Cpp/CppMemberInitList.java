package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppMemberInitList implements CppWriter, CppComponent
{
	protected ArrayList list = new ArrayList();

	public CppComponent add(CppComponent component)
	{
		if ( component instanceof CppMemberInit )
			list.add(component);
		return this;
	}

	public void write(PrintStream ps)
	{
		boolean first = true;
		for (Iterator it = list.iterator(); it.hasNext(); )
		{
			if ( first )
				first = false;
			else
				ps.print(", ");
			((CppMemberInit)it.next()).write(ps);
		}
	}
}

