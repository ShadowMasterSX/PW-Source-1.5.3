
package application.query;

import com.goldhuman.xml.*;
import java.util.*;
import java.sql.*;

public class handler extends xmlobject
{
	private static Map instance = new HashMap();
	private Map select_map = new HashMap();
	public application.query.column.handler[] column;
	public application.query.table.handler[] table;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		instance.put(name, this);
	}

	public void action()
	{
		xmlobject[] objs = children;

		Vector v = new Vector();
		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.query.table.handler )
			{
				objs[i].action();
				v.add(objs[i]);
			}
		table = new application.query.table.handler[v.size()];
		for (int i = 0; i < v.size(); i++)
			table[i] = (application.query.table.handler)v.get(i);

		v.clear();
		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.query.column.handler )
			{
				objs[i].action();
				v.add(objs[i]);
			}
		column = new application.query.column.handler[v.size()];
		for (int i = 0; i < v.size(); i++)
			column[i] = (application.query.column.handler)v.get(i);


		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.query.select.handler )
			{
				objs[i].action();
				if ( select_map.put(objs[i].name, objs[i]) != null )
					System.err.println("In Query '" + name + "' Duplicate select '" + objs[i].name + "'");
			}

		if ( application.handler.debug )
			System.err.println(this);
	}

	public String toString()
	{
		StringBuffer sb = new StringBuffer();

		for (Iterator it = select_map.entrySet().iterator(); it.hasNext(); )
			sb.append( (application.query.select.handler)((Map.Entry)it.next()).getValue() ).append("\n");

		return sb.toString();
	}

	public Object[] executeQuery(String sql_clause, Object[] parameter_template, Object[] parameter, String conn_name) throws Exception
	{
		if ( parameter.length != parameter_template.length )
			throw new SQLException ( "Parameter number error" );

		Connection conn = application.connection.handler.get(conn_name);
		PreparedStatement ps = null;
		ResultSet rs = null;
		Vector vec = new Vector();
		try
		{
			ps = conn.prepareStatement(sql_clause);
			for (int i = 0; i < parameter.length; i++)
				ps.setObject(i + 1, parameter[i]);
			for ( rs = ps.executeQuery(); rs.next(); )
			{
				Object[] row = new Object[column.length];
				for (int i = 0; i < column.length; i++)
					row[i] = rs.getObject(i + 1);
				vec.add(row);
			}
		}
		catch (Exception e)
		{
			throw e;
		}
		finally
		{
			try { if (rs != null) rs.close(); } catch(Exception e) { }
			try { if (ps != null) ps.close(); } catch(Exception e) { }
			application.connection.handler.put(conn);
		}

		return vec.toArray();
	}

	public application.query.select.handler select(String name)
	{
		return (application.query.select.handler)select_map.get(name);
	}

	public static application.query.handler get(String name)
	{
		return (application.query.handler)instance.get(name);
	}
}

