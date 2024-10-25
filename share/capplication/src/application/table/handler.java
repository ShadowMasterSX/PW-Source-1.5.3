
package application.table;

import com.goldhuman.xml.*;
import java.util.*;
import java.sql.*;

public class handler extends xmlobject
{
	private static Map instance = new HashMap();
	private Set conn_set = new HashSet();
	private String   operate;
	private Map delete_map = new HashMap();
	private Map insert_map = new HashMap();
	private Map update_map = new HashMap();

	public  application.table.column.handler[] column;
	public  application.table.primarykey.handler primarykey;
	public  application.table.index.handler[] index;
	public	Vector keys = new Vector();

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

		for (Iterator it = conn_set.iterator(); it.hasNext(); )
		{
			String conn_name = (String)it.next();
			boolean found_connection = false;
			if ( application.handler.debug )
				System.err.println ("Table '" + name + "' Bind '" + conn_name + "'");
			for (int i = 0; i < objs.length; i++)
				if ( objs[i] instanceof application.connection.handler && objs[i].name.compareTo(conn_name) == 0 )
				{
					found_connection = true;
					break;
				}

			if ( !found_connection )
			{
				System.err.println ( "In Table '" + name + "' Connection '" + conn_name + "' Miss");
				return;
			}
		}

		objs = children;

		Set set = new HashSet();
		Vector v = new Vector();
		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.table.column.handler )
			{
				objs[i].action();
				if ( set.add(objs[i].name) )
					v.add(objs[i]);
				else
					System.err.println("In Table '" + name + "' Duplicate column '" + objs[i].name + "'");
			}

		column = new application.table.column.handler[v.size()];
		for (int i = 0; i < v.size(); i++)
			column[i] = (application.table.column.handler)v.get(i);

		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.table.primarykey.handler )
			{
				objs[i].action();
				primarykey = (application.table.primarykey.handler)objs[i];
				break;
			}

		set.clear();
		v.clear();
		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.table.index.handler )
			{
				objs[i].action();
				if ( set.add(objs[i].name) )
					v.add(objs[i]);
				else
					System.err.println("In Table '" + name + "' Duplicate index '" + objs[i].name + "'");
			}
		index = new application.table.index.handler[v.size()];
		for (int i = 0; i < v.size(); i++)
			index[i] = (application.table.index.handler)v.get(i);

		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.table.insert.handler )
			{
				objs[i].action();
				if ( insert_map.put(objs[i].name, objs[i]) != null )
					System.err.println("In Table '" + name + "' Duplicate insert '" + objs[i].name + "'");
			}

		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.table.delete.handler )
			{
				objs[i].action();
				if ( delete_map.put(objs[i].name, objs[i]) != null )
					System.err.println("In Table '" + name + "' Duplicate delete '" + objs[i].name + "'");
			}

		for (int i = 0; i < objs.length; i++)
			if ( objs[i] instanceof application.table.update.handler )
			{
				objs[i].action();
				if ( update_map.put(objs[i].name, objs[i]) != null )
					System.err.println("In Table '" + name + "' Duplicate update '" + objs[i].name + "'");
			}

		if ( primarykey != null )
			keys.add(primarykey.column);
		if ( index != null )
			for (int i = 0; i < index.length; i++)
				if ( index[i].unique )
					keys.add(index[i].column);

		Collections.sort ( keys, new Comparator()
			{
				public int compare(Object o1, Object o2)
				{
					return java.lang.reflect.Array.getLength(o1) - java.lang.reflect.Array.getLength(o2);
				}
			}
		);

		LinkedList ll = new LinkedList(keys);
		keys.clear();

		while ( !ll.isEmpty() )
		{
			application.table.column.handler[] key = (application.table.column.handler[]) ll.removeFirst();
			next: for (Iterator it = ll.listIterator(); it.hasNext(); )
			{
				application.table.column.handler[] candidate = (application.table.column.handler[]) it.next();
				for (int i = 0; i < key.length; i++)
					if ( key[i] != candidate[i] )
						continue next;
				it.remove();
			}
			keys.add(key);
		}

		if ( application.handler.debug )
			System.err.println(this);

		if ( operate != null )
		{
			if ( operate.compareTo("create") == 0 )
				Create();
			if ( operate.compareTo("drop") == 0 )
				Drop();
			if ( operate.compareTo("replace") == 0 )
			{
				Drop();
				Create();
			}
		}
	}

	private String DDLTable()
	{
		StringBuffer sb = new StringBuffer( "CREATE TABLE " + name + " (\n");

		for (int i = 0; i < column.length; i++)
			sb.append('\t').append(column[i]).append(",\n");

		if (primarykey != null) sb.append('\t').append(primarykey).append(",\n");

		return sb.deleteCharAt(sb.lastIndexOf(",")).append(")\n").toString();
	}

	public String toString()
	{
		StringBuffer sb = new StringBuffer(DDLTable());
		for (int i = 0; i < index.length; i++)
			sb.append(index[i]);

		for (int i = 0; i < keys.size(); i++)
		{
			application.table.column.handler[] key = (application.table.column.handler[])keys.get(i);
			sb.append ("Key[").append(i).append("]:");
			for (int j = 0; j < key.length; j++)
				sb.append(' ').append(key[j].name);
			sb.append('\n');
		}

		for (Iterator it = insert_map.entrySet().iterator(); it.hasNext(); )
			sb.append( (application.table.insert.handler)((Map.Entry)it.next()).getValue() ).append("\n");

		for (Iterator it = delete_map.entrySet().iterator(); it.hasNext(); )
			sb.append( (application.table.delete.handler)((Map.Entry)it.next()).getValue() ).append("\n");

		for (Iterator it = update_map.entrySet().iterator(); it.hasNext(); )
			sb.append( (application.table.update.handler)((Map.Entry)it.next()).getValue() ).append("\n");

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
				st.executeUpdate("DROP TABLE " + name);
				st.close();
			}
			catch (Exception e)
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
				st.addBatch(DDLTable());
				for (int i = 0; i < index.length; i++)
					st.addBatch(index[i].toString());
				st.executeBatch();
				st.close();
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
			application.connection.handler.put(conn);
		}
	}

	public int executeUpdate(String sql_clause, Object[] parameter_template, Object[] parameter, String conn_name) throws Exception
	{
		if ( parameter.length != parameter_template.length )
			throw new SQLException ( "Parameter number error" );

		if ( conn_name !=null && ! conn_set.contains(conn_name) )
			throw new SQLException ( "Connection '" + conn_name + "' NOT Match" );

		Connection conn = application.connection.handler.get(conn_name);
		PreparedStatement ps = null;
		int retval = -1;
		try
		{
			ps = conn.prepareStatement ( sql_clause );
			for (int i = 0; i < parameter.length; i++)
				ps.setObject(i + 1, parameter[i]);
			retval = ps.executeUpdate();
		}
		catch (Exception e)
		{
			throw e;
		}
		finally
		{
			try { if (ps != null) ps.close(); } catch(Exception e) { }
			application.connection.handler.put(conn);
		}
		return retval;
	}

	public application.table.delete.handler delete(String name)
	{
		return (application.table.delete.handler) delete_map.get(name);
	}

	public application.table.insert.handler insert(String name)
	{
		return (application.table.insert.handler) insert_map.get(name);
	}

	public application.table.update.handler update(String name)
	{
		return (application.table.update.handler) update_map.get(name);
	}

	public static application.table.handler get(String name)
	{
		return (application.table.handler)instance.get(name);
	}
}
