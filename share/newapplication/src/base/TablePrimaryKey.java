package com.wanmei.db;

import org.w3c.dom.*;
import java.sql.*;

class TablePrimaryKey
{
	private Table table;
	private String name;
	private String column;
	private String java_type;
	private boolean not_null;

	TablePrimaryKey( Table tab, Element elem ) throws Exception
	{
		table  = tab;
		name   = elem.getAttribute( "name" );
		column = elem.getAttribute( "column" );
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( "ALTER TABLE " + table.getName() + " ADD CONSTRAINT " + name + " PRIMARY KEY (" + column + ")");
		return sb.toString();
	}

	void verify() throws Exception
	{
		for ( String i : column.split( "[\\s,]+" ) )
			if ( table.getTableColumn(i) == null )
				throw new Exception( "[" + DDL() + "] Column '" + i + "' NOT FOUND" );
	}
}
