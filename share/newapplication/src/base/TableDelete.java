package com.wanmei.db;

import org.w3c.dom.*;
import java.io.*;
import java.util.*;

class TableDelete
{
	private Table table;
	private String name;
	private String from;
	private String condition;

	TableDelete( Table tab, Element elem ) throws Exception
	{
		table     = tab;
		name      = elem.getAttribute( "name" );
		from      = elem.getAttribute( "from" );
		condition = elem.getAttribute( "condition" );
	}

	String DML()
	{
		StringBuffer sb = new StringBuffer("DELETE " + table.getName() + " " );
		if ( from.length() > 0 ) sb.append( "FROM " + from + " " );
		else sb.append( "FROM " + table.getName() + " " );
		sb.append( condition );
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
		ps.println( "public static int delete"+name+"(Context ctx" + parameterList() + ")" );
		table.makeHead( ps, DML(), "delete" + name  );
		int length = parameterCount();
		for ( int i = 1; i <= length; i++ )
		ps.println( "		ps.setObject(" + i + ", _p" + i + "); " );
		Table.makeTail( ps );

		String pl = parameterList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public static int delete"+name+"(" + pl + ")" );
		ps.println( "{" );
		ps.println( "	return delete"+name+"(ApplicationThreadFrameWork.getContext()" + parameterCallList() + ");" );
		ps.println( "}" );
	}

	void verify()
	{
		if ( condition.length() == 0 )
		{
			System.err.println( "[" + DML() + "] without WHERE Clause" );
		}
	}
}
