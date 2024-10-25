
package Cpp;

import java.io.PrintStream;
import java.util.*;
import java.io.*;

public class CppCppFile implements CppWriter, CppComponent
{
	protected String namespace;
	private ArrayList includes        = new ArrayList();
	private ArrayList components      = new ArrayList();

	public CppCppFile( String namespace )
	{
		this.namespace = namespace;
	}

	public void write(PrintStream ps)
	{
		Iterator it = null;
		for (it = includes.iterator(); it.hasNext(); )
			ps.println( (String)it.next() );

		ps.println( "namespace " + namespace );
		ps.println( "{" );

		for (it = components.iterator(); it.hasNext(); )
			((CppWriter)it.next()).write(ps);

		ps.println( "}" );
	}

	public CppComponent add(String str)
	{
		includes.add(str);
		return this;
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

