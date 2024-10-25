
package com.wanmei.db.cache;

/*
create table account (
		id integer not null,
		name char(16) not null,
		passwd binary(16) not null,
		creatime datetime not null,
		usertype integer not null
		) lock datarows on seg_account
go
*/

public class AccountBean
{
	private Integer id;
	private String name;
	private byte[] passwd = new byte[16];
	private int creatime;
	private int usertype;

	public void setId( Integer id ) { this.id = id; }
	public Integer getId() { return id; }

	public void setName( String name ) { this.name = name; }
	public String getName() { return name; }

	public void setPasswd( byte[] passwd ) { this.passwd = passwd; }
	public byte[] getPasswd() { return passwd; }

	public void setCreatime( int creatime ) { this.creatime = creatime; }
	public int getCreatime() { return creatime; }

	public void setUsertype( int usertype ) { this.usertype = usertype; }
	public int getUsertype() { return usertype; }

/*
	public boolean equals(Object obj) { return name.equals(((AccountBean)obj).name); }
	public int hashCode() { return name.hashCode(); }
*/
}
