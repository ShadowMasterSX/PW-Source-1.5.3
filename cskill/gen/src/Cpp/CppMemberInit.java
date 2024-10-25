package Cpp;

import java.io.PrintStream;

public class CppMemberInit implements CppWriter, CppComponent
{
	protected String member;
	protected String initializer;

	public CppMemberInit ( String member, String initializer )
	{
		this.member = member;
		this.initializer = initializer;
	}

	public CppComponent add(CppComponent component) { return this; }

	public void write(PrintStream ps)
	{
		ps.print ( member + "(" + initializer + ")"  );
	}
}

