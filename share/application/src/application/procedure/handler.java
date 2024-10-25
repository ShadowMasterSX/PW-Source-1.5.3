
package application.procedure;

import java.util.*;
import java.sql.*;
import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	private static Map instance = new HashMap();
	private Set conn_set = new HashSet();
	private String operate;
	private application.procedure.parameter.handler[] parameter;
	private String sql_clause;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		conn_set.addAll(Arrays.asList(attr.getValue("connection").split("[ \n\t,]+")));
		operate = attr.getValue("operate");
		instance.put(name, this);
	}

	public void action()
	{
		xmlobject[] objs = parent.children;
		for(Iterator it = conn_set.iterator(); it.hasNext(); )
		{
			String conn_name = (String)it.next();
			boolean found_connection = false;
			if (application.handler.debug)
				System.err.println("Procedure '" + name + "' Bind '" + conn_name + "'");
			for (int i = 0; i < objs.length; i ++)
				if ( objs[i] instanceof application.connection.handler && objs[i].name.compareTo(conn_name) == 0 )
				{
					found_connection = true;
					break;
				}

			if (!found_connection)
			{
				System.err.println ( "In Procedure '" + name + "' Connection '" + conn_name + "' Miss");
				return;
			}
		}

		objs = children;

		Set set = new HashSet();
		Vector v = new Vector();
		for (int i = 0; i < objs.length; i ++)
			if (objs[i] instanceof application.procedure.parameter.handler)
			{	
				objs[i].action();
				if (set.add(objs[i].name))
					v.add(objs[i]);
				else
					System.err.println("In Procedure '" + name + "' Duplicate parameter '" + objs[i].name + "'");
			}

		parameter = new application.procedure.parameter.handler[v.size()];
		for (int i = 0; i < v.size(); i ++)
			parameter[i] = (application.procedure.parameter.handler)v.get(i);

		if (application.handler.debug)
			System.err.println(this);

		if (operate != null)
		{
			if (operate.compareTo("create") == 0)
				Create();
			if (operate.compareTo("drop") == 0)
				Drop();
			if (operate.compareTo("replace") == 0)
			{
				Drop();
				Create();
			}
		}

		StringBuffer sb = new StringBuffer("{?=call ");
		sb.append(name).append("(");
		for (int i = 0; i < parameter.length; i ++)
			sb.append("?,");
		sb.setCharAt(sb.lastIndexOf(","), ')');
		sql_clause = sb.append("}").toString();
	}

	public String toString()
	{
		StringBuffer sb = new StringBuffer("CREATE PROCEDURE ");
		sb.append(name).append("\n");
		for(int i = 0; i < parameter.length; i ++)
		{
			sb.append("\t@").append(parameter[i].name).append("\t").append(parameter[i].sql_type);
			if (parameter[i].out) sb.append(" out");
			sb.append(",\n");
		}
		sb.deleteCharAt(sb.lastIndexOf(","));
		sb.append("AS\nBEGIN\n\t").append(content.trim()).append("\nEND");

		return sb.toString();
	}

	private void Drop()
	{
		for (Iterator it = conn_set.iterator(); it.hasNext(); )
		{
			Connection conn = application.connection.handler.get((String)it.next());
			try
			{
				Statement st = conn.createStatement();
				st.executeUpdate("DROP PROCEDURE " + name);
				st.close();
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
			application.connection.handler.put(conn);
		}
	}

	private void Create()
	{
		for (Iterator it = conn_set.iterator(); it.hasNext(); )
		{
			Connection conn = application.connection.handler.get((String)it.next());
			try
			{
				Statement st = conn.createStatement();
				st.executeUpdate(toString());
				st.close();
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
			application.connection.handler.put(conn);
		}
	}

	public static application.procedure.handler get(String name)
	{
		return (application.procedure.handler)instance.get(name);
	}

	public int execute(Object[] para, String conn_name) throws Exception
	{
		if ( para.length != parameter.length )
			throw new SQLException ( "Parameter number error" );

		if (conn_name != null && !conn_set.contains(conn_name))
			throw new SQLException("Connection '" + conn_name + "' NOT Match" );

		int retval = 0;
		Connection conn = application.connection.handler.get(conn_name);
		CallableStatement cs = null;
		try
		{
			cs = conn.prepareCall(sql_clause);
			cs.registerOutParameter(1, Types.INTEGER);
			for (int i = 0; i < para.length; i ++)
			{
				cs.setObject(i+2, para[i]);
				if (parameter[i].out) cs.registerOutParameter(i+2, parameter[i].out_type);
			}
			cs.execute();
			retval = cs.getInt(1);

			for (int i = 0; i < para.length; i ++)
				if (parameter[i].out) para[i] = cs.getObject(i+2);
		}
		catch(Exception e)
		{
			throw e;
		}
		finally
		{
			try { if (cs != null) cs.close(); } catch(Exception e) { }
			application.connection.handler.put(conn);
		}
		return retval;
	}

	public int execute(Object[] para) throws Exception
	{
		return execute(para, null);
	}
}
