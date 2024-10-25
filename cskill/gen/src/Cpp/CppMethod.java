package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppMethod implements CppWriter, CppComponent
{
	protected String type;
	protected String name;
	protected boolean isStatic;
	protected boolean isConst;
	protected int scope;

	protected CppParameterList parameterList;
	protected CppBlock block;

	CppMethod ( String type, String name, boolean isStatic, boolean isConst, int scope )
	{
		this.type = type;
		this.name = name;
		this.isStatic = isStatic;
		this.isConst = isConst;
		this.scope = scope;
	}

	public CppComponent add(CppComponent component)
	{
		if ( component instanceof CppParameterList )
			this.parameterList = (CppParameterList)component;

		if ( component instanceof CppBlock )
			this.block = (CppBlock)component;

		return this;
	}

	public void write(PrintStream ps)
	{
		if ( scope==1 )
			ps.println( "#ifdef _SKILL_SERVER" );
		else if ( scope==2 )
			ps.println( "#ifdef _SKILL_CLIENT" );

		if ( isStatic )
			ps.print ("static ");

		ps.print ( type + " " + name );
		parameterList.write(ps);

		if ( isConst )
			ps.print (" const");

		block.write(ps);

		if ( scope!=0 )
			ps.println( "#endif" );
	}
}

