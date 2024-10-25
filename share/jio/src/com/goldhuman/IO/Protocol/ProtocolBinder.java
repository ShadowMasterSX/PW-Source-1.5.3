
package com.goldhuman.IO.Protocol;

import java.lang.*;
import java.util.*;

import com.goldhuman.Common.Marshal.*;

public final class ProtocolBinder extends Protocol
{
	private Vector binder = new Vector();	

	public OctetsStream marshal(OctetsStream os)
	{
		synchronized(binder)
		{
			for (Iterator it = binder.iterator(); it.hasNext(); ((Protocol)it.next()).Encode(os));
		}	
		return os;
	}

	public OctetsStream unmarshal(OctetsStream os)
	{
		Stream is = (Stream)os;
		synchronized(binder)
		{
			try
			{
				for (Protocol protocol; (protocol = Protocol.Decode(is)) != null; binder.add(protocol));
				for (Iterator it = binder.iterator(); it.hasNext(); )
					Task.Dispatch(is.session.manager, is.session, (Protocol)it.next());
			}
			catch (Exception e)
			{
				is.session.Close();
			}
		}
		return os;
	}

	public Object clone()
	{
		try
		{
			ProtocolBinder o = (ProtocolBinder)super.clone();
			synchronized(binder)
			{
				for (Iterator it = binder.iterator(); it.hasNext(); o.binder.add(((Protocol)it.next()).clone()) );
				return o;
			}
		} catch (Exception e) { }
		return null;
	}

	public void Process(Manager manager, Session session) throws ProtocolException { }

	ProtocolBinder bind(Protocol protocol)
	{
		synchronized(binder)
		{
			binder.add(protocol); 
		}
		return this;
	}
}
