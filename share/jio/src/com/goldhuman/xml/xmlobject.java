
package com.goldhuman.xml;

import java.util.*;

public abstract class xmlobject
{
	public String name;
	public String content = "";
	public xmlobject parent;
	public xmlobject[] children = new xmlobject[0];

	protected void setchild (xmlobject child)
	{
		xmlobject[] tmp = new xmlobject[children.length+1];
		System.arraycopy(children, 0, tmp, 0, children.length);
		tmp[children.length] = child;
		children = tmp;
	}

	protected void setparent(xmlobject parent) { this.parent = parent; }

	protected void setattr(org.xml.sax.Attributes attr) { name = attr.getValue("name"); }
	public abstract void action();
}
