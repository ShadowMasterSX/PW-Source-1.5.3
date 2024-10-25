package com.wanmei.db;

import java.io.*;
import org.w3c.dom.*;
import java.sql.*;

class ProcedureParameter
{
	private Procedure procedure;
	private String name;
	private String sql_type;
	private String java_type;
	private boolean in;
	private boolean out;

	ProcedureParameter( Procedure proc, Element elem ) throws Exception
	{
		procedure = proc;
		name      = elem.getAttribute( "name" );
		sql_type  = elem.getAttribute( "sql-type" );
		java_type = elem.getAttribute( "java-type" );
		in        = Boolean.parseBoolean( elem.getAttribute( "in" ) );
		out       = Boolean.parseBoolean( elem.getAttribute( "out" ) );
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( name + " " + sql_type + " " );
		if ( out ) sb.append( "OUT" );
		return sb.toString();
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

	void makeBeans( PrintStream ps ) throws Exception
	{
		procedure.getApplication().makeBeans( ps, java_type, name );
	}

	void registerParameter( PrintStream ps, int idx ) throws Exception
	{
		ps.println( "		cs.setObject(" + idx + ", " + name + ");" );
		if ( !out ) return;
		ps.println( "		cs.registerOutParameter(" + idx + ", " + getOutType() + " );" );
	}

	String javaParameter()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( java_type + " " + name );
		if ( out ) sb.append( "[]" );
		return sb.toString();
	}

	boolean isOut() { return out; }

	String getName() { return name; }
	String getJava_type() { return java_type; }
}
