package com.wanmei.db;

import org.w3c.dom.*;
import java.sql.*;

class TableIndex
{
	private Table table;
	private String name;
	private String column;
	private boolean unique;

	TableIndex( Table tab, Element elem ) throws Exception
	{
		table  = tab;
		name   = elem.getAttribute( "name" );
		column = elem.getAttribute( "column" );
		unique = Boolean.parseBoolean(elem.getAttribute( "unique" ));
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( "CREATE " );
		if ( unique )
			sb.append("UNIQUE ");
		sb.append("INDEX " + name + " ON " + table.getName() + "(" + column + ")");
		return sb.toString();
	}


	void verify() throws Exception
	{
		for ( String i : column.split( "[\\s,]+" ) )
			if ( table.getTableColumn(i) == null )
				throw new Exception( "[" + DDL() + "] Column '" + i + "' NOT FOUND" );
	}

}
