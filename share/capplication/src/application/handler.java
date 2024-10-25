
package application;

import java.util.*;
import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	public static boolean debug;

	private void traverse(Class c)
	{
		xmlobject[] objs = children;
		Set set = new HashSet();
		
		Vector v = new Vector();
		for (int i = 0; i < objs.length; i++)
			if ( c.isInstance(objs[i]) )
				if ( set.add(objs[i].name) )
					objs[i].action();
				else
					System.err.println("Duplicate " + c.getName() + " " + objs[i].name);
	}

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		String x = attr.getValue("debug");
		debug = ( x != null && x.compareTo("true") == 0 );
	}

	public void action()
	{
		traverse(application.driver.handler.class);
		traverse(application.connection.handler.class);
		traverse(application.table.handler.class);
		traverse(application.query.handler.class);
		traverse(application.procedure.handler.class);
	}
}
