
package com.wanmei.db.cache;

import java.util.*;
import java.net.*;
import java.io.*;

import javax.management.*;
import java.util.concurrent.atomic.*;

public abstract class AbstractCache<K extends Serializable> implements AbstractCacheMBean, Runnable
{
	private static class KeyQueue<X extends Serializable>
	{
		private int size;
		ArrayList<X> list = new ArrayList<X>();
		public KeyQueue( int size )
		{
			this.size = size;
		}

		public void update( X s )
		{
			list.add(s);
			if ( list.size() > size )
				list.remove(0);
		}

		public byte[] encode() throws IOException
		{
			ObjectOutputStream oos = null;
			try
			{
				ByteArrayOutputStream aos = new ByteArrayOutputStream();;
				oos = new ObjectOutputStream( aos );
				oos.writeObject( list );
				return aos.toByteArray();
			}
			finally
			{
				try { oos.close(); } catch ( Exception e ) { }
			}
		}

		public static <X extends Serializable> ArrayList<X> decode( byte[] data ) 
			throws IOException, ClassNotFoundException
		{
			ObjectInputStream ois = null;
			try
			{
				ois = new ObjectInputStream( new ByteArrayInputStream( data ) );
				return (ArrayList<X>) ois.readObject();
			}
			finally
			{
				try { ois.close(); } catch ( Exception e ) { }
			}
		}
	}

	protected int max_size;
	protected int partition;
	private int port;
	private InetAddress group;
	private MulticastSocket sock;
	private KeyQueue<K> eq = new KeyQueue<K>(3);

	/////////////////////////////////////////////////////////////////////////////
	// mbean
	protected AtomicLong count_put = new AtomicLong();
	protected AtomicLong count_get = new AtomicLong();
	protected AtomicLong count_hit = new AtomicLong();

	protected AtomicLong count_remove_passive = new AtomicLong();
	protected AtomicLong count_remove_active  = new AtomicLong();

	public abstract int getSize();

	// XXX synchronized
	public int  getMaxSize()            { return max_size; }
	public void setMaxSize(int max)     { max_size = max; }

	public long getCountPut()           { return count_put.get(); }
	public long getCountGet()           { return count_get.get(); }
	public long getCountHit()           { return count_hit.get(); }
	public long getCountRemovePassive() { return count_remove_passive.get(); }
	public long getCountRemoveActive()  { return count_remove_active.get(); }
	public int  getPartition()			{ return partition + 1; }

	public String getHitPercent() { return String.format("%.2f", (double)getCountHit() / getCountGet()); }

	/////////////////////////////////////////////////////////////////////////////////////////
	protected abstract void _remove( K key );

	public void remove( K key )
	{
		count_remove_active.incrementAndGet();
		try
		{
			byte [] data = null;
			synchronized(eq)
			{
				eq.update( key );
				data = eq.encode();
			}

			DatagramPacket dp = new DatagramPacket( data, data.length, group, port );
			//System.out.println("send packet " + key );
			sock.send( dp );
			_remove( key );
		}
		catch ( Exception e )
		{
			e.printStackTrace();
		}
	}

	public void run()
	{
		byte[] buf = new byte[65536];
		while ( true )
		{
			try
			{
				DatagramPacket dp = new DatagramPacket(buf, buf.length);
				sock.receive(dp);
				//System.out.println( "received" );
				for ( K key : KeyQueue.<K>decode( dp.getData()))
				{
					count_remove_passive.incrementAndGet();
					//System.out.println( "Do remove " + key );
					_remove( key );
				}
			}
			catch ( Exception e )
			{
				e.printStackTrace();
			}
		}
	}

	public AbstractCache( String ip, String local_ip, int port, int size, int partition_mod ) throws Exception
	{
		this.partition = ( 1 << partition_mod ) - 1;
		this.max_size  = size;
		this.port = port;
		group = InetAddress.getByName(ip);
		sock = new MulticastSocket(port);
		sock.setInterface( InetAddress.getByName(local_ip) );
		sock.joinGroup( group );
		// true to disable the LoopbackMode. XXX no effect on some system.
		sock.setLoopbackMode( true );
		//System.out.println( "loopback=" + sock.getLoopbackMode() );

		Thread thr = new Thread ( this );
		thr.setName( "AbstractCacheReceiver:" + ip + ":" + port );
		thr.setDaemon( true );
		thr.start();

		// register mbean "MBeanName:type=,name="
		MBeanServer server = java.lang.management.ManagementFactory.getPlatformMBeanServer();
		ObjectName objectname = new ObjectName("AbstractCache:type=" + ip + "-" + port);
		server.registerMBean(this, objectname);
	}
}
