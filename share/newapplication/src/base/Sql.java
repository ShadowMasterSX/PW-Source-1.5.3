
package com.wanmei.db;

import java.io.*;
import java.util.*;
import java.util.regex.*;
import org.w3c.dom.*;

class Sql
{
	class SqlColumn
	{
		private String name;
		private String alias;
		private String java_type;

		SqlColumn ( Element e )
		{
			name      = e.getAttribute( "name" );
			alias     = e.getAttribute( "alias" );
			java_type = e.getAttribute( "java-type" );
		}

		SqlColumn ( String name, String alias, String java_type )
		{
			this.name      = name;
			this.alias     = alias == null ? name : alias;
			this.java_type = java_type;
		}

		SqlColumn ( SqlColumn c )
		{
			this.name      = c.name;
			this.alias     = c.alias;
			this.java_type = c.java_type;
		}

		String getName() { return name; }
		String getAlias() { return alias; }
		String getJava_type() { return java_type; }
	}

	class SqlFrom
	{
		private String name;
		private String alias;
		private Table  table;

		SqlFrom( String name, String alias ) throws Exception
		{
			this.name  = name;
			this.alias = alias == null ? name : alias;
			table = application.getTable(name);
			if ( table == null )
				throw new Exception( sql_clause + " table " + name + " not found " );
		}

		TableColumn getTableColumn( String column_name )
		{
			String w[] = column_name.split( "[\\.]+" );
			if ( w.length == 1 )
				return table.getTableColumn(w[0]);
			if ( w[0].compareTo(name) == 0 || w[0].compareTo(alias) == 0 )
				return table.getTableColumn(w[1]);
			return null;
		}
	}

	class SqlParameter
	{
		private String name;
		private String java_type;

		SqlParameter( Element e )
		{
			name      = e.getAttribute( "name" );
			java_type = e.getAttribute( "java-type" );
		}

		String getName() { return name; }
		String getJava_type() { return java_type; }
	}

	private static Pattern select_pattern = Pattern.compile( 
			"SELECT\\s+(?:(?:DIST\\s+)|(?:DISTINCT\\s+))?(.*)FROM(.*?)(?=(?:WHERE|HAVING|GROUP|ORDER))",
			Pattern.CASE_INSENSITIVE | Pattern.MULTILINE | Pattern.DOTALL);
	private static Pattern from_pattern = Pattern.compile( 
			"([^\\,\\s]+)\\s*([^\\,\\s]+)?\\s*(?=(?:,|$))",
			Pattern.CASE_INSENSITIVE | Pattern.MULTILINE | Pattern.DOTALL);
	private static Pattern column_pattern = Pattern.compile( 
			"([^\\,\\s]+)\\s*(?:as\\s+([^\\,\\s]+))?\\s*(?=(?:,|$))",
			Pattern.CASE_INSENSITIVE | Pattern.MULTILINE | Pattern.DOTALL);
	private Application application;
	private String name;
	private String sql_clause;
	private String sql_parser;
	private List<SqlParameter> parameter_list = new ArrayList<SqlParameter>();
	private List<SqlColumn   >    column_list = new ArrayList<SqlColumn   >();
	private	List<SqlColumn   >   compute_list = new ArrayList<SqlColumn   >();

	void verify() throws Exception
	{
		for ( Matcher matcher = select_pattern.matcher(sql_parser); matcher.find(); )
		{
			List<SqlFrom> from_list = new ArrayList<SqlFrom>();
			for ( Matcher m = from_pattern.matcher( matcher.group(2) ); m.find(); )
				from_list.add(new SqlFrom(m.group(1), m.group(2)));
			for ( Matcher m = column_pattern.matcher( matcher.group(1) ); m.find(); )
			{
				String cn = m.group(1);
				String ca = m.group(2);
				if ( cn.compareTo("#") == 0 )
					column_list.add ( new SqlColumn( compute_list.remove(0) ) );
				else
				{
					TableColumn table_column = null;
					int match_count = 0;
					for ( SqlFrom s : from_list )
					{
						TableColumn c = s.getTableColumn(cn);
						if ( c != null )
						{
							if ( match_count++ > 0 )
								throw new Exception(sql_clause+" Ambiguous column "+cn);
							table_column = c;
						}
					}
					if ( table_column == null )
						throw new Exception ( sql_clause + " column " + cn + " not found" );
					column_list.add( new SqlColumn( cn, ca, table_column.getJava_type() ));
				}
			}
		}
	}	

	Sql( Application app, Element elem ) throws Exception
	{
		application = app;
		name = elem.getAttribute( "name" );
		StringBuffer sb1 = new StringBuffer();
		StringBuffer sb2 = new StringBuffer();
		NodeList nl  = elem.getChildNodes();
		for ( int i = 0; i < nl.getLength(); i++ )
		{
			Node node = nl.item(i);
			switch ( node.getNodeType() )
			{
			case Node.ELEMENT_NODE:
				Element e = (Element)node;
				String nodeName = e.getNodeName();
				if ( nodeName.compareToIgnoreCase( "parameter" ) == 0 )
				{
					parameter_list.add( new SqlParameter(e) );
					sb1.append( "?" );
					sb2.append( "?" );
				}
				else if ( nodeName.compareToIgnoreCase( "column" ) == 0 )
				{
					SqlColumn c = new SqlColumn(e);
					compute_list.add( c );
					sb1.append( c.getName() + " AS " + c.getAlias() );
					sb2.append( "#" );
				}
				break;
			case Node.TEXT_NODE:
				sb1.append( node.getTextContent() );
				sb2.append( node.getTextContent() );
				break;
			}
		}
		sql_clause = sb1.toString().trim().replaceAll("[\\r\\n]+", " ");
		sql_parser = sb2.toString().trim().replaceAll("[\\r\\n]+", " ");;
	}

	String parameterList()
	{
		StringBuffer sb = new StringBuffer();
		for ( SqlParameter s : parameter_list )
			sb.append ( ", " + s.getJava_type() + " _" + s.getName() );
		return sb.toString();
	}

	String parameterCallList()
	{
		StringBuffer sb = new StringBuffer();
		for ( SqlParameter s : parameter_list )
			sb.append ( ",  _" + s.getName() );
		return sb.toString();
	}

	void makeBeans( String beans, String beanbase ) throws Exception
	{
		PrintStream ps = application.makeClassHead( beans, beanbase, name, "sql" );
		String beanname = application.translateBeanName(name);
		for ( SqlColumn s : column_list )
			application.makeBeans( ps, s.getJava_type(), s.getAlias() );
		ps.println( "public static List<" + beanname + "> execute(Context ctx" + parameterList()+")" );
		ps.println( "{" );
		ps.println( "	List<" + beanname + "> r = null; " );

		ps.println( "	CounterHolder.storage.increment(\"Sql." + beanname + "\");");
		ps.println( "	try" );
		ps.println( "	{" );
		ps.println( "		java.sql.PreparedStatement ps = ctx.prepareStatement(\"" + sql_clause + "\");" );
		int i = 1;
		for ( SqlParameter s : parameter_list )
		ps.println( "		ps.setObject(" + i++ + ", _" + s.getName() + "); " );
		ps.println( "		r = " + beanname + ".getResult(ps.executeQuery());" );
		ps.println( "	}" );
		ps.println( "	catch (Exception e)" );
		ps.println( "	{" );
		ps.println( "		ctx.setException(e);" );
		ps.println( "	}" );
		ps.println( "	finally" );
		ps.println( "	{" );
		ps.println( "		ctx.doFinally(); " );
		ps.println( "	}" );
		ps.println( "	return r;" );
		ps.println( "}" );
		ps.println( "public " + beanname + "(java.sql.ResultSet rs) throws Exception" );
		ps.println( "{");
		int count = 1;
		for ( SqlColumn s : column_list )
		ps.println( "   " + s.getAlias() + "=("+s.getJava_type()+")rs.getObject(" + count++ + ");" );
		ps.println( "}");
		ps.println( "public static List<" + beanname + "> getResult(java.sql.ResultSet rs) throws Exception" );
		ps.println( "{");
		ps.println( "	List<" + beanname + "> r = new ArrayList<" + beanname + ">();");
		ps.println( "	while ( rs.next() ) r.add( new " + beanname + "(rs));" );
		ps.println( "	return r;");
		ps.println( "}");

		String pl = parameterList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public static List<" + beanname + "> execute(" + pl +")" );
		ps.println( "{" );
		ps.println( "	return execute(ApplicationThreadFrameWork.getContext()" + parameterCallList()+");" );
		ps.println( "}" );

		application.makeClassTail( ps );
	}

	String getName() { return name; }
}

