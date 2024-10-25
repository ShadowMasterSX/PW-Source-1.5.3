
package com.wanmei.db.cache;

public class Test
{
	public static void main(String args[]) throws Exception
	{
		DirectCache<String, AccountBean> c = new DirectCache<String, AccountBean>( "225.1.1.1", args[0], 4444, 100, 0 );

		if ( args[1].equals("mem") )
		{
			int max = 1000000;
			c.setMaxSize(max);
			for (int i = 0; i < max; ++i)
			{
				AccountBean b = new AccountBean();
				b.setName("name_" + i);
				b.setId(i);
				c.put( b.getName(), b );
			}
			System.out.println("size=" + c.getSize());
			System.out.println("press enter to exit");
			System.in.read();
		}
		else if ( args[1].compareTo("load") == 0 )
		{
			AccountBean b1 = new AccountBean();
			AccountBean b2 = new AccountBean();
			AccountBean b3 = new AccountBean();

			b1.setName("first");
			b1.setId(1);

			b2.setName("second");
			b2.setId(2);

			b3.setName("third");
			b3.setId(3);

			c.put( b1.getName(), b1 );
			c.put( b2.getName(), b2 );
			c.put( b3.getName(), b3 );

			while ( true )
			{
				try { Thread.sleep(1000); } catch ( Exception e ) { }
				System.out.println ( "first:" + c.get(b1.getName()) );
				System.out.println ( "second:" + c.get(b2.getName()) );
				System.out.println ( "third:" + c.get(b3.getName()) );
				System.out.println ();
			}
		}
		else if ( args[1].compareTo("first") == 0 )
		{
			c.remove("first");
		}
		else if ( args[1].compareTo("second") == 0 )
		{
			c.remove("second");
		}
		else if ( args[1].compareTo("third") == 0 )
		{
			c.remove("third");
		}
		else 
		{
			c.remove("first");
			try { Thread.sleep(5000); } catch ( Exception e ) { }
			c.remove("third");
			try { Thread.sleep(5000); } catch ( Exception e ) { }
			c.remove("second");
			try { Thread.sleep(5000); } catch ( Exception e ) { }
		}
	}
}
