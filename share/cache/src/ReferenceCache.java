
package com.wanmei.db.cache;

import java.io.Serializable;
import java.lang.ref.SoftReference;
	
public class ReferenceCache<K extends Serializable, V> extends AbstractCache<K>
{
	private final WeakLruMap<K, SoftReference<V> >[] cache;

	public int getSize()
	{
		int size = 0;
		for ( WeakLruMap<K, SoftReference<V> > map : cache )
			synchronized ( map ) { size += map.size(); }
		return size;
	}

	public void setMaxSize(int max)
	{
		super.setMaxSize(max);
		for ( WeakLruMap<K, SoftReference<V> > map : cache )
			synchronized ( map ) { map.setCapacity(max); }
	}

	public SoftReference<V> put( K key, V val )
	{
		return put( key, new SoftReference<V>(val) );
	}

	public SoftReference<V> put( K key, SoftReference<V> sr )
	{
		count_put.incrementAndGet();
		WeakLruMap<K, SoftReference<V> > map = cache[key.hashCode() & partition];
		synchronized( map )
		{
			map.put( key, sr );
		}
		return sr;
	}

	public V get( K key )
	{
		count_get.incrementAndGet();
		WeakLruMap<K, SoftReference<V> > map = cache[key.hashCode() & partition];
		synchronized( map )
		{
			SoftReference<V> sr = map.get(key);
			if ( sr == null )
				return null;
			V val = sr.get();
			if ( val == null )
				map.remove(key);
			else
				count_hit.incrementAndGet();
			return val;
		}
	}

	protected void _remove( K key )
	{
		WeakLruMap<K, SoftReference<V> > map = cache[key.hashCode() & partition];
		synchronized( map )
		{
			map.remove( key );
		}
	}

	public ReferenceCache( String ip, String local_ip, int port, int size, int partition_mod ) throws Exception
	{
		super( ip, local_ip, port, size, partition_mod );
		partition_mod = 1 << partition_mod;
		cache = new WeakLruMap[partition_mod];
		for ( int i = 0; i < partition_mod; i++ )
			cache[i] = new WeakLruMap<K, SoftReference<V> > (getMaxSize());
	}
}
