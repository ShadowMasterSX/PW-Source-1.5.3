
package com.goldhuman.account;

import java.io.*;
import java.security.*;
import java.util.*;
import java.text.*;
import com.goldhuman.xml.*;


public class storage
{
	final static private java.text.DateFormat datefmt = new java.text.SimpleDateFormat("yyyy-MM-dd");
	final static private java.text.DateFormat datefmt2 = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	final static private java.text.DateFormat datefmt3 = new java.text.SimpleDateFormat("yyyy.MM.dd HH:mm:ss");

	public static String getNameById(Integer id)
	{
		try
		{
			Object[] rows = application.query.handler.get("getUsername").select("byId").execute(new Object []{ id }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (String)((Object[])rows[0])[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	/*
	public static Integer getIdByName(String name)
	{
		try
		{
			Object[] rows = application.query.handler.get("getUserid").select("byName").execute(new Object []{ name.toLowerCase() }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (Integer)((Object[])rows[0])[0];
		}
		catch (Exception e) { }
		return null;
	}	
	public static Object[] getUserInfobyName(String name)
	{
		try
		{
			Object[] rows = application.query.handler.get("getUserInfo").select("byName").execute(new Object[]{name.toLowerCase() }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] acquireIdPasswd2(String account)
	{
		try
		{
			Object[] rows = application.query.handler.get("acquireIdPasswd2").select("byName").execute(new Object []{ account.toLowerCase() }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] acquireIdPasswd(String account)
	{
		try
		{
			Object[] rows = application.query.handler.get("acquireIdPasswd").select("byName").execute(new Object []{ account.toLowerCase() }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	*/

	//for add new procedure  ljh
	public static Object[] acquireIdPasswd(String account)
	{
		try
		{
			Integer uid = new Integer(0);
			byte[] passwd = null;
			String passwdstring = "";
			Integer creatime = new Integer(0);
			Object[] parameters = new Object[] { account.toLowerCase(), uid, passwdstring };
			if( application.procedure.handler.get("acquireuserpasswd").execute(parameters,"auth0")==0 )
			{
				uid = (Integer)parameters[1];
				passwdstring = (String)parameters[2];
				if( null != passwdstring )
					passwd = StringPassword( passwdstring );

				if( null == uid || null == passwd || uid.intValue() == 0 )
				{
					System.out.println("acquireIdPasswd procedure return null:account="+account+",uid="+uid+",passwd="+passwd);
					return null;
				}
				return new Object[] { uid, passwd, creatime };
			}
			System.out.println("acquireIdPasswd procedure failed:account="+account);
			return null;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		System.out.println("acquireIdPasswd exception:account="+account);
		return null;
	}

	public static Object[] acquireIdPasswd2(String account)
	{
		try
		{
			Integer uid = new Integer(0);
			byte[] passwd = null;
			String passwdstring = "";
			Object[] parameters = new Object[] { account.toLowerCase(), uid, passwdstring };
			if( application.procedure.handler.get("acquireuserpasswd2").execute(parameters,"auth0")==0 )
			{
				uid = (Integer)parameters[1];
				passwdstring = (String)parameters[2];
				if( null != passwdstring )
					passwd = StringPassword( passwdstring );

				if( null == uid || null == passwd || uid.intValue() == 0 )
				{
					System.out.println("acquireIdPasswd2 procedure return null:account="+account+",uid="+uid+",passwd="+passwd);
					return null;
				}
				return new Object[] { uid, passwd };
			}
			System.out.println("acquireIdPasswd2 procedure failed:account="+account);
			return null;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		System.out.println("acquireIdPasswd2 exception:account="+account);
		return null;
	}

	public static Integer getIdByName(String name)
	{
		try
		{
			Integer uid = new Integer(0);
			Object[] parameters = new Object[] { name.toLowerCase(), uid };
			if (application.procedure.handler.get("getuseridbyname").execute(parameters, "auth0")==0)
			{
				uid = (Integer)parameters[1];
				if (null == uid || 0 == uid.intValue())
				{
					System.out.println("getIdByName uid is fail. name = " + name);
					return null;
				}
				return uid;
			}

			System.out.println("getIdByName execute false. name = " + name);
			return null;
		}
		catch (Exception e) { e.printStackTrace(System.out); } 
		return null;
	}

	public static Object[] getUserInfobyName(String name)
	{
		try
		{
			Integer id = new Integer(0);
			String prompt = "";
			String answer = "";
			String truename = "";
			String idnumber = "";
			String email = "";
			String mobilenumber = "";
			String province = "";
			String city = "";
			String phonenumber = "";
			String address = "";
			String postalcode = "";
			Integer gender = new Integer(0);
			String birthday = "";
			String creatime = "";
			String qq = "";


			Object[] parameters = new Object[] 
				{ 
					name.toLowerCase(), id, prompt, answer, truename, 
					idnumber, email, mobilenumber, province, city, 
					phonenumber, address, postalcode, gender, birthday, 
					creatime, qq
				};
			Integer ret =  application.procedure.handler.get("getuserinfobyname").execute( parameters, "auth0" );
			if (0 == ret) 
			{
				id = (Integer)parameters[1];
				prompt = (String)parameters[2];
				answer = (String)parameters[3];
				truename = (String)parameters[4];
				idnumber = (String)parameters[5];
				email = (String)parameters[6];
				mobilenumber = (String)parameters[7];
				province = (String)parameters[8];
				city = (String)parameters[9];
				phonenumber = (String)parameters[10];
				address = (String)parameters[11];
				postalcode = (String)parameters[12];
				gender = (Integer)parameters[13];
				birthday = (String)parameters[14];
				creatime = (String)parameters[15];
				qq = (String)parameters[16];

				if (null == id || 0 == id.intValue())
				{
					System.out.println("getUserInfoByName id is fail. name =" + name + " ret = " + ret);
					return null;
				}

				java.util.Date ret_birthday = null;
				java.sql.Timestamp ret_creatime = null;
				if (birthday != null && birthday.length() > 0)
					ret_birthday = datefmt2.parse(birthday);
				else
					ret_birthday = new java.util.Date(0);
				if (creatime != null && birthday.length() > 0)
					ret_creatime = new java.sql.Timestamp(datefmt2.parse(creatime).getTime());
				else 
					ret_creatime = new java.sql.Timestamp(0);

				return new Object[] { id, prompt, answer, truename, idnumber, 
					email, mobilenumber, province, city, phonenumber, 
					address, postalcode, gender, ret_birthday, ret_creatime,
					qq };
			}
			System.out.println("getUserInfoByName return false. name =" + name + " ret = " + ret);
			return  null;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		System.out.println("getUserInfoByName execute fail. name =" + name);
		return null;
	}

	public static boolean addUser(String name, String passwd, String prompt,String answer,String truename,String idnumber,String email,String mobilenumber,String province,String city,String phonenumber,String address,String postalcode,Integer gender,String birthday,String qq,String passwd2 )
	{
		try
		{
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());

			MessageDigest md2 = MessageDigest.getInstance("MD5");
			md2.update(name.toLowerCase().getBytes());
			md2.update(passwd2.getBytes());

			if ( prompt == null ) prompt = "";
			if ( answer == null ) answer = "";
			if ( truename == null ) truename = "";
			if ( idnumber == null ) idnumber = "";
			if ( email == null ) email = "";
			if ( mobilenumber == null ) mobilenumber = "";
			if ( province == null ) province = "";
			if ( city == null ) city = "";
			if ( phonenumber == null ) phonenumber = "";
			if ( address == null ) address = "";
			if ( postalcode == null ) postalcode = "";
			if ( gender == null ) gender = new Integer(2);
			if ( birthday == null ) birthday = "";
			if ( qq == null ) qq = "";
			Object[] parameters = new Object[] { name.toLowerCase(), md.digest(), prompt.getBytes("UTF-16LE"), answer.getBytes("UTF-16LE"), truename.getBytes("UTF-16LE"), idnumber.getBytes("UTF-16LE"), email.getBytes("UTF-16LE"), mobilenumber.getBytes("UTF-16LE"), province.getBytes("UTF-16LE"), city.getBytes("UTF-16LE"), phonenumber.getBytes("UTF-16LE"), address.getBytes("UTF-16LE"), postalcode.getBytes("UTF-16LE"), gender, birthday, qq.getBytes("UTF-16LE"), md2.digest() };
			return application.procedure.handler.get("adduser").execute ( parameters, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean updateUserInfo(String name,String prompt,String answer,String truename,String idnumber,String email,String mobilenumber,String province,String city,String phonenumber,String address,String postalcode,Integer gender,String birthday,String qq)
	{
		try
		{
			if ( prompt == null ) prompt = "";
			if ( answer == null ) answer = "";
			if ( truename == null ) truename = "";
			if ( idnumber == null ) idnumber = "";
			if ( email == null ) email = "";
			if ( mobilenumber == null ) mobilenumber = "";
			if ( province == null ) province = "";
			if ( city == null ) city = "";
			if ( phonenumber == null ) phonenumber = "";
			if ( address == null ) address = "";
			if ( postalcode == null ) postalcode = "";
			if ( gender == null ) gender = new Integer(2);
			if ( birthday == null ) birthday = "";
			if ( qq == null ) qq = "";
			Object[] parameters = new Object[] { name.toLowerCase(), prompt.getBytes("UTF-16LE"), answer.getBytes("UTF-16LE"), truename.getBytes("UTF-16LE"), idnumber.getBytes("UTF-16LE"), email.getBytes("UTF-16LE"), mobilenumber.getBytes("UTF-16LE"), province.getBytes("UTF-16LE"), city.getBytes("UTF-16LE"), phonenumber.getBytes("UTF-16LE"), address.getBytes("UTF-16LE"), postalcode.getBytes("UTF-16LE"),gender,birthday, qq.getBytes("UTF-16LE") };
			return application.procedure.handler.get("updateUserInfo").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(); }
		return false;
	}
	public static boolean tryLogin(String name,String passwd)
	{
		try
		{
			Object[] pwds = acquireIdPasswd(name.toLowerCase());
			if( null == pwds )
				return false;
			byte[] pwd = (byte[])pwds[1];

			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			byte[] tmppwd = md.digest();

			for(int i=0; i<pwd.length; ++i)
			{
				if( pwd[i] != tmppwd[i] ) return false;
			}
			System.out.println("");
			return true;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean tryLogin2(String name,String passwd2)
	{
		try
		{
			Object[] pwds = acquireIdPasswd2(name.toLowerCase());
			if( null == pwds )
				return false;
			byte[] pwd = (byte[])pwds[1];

			MessageDigest md2 = MessageDigest.getInstance("MD5");
			md2.update(name.toLowerCase().getBytes());
			md2.update(passwd2.getBytes());
			byte[] tmppwd = md2.digest();

			for(int i=0; i<pwd.length; ++i)
			{
				if( pwd[i] != tmppwd[i] ) return false;
			}

			return true;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean changePasswdWithOld(String name, String passwd, String oldpwd)
	{
		try
		{
			Object[] pwds = acquireIdPasswd(name.toLowerCase());
			if( null == pwds )
				return false;
			byte[] pwd = (byte[])pwds[1];

			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(oldpwd.getBytes());
			byte[] tmppwd = md.digest();

			for(int i=0; i<pwd.length; ++i)
			{
				if( pwd[i] != tmppwd[i] ) return false;
			}

			md.reset();
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			
			return application.procedure.handler.get("changePasswd").
				execute ( new Object[] { name.toLowerCase(), md.digest() }, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean changePasswdWithOld2(String name, String passwd, String oldpwd)
	{
		try
		{
			Object[] pwds = acquireIdPasswd2(name.toLowerCase());
			if( null == pwds )
				return false;
			byte[] pwd = (byte[])pwds[1];

			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(oldpwd.getBytes());
			byte[] tmppwd = md.digest();

			for(int i=0; i<pwd.length; ++i)
			{
				if( pwd[i] != tmppwd[i] ) return false;
			}

			md.reset();
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			
			return application.procedure.handler.get("changePasswd").
				execute ( new Object[] { name.toLowerCase(), md.digest() }, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean changePasswd2WithOld2(String name, String passwd, String oldpwd)
	{
		try
		{
			Object[] pwds = acquireIdPasswd2(name.toLowerCase());
			if( null == pwds )
				return false;
			byte[] pwd = (byte[])pwds[1];

			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(oldpwd.getBytes());
			byte[] tmppwd = md.digest();

			for(int i=0; i<pwd.length; ++i)
			{
				if( pwd[i] != tmppwd[i] ) return false;
			}

			md.reset();
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			
			return application.procedure.handler.get("changePasswd2").
				execute ( new Object[] { name.toLowerCase(), md.digest() }, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean changePasswd(String name, String passwd)
	{
		try
		{
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			return application.procedure.handler.get("changePasswd").
				execute ( new Object[] { name.toLowerCase(), md.digest() }, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean changePasswd2(String name, String passwd2)
	{
		try
		{
			MessageDigest md2 = MessageDigest.getInstance("MD5");
			md2.update(name.toLowerCase().getBytes());
			md2.update(passwd2.getBytes());
			return application.procedure.handler.get("changePasswd2").
				execute ( new Object[] { name.toLowerCase(), md2.digest() }, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}


	public static byte[] StringPassword( String s )
	{
		byte[] bin = new byte[16];
		for ( int i = 1; i <= 16; i++ )
			bin[i-1] = (byte)Integer.parseInt(s.substring( i * 2, i * 2 + 2 ),16);

		return bin;
	}


	public static Object[] getIPLimit(Integer uid)
	{
		try
		{
			Object[] rows = (Object[])application.query.handler.get("getIPLimit").select("byUid").execute(new Object[]{uid},"auth0");
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];

			return null;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static int checkIPLimit(Integer uid, Integer ip)
	{
		try
		{
			Object[] rows = getIPLimit(uid);
			if( null == rows )	return 0;
			if( null != rows[8] && ((String)rows[8]).equals("t") )
				return 1;
			if( null == rows[7] || !((String)rows[7]).equals("t") )
				return 0;
			boolean haslimit = false;
			for( int i=1; i<=5; i+= 2 )
			{
				if( null != rows[i] && null != rows[i+1] )
				{
					int ipaddr = ((Integer)rows[i]).intValue();
					byte ipmask = Byte.valueOf((String)rows[i+1]).byteValue();
					if( ipmask <= (byte)0 || ipmask > (byte)32 )
						continue;
					haslimit = true;
					int netbits = (32 - ipmask);
					System.out.println( "checkIPLimit: uid="+uid+",ip="+ip
									+",ipaddr="+ipaddr+",ipmask="+ipmask );
					if( (ip.intValue() & (~0x00 << netbits)) == (ipaddr & (~0x00 << netbits)) )
						return 0;
				}
			}
			return haslimit ? 2 : 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return 0;
	}
	public static boolean setIPLimit(Integer userid,Integer ipaddr1,Byte ipmask1,Integer ipaddr2,Byte ipmask2,Integer ipaddr3,Byte ipmask3)
	{
		try
		{
			Object[] parameters = new Object[] { userid, ipaddr1,ipmask1.toString(), ipaddr2,ipmask2.toString(), ipaddr3,ipmask3.toString()};
			return application.procedure.handler.get("setiplimit").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean enableIPLimit(Integer userid,boolean enable)
	{
		try
		{
			String se = (enable ? new String("t") : new String("f"));
			Object[] parameters = new Object[] { userid, se };
			return application.procedure.handler.get("enableiplimit").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean getLockStatus(Integer uid)
	{
		try
		{
			Object[] rows = (Object[])getIPLimit(uid);
			if( null != rows && null != rows[8] && ((String)rows[8]).equals("t") )
				return true;
			return false;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean lockUser(Integer userid,boolean lockstatus)
	{
		try
		{
			String se = (lockstatus ? new String("t") : new String("f"));
			Object[] parameters = new Object[] { userid, se };
			return application.procedure.handler.get("lockuser").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static Object[] acquirePrivilegeByUidZid(Integer uid, Integer zid)
	{
		try
		{
			return (Object[])application.query.handler.get("acquirePrivilege").select("byUidZid").execute(new Object[]{uid,zid}, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] acquirePrivilegeByUid(Integer uid)
	{
		try
		{
			return (Object[])application.query.handler.get("acquirePrivilege").select("byUid").execute(new Object[]{uid}, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] acquirePrivilegeByZid(Integer zid)
	{
		try
		{
			return (Object[])application.query.handler.get("acquirePrivilege").select("byZid").execute(new Object[]{zid}, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] acquirePrivilege()
	{
		try
		{
			return (Object[])application.query.handler.get("acquirePrivilege").select("byAll").execute(new Object[0], "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static int acquireUserCreatime(Integer userid)
	{
		try
		{
			Object[] rows = (Object[])application.query.handler.get("acquireUserCreatime").select("byUid").execute(new Object[]{ userid }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (int)(((java.util.Date)((Object[])rows[0])[0]).getTime()/1000);
			return 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return 0;
	}
	public static void addUserPriv(Integer userid,Integer zoneid,Integer rid)
	{
		try
		{
			Object[] parameters = new Object[] { userid,zoneid, rid };
			application.procedure.handler.get("addUserPriv").execute( parameters, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}

	public static void delUserPriv(Integer userid,Integer zoneid,Integer rid)
	{
		try
		{
			Object[] parameters = new Object[] { userid,zoneid, rid ,new Integer(0)};
			application.procedure.handler.get("delUserPriv").execute( parameters, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}
	public static void replaceUserPriv(Integer userid, Integer zoneid, Set privs)
	{
		try
		{
			Object[] parameters = new Object[] { userid,zoneid, new Integer(0) ,new Integer(1)};
			application.procedure.handler.get("delUserPriv").execute( parameters, "auth0" );
			parameters = new Object[] { userid,zoneid,null };
			Iterator iter = privs.iterator();
			while( iter.hasNext() )
			{
				parameters[2] = (Integer)iter.next();
				application.procedure.handler.get("addUserPriv").execute(parameters,"auth0");
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}
	public static void delUserPriv(Integer userid,Integer zoneid)
	{
		try
		{
			Object[] parameters = new Object[] { userid,zoneid, new Integer(0),new Integer(1) };
			application.procedure.handler.get("delUserPriv").execute( parameters, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}
	public static void delUserPriv(Integer userid)
	{
		try
		{
			Object[] parameters = new Object[] { userid,new Integer(0),new Integer(0),new Integer(2) };
			application.procedure.handler.get("delUserPriv").execute( parameters, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}
	public static Object[] acquireUserPrivilege(Integer uid,Integer zid)
	{
		try
		{
			return (Object[])application.query.handler.get("acquireUserPrivilege").select("byUidZid").execute(new Object []{ uid,zid }, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUserForbidByName(String name)
	{
		try
		{
			return (Object[])application.query.handler.get("acquireForbid").select("byName").execute(new Object[]{name.toLowerCase()},"auth0");
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] acquireForbid(Integer uid)
	{
		try
		{
			return (Object[])application.query.handler.get("acquireForbid").select("byUid").execute(new Object[]{uid},"auth0");
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static boolean deleteTimeoutForbid(Integer uid)
	{
		try
		{
			Object[] parameters = new Object[] { uid };
			return application.procedure.handler.get("deleteTimeoutForbid").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean addForbid(Integer userid,Integer type,Integer forbid_time,byte[] reason,Integer gmroleid)
	{
		try
		{
			Object[] parameters = new Object[] { userid, type, forbid_time, reason, gmroleid};
			return application.procedure.handler.get("addForbid").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static Object[] getUserOnlineInfo(Integer uid)
	{
		try
		{
			return (Object[]) application.query.handler.get("getUserOnlineInfo").select("byUid").execute(new Object[]{ uid }, "auth0");
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static void clearOnlineRecords(Integer zoneid,Integer aid)
	{
		try
		{
			Object[] parameters = new Object[] { zoneid, aid };
			application.procedure.handler.get("clearonlinerecords").execute( parameters, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}

	public static boolean recordUserOnline(Object[] info, Integer userid, Integer aid)
	{
		try
		{
			Object[] parameters = new Object[] { userid, aid, info[0], info[1], info[2] };
			if ( application.procedure.handler.get("recordonline").execute ( parameters, "auth0" ) == 0 )
			{
				info[0] = parameters[2];
				info[1] = parameters[3];
				info[2] = parameters[4];
				return true;
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean recordUserOffline(Object[] info, Integer userid, Integer aid)
	{
		try
		{
			Object[] parameters = new Object[] { userid, aid, info[0], info[1], info[2] };
			if ( application.procedure.handler.get("recordoffline").execute ( parameters, "auth0" ) == 0 )
			{
				info[2] = parameters[4];
				return true;
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static Object[] getUserPoints(Integer userid)
	{
		try
		{
			return application.query.handler.get("getUserPoints").select("byuid").execute(new Object []{userid}, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] acquireRemainTime(Integer userid, Integer aid)
	{
		try
		{
			Object[] parameters = new Object[] { userid, aid, new Integer(0), new Integer(0) };
			application.procedure.handler.get("remaintime").execute( parameters, "auth0" );
			Integer freetimeleft = new Integer(0);
			if( null != parameters[3] && ((Integer)parameters[3]).intValue() > 0 )
				freetimeleft = (Integer)parameters[3];
			return new Object[] { (Integer)parameters[2], freetimeleft };
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static boolean addUserPoint(Integer userid, Integer aid, Integer time)
	{
		try
		{
			Object[] parameters = new Object[] { userid, aid, time };
			return (application.procedure.handler.get("adduserpoint").execute( parameters, "auth0" ) == 0);
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static int useCash(Integer userid,Integer zoneid,Integer sn,Integer aid,Integer point,Integer cash, Integer status)
	{
		try
		{
			Integer error=new Integer(0);
			Object[] parameters = new Object[] { userid,zoneid,sn,aid,point,cash,status,error };
			application.procedure.handler.get("usecash").execute( parameters, "auth0" );
			return ((Integer)parameters[7]).intValue();
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	public static Object[] getUseCashNow(Integer status)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("bystatus").execute(new Object[] {status});
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUseCashNow(Integer userid, Integer zoneid)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("byuserzone").execute(new Object[] {userid,zoneid});
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUseCashNow(Integer userid, Integer zoneid, Integer sn)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("byuserzonesn").execute(new Object[] {userid,zoneid,sn});
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUseCashNowByUser(Integer userid)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("byuser").execute(new Object[] {userid});
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUseCashLog(Integer userid, Integer zoneid)
	{
		try
		{
			return application.query.handler.get("getusecashlog").select("byuserzone").execute(new Object[] {userid,zoneid});
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUseCashLog(Integer userid, Integer zoneid, Integer sn)
	{
		try
		{
			return application.query.handler.get("getusecashlog").select("byuserzonesn").execute(new Object[] {userid,zoneid,sn});
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUseCashLogByUser(Integer userid)
	{
		try
		{
			return application.query.handler.get("getusecashlog").select("byuser").execute(new Object[] {userid});
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	private static String toHexString(byte[] x)
	{
		StringBuffer sb = new StringBuffer(x.length * 2);
		for ( int i = 0; i < x.length; i++ )
		{
			byte n = x[i];
			int nibble = (int)(n >> 4)&0xf;
			sb.append ( (char)(nibble >= 10 ? 'a' + nibble - 10 : '0' + nibble) );
			nibble = (int)(n & 0xf);
			sb.append ( (char)(nibble >= 10 ? 'a' + nibble - 10 : '0' + nibble) );
		}
		return sb.toString();
	}

	public static void main(String[] args)
	{
		try
		{
			parser.parse(new java.io.FileInputStream(args[0]));
			System.out.println("Done!");

			Object[] row = acquireIdPasswd("zengpan");
			Integer uid = (Integer)row[0];
			byte[] passwd = (byte[])row[1];
			System.out.println( "uid="+uid + ",passwd="+passwd );

			for( int i=0; i<passwd.length; i++ )
			{
				System.out.print( Byte.toString(passwd[i]) );
			}
			System.out.println("");

		}
		catch (Exception e) { e.printStackTrace(); }
	}
}

