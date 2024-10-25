
package application.query.table;

import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	public String alias;
	public application.table.handler table;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		alias = attr.getValue("alias");
	}

	public void action()
	{
		xmlobject[] objs = parent.parent.children;

		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.table.handler && objs[i].name.compareTo(name) == 0)
			{
				table = (application.table.handler)objs[i];
				break;
			}

		if ( table == null )
			System.err.println ("In Query '" + parent.name + "' table '" + name + "' Miss");
	}
}
