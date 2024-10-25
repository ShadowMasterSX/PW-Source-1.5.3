package com.wanmei.db;


import org.w3c.dom.*;
import java.util.concurrent.*;
import java.sql.*;

class Connection
{
	private Application application;
	private String name;
	private String url;
	private String username;
	private String password;
	private LinkedBlockingQueue<java.sql.Connection> pool;

	Connection( Application app, Element elem ) throws Exception
	{
		application = app;
		name     = elem.getAttribute( "name" );
		url      = elem.getAttribute( "url" );
		username = elem.getAttribute( "username" );
		password = elem.getAttribute( "password" );
		pool     = new LinkedBlockingQueue<java.sql.Connection>(Integer.parseInt(elem.getAttribute( "poolsize" )));
		while ( pool.remainingCapacity() > 0 )
			pool.offer( DriverManager.getConnection(url,username,password) );
	}

	java.sql.Connection getConnection()
	{
		try
		{
			return pool.take();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}

	void returnConnection( java.sql.Connection conn )
	{
		try
		{
			pool.put( conn );
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	void discardConnection( java.sql.Connection conn )
	{
		try { conn.close(); } catch ( Exception e ) { }
		while ( true )
		{
			try
			{
				conn = DriverManager.getConnection(url, username, password );
				if ( conn == null )
				{
					Thread.sleep(1000);
					continue;
				}
				pool.put( conn );
				break;
			}
			catch (Exception e)
			{
				e.printStackTrace();
				try { Thread.sleep(1000); } catch (Exception e2) { }
			}
		}
	}

	String getName() { return name; }
}

