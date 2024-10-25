package com.goldhuman.web;

public class MobileKeyStorage {

	/**
	 * get used mobilekey information by userId 
	 * @param Id of user
	 * @return value null if no mobilekey used record be found; value Object[] contained all rows of relative usedmobilekeyinfo when success.
	 *         Object[][0]:userid  Object[][1]|:mobilekeyid Object[][2]:mobilekey Object[][3]:algorithm Object[4]:operatetype Object[5]:operatedate Object[6]:ip 
	 */
	public static Object[] getMobileKeyUsedByUserId(Integer userId) {
		try
		{
			Object[] rows  ;
			rows = 
				application.query.handler.get("querymobilekeyused").select("userid").execute( (new Object[]{ userId }), "auth0");
			if (rows.length == 0) {
				return null;
			} else {
				return rows;
			}
		}
		catch (Exception e) { e.printStackTrace(); }
		return null;
		
	}

	/**
	 * get bound mobilekey information by userId 
	 * @param Id of user
	 * @return value null if no mobilekey record be found; value Object[] contained one row of mobilekeyinfo when success.
	 *         Object[0]:userid   Object[1]:algorithm  Object[2]:mobilekey
	 */
	public static Object[] getMobileKeyByUserId(Integer userId) {
		try
		{
			Object[] parameters = new Object[]{userId, new Integer(0) , new Integer(0) };
			if (application.procedure.handler.get("querymobilekeybyid").execute(parameters, "auth0") == 0) {
				return parameters;
			} else {
				return null;
			}
		}
		catch (Exception e) { e.printStackTrace(); }
		return null;

	}
	
	public static int bindMobileKey( Integer userId, Integer key, Integer algorithm)	{
		
		try
		{
			Object[] parameters = new Object[] { userId, key, algorithm };
			return application.procedure.handler.get("bindmobilekey").execute(parameters, "auth0");
		}
		catch (Exception e) {  }
		return -1;
	}
	
	public static int unbindMobileKey( Integer userId)	{
		
		try
		{
			Object[] parameters = new Object[] { userId };
			return application.procedure.handler.get("unbindmobilekey").execute(parameters, "auth0"); 
		}
		catch (Exception e) { e.printStackTrace(); }
		return -1;
	}
}
