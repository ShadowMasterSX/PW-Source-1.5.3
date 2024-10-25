package com.wanmei.db;

import org.w3c.dom.*;
import java.io.*;
import java.sql.*;

class QueryColumn
{
	private Query query;
	private String name;
	private String column;
	private String compute;
	private String java_type;

	QueryColumn( Query q, Element elem ) throws Exception
	{
		query     = q;
		name      = elem.getAttribute( "name" );
		column    = elem.getAttribute( "column" );
		compute   = elem.getAttribute( "compute" );
		java_type = elem.getAttribute( "java-type" );
	}

	boolean isComputeAsk()
	{
		return compute.equals("?");
	}

	String DDL()
	{
		return column + compute + " AS " + name;
	}

	void verify() throws Exception
	{
		String[] c = column.split("[\\.]+");
		if ( c.length != 2 )
			return;
		QueryTable t = query.getQueryTable( c[0] );
		if ( t == null )
			throw new Exception ( "[" + query.DDL() + "] Alias '" + c[0] + "' NOT FOUND" );
		TableColumn col = query.getApplication().getTable(t.getName()).getTableColumn( c[1] );
		if ( col == null )
			throw new Exception ( "[" + query.DDL() + "] Column '" + c[1] + "' NOT FOUND IN Table '" + t.getName() + "'"  );
		java_type = col.getJava_type();
	}

	void makeBeans( PrintStream ps ) throws Exception
	{
		query.getApplication().makeBeans( ps, java_type, name );
	}


	String getName() { return name; }
	String getJava_type() { return java_type; }
}
