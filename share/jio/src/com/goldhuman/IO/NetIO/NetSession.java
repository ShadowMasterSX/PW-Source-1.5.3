
package com.goldhuman.IO.NetIO;

import java.lang.*;
import java.net.*;
import com.goldhuman.Common.*;
import com.goldhuman.Common.Security.Security;

public abstract class NetSession implements Cloneable
{
	private static final int DEFAULTIOBUF = 8192;
	protected Octets ibuffer;
	protected Octets obuffer;
	protected Octets isecbuf;
	Security isec;
	Security osec;

	protected boolean closing = false;

	protected SocketAddress peeraddress = null;

	public synchronized void setPeerAddress(SocketAddress peer) { peeraddress = peer; }
	public synchronized SocketAddress getPeerAddress()          { return peeraddress; }

	public NetSession()
	{
		ibuffer = new Octets(DEFAULTIOBUF);
		obuffer = new Octets(DEFAULTIOBUF);
		isecbuf = new Octets(DEFAULTIOBUF);
		isec = Security.Create("NULLSECURITY");
		osec = Security.Create("NULLSECURITY");
	}

	protected boolean Output(Octets data)
	{
		if (data.size() + obuffer.size() > obuffer.capacity()) return false;
		osec.Update(data);
		obuffer.insert(obuffer.size(), data);
		return true;
	}

	protected Octets Input()
	{
		isec.Update(ibuffer);
		isecbuf.insert(isecbuf.size(), ibuffer);
		ibuffer.clear();
		return isecbuf;
	}

	public void SetISecurity(String type, Octets key)
	{
		synchronized(this)
		{
			isec = Security.Create(type);
			isec.SetParameter(key);
		}
	}

	public void SetOSecurity(String type, Octets key)
	{
		synchronized(this)
		{
			osec = Security.Create(type);
			osec.SetParameter(key);
		}
	}

	public void LoadConfig()
	{
		Conf conf = Conf.GetInstance();
		String section = Identification();
		try { ibuffer.reserve(Integer.parseInt(conf.find(section, "ibuffermax"))); } catch (Exception e) { }
		try { obuffer.reserve(Integer.parseInt(conf.find(section, "obuffermax"))); } catch (Exception e) { }
		try
		{
			SetISecurity(conf.find(section, "isec").trim(), new Octets(conf.find(section, "iseckey").getBytes()));
		}
		catch (Exception e) { }
		try
		{
			SetOSecurity(conf.find(section, "osec").trim(), new Octets(conf.find(section, "oseckey").getBytes()));
		}
		catch (Exception e) { }
	}

	protected void Close() { closing = true; }

	protected abstract void OnRecv();
	protected abstract void OnSend();
	protected abstract void OnOpen();
	protected abstract void OnClose();
	public void OnAbort() { }
	public abstract String Identification();
	public SocketAddress OnCheckAddress(SocketAddress sa) { return sa; }
	public Object clone()
	{
		try
		{
			NetSession s = (NetSession)super.clone();
			//s.ibuffer.reserve(ibuffer.capacity());
			//s.obuffer.reserve(obuffer.capacity());
			//s.isecbuf.reserve(isecbuf.capacity());

			s.ibuffer = new Octets(ibuffer.capacity());
			s.obuffer = new Octets(obuffer.capacity());
			s.isecbuf = new Octets(isecbuf.capacity());
			s.isec = (Security)isec.clone();
			s.osec = (Security)osec.clone();
			return s;
		}
		catch (Exception e) { }
		return null;
	}
}
