package com.wanmei.db;

import org.w3c.dom.*;
import java.io.*;
import java.util.*;

class TableInsert
{
	private Table table;
	private String name;
	private List<String> columnName = new ArrayList<String>();
	private String condition;

	TableInsert( Table tab, Element elem ) throws Exception
	{
		table     = tab;
		name      = elem.getAttribute( "name" );
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
		String parm = Application.makeListString( columnName );
		String data = Application.makeListString( "?", columnName.size() );
		if ( condition.length() == 0 )
			return "INSERT INTO " + table.getName() + parm + " VALUES " + data;
		return "INSERT INTO " + table.getName() + parm + " " + condition;
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

	private void makeBeans0( PrintStream ps ) throws Exception
	{
		ps.println( "public int insert"+name+"(Context ctx)" );
		table.makeHead( ps, DML(), "insert1" + name );
		int count = 1;
		for ( String i : columnName )
		ps.println( "		ps.setObject(" + count++ + "," + i + ");" );
		Table.makeTail( ps );

		ps.println( "public int insert"+name+"()" );
		ps.println( "{" );
		ps.println( "	return insert"+name+"(ApplicationThreadFrameWork.getContext());" );
		ps.println( "}" );

		ps.println( "public static int insert"+name+"(Context ctx" + columnList() +")" );
		table.makeHead( ps, DML(), "insert2" + name );
		count = 1;
		for ( String i : columnName )
		ps.println( "		ps.setObject(" + count++ + ",_" + i + ");" );
		Table.makeTail( ps );

		String pl = columnList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public static int insert"+name+"(" + pl +")" );
		ps.println( "{" );
		ps.println( "	return insert"+name+"(ApplicationThreadFrameWork.getContext()"+columnCallList()+");" );
		ps.println( "}" );
	}

	private void makeBeans1( PrintStream ps ) throws Exception
	{
		ps.println( "public static int insert"+name+"(Context ctx" + parameterList() +")" );
		table.makeHead( ps, DML(), "insert" + name );
		int length = parameterCount();
		for ( int i = 1; i <= length; i++ )
		ps.println( "		ps.setObject(" + i + ", _p" + i + "); " );
		Table.makeTail( ps );

		String pl = parameterList();
		if ( pl.length() > 0 ) pl = pl.substring(1);
		ps.println( "public static int insert"+name+"(" + pl +")" );
		ps.println( "{" );
		ps.println( "	return insert"+name+"(ApplicationThreadFrameWork.getContext()"+parameterCallList()+");" );
		ps.println( "}" );
	}

	void makeBeans( PrintStream ps ) throws Exception
	{
		if ( condition.length() == 0 ) makeBeans0( ps );
		else makeBeans1( ps );
	}

	void verify() throws Exception
	{
		Set<String> all = new HashSet<String>();
		for ( TableColumn i : table.getColumnList() ) all.add( i.getName() );
		for ( String i : columnName )
		{
			if ( table.getTableColumn(i) == null )
				throw new Exception( "[" + DML() + "] Column '" + i + "' NOT FOUND" );
			all.remove(i);
		}

		for ( String i : all )
		{
			if ( ! table.getTableColumn(i).canNull() )
				throw new Exception( "[" + DML() + "] Column '" + i + "' CANNOT BE NULL" );
		}
	}

}
