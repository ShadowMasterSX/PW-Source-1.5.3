
package Cpp;

import java.io.PrintStream;
import java.util.*;

public class CppClass implements CppWriter, CppComponent
{
	protected String name;
	protected ArrayList parents      = new ArrayList();
	protected ArrayList nestclass    = new ArrayList();
	protected ArrayList fields       = new ArrayList();
	protected ArrayList constructors = new ArrayList();
	protected ArrayList methods      = new ArrayList();
	protected ArrayList scopes       = new ArrayList();
	protected CppDestructor destructor;
	protected boolean isOnlyServer;

	public CppClass( String name ) { this.name = name; }

	public CppClass( String name, String parent )
	{
		this.name = name;
		parents.add (parent);
	}

	public CppClass( String name, String parent, boolean  isOnlyServer)
	{
		this.name = name;
		parents.add (parent);
		this.isOnlyServer = isOnlyServer;
	}

	public CppClass( String name, List parent )
	{
		this.name = name;
		parents.addAll(parent);
	}

	public String getName() { return name; }

	public void write(PrintStream ps)
	{
		if ( isOnlyServer )
			ps.println( "#ifdef _SKILL_SERVER" );
		ps.print ( "class " + name );
		boolean first = true;
		for (Iterator it = parents.iterator(); it.hasNext(); )
		{
			if ( first )
			{
				first = false;
				ps.print(":");
			}
			else
			{
				ps.print(",");
			}
			String parent = (String)it.next();
			ps.print( "public " + parent );
		}
		ps.println();
		ps.println("{");
		ps.println("public:");

		for (Iterator it = nestclass.iterator(); it.hasNext(); )
			((CppClass)it.next()).write(ps);

		for (Iterator it = fields.iterator(); it.hasNext(); )
			((CppField)it.next()).write(ps);

		for (Iterator it = constructors.iterator(); it.hasNext(); )
			((CppConstructor)it.next()).write(ps);

		if ( destructor != null )
			destructor.write(ps);
	
		for (Iterator it = methods.iterator(); it.hasNext(); )
			((CppMethod)it.next()).write(ps);

		for (Iterator it = scopes.iterator(); it.hasNext(); )
			((CppDefineScope)it.next()).write(ps);

		ps.println("};");
		if ( isOnlyServer )
			ps.println( "#endif" );
	}

	public CppComponent add(CppComponent component)
	{
		if (component instanceof CppClass ) nestclass.add(component);
		else if (component instanceof CppConstructor ) constructors.add(component);
		else if (component instanceof CppDestructor ) destructor = (CppDestructor)component;
		else if (component instanceof CppField ) fields.add(component);
		else if (component instanceof CppMethod ) methods.add(component);
		else if (component instanceof CppDefineScope ) scopes.add(component);
		return this;
	}
}

