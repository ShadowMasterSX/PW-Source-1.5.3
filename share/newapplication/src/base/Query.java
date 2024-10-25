package com.wanmei.db;

import java.util.*;
import org.w3c.dom.*;
import java.io.*;
import java.sql.*;

class Query
{
	private Application application;
	private String name;
	private String cachevalue;

	private List<QueryColumn> column_list = new ArrayList<QueryColumn>();
	private List<QueryTable >  table_list = new ArrayList<QueryTable >();
	private List<QuerySelect> select_list = new ArrayList<QuerySelect>();
	private List<Cache>        cache_list = new ArrayList<Cache>();

	Query( Application app, Element elem ) throws Exception
	{
		application = app;
		name = elem.getAttribute( "name" );
		cachevalue = elem.getAttribute( "cachevalue" );

		List<Element> column_elements = new ArrayList<Element>();
		List<Element>  table_elements = new ArrayList<Element>();
		List<Element> select_elements = new ArrayList<Element>();

		NodeList nl = elem.getChildNodes();
		for ( int i = 0; i < nl.getLength(); i++ )
		{
			Node node = nl.item(i);
			if ( node.getNodeType() != Node.ELEMENT_NODE ) continue;
			Element e = (Element)node;
			String nodeName = e.getNodeName();
			if      ( nodeName.compareToIgnoreCase( "column" ) == 0 ) column_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "table"  ) == 0 ) table_elements. add( e );
			else if ( nodeName.compareToIgnoreCase( "select" ) == 0 ) select_elements.add( e );
		}

		for ( Element e : column_elements ) column_list.add ( new QueryColumn( this, e ) );
		for ( Element e :  table_elements )  table_list.add ( new QueryTable ( this, e ) );
		for ( Element e : select_elements ) select_list.add ( new QuerySelect( this, e ) );
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		sb.append ( "[Query " + name + "]\n\n" );
		for ( QuerySelect i : select_list )
		{
			sb.append ( "\t" );
			sb.append ( i.DDL() );
			sb.append ( "\n" );
		}
		return sb.toString();
	}

	void verify() throws Exception
	{
		for ( QueryTable i  : table_list  ) i.verify();
		for ( QueryColumn i : column_list ) i.verify();
		for ( QuerySelect i : select_list ) i.verify();
	}

	void makeBeans( String beans, String beanbase ) throws Exception
	{
		PrintStream ps = application.makeClassHead( beans, beanbase, name, "query" );
		String beanname = application.translateBeanName(name);
		for ( QueryColumn i : column_list ) i.makeBeans( ps );
		for ( QuerySelect i : select_list ) i.makeBeans( ps );
		ps.println( "public " + beanname + "(java.sql.ResultSet rs) throws Exception" );
		ps.println( "{");
		int count = 1;
		for ( QueryColumn i : column_list )
		ps.println( "   " + i.getName() + "= (" + i.getJava_type() + ")rs.getObject(" + count++ + ");" );
		ps.println( "}");
		ps.println( "public static List<" + beanname + "> getResult(java.sql.ResultSet rs) throws Exception" );
		ps.println( "{");
		ps.println( "	List<" + beanname + "> r = new ArrayList<" + beanname + ">();");
		ps.println( "	while ( rs.next() ) r.add( new " + beanname + "(rs));" );
		ps.println( "	return r;");
		ps.println( "}");
		application.makeClassTail( ps );
	}

	String getName() { return name; }
	List<QueryColumn> getColumnList() { return column_list; }
	List<QueryTable>  getTableList() { return table_list; }
	Application getApplication() { return application; }
	QueryTable getQueryTable( String name )
	{
		for ( QueryTable i : table_list )
			if ( name.compareTo( i.getName() ) == 0 || name.compareTo( i.getAlias() ) == 0 )
				return i;
		return null;
	}

	void registerCache( Cache cache )
	{
		String beanname = application.translateBeanName(name);
		cache.setValType( beanname, getCacheValue() );
		cache_list.add(cache);
	}

	String getCacheValue()
	{
		return cachevalue;
	}

	static boolean ismulti(String cv)
	{
		return null != cv && (cv.equals("multi") || cv.equals("empty"));
	}

	int getCacheValueMode()
	{
		String cv = getCacheValue();
		if (cv.equals("multi")) return 1;
		if (cv.equals("empty")) return 2;
		return 0;
	}
}

