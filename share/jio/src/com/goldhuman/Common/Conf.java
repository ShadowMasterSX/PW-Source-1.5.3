
package com.goldhuman.Common;

import java.io.*;
import java.util.*;
import java.net.*;

public final class Conf
{
	private static Conf instance;
	private File    conffile; 
	private long    mtime;
	private HashMap confhash;
	private String  charset=new String("GBK");

	static { instance = new Conf(); }

	private Conf () { confhash = new HashMap(); }

	private void parse(BufferedReader br) throws IOException
	{
		String  section = null;
		HashMap sechash = new HashMap();
		confhash.clear();
		while (br.ready())
		{
			String line = br.readLine().trim();
			if (line.length() == 0) continue;
			char c = line.charAt(0);
			if (c == '#' || c == ';') continue;
			
			if (c == '[')
			{
				line = line.substring(1, line.indexOf(']')).trim();
				if (section != null)
				{
					confhash.put(section, sechash);
					sechash = new HashMap();
				}
				section = line;
			}
			else
			{
				String[] key_value = line.split("=", 2);
				sechash.put(key_value[0].trim(), key_value[1].trim());
			}
		}
		if (section != null)
			confhash.put(section, sechash);
	}

	private void reload()
	{
		try
		{
			for (long last = conffile.lastModified(); last != mtime; last = conffile.lastModified())
			{
				mtime = last;
				URL url=conffile.toURI().toURL();
				BufferedReader br = new BufferedReader(new InputStreamReader(url.openStream(),charset));
				parse(br);
				br.close();
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public synchronized String find(String section, String key)
	{
		HashMap sechash = (HashMap)confhash.get(section);
		if (sechash != null)
		{
			String val = (String)sechash.get(key);
			if (val != null)
				return new String(val);
		}
		return new String();
	}
	public synchronized void put( String section, String key, String val)
	{
		HashMap sechash = (HashMap)confhash.get(section);
		if( null == sechash)
		{
			sechash = new HashMap();
			confhash.put( section, sechash);
		}
		sechash.put( key, val);
	}
	
	public synchronized static Conf GetInstance(String filename,String charset)
	{
		if ( charset!=null )
			instance.charset=charset;
		File file = new File(filename);
		if (file.isFile())
		{
			instance.conffile = file;
			instance.reload();
		}
		return instance;
	}

	public synchronized static Conf GetInstance(URL url,String charset)
	{
		try
		{
			if ( charset!=null )
				instance.charset=charset;
			BufferedReader br = new BufferedReader(new InputStreamReader(url.openStream(),instance.charset));
			instance.parse(br);
			br.close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		return instance;
	}

	public synchronized static Conf GetInstance() { return instance; }

	public static void main(String args[])
	{
		Conf.GetInstance(args[0],null);
		System.out.println(Conf.GetInstance().find(args[1], args[2]));
	}
}
