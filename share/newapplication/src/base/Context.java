
package com.wanmei.db;
import java.util.*;

public class Context
{
	private Application application;
	private String connection_name;
	private Connection connection;
	private Exception exception;
	private Exception lastException;

	private java.sql.Connection conn;
	private java.sql.PreparedStatement ps;
	private java.sql.CallableStatement cs;
	private java.sql.ResultSet rs;

	public Context( Application app )
	{
		application = app;
		connection_name = app.getDefaultConnectionName();
		connection      = app.getConnection(connection_name);
	}

	public String getConnectionName() { return connection_name; }
	public void setConnectionName( String s )
	{
		connection      = application.getConnection(s);
		connection_name = s;
	}

	public java.sql.Connection getConnection() { return connection.getConnection(); }
	public void returnConnection( java.sql.Connection c ) { connection.returnConnection( c ); }
	public void discardConnection( java.sql.Connection c ) { connection.discardConnection( c ); }
	public void setException( Exception e ) { exception = lastException = e; }
	public Exception getException()  { Exception e = exception; exception = null; return e; }
	public Exception peekException() { return exception; }

	public java.sql.PreparedStatement prepareStatement( String sql ) throws Exception
	{
		return ps = (conn = connection.getConnection()).prepareStatement( sql );
	}

	public java.sql.CallableStatement prepareCall( String sql ) throws Exception
	{
		return cs = ( conn = connection.getConnection() ).prepareCall( sql );
	}

	public java.sql.ResultSet executeQuery() throws Exception
	{
		return rs = ps.executeQuery();
	}

	public void doFinally()
	{
		boolean close_ok = true;
		try { if (rs != null) rs.close(); } catch(Exception e) { close_ok = false; }
		try { if (ps != null) ps.close(); } catch(Exception e) { close_ok = false; }
		try { if (cs != null) cs.close(); } catch(Exception e) { close_ok = false; }
		if ( lastException != null && close_ok == true )
		{
			try
			{
				ps = conn.prepareStatement("SELECT 1");
				rs = ps.executeQuery();
			}
			catch (Exception e)
			{
				close_ok = false;
			}
			finally
			{	
				try { if ( rs != null ) rs.close(); } catch(Exception e) { close_ok = false; }
				try { if ( ps != null ) ps.close(); } catch(Exception e) { close_ok = false; }
			}
		}
		if ( close_ok ) returnConnection(conn); else discardConnection(conn);
		rs = null; ps = null; cs = null; conn = null; lastException = null;
	}

	public int insert( String table_name, List<String> columnName, List<Object> columnObj ) throws Exception
	{
		prepareStatement( "INSERT INTO " + table_name + Application.makeListString(columnName) +  " VALUES " + Application.makeListString( "?", columnName.size()));
		int c = 1;
		for ( Object o : columnObj )
			ps.setObject( c++, o );
		return ps.executeUpdate();
	}
} 
