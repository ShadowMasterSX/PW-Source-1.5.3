package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppBlock implements CppWriter, CppComponent
{
	protected ArrayList list = new ArrayList();

	public CppComponent add(CppComponent component)
	{
		if ( component instanceof CppBlock ) this.list.add(component);
		else if (component instanceof CppEvaluate ) this.list.add(component);
		return this;
	}

	public CppComponent add(String body)
	{
		list.add(body);
		return this;
	}

	public void write(PrintStream ps)
	{
		ps.print("{");
		for (Iterator it = list.iterator(); it.hasNext(); )
		{
			Object item = it.next();
			if (item instanceof String)
			{
				if(list.size()>1) ps.println(item);
				else	ps.print(item);
			}
			else if (item instanceof CppWriter)
				((CppWriter)item).write(ps);
		}
		ps.println("}");
	}
}

