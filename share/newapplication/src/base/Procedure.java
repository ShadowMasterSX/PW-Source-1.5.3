package com.wanmei.db;

import java.io.*;
import java.util.*;
import org.w3c.dom.*;
import java.sql.*;

class Procedure
{
	class ProcedureParameter
	{
		private String name;
		private String sql_type;
		private String java_type;
		private boolean in;
		private boolean out;
		private String cache;

		ProcedureParameter( Procedure proc, Element elem ) throws Exception
		{
			name      = elem.getAttribute( "name" );
			sql_type  = elem.getAttribute( "sql-type" );
			java_type = elem.getAttribute( "java-type" );
			in        = Boolean.parseBoolean( elem.getAttribute( "in" ) );
			out       = Boolean.parseBoolean( elem.getAttribute( "out" ) );
			cache     = elem.getAttribute( "cache" );
		}

		int getOutType()
		{
			int type;
			try
			{
				type = Class.forName("java.sql.Types").getField(sql_type.split("[\\W]+")[0].toUpperCase()).getInt(null);
			}
			catch ( Exception e )
			{
				type = java.sql.Types.NULL;
			}
			return type;
		}

		boolean isOut() { return out; }
		String getName() { return name; }
		String getJava_type() { return java_type; }
		String getSql_type() { return sql_type; }
		String[] getCache() { return cache.split(";"); }
	}
	private Application application;
	private String name;
	private String content;
	private List<ProcedureParameter> parameter_list   = new ArrayList<ProcedureParameter>();
	private String proc_cache;
	private String proc_cache_key;
	private Cache  cache;

	void verify() throws Exception
	{
		if ( proc_cache.length() == 0 || proc_cache_key.length() == 0 ) return;
		Cache c = application.getCache(proc_cache);
		if ( c == null )
			throw new Exception ( "[ Procedure: " + name + " ] Cache '" + proc_cache + "' NOT Found" );

		boolean found = false;
		for ( ProcedureParameter i : parameter_list )
		{
			if ( i.getName().compareTo( proc_cache_key ) == 0 )
			{
				c.setKeyName( i.getName() );
				c.setKeyType( i.getJava_type() );
				found = true;
				break;
			}
		}
		if ( ! found )
			throw new Exception ( "[ Procedure: " + name + " ] Cache Key '" + proc_cache_key + "' NOT Found" );

		registerCache( c );
	}

	void registerCache( Cache c )
	{
		String beanname = application.translateBeanName(name);
		c.setValType(beanname, null);
		cache = c;
	}

	Procedure( Application app, Element elem ) throws Exception
	{
		application = app;
		name = elem.getAttribute( "name" );

		proc_cache = elem.getAttribute( "cache" );
		proc_cache_key = elem.getAttribute( "key" );

		List<Element> parameter_elements = new ArrayList<Element>();

		NodeList nl = elem.getChildNodes();
		for ( int i = 0; i < nl.getLength(); i++ )
		{
			Node node = nl.item(i);
			switch ( node.getNodeType() )
			{
			case Node.ELEMENT_NODE:
				Element e = (Element)node;
				String nodeName = e.getNodeName();
				if ( nodeName.compareToIgnoreCase( "parameter" ) == 0 )
					parameter_elements.add( e );
				break;
			case Node.TEXT_NODE:
				content = node.getTextContent();
				break;
			}
		}

		for ( Element e : parameter_elements )
			parameter_list.add ( new ProcedureParameter( this, e ) );

		try { verify(); }catch(Exception e) { e.printStackTrace(); }
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( "CREATE PROCEDURE " + name + "\n" );
		for ( ProcedureParameter i : parameter_list )
		{
			sb.append("\t");
			sb.append( "@" + i.getName() + " " + i.getSql_type() + " " );
			if ( i.isOut() ) sb.append( "OUT" );
			sb.append(",\n");
		}
		int pos = sb.lastIndexOf(",");
		if ( pos != -1 )
			sb = sb.deleteCharAt(sb.lastIndexOf(","));
		sb.append( "AS\n" ).append( content ).append( "\n" );
		return sb.toString();
	}

	String parameterList()
	{
		StringBuffer sb = new StringBuffer("(");
		for ( ProcedureParameter i : parameter_list )
			sb.append ( i.getJava_type() + " _" + i.getName() + "," );
		sb.setCharAt(sb.lastIndexOf(","), ')');
		return sb.toString();
	}

	void makeBeans( String beans, String beanbase ) throws Exception
	{
		PrintStream ps = application.makeClassHead( beans, beanbase, name, "procedure" );
		for ( ProcedureParameter i : parameter_list )
			application.makeBeans( ps, i.getJava_type(), i.getName() );

		String beanname = application.translateBeanName(name);
		if ( parameter_list.size() > 0 )
		{
		ps.println( "public " + beanname + parameterList() );
		ps.println( "{" );
		for ( ProcedureParameter i : parameter_list )
		ps.println( "	" + i.getName() + " = _" + i.getName() + ";" );
		ps.println( "}" );
		}

		ps.println( "public int execute()" );
		ps.println( "{" );
		ps.println( "	return execute( ApplicationThreadFrameWork.getContext() ); " );
		ps.println( "}" );

		ps.println( "public int execute( Context ctx )" );
		ps.println( "{" );
		ps.println( "	int r = -1;" );

		if (null != cache) // try get from cache
		{
			ps.println( "		Querymatrixbyid qmbi = CacheHolder." + cache.getName() + ".get(this.get"
					+ application.translateBeanName(proc_cache_key) + "());" );
			ps.println( "		if (null != qmbi) { " );
			for ( ProcedureParameter i : parameter_list )
			{
				if ( i.isOut() )
					ps.println( "			this." + i.getName() + " = qmbi.get"
							+ application.translateBeanName(i.getName()) + "();");
			}
			ps.println( "		return 0; } " );
		}

		ps.println( "	CounterHolder.storage.increment(\"Procedure." + beanname + "\");");
		ps.println( "	try" );
		ps.println( "	{" );

		StringBuffer sb = new StringBuffer("{?=call ");
		sb.append(name).append("(");
		for (int i = 0; i < parameter_list.size(); i++) sb.append("?,");
		sb.setCharAt(sb.lastIndexOf(","), ')');
		String sql_clause = sb.append("}").toString();
		ps.println( "		java.sql.CallableStatement cs = ctx.prepareCall(\"" + sql_clause + "\");" );
		ps.println( "		cs.registerOutParameter(1, Types.INTEGER);" );
		int n = 2;

		String cacheupdate = "";
		for ( ProcedureParameter i : parameter_list )
		{
			ps.println( "		cs.setObject(" + n + ", " + i.getName() + ");" );
			if ( i.isOut() )
			ps.println( "		cs.registerOutParameter(" + n + ", " + i.getOutType() + " );" );
			for ( String c  : i.getCache() )
				if (application.getCache(c) != null)
					cacheupdate += "		CacheHolder." + c + ".remove(" + i.getName() + ");\n";
			n++;
		}

		if (cacheupdate.length() > 0) ps.println( cacheupdate );
		ps.println( "		cs.execute();" );
		ps.println( "		r = cs.getInt(1);" );
		n = 2;
		for ( ProcedureParameter i : parameter_list )
		{
			if ( i.isOut() )
			{
				
				ps.println( "		" + i.getName() + " = (" + i.getJava_type() + ") cs.getObject(" + n + ");");
			}
			n++;
		}
		if (null != cache) // update cache
		{
			ps.println( "		if (r == 0) CacheHolder.update"+beanname+"(this);" );
		}
		ps.println( "	}" );
		ps.println( "	catch (Exception e)" );
		ps.println( "	{" );
		ps.println( "		e.printStackTrace(System.out);" ); //ljh add
		ps.println( "		ctx.setException(e);" );
		ps.println( "	}" );
		ps.println( "	finally" );
		ps.println( "	{" );
		ps.println( "		ctx.doFinally(); " );
		ps.println( "	}" );
		ps.println( "	return r;" );
		ps.println( "}" );

		application.makeClassTail( ps );
	}

	String getName() { return name; }
	Application getApplication() { return application; }
}

