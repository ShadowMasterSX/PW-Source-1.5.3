package com.goldhuman.web;

public class PhoneStorage {

	public static Object[] getPhone(Integer userId)
	{
		try
		{
			Object[] rows;
			rows = application.query.handler.get("getUserPhone").select("byUid").execute( (new Object[]{ userId }), "auth0");
			if (rows.length == 0) {
				return null;
			} else {
				return rows;
			}
		}
		catch (Exception e) { e.printStackTrace(); }
		return null;
	}

	public static Object[] getPhoneByName(String username)
	{
		try
		{
			Object[] rows;
			rows = application.query.handler.get("getUserPhone").select("byName").execute( (new Object[]{ username }), "auth0");
			if (rows.length == 0) {
				return null;
			} else {
				return rows;
			}
		}
		catch (Exception e) { e.printStackTrace(); }
		return null;
	}

	public static Object[] getUserByPhone(String phone)
	{
		try
		{
			Object[] rows;
			rows = application.query.handler.get("getPhoneUser").select("byPhone").execute( (new Object[]{ phone }), "auth0");
			if (rows.length == 0) {
				return null;
			} else {
				return rows;
			}
		}
		catch (Exception e) { e.printStackTrace(); }
		return null;
	}

	public static int bindPhone( String username, String phone )
	{
		try
		{
			Object[] parameters = new Object[] { username, phone, new Integer(-1) };
			if( 0 == application.procedure.handler.get("bindPhone").execute(parameters, "auth0") )
				return ((Integer)parameters[2]).intValue();
		}
		catch (Exception e) { e.printStackTrace(); }
		return -1;
	}
	
	public static int unbindPhone( String username, String phone )
	{
		try
		{
			Object[] parameters = new Object[] { username, phone, new Integer(-1) };
			if( 0 == application.procedure.handler.get("unbindPhone").execute(parameters, "auth0") )
				return ((Integer)parameters[2]).intValue();
		}
		catch (Exception e) { e.printStackTrace(); }
		return -1;
	}

	public static int clearUserPhone( String username )
	{
		try
		{
			Object[] parameters = new Object[] { username, new Integer(-1) };
			if( 0 == application.procedure.handler.get("clearUserPhone").execute(parameters, "auth0") )
				return ((Integer)parameters[1]).intValue();
		}
		catch (Exception e) { e.printStackTrace(); }
		return -1;
	}

}

