package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppConstructor implements CppWriter, CppComponent
{
	protected String name;
	protected CppParameterList parameterList;
	protected CppMemberInitList memberInitList;
	protected CppBlock block;

	CppConstructor ( CppClass cppClass )
	{
		name = cppClass.name;
	}

	public CppComponent add(CppComponent component)
	{
		if ( component instanceof CppParameterList )
			this.parameterList = (CppParameterList)component;
		else if ( component instanceof CppMemberInitList )
			this.memberInitList = (CppMemberInitList)component;
		else if ( component instanceof CppBlock )
			this.block = (CppBlock)component;
		return this;
	}

	public void write(PrintStream ps)
	{
		ps.print ( name );
		parameterList.write(ps);
		if ( memberInitList != null )
		{
			ps.print (":");
			memberInitList.write(ps);
		}
		block.write(ps);
	}
}

