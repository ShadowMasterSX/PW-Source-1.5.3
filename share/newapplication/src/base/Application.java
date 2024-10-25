
package com.wanmei.db;

import java.util.*;
import java.sql.*;
import java.io.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;

public class Application
{
	private String beans;
	private String beanbase;
	private String default_connection_name;

	private Map<String, Driver    >     driver_map = new HashMap<String, Driver    >();
	private Map<String, Connection> connection_map = new HashMap<String, Connection>();
	private Map<String, Table     >      table_map = new HashMap<String, Table     >();
	private Map<String, Procedure >  procedure_map = new HashMap<String, Procedure >();
	private Map<String, Query     >      query_map = new HashMap<String, Query     >();
	private Map<String, Sql       >        sql_map = new HashMap<String, Sql       >();
	private Map<String, Cache     >      cache_map = new HashMap<String, Cache     >();
	private Map<String, List<Cache> >  cache_val_map = new HashMap<String, List<Cache> >();

	public Application( File file ) throws Exception
	{
		Document doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse( file );
		Element root = doc.getDocumentElement();

		beans    = root.getAttribute( "beans" );
		beanbase = root.getAttribute( "beanbase" );

		NodeList nl  = root.getChildNodes();

		List<Element>     driver_elements = new ArrayList<Element>();
		List<Element> connection_elements = new ArrayList<Element>();
		List<Element>      cache_elements = new ArrayList<Element>();
		for ( int i = 0; i < nl.getLength(); i++ )
		{
			Node node = nl.item(i);
			if ( node.getNodeType() != Node.ELEMENT_NODE ) continue;
			Element e = (Element)node;
			String nodeName = e.getNodeName();
			if      ( nodeName.compareToIgnoreCase( "driver"     ) == 0 )     driver_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "connection" ) == 0 ) connection_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "cache"      ) == 0 )      cache_elements.add( e );
		}

		for ( Element e :     driver_elements )     driver_map.put( e.getAttribute("name"), new Driver    (this, e));
		for ( Element e : connection_elements ) connection_map.put( e.getAttribute("name"), new Connection(this, e));
		for ( Element e :      cache_elements )      cache_map.put( e.getAttribute("name"), new Cache     (this, e));

		default_connection_name = connection_map.values().iterator().next().getName();
	}

	private Application()
	{
	}

	private void loadForMakeBeans( File file ) throws Exception
	{
		Document doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse( file );
		Element root = doc.getDocumentElement();

		beans    = root.getAttribute( "beans" );
		beanbase = root.getAttribute( "beanbase" );

		NodeList nl  = root.getChildNodes();

		List<Element>      cache_elements = new ArrayList<Element>();
		List<Element>      table_elements = new ArrayList<Element>();
		List<Element>  procedure_elements = new ArrayList<Element>();
		List<Element>      query_elements = new ArrayList<Element>();
		List<Element>        sql_elements = new ArrayList<Element>();
		for ( int i = 0; i < nl.getLength(); i++ )
		{
			Node node = nl.item(i);
			if ( node.getNodeType() != Node.ELEMENT_NODE ) continue;
			Element e = (Element)node;
			String nodeName = e.getNodeName();
			if      ( nodeName.compareToIgnoreCase( "cache"      ) == 0 )      cache_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "table"      ) == 0 )      table_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "procedure"  ) == 0 )  procedure_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "query"      ) == 0 )      query_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "sql"        ) == 0 )        sql_elements.add( e );
		}

		for ( Element e :      cache_elements )      cache_map.put( e.getAttribute("name"), new Cache     (this, e));
		for ( Element e :      table_elements )      table_map.put( e.getAttribute("name"), new Table     (this, e));
		for ( Element e :  procedure_elements )  procedure_map.put( e.getAttribute("name"), new Procedure (this, e));
		for ( Element e :      query_elements )      query_map.put( e.getAttribute("name"), new Query     (this, e));
		for ( Element e :        sql_elements )        sql_map.put( e.getAttribute("name"), new Sql       (this, e));

		verify();
	}

	String dump()
	{
		StringBuffer sb = new StringBuffer();
		for ( Table     i :     table_map.values() ) { sb.append( i.DDL() ); sb.append( i.DML() ); }
		for ( Procedure i : procedure_map.values() ) sb.append( i.DDL() );
		for ( Query     i :     query_map.values() ) sb.append( i.DDL() );
		return sb.toString();
	}

	void verify() throws Exception
	{
		Map<String, String> dup_check = new HashMap<String, String>();
		for ( Table     i :     table_map.values() )
		{
			String prev = dup_check.put( i.getName().toLowerCase(), "Table" );
			if ( prev != null )
				throw new Exception( "Table and " + prev + " use same name [" + i.getName() + "]" );
		}
		for ( Procedure i : procedure_map.values() )
		{
			String prev = dup_check.put( i.getName().toLowerCase(), "Procedure" );
			if ( prev != null )
				throw new Exception( "Procedure and " + prev + " use same name [" + i.getName() + "]" );
		}
		for ( Query     i :     query_map.values() )
		{
			String prev = dup_check.put( i.getName().toLowerCase(), "Query" );
			if ( prev != null )
				throw new Exception( "Query and " + prev + " use same name [" + i.getName() + "]" );
		}

		for ( Sql        i :       sql_map.values() )
		{
			String prev = dup_check.put( i.getName().toLowerCase(), "Sql" );
			if ( prev != null )
				throw new Exception( "Sql and " + prev + " use same name [" + i.getName() + "]" );
		}

		for ( Table i : table_map.values() ) i.verify();
		for ( Query i : query_map.values() ) i.verify();
		for ( Sql   i :   sql_map.values() ) i.verify();
	}

	void makeBeans() throws Exception
	{
		for ( Table     i :     table_map.values() ) i.makeBeans( beans, beanbase );
		for ( Procedure i : procedure_map.values() ) i.makeBeans( beans, beanbase );
		for ( Query     i :     query_map.values() ) i.makeBeans( beans, beanbase );
		for ( Sql       i :       sql_map.values() ) i.makeBeans( beans, beanbase );
		Cache.makeHolder( this, beans, beanbase );
	}

	static String translateBeanName( String name )
	{
		return name.substring( 0, 1 ).toUpperCase() + name.substring(1);
	}

	static String toQueryName( String name )
	{
		return name.substring( 0, 1 ).toLowerCase() + name.substring(1);
	}

	static PrintStream makeClassHead( String packagePrefix, String base, String name, String type ) throws Exception
	{
		String path = base + "/" + type;
		new File( path ).mkdirs();
		String beanname = translateBeanName(name);
		PrintStream ps = new PrintStream( new FileOutputStream( new File( path + "/" + beanname + ".java" ) ) );
		ps.println ( "package " + packagePrefix + "." + type + ";" );
		ps.println ( "import "+packagePrefix+".*;");
		ps.println ( "import java.sql.*;");
		ps.println ( "import java.util.*;");
		ps.println ( "import com.wanmei.db.*;");
		ps.println ( "public final class " + beanname );
		ps.println ( "{" );
		ps.println ( "public " + beanname + "(){}" );
		return ps;
	}

	static void makeClassTail( PrintStream ps ) throws Exception
	{
		ps.println ( "}" );
		ps.close();
	}

	static void makeBeans( PrintStream ps, String java_type, String name ) throws Exception
	{
		String beanname = translateBeanName(name);
		ps.println ( "private " + java_type + " " + name + ";" );
		ps.println ( "public " + java_type + " get" + beanname + "(){return " + name + ";}" );
		ps.println ( "protected void set" + beanname + "(" + java_type + " x){" + name + " = x;}" );
	}

	Table getTable(String name) { return table_map.get(name); }
	Connection getConnection( String name ) { return connection_map.get(name); }

	String getDefaultConnectionName() { return default_connection_name; }

	public Cache getCache(String name) { return cache_map.get(name); }
	void registerCache( String val_type, Cache cache )
	{
		List<Cache> list = cache_val_map.get(val_type);
		if ( list == null )
		{
			list = new ArrayList<Cache> ();
			cache_val_map.put( val_type, list );
		}
		list.add( cache );
	}
	Map<String, Cache> getCacheMap() { return cache_map; }
	Map<String, List<Cache> > getCacheValMap() { return cache_val_map; }

	public Context createContext()
	{
		return new Context( this );
	}

	public static String makeListString( List<String> list )
	{
		StringBuffer sb = new StringBuffer ("(");
		for ( String i : list )
			sb.append(i).append(',');
		sb.setCharAt(sb.lastIndexOf(","), ')');
		return sb.toString();
	}
	
	public static String makeListString( String s, int count )
	{
		StringBuffer sb = new StringBuffer ("(");
		for ( int i = 0; i < count; i++ )
			sb.append(s).append(',');
		sb.setCharAt(sb.lastIndexOf(","), ')');
		return sb.toString();
	}

	public static void main(String args[]) throws Exception
	{
		Application app = new Application( );
		app.loadForMakeBeans(new File(args[0]));
		app.makeBeans();
		if ( args.length > 1 && args[1].compareTo("dump") == 0 )
			System.out.println( app.dump() );
	}

	Query getQuery(String name)
	{
		return query_map.get(name);
	}
}

