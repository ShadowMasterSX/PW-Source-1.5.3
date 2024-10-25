
package application.query.select.cache;

import com.goldhuman.Common.*;
import com.goldhuman.xml.*;

public class handler extends xmlobject
{
	private static String DEFAULT_KEY = "DEFAULT";
	private int size = Cache.default_size;
	private int timeout = Cache.default_timeout;
	public Cache cache;

	protected void setattr(org.xml.sax.Attributes attr)
	{
		super.setattr(attr);
		String x = attr.getValue("size");
		if ( x != null ) size = Integer.parseInt(x);
		x = attr.getValue("timeout");
		if ( x != null ) timeout = Integer.parseInt(x);
	}

	public void action()
	{
		Object[] parameter = ((application.query.select.handler)parent).parameter;

		int len = parameter.length;

		if ( len == 0 )
			len = 1; 

		int[] key_pos = new int[len];
		for (int i = 0; i < len; i++) key_pos[i] = i;

		cache = Cache.Create(parent.parent.name + "_" + parent.name, len + 1, key_pos, size, timeout);
	}

	public Object[] search(Object[] parameter)
	{
		try
		{
			Cache.Item item = cache.newItem();
			if ( parameter.length > 0 )
			{
				for (int i = 0; i < parameter.length; i++)
					item.set(i, parameter[i]);
				return (Object[]) cache.find(item).get( parameter.length );
			}
			return (Object[]) cache.find(item.set(0, DEFAULT_KEY)).get( 1 );
		}
		catch (Exception e)
		{
		}
		return null;
	}

	public void append(Object[] parameter, Object data)
	{
		try
		{
			Cache.Item item = cache.newItem();
			if ( parameter.length > 0 )
			{
				for (int i = 0; i < parameter.length; i++)
					item.set(i, parameter[i]);
				item.set(parameter.length, data).commit();
			}
			else
				item.set(0, DEFAULT_KEY).set(1, data).commit();
		}
		catch (Exception e)
		{
		}
	}
}
