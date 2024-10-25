package application.connection;

import com.goldhuman.xml.*;
import java.sql.*;
import javax.naming.*;
import javax.sql.*;

import java.util.*;

public class handler extends xmlobject
{
	
	static {
		new Timer().schedule(new CheckTask(),600*1000,600*1000);
	}
	static class CheckTask extends java.util.TimerTask
	{
		public void run()
		{
			Iterator iter = pools.entrySet().iterator();
			while( iter.hasNext() )
			{
				SimplePool p = (SimplePool)((Map.Entry)iter.next()).getValue();
				p.doCheck();
			}
		}
	}
	class SimplePool
	{       
    	private Dummy[] pool = null;
		private int size;
		String connurl;
		String usrname;
		String pwd;
	    SimplePool(int _size, String driver, String _connurl, String _username, String _pwd)
   		{
			size = _size;
			connurl = _connurl;
			username = _username;
			pwd = _pwd;
        	try {
        		pool = new Dummy[size];
				if( driver != null )
	            	Class.forName(driver);
            	for(int i=0; i<size; ++i)
				{
					try {
	                	pool[i] = new Dummy();
	                	pool[i].conn = DriverManager.getConnection(connurl,username,pwd);
						pool[i].isValid = true;
					} catch (Exception ex ) { }
				}
        	}
	        catch( Exception ex ) { ex.printStackTrace(); }
    	}
    	private class Dummy
	    {
    	    Connection conn;
        	boolean isActive = false;
			boolean isValid = false;
    	}
		public synchronized void doCheck()
		{
			for(int i =0; i<size; ++i)
			{
				try {
					pool[i].isValid = !pool[i].conn.isClosed();
				} catch ( Exception ex ) { pool[i].isValid = false; }
				if( pool[i].isValid == false )
				{
					try {
						pool[i].conn = DriverManager.getConnection(connurl,username,pwd);
						pool[i].isValid = true;
        	        	notifyAll();
					}
					catch( Exception ex ) { }
				}
			}
		}
    	private Connection _getConnection()
	    {
    	    for(int i=0 ; i<size ; i++)
        	{
            	if( pool[i].isValid && pool[i].isActive == false )
	            {
    	            pool[i].isActive = true;
        	        return pool[i].conn;
            	}
	        }
    	    return null;
	    }
		public synchronized Connection getConnection()
	    {   
    	    Connection conn = null;
        	while ( (conn = _getConnection()) == null )
	        {   
    	        try {wait(); } catch ( Exception ex ) { }
        	}
	        return conn;
    	}
    	public synchronized void returnConnection(Connection _conn)
	    {   
    	    for(int i=0 ; i<size; i++)
        	{   
            	if( pool[i].conn == _conn )
	            {   
    	            pool[i].isActive =false;
        	        try { notifyAll(); } catch ( Exception ex ) { }
            	    return;
	            }
    	    }
	    }
	}
	private static Map pools  = new HashMap();

	private String url;
	private String username;
	private String password;
	private int initSize;

	public int getInitSize() { return initSize; }

	public static  Connection get(String name)
	{
		if( name == null ) 
			return ((SimplePool)((Map.Entry)pools.entrySet().iterator().next()).getValue()).getConnection();
		return ((SimplePool)pools.get(name)).getConnection();
	}

	public static Connection get()
	{
		return get(null);
	}

	public static void put(Connection conn)
	{
		try{
			for(Iterator iter =pools.entrySet().iterator(); iter.hasNext() ;)
			{
				((SimplePool)((Map.Entry)iter.next()).getValue()).returnConnection(conn);
			}
		}catch(Exception e){}
	}

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		url      = attr.getValue("url");
		username = attr.getValue("username");
		password = attr.getValue("password");
		String _initSize = attr.getValue("poolsize");
		initSize =(_initSize == null ? 10 : Integer.parseInt(_initSize));
	}
	public void action()
	{
		try
		{
			if ( application.handler.debug )
				System.err.print("Connect to " + url);
			pools.put(name, new SimplePool(initSize,null,url,username,password));

			if (application.handler.debug)				
				System.err.println("pool of "+name+" init successed");
			if ( application.handler.debug )
				System.err.println();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
