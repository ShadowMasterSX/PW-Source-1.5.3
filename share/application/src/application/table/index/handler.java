
package application.table.index;
import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	private String column_name;
	public  boolean unique = false;
	public  application.table.column.handler[] column;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		column_name = attr.getValue("column");
		String u = attr.getValue("unique");
		if ( u != null && u.compareTo("true") == 0)
			unique = true;
	}

	public void action()
	{
		String[] s = column_name.split("[ \n\t,]+"); column_name = null;
		column = new application.table.column.handler[s.length];
		application.table.column.handler[] all_column = ((application.table.handler)parent).column;

		next: for (int i = 0; i < s.length; i++)
		{
			for (int j = 0; j < all_column.length; j++)
				if ( all_column[j].name.compareTo(s[i]) == 0 )
				{
					column[i] = (application.table.column.handler)all_column[j];
					continue next;
				}
			System.err.println("INDEX '" + name + "' REF '" + parent.name + "." + s[i] + "' Miss"); 
		}
	}

	public String toString() 
	{
		StringBuffer sb = new StringBuffer ("CREATE " + (unique ? "UNIQUE " : "" ) + "INDEX " + name + " ON " +  parent.name + " (");

		for (int i = 0; i < column.length; i++)
			sb.append(column[i].name).append(',');

		return sb.deleteCharAt(sb.lastIndexOf(",")).append(")\n").toString();
	}
}

