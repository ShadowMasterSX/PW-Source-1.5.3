
package com.wanmei.db.cache;

import java.lang.ref.WeakReference;
import java.lang.ref.SoftReference;
import java.lang.ref.ReferenceQueue;

class WeakLruMap<K, V extends SoftReference>
{
	private int size;
	private int capacity;
	private final ReferenceQueue<K> rq = new ReferenceQueue<K>();
	private final Object NULL_KEY = new Object();
	private Entry<K, V>[] table;
	private final Entry<K, V> header = new Entry<K, V>((K)NULL_KEY, null, rq, -1, null);

	private static<K> int hash(K key)
	{
		int h = key.hashCode();
		h ^= (h >>> 20) ^ (h >>> 12);	// From java.util.HashMap.hash
		return h ^ (h >>> 7) ^ (h >>> 4);
	}

	private static int indexFor(int h, int length)
	{
		return h & ( length - 1 );
	}

	private static boolean eq(Object x, Object y)
	{
		return x == y || x.equals(y);
	}

	private static class Entry<K, V extends SoftReference> extends WeakReference<K>
	{
		private V val;
		private final int hash;
		private Entry<K, V> next, before, after;

		Entry(K key, V val, ReferenceQueue<K> rq, int hash, Entry<K, V> next)
		{
			super( key, rq );
			this.val  = val;
			this.hash = hash;
			this.next = next;
		}

		private void remove()
		{
			before.after = after;
			after.before = before;
		}

		private void erase()
		{
			remove();
			next   = null;
			before = null;
			after  = null;
			val.clear();
			val    = null;
		}

		private void addBefore(Entry<K, V> existingEntry)
		{
			after  = existingEntry;
			before = existingEntry.before;
			before.after = this;
			after.before = this;
		}

		private void recordAccess(WeakLruMap<K, V> m)
		{
			remove();
			addBefore(m.header);
		}
	}

	private void expungeStaleEntries()
	{
		for ( Entry<K, V> e; (e = (Entry<K, V>) rq.poll()) != null; size-- )
		{
			int i = indexFor(e.hash, table.length);
			Entry<K, V> p = table[i];
			if ( p == e )
			{
				table[i] = e.next;
			}
			else
			{
				for ( ; p.next != e; p = p.next );
				p.next = e.next;
			}
			e.erase();
		}
	}

	private void transfer(Entry[] src, Entry[] dest)
	{
		for (int j = 0; j < src.length; src[j++] = null )
			for ( Entry<K, V> next, e = src[j]; e != null; e = next )
			{
				next  = e.next;
				K key = e.get();
				if (key == null)
				{
					e.erase();
					size--;
				}
				else
				{
					int i = indexFor(e.hash, dest.length);
					e.next = dest[i];
					dest[i] = e;
				}
			}
	}

	public void setCapacity( int newCapacity )
	{
		int c = 1;
		for ( c = 1; c < newCapacity; c <<= 1 );
		capacity = c;
		expungeStaleEntries();
		while ( size > capacity )	
			remove(header.after.get());
		Entry<K, V>[] new_table = new Entry[c];
		transfer( table, new_table );
		table = new_table;
	}

	public WeakLruMap( int initCapacity )
	{
		int c = 1;
		for ( c = 1; c < initCapacity; c <<= 1 );
		this.table = new Entry[c];
		this.capacity = c;
		this.header.before = this.header.after = this.header;
	}

	public int size()
	{
		if ( size == 0 )
			return 0;
		expungeStaleEntries();
		return size;
	}

	public void put(K key, V val)
	{
		if ( key == null || val == null ) return;
		int h = hash( key );
		expungeStaleEntries();
		int i = indexFor( h, table.length );			
		for ( Entry<K, V> e = table[i]; e != null; e = e.next )
			if ( h == e.hash && eq(key, e.get()))
			{
				e.val = val;
				e.recordAccess(this);
				return;
			}

		if ( size == capacity )
			remove(header.after.get());
		(table[i] = new Entry<K,V>( key, val, rq, h, table[i] )).addBefore(header);
		size ++;
	}

	public V get(K key)
	{
		if ( key == null ) return null;
		int h = hash( key );
		expungeStaleEntries();
		int i = indexFor( h, table.length );			

		for ( Entry<K, V> e = table[i]; e != null; e = e.next )
			if ( h == e.hash && eq(key, e.get()))
			{
				e.recordAccess(this);
				return e.val;
			}
		return null;
	}

	public void remove(K key)
	{
		if ( key == null ) return;
		int h = hash( key );
		expungeStaleEntries();
		int i = indexFor( h, table.length );			

		for ( Entry<K, V> p = table[i], e = table[i]; e != null; e = (p = e).next )
			if ( h == e.hash && eq(key, e.get()))
			{
				size--;
				if (p == e)
					table[i] = e.next;
				else
					p.next = e.next;
				V val = e.val;
				e.erase();
			}
	}
/*
	public static void main(String args[])
	{
		WeakLruMap<Integer, String> map = new WeakLruMap<Integer, String>(2);
		map.put( new Integer(1), "A" );
		map.put( new Integer(2), "B" );
		map.put( new Integer(3), "C" );
		map.put( new Integer(4), "D" );
		map.put( new Integer(5), "E" );
		map.get(4);
		map.put( new Integer(6), "F" );

		//System.gc();
		
		map.remove(7);
		map.remove(5);
		map.remove(2);
		map.remove(1);
		
		System.out.println( "size = " + map.size() );
		System.out.println ( map.get(1) );
		System.out.println ( map.get(2) );
		System.out.println ( map.get(3) );
		System.out.println ( map.get(4) );
		System.out.println ( map.get(5) );
		System.out.println ( map.get(6) );

	}
*/
}


