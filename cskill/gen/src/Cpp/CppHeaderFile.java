
package Cpp;

import java.io.PrintStream;
import java.util.*;
import java.io.*;

public class CppHeaderFile implements CppWriter, CppComponent
{
	protected String name;
	protected String namespace;
	private ArrayList components      = new ArrayList();

	public CppHeaderFile( String name, String namespace )
	{
		this.name = name;
		this.namespace = namespace;
	}

	public void write(PrintStream ps)
	{
		ps.println( "#ifndef __CPPGEN_" + namespace.toUpperCase() + "_" + name.toUpperCase() );
		ps.println( "#define __CPPGEN_" + namespace.toUpperCase() + "_" + name.toUpperCase() );

		ps.println( "namespace " + namespace );
		ps.println( "{" );

		for (Iterator it = components.iterator(); it.hasNext(); )
			((CppClass)it.next()).write(ps);

		ps.println( "}" );
		
		ps.println("#endif");
	}

	public CppComponent add(CppComponent component)
	{
		components.add(component);
		return this;
	}

	public void write( String filename )
	{
		try
		{
			PrintStream ps = new PrintStream(new FileOutputStream(filename,false),false,"GBK");
			write( ps );
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}
	
	}
}

