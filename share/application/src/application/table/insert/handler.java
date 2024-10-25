
package application.table.insert;

import com.goldhuman.xml.*;
import java.sql.*;

public class handler extends xmlobject
{
	private String condition;
	private String sql_clause;
	private Object[] parameter;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		condition = attr.getValue("condition");
	}

	public void action()
	{
		if ( condition == null )
		{
			application.table.column.handler[] column = ((application.table.handler)parent).column;
			StringBuffer parm = new StringBuffer ("(");
			StringBuffer data = new StringBuffer ("(");

			parameter = new Object[column.length];

			for (int i = 0; i < column.length; i++)
			{
				parm.append(column[i].name).append(',');
				data.append( "?,");
			}

			parm.setCharAt(parm.lastIndexOf(","), ')');
			data.setCharAt(data.lastIndexOf(","), ')');

			sql_clause = "INSERT INTO " + parent.name + " " + parm + " VALUES " + data;
		}
		else
		{
			int length = condition.length();
			int count = 0;
			for (int i = 0; i < length; i++)
				if (condition.charAt(i) == '?')
					count ++;
			parameter = new Object[count];
	
			sql_clause = "INSERT INTO " + parent.name + " " + condition;
		}
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
