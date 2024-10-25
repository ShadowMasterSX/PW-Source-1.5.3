package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppDestructor implements CppWriter, CppComponent
{
	protected String name;
	protected CppBlock block;

	CppDestructor ( CppClass cppClass )
	{
		name = cppClass.name;
	}

	public CppComponent add(CppComponent component)
	{
		if ( component instanceof CppBlock )
			this.block = (CppBlock)component;
		return this;
	}

	public void write(PrintStream ps)
	{
		ps.print ( "virtual ~" + name + "()" );
		block.write(ps);
	}
}

