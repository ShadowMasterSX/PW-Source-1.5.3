
package application.driver;

import com.goldhuman.xml.*;
import java.sql.*;

public class handler extends xmlobject
{
	public void action()
	{
		if ( application.handler.debug )
			System.err.println("Load Driver " + name);

		try
		{
			DriverManager.registerDriver((Driver)Class.forName(name).newInstance());
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
