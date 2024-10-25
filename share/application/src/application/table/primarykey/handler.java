
package application.table.primarykey;
import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	private String column_name;
	public  application.table.column.handler[] column;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		column_name = attr.getValue("column");
	}

	public void action()
	{
		String[] s = column_name.split("[ \n\t,]+"); column_name = null;
		column = new application.table.column.handler[s.length];
		application.table.column.handler[] all_column = ((application.table.handler)parent).column;

		for (int i = 0; i < s.length; i++)
		{
			for (int j = 0; j < all_column.length; j++)
				if ( all_column[j].name.compareTo(s[i]) == 0 )
				{
					column[i] = (application.table.column.handler)all_column[j];
					break;
				}
			if (column[i] == null)
				System.err.println("PRIMARY KEY '" + name + "' REF '" + parent.name + "." + s[i] + "' Miss"); 
		}
	}

	public String toString() 
	{
		StringBuffer sb = new StringBuffer ("PRIMARY KEY (");

		for (int i = 0; i < column.length; i++)
			sb.append(column[i].name).append(',');

		return sb.deleteCharAt(sb.lastIndexOf(",")).append(")").toString();
	}
}
