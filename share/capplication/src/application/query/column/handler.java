
package application.query.column;

import com.goldhuman.xml.*;
import java.util.*;

public class handler extends xmlobject
{
	private String column_name;
	public String canonical_name;
	public String compute;
	public Class  java_type;
	public application.table.column.handler column;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		column_name = attr.getValue("column");
		compute = attr.getValue("compute");
		try
		{
			java_type = Class.forName(attr.getValue("java-type"));
		}
		catch (Exception e)
		{
		}
	}

	public void action()
	{
		if ( compute != null )
		{
			if ( java_type == null )
				System.err.println ( "In Query '" + parent.name + "' Compute Column '" + compute + "' MUST Have java-type");
			return;
		}

		String[] s = column_name.split("[\\.]+");

		application.query.table.handler[] table = ((application.query.handler)parent).table;
		application.table.column.handler[] columns = null;

		outer: switch ( s.length )
		{
		case 2:
			for (int i = 0; i < table.length; i++)
				if ( table[i].name.compareTo(s[0]) == 0 || 
					( table[i].alias != null && table[i].alias.compareTo(s[0]) == 0 ) )
				{
					columns = (application.table.column.handler[])table[i].table.column;
					canonical_name = ( table[i].alias != null ? table[i].alias : table[i].name ) + ".";
					break;
				}
			if ( columns != null )
				for (int i = 0; i < columns.length; i++)
					if ( columns[i].name.compareTo(s[1]) == 0 )
					{
						column = columns[i];
						canonical_name += s[1];
						break outer;
					}
			break;
		case 1:
			for (int i = 0; i < table.length; i++)
			{
				columns = table[i].table.column;
				for (int j = 0; j < columns.length; j++)
					if ( columns[j].name.compareTo(s[0]) == 0 )
					{
						column = columns[j];
						canonical_name = ( table[i].alias != null ? table[i].alias : table[i].name ) + "." + s[0];
						break outer;
					}
			}
			break;
		default:
			System.err.println ( "In Query '" + parent.name + "' Column format MUST [table].column ");
			return;
			
		}

		if ( column == null )
			System.err.println ( "In Query '" + parent.name + "' Column '" + column_name + "' Miss");

	}
}
