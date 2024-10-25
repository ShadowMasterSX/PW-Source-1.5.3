
package com.goldhuman.Common.Security;

import java.util.*;
import java.io.*;
import java.lang.*;
import javax.xml.parsers.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;

import com.goldhuman.Common.Octets;

public abstract class Security implements Cloneable
{
	private static final HashMap map = new HashMap();
	private int type;

	static 
	{
		try
		{
			SAXParser sap = SAXParserFactory.newInstance().newSAXParser();
			sap.parse(
				Security.class.getResource("/config.xml").openStream(),
				new DefaultHandler()
				{
					private boolean parsing = false;
					public void startElement(String uri, String localName, String qName, Attributes attributes)
					{
						if (qName.compareTo("security") == 0)
							parsing = true;
						if (parsing && qName.compareTo("entity") == 0)
						{
							try
							{
								String cname = attributes.getValue("class").trim();
								String name  = attributes.getValue("name").trim().toUpperCase();
								String type  = attributes.getValue("type").trim();
								Security instance = (Security)Class.forName(cname).newInstance();
								instance.type = Integer.parseInt(type);
								map.put(name, instance);
								map.put(type, instance);
							}
							catch (Exception e)
							{
								e.printStackTrace();
							}
						}
					}

					public void endElement(String uri, String localName, String qName)
					{
						if (qName.compareTo("security") == 0)
							parsing = false;
					}

				}
			);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public void SetParameter(Octets o) { }
	public void GetParameter(Octets o) { }
	public Octets Update(Octets o) { return o; }
	public Octets Final (Octets o) { return o; }

	public Object clone()
	{
		try
		{
			return super.clone();
		}
		catch (Exception e) { }
		return null;
	}

	public static Security Create(String name)
	{
		Security stub = (Security)map.get(name.toUpperCase());
		return stub == null ? new NullSecurity() : (Security)stub.clone();
	}

	public static Security Create(int type)
	{
		return Create(Integer.toString(type));
	}
}
