
package application.table.update;

import com.goldhuman.xml.*;
import java.sql.*;

public class handler extends xmlobject
{
	private String condition;
	private String column_name;
	private String sql_clause;
	private Object[] parameter;
	private application.table.column.handler[] column;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		column_name = attr.getValue("column");
		condition = attr.getValue("condition");
	}

	public void action()
	{
		int count = 0;

		StringBuffer sb = new StringBuffer("UPDATE ");	
		sb.append (parent.name).append(" SET ");

		application.table.column.handler[] all_column = ((application.table.handler)parent).column;

		String[] s = column_name.split("[ \n\t,]+"); column_name = null;
		column = new application.table.column.handler[s.length];
		next: for (int i = 0; i < s.length; i++)
		{
			for (int j = 0; j < all_column.length; j++)
				if ( all_column[j].name.compareTo(s[i]) == 0)
				{
					column[count++] = (application.table.column.handler)all_column[j];
					sb.append(s[i]).append("=?,");
					continue next;
				}
			System.err.println("UPDATE '" + name + "' REF '" + parent.name + "." + s[i] + "' Miss");
		}

		sb.setCharAt(sb.lastIndexOf(","), ' ');

		if ( condition != null )
		{
			sb.append(condition);
			int length = condition.length();
			for (int i = 0; i < length; i++)
				if (condition.charAt(i) == '?')
					count ++;
		}

		parameter = new Object[count];
		sql_clause = sb.toString();
	}

	public String toString() { return sql_clause; }

	public int execute(Object[] parameter, String conn_name) throws Exception
	{
		return ((application.table.handler)parent).executeUpdate( sql_clause, this.parameter, parameter, conn_name );
	}

	public int execute(Object[] parameter) throws Exception
	{
		return execute(parameter, null);
	}
}
