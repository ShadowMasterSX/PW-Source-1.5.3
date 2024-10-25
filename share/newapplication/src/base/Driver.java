package com.wanmei.db;

import org.w3c.dom.*;
import java.sql.*;

class Driver
{
	private Application application;

	Driver( Application app, Element elem ) throws Exception
	{
		application = app;
		DriverManager.registerDriver((java.sql.Driver)Class.forName(elem.getAttribute("name")).newInstance());
	}
}
