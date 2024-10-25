package com.wanmei.db;

import org.w3c.dom.*;
import java.io.*;
import java.util.*;

class TableUpdate
{
	private Table table;
	private String name;
	private String from;
	private List<String> columnName = new ArrayList<String>();
	private String condition;

	TableUpdate( Table tab, Element elem ) throws Exception
	{
		table     = tab;
		name      = elem.getAttribute( "name" );
		from      = elem.getAttribute( "from" );
		condition = elem.getAttribute( "condition" );
		String c  = elem.getAttribute( "column" );
		if ( c.length() == 0 )
			for ( TableColumn i : table.getColumnList() )
				columnName.add(i.getName());
		else
			for ( String i : c.split( "[\\s,]+" ) )
				columnName.add(i);
	}

	String DML()
	{
		StringBuffer sb = new StringBuffer("UPDATE " + table.getName() + " " );
		if ( from.length() > 0 ) sb.append( "FROM " + from + " " );
		sb.append( "SET " );
		for ( String i : columnName )
			sb.append ( i + "=?," );
		sb.setCharAt(sb.lastIndexOf(","), ' ');
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

	String columnList()
	{
		StringBuffer sb = new StringBuffer();
		for ( String i : columnName )
		{
			TableColumn c = table.getTableColumn(i);
			sb.append ( ", " + c.getJava_type() + " _" + c.getName() );
		}
		return sb.toString();
	}

	String columnCallList()
	{
		StringBuffer sb = new StringBuffer();
		for ( String i : columnName )
		{
			TableColumn c = table.getTableColumn(i);
			sb.append ( ", _" + c.getName() );
		}
		return sb.toString();
	}

	void makeBeans( PrintStream ps ) throws Exception
	{
		ps.println( "public int update"+name+"(Context ctx" + parameterList() +")" );
		table.makeHead( ps, DML(), "update1" + name );
		int count = 1;
		for ( String i : columnName )
		ps.println( "		ps.setObject(" + count++ + "," + i + ");" );
		int length = parameterCount();
		for ( int i = 1; i <= length; i++ )
		ps.println( "		ps.setObject(" + count++ + ", _p" + i + "); " );
		Table.makeTail( ps );

		String pl = parameterList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public int update"+name+"(" + pl +")" );
		ps.println( "{");
		ps.println( "	return update"+name+"(ApplicationThreadFrameWork.getContext()"+parameterCallList()+");" );
		ps.println( "}");

		ps.println( "public static int update"+name+"(Context ctx" + columnList() + parameterList() + ")" );
		table.makeHead( ps, DML(), "update2" + name  );
		count = 1;
		for ( String i : columnName )
		ps.println( "		ps.setObject(" + count++ + ",_" + i + ");" );
		length = parameterCount();
		for ( int i = 1; i <= length; i++ )
		ps.println( "		ps.setObject(" + count++ + ", _p" + i + "); " );
		Table.makeTail( ps );

		pl = columnList() + parameterList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public static int update"+name+"(" + pl + ")" );
		ps.println( "{");
		ps.println( "	return update"+name+"(ApplicationThreadFrameWork.getContext()"+columnCallList()+parameterCallList()+");" );
		ps.println( "}");
	}

	void verify() throws Exception
	{
		if ( condition.length() == 0 )
			throw new Exception( "[" + DML() + "] without WHERE Clause" );
		for ( String i : columnName )
			if ( table.getTableColumn(i) == null )
				throw new Exception( "[" + DML() + "] Column '" + i + "' NOT FOUND" );
	}
}
