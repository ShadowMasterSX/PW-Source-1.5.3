
package application.table.column;
import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	private String  sql_type;
	private Class	java_type;
	private boolean not_null = false;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		sql_type = attr.getValue("sql-type");
		try
		{
			java_type = Class.forName(attr.getValue("java-type"));
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		String nn = attr.getValue("not-null");
		not_null = ( nn != null && nn.compareTo("true") == 0 );
	}

	public void action()
	{
	}

	public String toString() { return name + " " + sql_type + ( not_null ? " NOT " : " " ) + "NULL"; }

	public Class type() { return java_type; }
}
