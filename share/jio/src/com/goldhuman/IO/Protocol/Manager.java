
package com.goldhuman.IO.Protocol;

import java.net.*;
import java.util.*;
import com.goldhuman.IO.NetIO.*;
import com.goldhuman.Common.Octets;
import com.goldhuman.Common.Marshal.*;

public abstract class Manager
{
	private Set set = Collections.synchronizedSet(new HashSet());

	protected void AddSession(Session session)
	{
		set.add(session);
		try
		{
			OnAddSession(session);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	protected void DelSession(Session session)
	{
		try
		{
			OnDelSession(session);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		set.remove(session);
	}

	protected void AbortSession(Session session)
	{
		try
		{
			OnAbortSession(session);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public boolean SetISecurity(Session session, String type, Octets key)
	{
		if (!set.contains(session)) return false;
		session.SetISecurity(type, key);	
		return true;
	}

	public boolean SetOSecurity(Session session, String type, Octets key)
	{
		if (!set.contains(session)) return false;
		session.SetOSecurity(type, key);	
		return true;
	}

	public boolean Send(Session session, Protocol protocol)
	{
		if (!set.contains(session)) return false;
		return session.Send(protocol);
	}

	public boolean Close(Session session)
	{
		if (!set.contains(session)) return false;
		session.Close();
		return true;
	}

	public boolean ChangeState(Session session, String name)
	{
		if (!set.contains(session)) return false;
		session.ChangeState(name);
		return true;
	}
	protected abstract void OnAddSession(Session session);
	protected abstract void OnDelSession(Session session);
	protected void OnAbortSession(Session session) { }
	protected abstract State GetInitState();
	protected int PriorPolicy(int type)
	{
		return Protocol.GetStub(type).PriorPolicy();
	}

	protected boolean InputPolicy(int type, int size)
	{
		return Protocol.GetStub(type).SizePolicy(size);
	}

	protected abstract String Identification();
	protected SocketAddress OnCheckAddress(SocketAddress sa) { return sa; }
	protected Manager() { }
}
