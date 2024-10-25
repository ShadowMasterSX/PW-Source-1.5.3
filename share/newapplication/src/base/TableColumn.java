package com.wanmei.db;

import java.io.*;
import org.w3c.dom.*;
import java.sql.*;

class TableColumn
{
	private Table table;
	private String name;
	private String sql_type;
	private String java_type;
	private boolean not_null;

	TableColumn( Table tab, Element elem ) throws Exception
	{
		table     = tab;
		name      = elem.getAttribute( "name" );
		sql_type  = elem.getAttribute( "sql-type" );
		java_type = elem.getAttribute( "java-type" );
		not_null  = Boolean.parseBoolean( elem.getAttribute( "not-null" ) );
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( name + " " + sql_type + " " );
		if ( not_null )
		{
			sb.append( "NOT NULL" );
		}
		return sb.toString();
	}

	void makeBeans( PrintStream ps ) throws Exception
	{
		table.getApplication().makeBeans( ps, java_type, name );
	}

	String getName() { return name; }
	String getJava_type() { return java_type; }
	boolean canNull() { return ! not_null; }
}
