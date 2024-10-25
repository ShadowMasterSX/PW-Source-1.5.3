
package com.wanmei.db.cache;

import java.util.Map;
import java.util.LinkedHashMap;
import java.io.Serializable;

public class DirectCache<K extends Serializable, V> extends AbstractCache<K>
{
	private final Map<K, V>[] cache;

	public int getSize()
	{
		int size = 0;
		for ( Map<K, V> map : cache )
			synchronized ( map ) { size += map.size(); }
		return size;
	}

	public void put( K key, V val )
	{
		count_put.incrementAndGet();
		Map<K, V> map = cache[key.hashCode() & partition];
		synchronized ( map )
		{
			map.put( key, val );
		}
	}

	public V get( K key )
	{
		count_get.incrementAndGet();
		Map<K, V> map = cache[key.hashCode() & partition];
		synchronized ( map )
		{
			V v = map.get(key);
			if (null != v)
				count_hit.incrementAndGet();
			return v;
		}
	}

	protected void _remove( K key )
	{
		Map<K, V> map = cache[key.hashCode() & partition];
		synchronized( map )
		{
			map.remove( key );
		}
	}

	public DirectCache( String ip, String local_ip, int port, int size, int partition_mod ) throws Exception
	{
		super( ip, local_ip, port, size, partition_mod );
		partition_mod = 1 << partition_mod;
		cache = new Map[partition_mod];
		for ( int i = 0; i < partition_mod; i++ )
			cache[i] = new LinkedHashMap<K, V> (16, 0.75f, true)
			{
				protected boolean removeEldestEntry(Map.Entry eldest) { return size() > max_size; }
			};
	}
}
