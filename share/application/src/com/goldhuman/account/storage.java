
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

	public static int userCodeToID( int code )
	{
		return (code ^ 0x5ACD35A1);
	}

	public static int userCodeToID( String code )
	{
		int icode = Integer.parseInt(code);
		return (icode ^ 0x5ACD35A1);
	}

	public static int userIDToCode( int userid )
	{
		return (userid ^ 0x5ACD35A1);
	}

	public static int getAreaID( int az )
	{
		return (az & 0x0000FFFF);
	}

	public static int getZoneID( int az )
	{
		return ( (az & 0xFFFF0000) >> 16 );
	}

	public static int makeAreaZone( int aid, int zoneid )
	{
		return ( aid | (zoneid << 16) );
	}

	private static String getConnStr()
	{
		Calendar c = Calendar.getInstance();
		c.setTimeInMillis( System.currentTimeMillis() );
		int h = c.get(Calendar.HOUR_OF_DAY);
		if( 23 == h || 0 == h || 1 == h )
			return "auth0";
		else
			return "auth1";
	}
	
	public static Object[] queryUserSession(String username,Integer aid,Integer zoneid)
	{
		try
		{
			//TODO
			//Object[] parameters = new Object[] { status,zoneid,serial,seller,sellid,buyer,price,point,aid };
			//if(0==application.procedure.handler.get("queryusersession").execute( parameters, "auth0" ))
			//	return (Integer)parameters[0];
			return null;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	
	public static Integer sellPoint(Integer zoneid,Integer serial,Integer seller,Integer sellid,Integer buyer,Integer price, Integer point,Integer aid)
	{
		try
		{
			Integer status=new Integer(0);
			Object[] parameters = new Object[] { status,zoneid,serial,seller,sellid,buyer,price,point,aid };
			if(0==application.procedure.handler.get("sellpoint").execute( parameters, "auth0" ))
				return (Integer)parameters[0];
			return null;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static int useCash(Integer userid,Integer zoneid,Integer sn,Integer aid,Integer point,Integer cash, Integer status)
	{
		try
		{
			Integer error=new Integer(0);
			Object[] parameters = new Object[] { userid,zoneid,sn,aid,point,cash,status,error };
			if(0==application.procedure.handler.get("usecash").execute( parameters, "auth0" ))
				return ((Integer)parameters[7]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
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
	public static Object[] getUserAwardPoints(Integer userid)
	{
		try
		{
			return application.query.handler.get("getUserAwardPoints").select("byuid").execute(new Object []{userid}, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUserSellPoints(Integer userid)
	{
		try
		{
			return application.query.handler.get("getUserSellPoints").select("byuid").execute(new Object []{userid}, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
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
	public static String getNameById(Integer id)
	{
		try
		{
			Object[] rows = application.query.handler.get("getUsername").select("byId").execute(new Object []{ id }, getConnStr() );
			if( null != rows && rows.length > 0 )
				return (String)((Object[])rows[0])[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Integer getIdByName(String name)
	{
		try
		{
			Object[] rows = application.query.handler.get("getUserid").select("byName").execute(new Object []{ name.toLowerCase() }, getConnStr() );
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
			Object[] rows = application.query.handler.get("getUserInfo").select("byName").execute(new Object[]{name.toLowerCase() }, getConnStr() );
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static int existsUsername( String username )
	{
		try
		{
			if( null == username || username.length() < 1 )
				return 0;

			return application.procedure.handler.get("existsUsername").execute(new Object[] { username },getConnStr());
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	public static Object[] getIndulgeUserInfo(Integer userid)
	{
		try
		{
			Object[] rows = application.query.handler.get("getIndulgeUserInfo").select("byId").execute(new Object[]{userid }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getIndulgeUserInfobyName(String name)
	{
		try
		{
			Object[] rows = application.query.handler.get("getIndulgeUserInfo").select("byName").execute(new Object[]{name.toLowerCase() }, "auth0" );
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getIndulgeUserInfobyIdcard(String idcard)
	{
		try
		{
			Object[] rows = application.query.handler.get("getIndulgeUserInfo").select("byIdcard").execute(new Object[]{idcard}, "auth0" );
			return rows;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static int recordIndulgeUser(String name, byte[] truename, String idcard, Integer verfied )
	{
		try
		{
			Object[] parameters = new Object[] { name.toLowerCase(), truename, idcard, verfied };
			return application.procedure.handler.get("recordIndulgeUser").execute ( parameters, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	public static Object[] queryCardRecordInfo(String code)
	{
		try
		{
			Object[] rows = (Object[]) application.query.handler.get("cardrecordinfo").select("bycode").
				execute( new Object[] { code }, "auth0" );
			return (Object[]) rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] queryCardUsed(String toname)
	{
		try
		{
			return application.query.handler.get("getcardused").select("byname").execute(new Object[] {toname.toLowerCase()}, "auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] queryCardUsedByCard(String number)
	{
		try
		{
			if( null == number || number.length() < 10 )
				return null;
			if( number.length() < 25 )
				number = number + "%";
			Object[] rows = application.query.handler.get("getcardused").select("bycard").execute(new Object[] {number}, "auth0");
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
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
	public static int acquireUserType(String username)
	{
		try
		{
			Object[] rows = (Object[])application.query.handler.get("acquireUserType").select("byName").execute(new Object[]{ username.toLowerCase() }, "auth0" );
			if( null != rows && rows.length > 0 )
				return ((Integer)((Object[])rows[0])[0]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
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
			if( null == reason || reason.length < 1 )
				return false;
			Object[] parameters = new Object[] { userid, type, forbid_time, reason, gmroleid};
			return application.procedure.handler.get("addForbid").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
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

			if( null == rows )	return 0; // no record
			if( null != rows[8] && ((String)rows[8]).equals("t") ) return 1; // lock
            int r = ( null != rows[9] && ((String)rows[9]).equals("t") ) ? 3 : 0; // autolock?
			if( null == rows[7] || !((String)rows[7]).equals("t") ) return r; // iplimit disabled

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
					//System.out.println( "checkIPLimit: uid="+uid+",ip="+ip
					//				+",ipaddr="+ipaddr+",ipmask="+ipmask );
					if( (ip.intValue() & (~0x00 << netbits)) == (ipaddr & (~0x00 << netbits)) )
						return r;
				}
			}
			return haslimit ? 2 : r;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return 0;
	}

	public static int checkIPLimit(Integer uid, Integer ip, Object[] rows)
	{
		try
		{
			if( null == rows )	return 0; // no record
			if( null != rows[8] && ((String)rows[8]).equals("t") ) return 1; // lock
            int r = ( null != rows[9] && ((String)rows[9]).equals("t") ) ? 3 : 0; // autolock?
			if( null == rows[7] || !((String)rows[7]).equals("t") ) return r; // iplimit disabled

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
					//System.out.println( "checkIPLimit: uid="+uid+",ip="+ip
					//				+",ipaddr="+ipaddr+",ipmask="+ipmask );
					if( (ip.intValue() & (~0x00 << netbits)) == (ipaddr & (~0x00 << netbits)) )
						return r;
				}
			}
			return haslimit ? 2 : r;
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
	public static boolean enableAutolock(Integer userid,boolean autolock)
	{
		try
		{
			String se = (autolock ? new String("t") : new String("f"));
			Object[] parameters = new Object[] { userid, se };
			return application.procedure.handler.get("enableautolock").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	public static boolean testAutolock(Integer uid)
	{
		try
		{
			Object[] rows = (Object[])getIPLimit(uid);
			if( null != rows && null != rows[9] && ((String)rows[9]).equals("t") )
				return true;
			return false;
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
	public static boolean testandlockUser(Integer userid)
	{
		
		try
		{
			Object[] parameters = new Object[] { userid };
			return application.procedure.handler.get("testandlockuser").execute(parameters,"auth0")==0;
			
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
			Object[] parameters = new Object[] { name.toLowerCase(), prompt.getBytes("x-EUC-CN"), answer.getBytes("x-EUC-CN"), truename.getBytes("x-EUC-CN"), idnumber.getBytes("x-EUC-CN"), email.getBytes("x-EUC-CN"), mobilenumber.getBytes("x-EUC-CN"), province.getBytes("x-EUC-CN"), city.getBytes("x-EUC-CN"), phonenumber.getBytes("x-EUC-CN"), address.getBytes("x-EUC-CN"), postalcode.getBytes("x-EUC-CN"),gender,birthday, qq.getBytes("x-EUC-CN") };
			return application.procedure.handler.get("updateUserInfo").execute(parameters,"auth0")==0;
		}
		catch (Exception e) { e.printStackTrace(); }
		return false;
	}

	public static Object[] queryCardNew(String cardnumber)
	{
		try
		{
			Object[] parameters = new Object[] { cardnumber, new String(""), new Integer(0), new String(""), new Integer(0), new Integer(0), new Integer(0), new Integer(0), new String(""), new Integer(0), new Integer(0), new Integer(0), new Integer(0), new Integer(-1) };
			if( application.procedure.handler.get("querycardnew").execute(parameters, "auth0") == 0 )
			{
				java.util.Date	endtime = null;
				try { endtime = datefmt3.parse( (String)parameters[8] ); } catch (Exception e) { }
				parameters[8] = endtime;
				return parameters;
			}
			return null;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static int useCard(String number, String fromname, String toname, Integer aid, Integer zoneid, Integer ip)
	{
		try
		{
			Object[] parameters = new Object[] { number, fromname.getBytes("x-EUC-CN"), toname.toLowerCase(), aid, zoneid, ip, new Integer(-1) };
			if( application.procedure.handler.get("usepointcard").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[6]).intValue();
			return -1;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	public static int batchUseCard(String number1, String number2, String fromname, String toname, Integer aid, Integer zoneid, Integer ip)
	{
		try
		{
			Object[] parameters = new Object[] { number1, number2, fromname.getBytes("x-EUC-CN"), toname.toLowerCase(), aid, zoneid, ip, new Integer(-1) };
			if( application.procedure.handler.get("batchusepointcard").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[7]).intValue();
			return -1;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int useAgentCard( Integer billid, String agent, String name, int aid, int cardtype, Integer money, Integer addpoint, Integer addscore, Integer cookie1, Integer cookie2, Integer error )
	{
		try
		{
			Object[] parameters = new Object[] { billid, agent, name.toLowerCase(), aid, cardtype, money, addpoint, addscore, cookie1, cookie2, error };
			if( application.procedure.handler.get("useagentcard").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[10]).intValue();
			return -1;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -2;
	}
	public static int useAgentCardNew( Integer billid, String agent, String name, int aid, int cardtype, Integer money, Integer addpoint, Integer addscore, Integer cookie1, Integer cookie2, Integer addcoin, Integer awarduid, Integer awardpoint, Integer awardscore, Integer error )
	{
		try
		{
			Object[] parameters = new Object[] { billid, agent, name.toLowerCase(), aid, cardtype, money, addpoint, addscore, cookie1, cookie2, addcoin, awarduid, awardpoint, awardscore, error };
			if( application.procedure.handler.get("useagentcardnew").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[14]).intValue();
			return -1;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -2;
	}
	public static int addInnerPoint( String name, Integer aid, Integer zoneid, Integer addpoint, String enddate, Integer monthcount, Integer addscore, String op )
	{
		try
		{
			if( enddate == null )	enddate = "";
			Object[] parameters = new Object[] {name.toLowerCase(),aid,zoneid,addpoint,enddate,monthcount,addscore,op.getBytes("x-EUC-CN"),new Integer(0)};
			if( application.procedure.handler.get("addinnerpoint").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[8]).intValue();
			return -1;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	public static int useAwardPoint( String name, Integer aid, String fromname )
	{
		try
		{
			Object[] parameters = new Object[] { name, aid, fromname, new Integer(0) };
			if( application.procedure.handler.get("useawardpoint").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[3]).intValue();
			return -1;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -2;
	}
	public static Object[] queryAgentBill( Integer billid, String agent )
	{
		try
		{
			Object[] rows = application.query.handler.get("queryAgentBill").select("byBillidAgent").execute(new Object []{ billid, agent }, "auth0" );
			if( null != rows && rows.length > 0 )
				return ((Object[])rows[0]);
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] queryAgentBill( Integer billid )
	{
		try
		{
			Object[] rows = application.query.handler.get("queryAgentBill").select("byBillid").execute(new Object []{ billid }, "auth0" );
			if( null != rows && rows.length > 0 )
				return ((Object[])rows[0]);
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] queryAgentBill( Integer uid, Date begin, Date end )
	{
		try
		{
			Object[] rows = application.query.handler.get("queryAgentBill").select("byUidDateRange").execute(new Object []{ uid, datefmt2.format(begin), datefmt2.format(end) }, "auth0" );
			return rows;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] exchangeMonth( String name, Integer aid, Integer monthcount )
	{
		try
		{
			if( monthcount.intValue() <= 0 )
				return null;
			Object[] parameters = new Object[] {name.toLowerCase(),aid,monthcount,new Integer(0),new Integer(0),new String()};
			if( application.procedure.handler.get("exchangemonth").execute(parameters, "auth0") == 0 )
			{
				java.util.Date enddate = null;
				try { enddate = datefmt3.parse( (String)parameters[5] ); } catch (Exception e) { }
				return new Object[] { (Integer)parameters[3], (Integer)parameters[4], enddate  };
			}
			return null;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static Object[] getUserMonthExchanged( String username, Date begin, Date end )
	{
		try
		{
			Object[] rows = application.query.handler.get("monthexchanged").select("byname").execute(new Object []{ username.toLowerCase(), datefmt2.format(begin), datefmt2.format(end) }, "auth0" );
			return rows;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	public static int exchangeArea( String name, Integer srcaid, Integer destaid, Integer time )
	{
		try
		{
			if( time.intValue() <= 0 )
				return 3;
			Object[] parameters = new Object[] {name.toLowerCase(),srcaid,destaid,time,new Integer(0)};
			if( application.procedure.handler.get("exchangearea").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[4]).intValue();
			return 3;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return 3;
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
			Object[] parameters = new Object[] { name.toLowerCase(), md.digest(), prompt.getBytes("x-EUC-CN"), answer.getBytes("x-EUC-CN"), truename.getBytes("x-EUC-CN"), idnumber.getBytes("x-EUC-CN"), email.getBytes("x-EUC-CN"), mobilenumber.getBytes("x-EUC-CN"), province.getBytes("x-EUC-CN"), city.getBytes("x-EUC-CN"), phonenumber.getBytes("x-EUC-CN"), address.getBytes("x-EUC-CN"), postalcode.getBytes("x-EUC-CN"), gender, birthday, qq.getBytes("x-EUC-CN"), md2.digest() };
			return application.procedure.handler.get("adduser").execute ( parameters, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}
	
	public static boolean addBonusUser(String name, String passwd, String prompt,String answer,String truename,String idnumber,String email,String mobilenumber,String province,String city,String phonenumber,String address,String postalcode, Integer gender, String birthday, String qq, String passwd2, Integer aid, Integer freepoint, Integer score, Integer func, Integer funcparm)
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
			Object[] parameters = new Object[] { name.toLowerCase(), md.digest(), prompt.getBytes("x-EUC-CN"), answer.getBytes("x-EUC-CN"), truename.getBytes("x-EUC-CN"), idnumber.getBytes("x-EUC-CN"), email.getBytes("x-EUC-CN"), mobilenumber.getBytes("x-EUC-CN"), province.getBytes("x-EUC-CN"), city.getBytes("x-EUC-CN"), phonenumber.getBytes("x-EUC-CN"), address.getBytes("x-EUC-CN"), postalcode.getBytes("x-EUC-CN"), gender, birthday, qq.getBytes("x-EUC-CN"), md2.digest(), aid, freepoint, score, func, funcparm };
			return application.procedure.handler.get("addbonususer").execute ( parameters, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}


	public static boolean tryLoginNew(String name,String passwd)
	{
		try
		{
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			byte[] tmppwd = md.digest();
			return 1==application.procedure.handler.get("tryLogin").execute(new Object[] { name, tmppwd },getConnStr());
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}

	public static boolean tryLogin(String name,String passwd)
	{
		try
		{
			Object[] pwds = acquireIdPasswd(name.toLowerCase());
			if( null == pwds )
				return false;
			byte[] pwd = (byte[])pwds[2];

			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			byte[] tmppwd = md.digest();

			for(int i=0; i<pwd.length; ++i)
			{
				if( pwd[i] != tmppwd[i] ) return false;
			}

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
			byte[] pwd = (byte[])pwds[2];

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
/*
	private static int GetMergeByte( byte a, byte b, byte c, byte d)
	{
		return (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) <<  8) | ((d & 0xff) <<  0));
	}

	public static byte[] queryusbkey(byte[] keysn)
	{
		try
		{
			Integer sn1 = new Integer(GetMergeByte(keysn[3], keysn[2], keysn[1], keysn[0]));
			Integer sn2 = new Integer(GetMergeByte(keysn[7], keysn[6], keysn[5], keysn[4]));
			Object[] rows = application.query.handler.get("queryusbkey").select("bysn").execute(new Object []{sn1,sn2}, "auth0" );
			if( null != rows && rows.length > 0 )
				return (byte[])((Object[])rows[0])[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
*/
	public static byte[] queryusbkey(byte[] keysn)
	{
		try
		{
			Object[] parameters = new Object[] { keysn, new byte[16] };
			if( 0 == application.procedure.handler.get("queryusbkey").execute ( parameters, "auth0" ) )
				return ((byte[])parameters[1]);
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static int bindKey(String name, byte[] keysn)
	{
		try
		{
			Object[] parameters = new Object[] { name.toLowerCase(), keysn, new Integer(-1) };
			if( 0 == application.procedure.handler.get("bindKey").execute ( parameters, "auth0" ) )
				return ((Integer)parameters[2]).intValue();
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int unbindKey(String name, String passwd, byte[] keysn)
	{
		try
		{
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.toLowerCase().getBytes());
			md.update(passwd.getBytes());
			Object[] parameters = new Object[] { name.toLowerCase(), md.digest(), keysn, new Integer(-1) };
			if( 0 == application.procedure.handler.get("unbindKey").execute ( parameters, "auth0" ) )
				return ((Integer)parameters[3]).intValue();
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int activeUser(String name, String code, int type )
	{
		try
		{
			Object[] parameters = new Object[] { name.toLowerCase(), code, new Integer(type), new Integer(-1) };
			if( 0 == application.procedure.handler.get("activeUser").execute ( parameters, "auth0" ) )
				return ((Integer)parameters[3]).intValue();
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static boolean recordAccountingInfo(Integer userid, Integer aid, Integer type)
	{
		try
		{
			return application.procedure.handler.get("accounting").execute( new Object[] { userid, aid, type }, "auth0" ) == 0;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
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
	public static Object[] getFunction(Integer userid)
	{
		try
		{
			Object[] rows = application.query.handler.get("getfunction").select("byuid").execute(new Object[] {userid},"auth0");
			if( null != rows && rows.length > 0 )
				return (Object[])rows[0];
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getUseCashNow(Integer status)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("bystatus").execute(new Object[] {status},"auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getUseCashNow(Integer userid, Integer zoneid)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("byuserzone").execute(new Object[] {userid,zoneid},"auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getUseCashNow(Integer userid, Integer zoneid, Integer sn)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("byuserzonesn").execute(new Object[] {userid,zoneid,sn},"auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getUseCashNowByUser(Integer userid)
	{
		try
		{
			return application.query.handler.get("getusecashnow").select("byuser").execute(new Object[] {userid},"auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getUseCashLog(Integer userid, Integer zoneid)
	{
		try
		{
			return application.query.handler.get("getusecashlog").select("byuserzone").execute(new Object[] {userid,zoneid},"auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getUseCashLog(Integer userid, Integer zoneid, Integer sn)
	{
		try
		{
			return application.query.handler.get("getusecashlog").select("byuserzonesn").execute(new Object[] {userid,zoneid,sn},"auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static Object[] getUseCashLogByUser(Integer userid)
	{
		try
		{
			return application.query.handler.get("getusecashlog").select("byuser").execute(new Object[] {userid},"auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static boolean addScore(Integer userid, Integer addscore )
	{
		try
		{
			Object[] parameters = new Object[] { userid, addscore };
			return (application.procedure.handler.get("addscore").execute( parameters, "auth0" ) == 0);
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}

	public static boolean setFunction(Integer userid, Integer func, Integer funcparm )
	{
		try
		{
			Object[] parameters = new Object[] { userid, func, funcparm };
			return (application.procedure.handler.get("setfunction").execute( parameters, "auth0" ) == 0);
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
	}

	public static boolean setFunction(String username, Integer func, Integer funcparm )
	{
		try
		{
			Object[] parameters = new Object[] { username.toLowerCase(), func, funcparm };
			return (application.procedure.handler.get("setfunctionbyname").execute( parameters, "auth0" ) == 0);
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
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

	public static boolean recordUserOnlineFree(Integer userid, Integer aid, Integer zoneid)
	{
		try
		{
			Object[] parameters = new Object[] { userid, aid, zoneid };
			if ( application.procedure.handler.get("recordonline_free").execute ( parameters, "auth0" ) == 0 )
				return true;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return false;
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

	static
	{
		try
		{
			// parser.parse(storage.class.getResource("/table.xml").openStream());
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
   	}

	public static Date calcEndTime(Date endtime, Integer pointime) 
	{
		Date now = new Date();
		if (endtime.after(now)) now = endtime;
		return new Date(now.getTime() + pointime.longValue()*1000);
	}

	private static void CreateActiveCode(String[] args) throws Exception
	{
		int count = 0;
		int start = -1, num = -1;
		String filename=null;
		if (args.length == 5 )
		{
			start=Integer.valueOf(args[2]).intValue(); if (start<0 || start>=100000000) start=-1;
			num=Integer.valueOf(args[3]).intValue(); if (num<0 || num>10000000) num=-1;
			filename= args[4];
		}
		if (args.length!=5 || num<0 || start<0)
		{
			System.out.println("Usage: Storage <conf> activecode <start> <num> <filename>");
			return;
		}
		RandomGen.StrGenerator gen=RandomGen.StrGenerator.GetInstance();
		FileOutputStream f=null;
		FileOutputStream err=null;
		try
		{
			f=new FileOutputStream(filename);
			err=new FileOutputStream(filename+".err");
			DataInputStream dis = new DataInputStream(new FileInputStream("/dev/urandom"));
			for (int i=0;i<num/10;i++)
			{
				LinkedList coden = new LinkedList();
				for( int j=0; j<10; j++ )
				{
					String code = "" + gen.Generate_Num(8,start+(count++)) + gen.Generate_Num(8);
					//String card = "11111111" + Integer.toString(Math.abs(dis.readInteger()));
					//code += card.substring(card.length()-7);
					coden.addLast( code );
				}
				if( 0 == application.procedure.handler.get("addactivecode").execute( coden.toArray(), "auth0" ) )
				{
					for( int k=0; k<coden.size(); k++ )
					{
						String output=coden.get(k)+"\n";
						System.out.print(""+i+": "+output);
						f.write(output.getBytes());
					}
				}
				else
				{
					for( int k=0; k<coden.size(); k++ )
					{
						String output=coden.get(k)+"\n";
						System.out.print("ERROR:"+i+": "+output);
						err.write(output.getBytes());
					}
				}
			}
			f.close();
			err.close();
			dis.close();
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}

	private static void CreateAccounts(String[] args) throws Exception
	{
		int name_sz = 6, passwd_sz = 6;
		int start = -1, num = -1;
		String filename=null;
		if (args.length == 5 )
		{
			start=Integer.valueOf(args[2]).intValue(); if (start<0 || start>=1000000) start=-1;
			num=Integer.valueOf(args[3]).intValue(); if (num<0 || num>1000000) num=-1;
			filename= args[4];
		}
		if (args.length!=5 || start<0 || num<0)
		{
			System.out.println("Usage: Storage <conf> account <start> <num> <filename>");
			return;
		}
		RandomGen.StrGenerator gen=RandomGen.StrGenerator.GetInstance();
		FileOutputStream f=null;
		FileOutputStream err=null;
		String name=null,passwd=null;
		try
		{
			f=new FileOutputStream(filename);
			err=new FileOutputStream(filename+".err");
			for (int i=0;i<num;i++)
			{
				name="wartest"+gen.Generate_Num(name_sz,start+i);
				passwd=gen.Generate_Mix(passwd_sz);
				String answer = gen.Generate_Mix(10);
				String idnumber = gen.Generate_Mix(10);
				//新人王活动 if (addUser(name,passwd,"prompt",answer," ",idnumber,"zllj.xinrenwang.world2",null,null,null,null,null,null,null,null,null,"mpykmkS9fW3wvcgd") )
				//武林内测 if (addUser(name,passwd,"prompt",answer," ",idnumber,"internaltest.wulin2",null,null,null,null,null,null,null,null,null,"nucjhjezY09Cmzdi") )
				if (addUser(name,passwd,"prompt",answer," ",idnumber,"wartest.wulin2",null,null,null,null,null,null,null,null,null,"nvwuD6sO0jfdounr") )
				{
					String output=name+"   "+passwd+"\n";
					System.out.print(""+i+": "+output);
					f.write(output.getBytes());
				}
				else
				{
					String output=name+"   "+passwd+"\n";
					System.out.print("ERROR:"+i+": "+output);
					err.write(output.getBytes());
				}
			}
			f.close();
			err.close();
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}

	private static void CreateRobots(String[] args)	
	{
		if (args.length < 6)
		{
			System.out.println("usage: storage robot <conf> <begin_id> <end_id> <aid> <point>");
			return;
		}
		System.out.println("Try to create robot from "+args[2]+" to "+args[3]+". and Save "+args[5]+" points on area "+args[4]);
		if (args[2]!=null && args[3]!=null && args[4]!=null && args[5]!=null)
		{
			int index;
			int nBegin,nEnd;
			nBegin = Integer.valueOf(args[2]).intValue();
			nEnd = Integer.valueOf(args[3]).intValue();
			
			Integer aid= new Integer(args[4]);
			Integer point=new Integer(args[5]);
			
			if (nBegin > nEnd ) {
				int tmp = nBegin;
				nEnd=nBegin;
				nBegin=tmp;
			}
			for (index=nBegin;index<=nEnd;index++)
			{
				if ( addBonusUser(new String().valueOf(160000+16*index),new String().valueOf(160000+16*index),null,null,null,null,"product@IRobot.com",null,null,null,null,null,null,null,null,null,new String().valueOf(160000+16*index),aid,point,new Integer(0),new Integer(0),new Integer(0)) )
				{
					System.out.println("Register robot "+String.valueOf(160000+16*index)+" succesfully.");
				}
				else
				{
					System.out.println("Register robot "+String.valueOf(160000+16*index)+" failed.");
				}
			}
		}
	}
	
	private static void ChangeRobotPasswd(String[] args)	
	{
		if (args.length < 5)
		{
			System.out.println("usage: storage <conf> robot_chgpwd <begin_id> <end_id> <new passwd>");
			return;
		}
		System.out.println("Try to change robot's passwd from "+args[2]+" to "+args[3]+". new password is "+args[4]);
		if (args[2]!=null && args[3]!=null && args[4]!=null)
		{
			int index;
			int nBegin,nEnd;
			nBegin = Integer.valueOf(args[2]).intValue();
			nEnd = Integer.valueOf(args[3]).intValue();
			
			String new_passwd= new String(args[4]);
			
			if (nBegin > nEnd ) {
				int tmp = nBegin;
				nEnd=nBegin;
				nBegin=tmp;
			}
			for (index=nBegin;index<=nEnd;index++)
			{
				if ( changePasswd(new String().valueOf(160000+16*index),new_passwd) )
				{
					System.out.println("Change robot "+String.valueOf(160000+16*index)+"'s passwd to "+new_passwd+" succesfully.");
				}
				else
				{
					System.out.println("Change robot "+String.valueOf(160000+16*index)+"'s passwd failed.");
				}
			}
		}
	}

	private static void GetUserPrivilege(String[] args)
	{
		if (args.length<4) 
		{
			System.out.println("usage: storage <conf> privilege <zoneid> <userid>");
			return;
		}
		Integer uid=new Integer(args[2]);
		Integer zid=new Integer(args[3]);
		Object[] res=acquireUserPrivilege(uid,zid);
		Object[] row=null;
		if (res!=null)
		{
			if (res.length>0)
			{
				System.out.println("user "+uid+"'s privilege in zone "+zid+":");
				for (int i=0;i<res.length;i++)
				{
					row=(Object[])res[i];
					System.out.print((Integer)row[0]+";");
				}
				System.out.println("");
			}
			else
				System.out.println("user "+uid+" does not have any privilege in zone "+zid);
		}
		else
			System.out.println("Get user privilege failed.");
	}

	private static void CreateTestAccounts( String filename, String header, String email, int count ) throws Exception
	{
		int name_sz,passwd_sz,num,aid,point;

		RandomGen.StrGenerator gen=RandomGen.StrGenerator.GetInstance();
		FileOutputStream f=null;
		String name=null,passwd=null;
		try
		{
			f = new FileOutputStream(filename);
			for (int i=0;i<count;i++)
			{
				name = header + gen.Generate_Num( 16 - header.length() );
				passwd = gen.Generate_Mix( 16 );
				if (addUser(name,passwd,null,null,"testaccount",null,email,null,null,null,null,null,null,null,null,null,passwd))
				{
					String output = name + "    " + passwd + "\n";
					System.out.print( i + ": " + output );
					f.write(output.getBytes());
				}
			}
			f.close();
		}
		catch (Exception e) { e.printStackTrace(System.out); }
	}

	public static void dumpresult( Object [] rows )
	{
		if( rows != null)
		{
			for(int i = 0;i <rows.length; ++i)
			{
				Object[] row = (Object[])rows[i];
				for(int j =0; j< row.length; ++j)
				{
					System.out.print(row[j]);
					System.out.print(" ");
				}
				System.out.println("");
			}
		}
	}	

	public static void main(String[] args)
	{
		try
		{
			System.out.println("Begin parse " + args[0] + "..." );
			parser.parse(new java.io.FileInputStream(args[0]));
			System.out.println("Done!");
		}
		catch (Exception e) { e.printStackTrace(); }

		try
		{
			//System.out.println("account sunzhenyu's id is:"+getIdByName("sunzhenyu"));

			//create a bulk of robots
			if (args.length<2) 
			{
				System.out.println("usage: storage <conf> <operation>[...]");
				return;
			}
			if (args[1].compareTo("account")==0)
			{
				CreateAccounts(args);
			}
			else if (args[1].compareTo("activecode")==0)
			{
				CreateActiveCode(args);
			}
			//else if (args[1].compareTo("robot")==0)
			//{
			//	CreateRobots(args);
			//}
			//else if (args[1].compareTo("robot_chgpwd")==0)
			//{
			//	ChangeRobotPasswd(args);
			//}
			//else if (args[1].compareTo("privilege")==0)
			//{
			//	GetUserPrivilege(args);
			//}
			else
			{
				System.out.println("Invalid operation. valid operations are account");
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		
	}
}
