package com.wanmei.db;

import java.util.*;
import org.w3c.dom.*;
import java.io.*;

class QuerySelect
{
	private Query query;
	private String name;
	private String condition;
	private boolean unique;

	private String cache;
	private String key;
	private Cache  c;

	QuerySelect( Query q, Element elem ) throws Exception
	{
		query     = q;
		name      = elem.getAttribute( "name" );
		condition = elem.getAttribute( "condition" );
		unique    = Boolean.parseBoolean(elem.getAttribute( "unique" ));

		cache     = elem.getAttribute( "cache" );
		key       = elem.getAttribute( "key"   );
	}

	void verify() throws Exception
	{
		if ( cache.length() == 0 || key.length() == 0 ) return;
		c = query.getApplication().getCache(cache);
		if ( c == null )
			throw new Exception ( "[" + query.DDL() + "] Cache '" + cache + "' NOT FOUND" );
		if ( parameterCount() != 1 )
			throw new Exception ( "[" + query.DDL() + "] Cache Key '" + key + "' NOT Match ParameterCount" );

		boolean found = false;
		for ( QueryColumn i : query.getColumnList() )
		{
			if ( i.getName().compareTo( key ) == 0 )
			{
				c.setKeyName( i.getName() );
				c.setKeyType( i.getJava_type() );
				found = true;
			}
		}
		if ( ! found )
			throw new Exception ( "[" + query.DDL() + "] Cache Key '" + key + "' NOT Found" );

		query.registerCache( c );
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		List<QueryColumn> column_list = query.getColumnList();
		List<QueryTable> table_list   = query.getTableList();
		sb.append ( "SELECT " );
		if ( unique ) sb.append( "DISTINCT " );
		for ( QueryColumn i : column_list )
		{
			sb.append ( i.DDL() );
			sb.append ( "," );
		}
		int pos = sb.lastIndexOf(",");
		if ( pos != -1 )
			sb = sb.deleteCharAt(sb.lastIndexOf(","));
		sb.append( " FROM " );
		for ( QueryTable i : table_list )
		{
			sb.append( i.DDL() );
			sb.append( "," );
		}
		pos = sb.lastIndexOf(",");
		if ( pos != -1 )
			sb = sb.deleteCharAt(sb.lastIndexOf(","));
		sb.append( " " + condition );
		return sb.toString();
	}

	int parameterCount()
	{
		int count = 0;
		for (int i = 0; i < condition.length(); i++)
			if (condition.charAt(i) == '?')
				count ++;
		return count;
	}

	int columnParameterCount()
	{
		int count = 0;
		for ( QueryColumn i : query.getColumnList() )
			if( i.isComputeAsk() )
				count ++;
		return count;
	}

	String parameterList()
	{
		StringBuffer sb = new StringBuffer();
		int length = parameterCount();
		for ( int i = 1; i <= length; i++ )
			sb.append ( ", Object _p" + i );
		return sb.toString();
	}


	String parameterCallList()
	{
		StringBuffer sb = new StringBuffer();
		int length = parameterCount();
		for ( int i = 1; i <= length; i++ )
			sb.append ( ", _p" + i );
		return sb.toString();
	}

	void makeBeans( PrintStream ps ) throws Exception
	{
		String beanname = Application.translateBeanName( query.getName() );

		ps.println( "public static List<" + beanname + "> select"+name+"(Context ctx" + parameterList()+")" );
		ps.println( "{" );
		ps.println( "	List<" + beanname + "> r = null; " );

		if ( c != null )
		{
			if (Query.ismulti(query.getCacheValue()))
			{
				ps.println( "	r = CacheHolder." + c.getName() + ".get(("+c.getKeyType()+") _p1 );" );
				ps.println( "	if ( r != null ) return r;" );
			}
			else // single
			{
				ps.println( "	" + beanname + " bean = CacheHolder."
					+ c.getName() + ".get(("+c.getKeyType()+") _p1 );" );
				ps.println( "	if ( bean != null ) { " );
				ps.println( "		r = new ArrayList<" + beanname + ">();");
				ps.println( "		r.add(bean); return r; } " );
			}
		}

		ps.println( "	CounterHolder.storage.increment(\"" + beanname + ".select" + name + "\");");
		ps.println( "	try" );
		ps.println( "	{" );
		ps.println( "		java.sql.PreparedStatement ps = ctx.prepareStatement(\"" + DDL() + "\");" );
		int length = parameterCount();
		int column_length = columnParameterCount();
		int n = 1;
		if  ( 1 == length )
		{
			for ( int i = 1;i <= column_length; i++ )
			{
			ps.println( "		ps.setObject(" + n++ + ", _p1); " );
			}
		}

		for ( int i = 1;i <= length; i++ )
		{
		ps.println( "		ps.setObject(" + n++ + ", _p" + i + "); " );
		}
		ps.println( "		r = " + beanname + ".getResult(ps.executeQuery());" );

		if ( c != null )
		{
			switch (query.getCacheValueMode())
			{
			case 0: ps.println( "		if (r.size() == 1) CacheHolder.update"+beanname+"(r.get(0));" ); break;
			case 1: ps.println( "		if (!r.isEmpty()) CacheHolder.update"+beanname+"( r );" ); break;
			case 2: ps.println( "		CacheHolder.update"+beanname+"(("+c.getKeyType()+") _p1, r );" ); break;
			}
		}

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

		String pl = parameterList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public static List<" + beanname + "> select"+name+"(" + pl + ")" );
		ps.println( "{" );
		ps.println( "	return select"+name+"(ApplicationThreadFrameWork.getContext()" + parameterCallList()+");" );
		ps.println( "}" );

		// get from cache only
		ps.println( "public static List<" + beanname + "> cache_select"+name+"(Context ctx" + parameterList()+")" );
		ps.println( "{" );
		ps.println( "	List<" + beanname + "> r = null; " );

		if ( c != null )
		{
			if (Query.ismulti(query.getCacheValue()))
			{
				ps.println( "	r = CacheHolder." + c.getName() + ".get(("+c.getKeyType()+") _p1 );" );
				ps.println( "	if ( r != null ) { " );
				ps.println( "		CounterHolder.storage.increment(\"" + beanname + ".cache_select" + name + "\");");
				ps.println( "		return r; } " );
			}
			else // single
			{
				ps.println( "	" + beanname + " bean = CacheHolder."
					+ c.getName() + ".get(("+c.getKeyType()+") _p1 );" );
				ps.println( "	if ( bean != null ) { " );
				ps.println( "		CounterHolder.storage.increment(\"" + beanname + ".cache_select" + name + "\");");
				ps.println( "		r = new ArrayList<" + beanname + ">();");
				ps.println( "		r.add(bean); return r; } " );
			}
		}

		ps.println( "	return r;" );
		ps.println( "}" );

		pl = parameterList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public static List<" + beanname + "> cache_select"+name+"(" + pl + ")" );
		ps.println( "{" );
		ps.println( "	return cache_select"+name+"(ApplicationThreadFrameWork.getContext()" + parameterCallList()+");" );
		ps.println( "}" );


	}
}
