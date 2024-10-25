
package com.goldhuman.IO.Protocol;

import com.goldhuman.Common.Marshal.OctetsStream;

public final class Stream extends OctetsStream
{
	protected Session session;
	protected boolean check_policy = true;
	protected int checked_size = 0;
	protected Stream(Session s) { session = s; }
	protected Stream(Session s, int size) { super(size); session = s; }
}

