package com.wanmei.db;

import org.w3c.dom.*;
import java.io.*;
import java.util.*;

public class Cache
{
	private Application application;
	private String name;
	private String type;
	private String ip;
	private int port;
	private String local_ip;
	private int size;
	private int partition;
	private String key;
	private String key_java_type;
	private String val_java_type;
	private String cachevalue = "";

	Cache( Application app, Element elem ) throws Exception
	{
		application = app;
		name = elem.getAttribute("name");
		type = elem.getAttribute("type");
		ip   = elem.getAttribute("ip");
		port = Integer.parseInt(elem.getAttribute("port"));
		local_ip = elem.getAttribute("local_ip");
		size = Integer.parseInt(elem.getAttribute("size"));
		partition = Integer.parseInt(elem.getAttribute("partition"));
	}

	void setKeyName( String name ) { key = name; }
	void setKeyType( String type ) { key_java_type = type; }
	void setValType( String type, String cv)
	{
		val_java_type = type;
		cachevalue = cv;
		application.registerCache( type, this );
	}

	public String getTypeString()
	{
		StringBuffer sb = new StringBuffer();
		if ( isDirect() )
			sb.append( "DirectCache" );
		else
			sb.append( "ReferenceCache" );

		if (!key_java_type.endsWith("String")
			&& !key_java_type.endsWith("Integer")
			)
		{
			throw new RuntimeException("Cache: unsupport key-type - " + key_java_type);
		}

		sb.append( "<" + key_java_type + "," + toValType(Query.ismulti(cachevalue), val_java_type) + ">" );
		return sb.toString();
	}

	static void makeHolder( Application app, String beans, String beanbase ) throws Exception
	{
		Map<String, Cache> cache_map = app.getCacheMap();
		Map<String, List<Cache> > cache_val_map = app.getCacheValMap();

		if ( cache_map.size() == 0 )
			return;
		PrintStream ps = new PrintStream( new FileOutputStream( new File( beanbase + "/" + "CacheHolder.java" ) ) );
		ps.println ( "package " + beans + ";" );
		ps.println ( "import " + beans + ".query.*;" );
		ps.println ( "import " + beans + ".procedure.*;" );
		ps.println ( "import java.util.List;" );
		ps.println ( "import java.lang.ref.*;");
		ps.println ( "import com.wanmei.db.cache.*;");
		ps.println ( "import com.wanmei.db.*;");
		ps.println ( "public final class CacheHolder");
		ps.println ( "{" );

		for ( Cache     i :     cache_map.values() )
		ps.println( "public static " + i.getTypeString() + i.getName() + ";" );

		ps.println ( "static" );
		ps.println ( "{" );
		ps.println ( "	try" );
		ps.println ( "	{" );
		ps.println ( "		Application app = ApplicationThreadFrameWork.getApplication();" );
		ps.println ( "		Cache c = null;" );

		for ( Cache     i :     cache_map.values() )
		{
		ps.println ( "		c = app.getCache(\"" + i.getName() + "\");" );
		ps.println ( "		" + i.getName() + " = new " + i.getTypeString());
		ps.println ( "			(c.getIp(),c.getLocalIp(),c.getPort(),c.getSize(),c.getPartition());" );
		}
		ps.println ( "	}" );
		ps.println ( "	catch(Exception e)" );
		ps.println ( "	{" );
		ps.println ( "		e.printStackTrace();" );
		ps.println ( "	}" );
		ps.println ( "}" );

		// generate update
		for ( String  valTypeName : cache_val_map.keySet() )
		{
			int cvm = 0;
			Query q = app.getQuery(app.toQueryName(valTypeName));
			if (null != q) cvm = q.getCacheValueMode();

			if (2 == cvm) // empty
			{
				List<Cache> lc = cache_val_map.get(valTypeName);
				if (lc.size() != 1)
					throw new RuntimeException("only one index support with empty cachevalue");

				Cache i = lc.get(0);
				if (false == i.isDirect())
					throw new RuntimeException("only direct cache support empty cachevalue");

				ps.println(String.format(
					"public static void update%s(%s k, List<%s> v)\n{\n%s.put(k, v);\n}\n"
					, valTypeName, i.getKeyType(), i.getValueType(), i.getName()));
				continue;
			}

			boolean ismulti = (1 == cvm);
			ps.println( "public static void update"+valTypeName+"(" + toValType(ismulti, valTypeName) + " val)" );
			ps.println ( "{" );
			List<Cache> list = new ArrayList<Cache>();
			for ( Cache i : cache_val_map.get(valTypeName) )
			{
				if ( false == i.isDirect() )
				{
					list.add( i );
					continue;
				}
				ps.println( makePutCode(ismulti, app, i, "val") );
			}
			if ( list.size() > 0 )
			{
				Cache i = list.remove(0);
				ps.println( "SoftReference<" + valTypeName + "> rval = " + makePutCode(ismulti, app, i, "val") );
			}
			for ( Cache i : list )
			{
				ps.println( makePutCode(ismulti, app, i, "rval") );
			}
			ps.println ( "}" );
		}
		ps.println ( "}" );
		ps.close();
	}

	boolean isDirect() { return type.compareToIgnoreCase( "direct" ) == 0; }
	public String getName() { return name; }
	public String getKeyName() { return key; }
	public String getKeyType() { return key_java_type; }
	public String getValueType() { return val_java_type; }
	public String getIp() { return ip; }
	public String getLocalIp() { return local_ip; }
	public Integer getPort() { return port; }
	public Integer getSize() { return size; }
	public Integer getPartition() { return partition; }

	static String toValType(boolean ismulti, String valTypeName)
	{
		return ismulti ? ("List<" + valTypeName + ">") : valTypeName;
	}

	static String makePutCode(boolean ismulti, Application app, Cache i, String valVarName)
	{
		String p = i.getName() + ".put( ";
		p = p + (ismulti ? "val.get(0).get" : "val.get");
		p = p + app.translateBeanName(i.getKeyName()) + "()";
		return p + ", " + valVarName + " );";
	}
}

