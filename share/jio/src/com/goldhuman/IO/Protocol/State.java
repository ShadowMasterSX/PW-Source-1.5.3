
package com.goldhuman.IO.Protocol;

import java.io.*;
import java.util.*;
import javax.xml.parsers.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;

import com.goldhuman.IO.Protocol.*;

public final class State
{
	private static final Map map = new HashMap();

	static
	{
		try
		{
			Parser.ParseState(map);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	private Set set = new HashSet();
	private long timeout;
	protected State(long t) { timeout = t; }
	protected State(State _s) { try { set = (HashSet)((HashSet)_s.set).clone();}
		catch(Exception ex) { ex.printStackTrace(); System.out.println("in state");System.exit(0);} timeout = _s.timeout; }
	protected void AddProtocolType(String type) { 
			set.add(type); }

	protected boolean TypePolicy(int type) { 
			return set.contains(Integer.toString(type)); 
	}
	protected boolean TimePolicy(long t)  { return timeout < 0 || t < timeout; }
	public static State Get(String name) { return (State)map.get(name.toUpperCase()); }
}
