package com.wanmei.db;

import org.w3c.dom.*;
import java.sql.*;

class QueryTable
{
	private Query query;
	private String name;
	private String alias;

	QueryTable( Query q, Element elem ) throws Exception
	{
		query  = q;
		name   = elem.getAttribute( "name" );
		alias  = elem.getAttribute( "alias" );
	}

	String DDL()
	{
		return name + " " + alias;
	}

	void verify() throws Exception
	{
		if ( query.getApplication().getTable( name ) == null )
			throw new Exception( "[" + query.DDL() + "] Table '" + name + "' NOT FOUND" );
	}

	String getName() { return name; }
	String getAlias() { return alias; }
}
