
package application.table.delete;

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
		int length = condition.length();
		int count = 0;
		for (int i = 0; i < length; i++)
			if (condition.charAt(i) == '?')
				count ++;
		parameter = new Object[count];

		sql_clause = "DELETE FROM " + parent.name + " " + condition;
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
