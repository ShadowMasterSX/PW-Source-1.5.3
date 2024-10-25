
package com.wanmei.db;

import java.io.*;

public class ApplicationThreadFrameWork
{
	private static Application application;
	private static String default_connection_name = null;
	public static void createInstance( File file ) throws Exception
	{
		application = new Application(file);
	}

	public static void createInstance( File file, String _default_connection_name ) throws Exception
	{
		default_connection_name = _default_connection_name;
		application = new Application(file);
	}
	
	private static final ThreadLocal<Context> context = new ThreadLocal<Context> ()
	{
		protected Context initialValue()
		{
			return application.createContext();
		}
	};

	public static Context getContext()
	{
		Context ctx = context.get();
		if (null != default_connection_name)
			ctx.setConnectionName(default_connection_name);
		return ctx;
	}

	public static Context getContext(String connectionName)
	{
		Context ctx = context.get();
		ctx.setConnectionName(connectionName);
		return ctx;
	}

	public static Application getApplication() { return application; }
}
