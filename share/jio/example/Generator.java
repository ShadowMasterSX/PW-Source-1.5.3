import com.goldhuman.IO.Protocol.Protocol;

public class Generator
{
	public static void main(String[] args)
	{
		try
		{
			Class.forName( "com.goldhuman.IO.Protocol.Protocol" );
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
