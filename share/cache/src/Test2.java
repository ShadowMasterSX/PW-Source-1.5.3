
package com.wanmei.db.cache;

public class Test2
{
	public static void main(String args[]) throws Exception
	{
		int max = 20000;
		ReferenceCache<String, AccountBean>pc=new ReferenceCache<String, AccountBean>
			("225.1.1.1",args[0],4444,max,0);
		ReferenceCache<Integer,AccountBean>ic=new ReferenceCache<Integer,AccountBean>
			("225.1.1.2",args[0],4444,max,0);
		if ( args[1].equals("mem") )
		{
			for (int i = 0; i < max; ++i)
			{
				AccountBean b = new AccountBean();
				b.setName("name_" + i);
				b.setId(i);
				ic.put( b.getId(), pc.put( b.getName(), b ));
			}
			//for (int i = 0; i < max; ++i) ic.get(i);
			//System.gc();
			System.out.println("size=" + pc.getSize() + " " + ic.getSize());
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

			ic.put( b1.getId(), pc.put( b1.getName(), b1 ));
			ic.put( b2.getId(), pc.put( b2.getName(), b2 ));
			ic.put( b3.getId(), pc.put( b3.getName(), b3 ));

			while ( true )
			{
				try { Thread.sleep(1000); } catch ( Exception e ) { }
				System.out.println ( "first:" +  pc.get(b1.getName()) );
				System.out.println ( "second:" + pc.get(b2.getName()) );
				System.out.println ( "third:" +  pc.get(b3.getName()) );
				System.out.println ( "first:" +  ic.get(b1.getId()) );
				System.out.println ( "second:" + ic.get(b2.getId()) );
				System.out.println ( "third:" +  ic.get(b3.getId()) );
				System.out.println ( pc.getSize() + " " + ic.getSize() );
				//pc.remove("first");
				//ic.remove(2);
			}
		}
		else if ( args[1].compareTo("first") == 0 )
		{
			pc.remove("first");
		}
		else if ( args[1].compareTo("second") == 0 )
		{
			pc.remove("second");
		}
		else if ( args[1].compareTo("third") == 0 )
		{
			pc.remove("third");
		}
		else if ( args[1].compareTo("1") == 0 )
		{
			ic.remove( Integer.parseInt("1") );
		}
		else if ( args[1].compareTo("2") == 0 )
		{
			ic.remove( Integer.parseInt("2") );
		}
		else if ( args[1].compareTo("3") == 0 )
		{
			ic.remove( Integer.parseInt("3") );
		}

		/*
		else 
		{
			c.remove("first");
			try { Thread.sleep(5000); } catch ( Exception e ) { }
			c.remove("third");
			try { Thread.sleep(5000); } catch ( Exception e ) { }
			c.remove("second");
			try { Thread.sleep(5000); } catch ( Exception e ) { }
		}
		*/
	}
}
