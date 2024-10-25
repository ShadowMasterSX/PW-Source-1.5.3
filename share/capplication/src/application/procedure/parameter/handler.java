
package application.procedure.parameter;

import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	public String sql_type;
	public Class java_type;
	public boolean in;
	public boolean out;
	public int out_type = java.sql.Types.NULL;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		sql_type = attr.getValue("sql-type");
		try
		{
			java_type = Class.forName(attr.getValue("java-type"));
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		String x = attr.getValue("in");
		in = (x != null && x.compareTo("true") == 0);
		x = attr.getValue("out");
		out = (x != null && x.compareTo("true") == 0);
			
		try
		{
			out_type = Class.forName("java.sql.Types").getField(sql_type.split("[\\W]+")[0].toUpperCase()).getInt(null);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

	public void action()
	{
	}
}
