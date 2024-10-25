
package application.query.select;

import com.goldhuman.xml.*;
import java.sql.*;

public class handler extends xmlobject
{
	private String sql_clause;
	private String condition;
	private boolean unique;
	public  Object[] parameter;

	private application.query.select.cache.handler cache;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		condition = attr.getValue("condition");
		String x = attr.getValue("unique");
		unique = ( x != null && x.compareTo("true") == 0 );
	}

	public void action()
	{
		application.query.column.handler[] column = ((application.query.handler)parent).column;
		application.query.table.handler[] table = ((application.query.handler)parent).table;

		StringBuffer sb = new StringBuffer("SELECT ");

		if (unique) sb.append ("DISTINCT ");

		for (int i = 0; i < column.length; i++)
			sb.append(column[i].compute == null ? column[i].canonical_name : column[i].compute)
				.append(" AS ").append(column[i].name).append(',');
		sb.setCharAt(sb.lastIndexOf(","), ' ');

		sb.append("FROM ");
		for(int i = 0; i < table.length; i++)
		{
			sb.append(table[i].name);
			if ( table[i].alias != null )
				sb.append(' ').append(table[i].alias);
			sb.append(',');
		}
		sb.setCharAt(sb.lastIndexOf(","), ' ');
	
		int count = 0;
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

		xmlobject[] objs = children;

		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.query.select.cache.handler )
			{
				objs[i].action();
				cache = (application.query.select.cache.handler)objs[i];
				break;
			}
	}

	public String toString() { return sql_clause; }

	public Object[] execute(Object[] parameter, String conn_name) throws Exception
	{
		Object[] data;
		if ( cache != null && ( data = cache.search(parameter) ) != null ) return data;
		data = ((application.query.handler)parent).executeQuery( sql_clause, this.parameter, parameter, conn_name );
		if ( cache != null )
			cache.append(parameter, data);
		return data;
	}

	public Object[] execute(Object[] parameter) throws Exception
	{
		return execute(parameter, null);
	}
}

