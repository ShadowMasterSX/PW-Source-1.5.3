package com.goldhuman.IO.Protocol;

import com.goldhuman.Common.*;
import com.goldhuman.Common.Marshal.*;
import com.goldhuman.IO.Protocol.*;

import java.util.*;


public class Rpc extends Protocol
{
	private static Map map = Collections.synchronizedMap(new HashMap());
	private static HouseKeeper housekeeper = new HouseKeeper();
	private XID xid = new XID();
	private TimerObserver.WatchDog timer = new TimerObserver.WatchDog();
	protected Data argument;
	protected Data result;
	protected long time_policy;
	
	private static class HouseKeeper implements Observer
	{
		public HouseKeeper()
		{
			TimerObserver.GetInstance().addObserver(this);
		}
		public void update(Observable o, Object arg)
		{
			ArrayList tmp = new ArrayList();
			synchronized(map)
			{
				for(Iterator it = map.entrySet().iterator(); it.hasNext(); )
				{
					Rpc rpc = (Rpc)((Map.Entry)it.next()).getValue();
					if (rpc.time_policy < rpc.timer.Elapse())
					{
						tmp.add(rpc);
						it.remove();
					}
				}
			}
			for ( Iterator it = tmp.iterator(); it.hasNext(); ((Rpc)(it.next())).OnTimeout() );
		}
	}

	private static class XID implements Marshal, Cloneable
	{
		public int count = 0;
		private boolean is_request = true;
		private static int xid_count = 0;
		private static Object xid_locker = new Object();

		public OctetsStream marshal(OctetsStream os)
		{
			return os.marshal(is_request ? count|0x80000000 : count&0x7fffffff);
		}

		public OctetsStream unmarshal(OctetsStream os) throws MarshalException
		{
			count = os.unmarshal_int();
			is_request = ((count&0x80000000) != 0);
			return os;
		}

		public boolean IsRequest()
		{
			return is_request;	
		}

		public void ClrRequest()
		{
			is_request = false;
		}

		public void SetRequest()
		{
			is_request = true;
			synchronized(xid_locker)
			{
				count = xid_count ++;
			}
		}

		public Object clone()
		{
			try
			{
				return super.clone();
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
			return null;
		}

		public boolean equals(Object o)
		{
			return (((XID)o).count&0x7fffffff) == (count&0x7fffffff);
		}

		public int hashCode()
		{
			return count & 0x7fffffff;
		}
	}

	public static abstract class Data implements Marshal, Cloneable
	{
		protected Object clone()
		{
			try
			{
				Data data = (Data)super.clone();
				return data;
			}
			catch(Exception e)
			{
			}
			return null;
		}

		public static class DataVector extends java.util.Vector implements Marshal, Cloneable
		{
			protected Data stub;
			private DataVector(){}

			public Object clone()
			{
				try
				{
					DataVector v = (DataVector)super.clone();
					v.stub = (Data)stub.clone();
					return v;
				}
				catch(Exception e)
				{
				}
				return null;
			}

			public DataVector(Data d)
			{
				stub = d;
			}

			public OctetsStream marshal(OctetsStream os)
			{
				os.compact_uint32((int)size());
				for(int i=0;i<size();i++)
					((Marshal)(get(i))).marshal(os);
				return os;
			}

			public OctetsStream unmarshal(OctetsStream os) throws MarshalException
			{
				int size = os.uncompact_uint32();
				for(int i=0;i<size;i++)
				{
					Data d  = (Data)stub.clone();
					d.unmarshal(os);
					add(d);
				}
				return os;
			}

		}
	}


	public OctetsStream marshal(OctetsStream os)
	{
		os.marshal(xid);
		os.marshal(xid.IsRequest() ? argument : result);
		return os;
	}

	public OctetsStream unmarshal(OctetsStream os) throws MarshalException
	{
		os.unmarshal(xid);
		if (xid.IsRequest()) return os.unmarshal(argument);
		Rpc rpc = (Rpc)map.get(xid);
		if (rpc != null) os.unmarshal(rpc.result);
		return os;
	}

	public Object clone()
	{
		try
		{
			Rpc rpc = (Rpc)super.clone();
			rpc.xid = (XID)xid.clone();
			rpc.timer = new TimerObserver.WatchDog();
			rpc.argument = (Data)argument.clone();
			rpc.result = (Data)result.clone();
			return rpc;
		}
		catch(Exception e)
		{
		}
		return null;
	}

	public void Process(Manager manager, Session session) throws ProtocolException
	{
		if (xid.IsRequest())
		{
			int origin_prior_policy	= prior_policy;
			Server(argument, result, manager, session);
			if ( prior_policy == origin_prior_policy )
			{
				xid.ClrRequest();
				manager.Send(session, this);
			}
			else
				Task.Dispatch( manager, session, this );
			/*
			Server(argument, result, manager, session);
			xid.ClrRequest();
			manager.Send(session, this);
			*/
			return;
		}
		Rpc rpc = (Rpc)map.remove(xid);
		if (rpc != null) rpc.Client(rpc.argument, rpc.result);
	}

	protected void Server(Data argument, Data result) throws ProtocolException { }
	protected void Server(Data argument, Data result, Manager manager, Session session) throws ProtocolException 
	{ 
		Server(argument, result);
	}
	protected void Client(Data argument, Data result) throws ProtocolException { }
	protected void OnTimeout() { }

	private static Rpc Call(Rpc rpc, Data arg)
	{
		rpc.xid.SetRequest();
		rpc.argument = arg;
		map.put(rpc.xid, rpc);
		return rpc;
	}

	public static Rpc Call(int type, Data arg)
	{
		return Call((Rpc)Protocol.Create(type), arg);
	}

	public static Rpc Call(String type, Data arg)
	{
		return Call((Rpc)Protocol.Create(type), arg);
	}
}
