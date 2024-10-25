
package com.goldhuman.IO.Protocol;

import java.util.*;
import java.net.*;
import java.io.*;
import com.goldhuman.Common.Marshal.*;
import com.goldhuman.IO.*;

public abstract class Protocol implements Marshal, Cloneable
{
	private static final Map map = new HashMap();
	private static final com.goldhuman.Common.Counter counter = new com.goldhuman.Common.Counter("Protocol");

	protected int type;
	protected int size_policy;
	protected int prior_policy;
	protected long time_wait = 0;

	static
	{
		try
		{
			Parser.ParseProtocol(map);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		try
		{
			Parser.ParseRpc(map);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public Object clone()
	{
		try
		{
			return super.clone();
		}
		catch (Exception e) { 
			e.printStackTrace();
		}
		return null;
	}

	public static Protocol GetStub(String name)
	{
		return (Protocol)map.get(name.toUpperCase());
	}

	public static Protocol GetStub(int type)
	{
		return GetStub(Integer.toString(type));
	}

	public static Protocol Create(String name)
	{
		Protocol stub = GetStub(name);
		return stub == null ? null : (Protocol)stub.clone();
	}

	public static Protocol Create(int type)
	{
		return Create(Integer.toString(type));
	}

	public int getType()
	{
		return type;
	}

	protected void Encode(OctetsStream os)
	{
		os.compact_uint32(type).marshal(new OctetsStream().marshal(this));
	}

	protected static Protocol Decode(Stream is) throws ProtocolException
	{
		if (is.eos()) return null;
		Protocol protocol = null;
		int type = 0, size = 0;
		try
		{
			if (is.check_policy)
			{
				is.Begin();
				type = is.uncompact_uint32();
				size = is.uncompact_uint32();
				is.Rollback();
				if (!is.session.StatePolicy(type) || !is.session.manager.InputPolicy(type, size))
				{
					System.out.println( "Protocol Decode CheckPolicy Error:type=" + type + ",size="+size );
					throw new ProtocolException();
				}
				is.check_policy = false;
				is.checked_size = size;
			}

			Stream data = new Stream(is.session, is.checked_size);
			is.Begin();
			type = is.uncompact_uint32();
			is.unmarshal(data);
			is.Commit();
			if ((protocol = Create(type)) != null)
			{
				protocol.unmarshal(data);
				counter.increment(protocol.getClass().getName());
			}
			is.check_policy = true;
		}
		catch (MarshalException e)
		{
			is.Rollback();
			if (protocol != null)
			{
				System.out.println( "Protocol Decode Unmarshal Error:type=" + type + ",size="+size );
				throw new ProtocolException();
			}
			else
				System.out.println( "Protocol Decode Warning:uncomplete data,protocol type=" + type + ",size="+size );
		}
		return protocol;
	}
	public static PassiveIO Server(Manager manager) { return PassiveIO.Open(new Session(manager)); }
	public static ActiveIO  Client(Manager manager) { return ActiveIO.Open (new Session(manager)); }

	protected int PriorPolicy() { return prior_policy; }
	protected boolean SizePolicy(int x) { return x <=0 || x < size_policy; }
	public abstract void Process(Manager manager, Session session) throws ProtocolException;

	public static void main(String argv[]) 
	{
		com.goldhuman.Common.TimerObserver.GetInstance().StopTimer();
	}

}
