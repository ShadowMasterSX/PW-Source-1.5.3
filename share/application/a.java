
import java.util.*;
import java.lang.*;
import java.io.*;
import java.security.*;
import java.text.*;

public class a
{
	private static void dump(String s)
	{
		try
		{
			int length = s.length();

			char [] buffer = new char[length];

			s.getChars(0, length, buffer, 0);

			for (int i = 0; i < length; i++)
			{
				System.out.print( (int) buffer[i] );
				System.out.print( ";" );
			}
			System.out.println("");
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	private static String hexString(byte[] x)
	{
		StringBuffer sb = new StringBuffer(x.length * 2);
		for ( int i = 0; i < x.length; i++ )
		{
			byte n = x[i];
			int nibble = (int)(n >> 4)&0xf;
			sb.append ( (char)(nibble >= 10 ? 'a' + nibble - 10 : '0' + nibble) );
			nibble = (int)(n & 0xf);
			sb.append ( (char)(nibble >= 10 ? 'a' + nibble - 10 : '0' + nibble) );
		}
		return sb.toString();
	}

	private static boolean addcard(Integer id, LinkedList cardn)
	{
		try
		{
			cardn.addFirst(id);
			return application.procedure.handler.get("addcard").execute( cardn.toArray(), "auth0" ) == 0;
		}
		catch(Exception e) { }
		return false;
	}

	public static void main(String[] args)
	{
/*		try
		{
			String name = "lijian";
			String passwd = "QPFASFU7DMBSY238";
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			System.out.println( hexString( md.digest() ) );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
*/
		try
		{
			String x = "帐号管理";
			dump(x);

			dump ( new String(x.getBytes(), "UTF-8") );
			dump ( new String(x.getBytes(), "GBK") );
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		try
		{
			java.security.MessageDigest md = java.security.MessageDigest.getInstance("MD5");
			md.update("中国".getBytes());
			byte[] x = md.digest();
			System.out.println(hexString(x));
		}
		catch (Exception e)
		{
		}

/*
		try
		{
			com.goldhuman.xml.parser.parse(new java.io.FileInputStream("table.xml"));

			int id = Integer.parseInt(args[0]);
			System.out.println( "adding " + args[1] + " with id=" + id );

			FileInputStream fis = new FileInputStream(args[1]);
			LineNumberReader	reader = new LineNumberReader(new InputStreamReader(fis));

			int number = 0;
			LinkedList cardn = new LinkedList();
			String line = reader.readLine();
			while( line != null )
			{
				line.trim();
				if( line.matches( "^\\d+$" ) && line.length() == 20 )
				{
					cardn.addLast( line );

					if( cardn.size() == 10 )
					{
						if ( addcard(new Integer(id), cardn) )
						{
							number += 10;
							System.out.println( "added " + number + " last " + line );
						}
						else
						{
							System.out.println( "error add card." + line );
							return;
						}
						cardn.clear();
					}
				}
				line = reader.readLine();
			}

			if( cardn.size() > 0 )
			{
				System.out.println( "ERRORERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
				if ( addcard(new Integer(id), cardn) )
				{
					number += 10;
					System.out.println( "added " + number );
				}
				else
				{
					System.out.println( "error add card." + line );
				}
				cardn.clear();
			}

		}
		catch (Exception e )
		{
			e.printStackTrace();
		}
*/
	}
}
