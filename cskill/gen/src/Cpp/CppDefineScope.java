
package Cpp;

import java.io.PrintStream;
import java.util.*;
import java.io.*;

public class CppDefineScope implements CppWriter, CppComponent
{
	protected String define;
	private ArrayList components      = new ArrayList();

	public CppDefineScope( String define )
	{
		this.define = define;
	}

	public void write(PrintStream ps)
	{
		ps.println( "#ifdef " + define );
		Iterator it = null;
		for (it = components.iterator(); it.hasNext(); )
			((CppWriter)it.next()).write(ps);
		ps.println( "#endif" );
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

