
package com.goldhuman.web;

import java.io.*;
import java.security.*;
import java.util.*;
import com.goldhuman.xml.*;

class addCardRecordTask implements Runnable
{

	final private java.text.Format datefmt = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	private String  code;
	private int     number;
	private Integer price;
	private Integer rate;
	private Integer pointcard;
	private Integer exchangepoint;
	private Date    endtime;
	private String  creator;
	private Integer func;
	private Integer funcparm;
	private Integer score;

	private static String cardgen(Long no, DataInputStream dis) throws Exception
	{
		/*
	     	String card = "0000000000000000000000000000" 
			+ Long.toString(Math.abs(dis.readLong())) 
			+ Integer.toString(Math.abs(dis .readInt()));
			return card.substring(card.length()-28);
		*/
		String card = "000000000000000" + no.toString();

		String passwd = "00000000000000000000" 
			+ Long.toString(Math.abs(dis.readLong())) 
			+ Integer.toString(Math.abs(dis .readInt()));
		return card.substring(card.length()-10) + passwd.substring(passwd.length()-15);
	}


	private static boolean addcard(Integer id, LinkedList cardn)
	{
		try
		{
			cardn.addFirst(new Integer(0)); //inactive each card
			cardn.addFirst(id);
			return application.procedure.handler.get("addcard").execute( cardn.toArray(), "auth0" ) == 0;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return false;
	}

	addCardRecordTask (String code, Integer number, Integer price, Integer rate, Integer pointcard, Integer exchangepoint, Date endtime, String creator, Integer func, Integer funcparm, Integer score)
	{
		this.code          = new String(code);
		this.number        = number.intValue();
		this.price         = new Integer(price.intValue());
		this.rate          = new Integer(rate.intValue());
		this.pointcard     = new Integer(pointcard.intValue());
		this.exchangepoint = new Integer(exchangepoint.intValue());
		this.endtime       = (Date)endtime.clone();
		this.creator       = new String(creator);
		this.func          = new Integer(func.intValue());
		this.funcparm      = new Integer(funcparm.intValue());
		this.score         = new Integer(score.intValue());
	}

	public void run()
	{
		
		try
		{
			long start = Long.parseLong(code);
			DataInputStream dis = new DataInputStream(new FileInputStream("/dev/urandom"));
			Object[] o;
			application.procedure.handler.get("addcardrecord").execute(o = new Object[] {
				code, price, rate, pointcard, exchangepoint, datefmt.format(endtime),
				creator, func, funcparm, score, new Integer(0) }, "auth0");
			Integer id = (Integer)o[10];
			if (id.intValue() > -1) {
				while(number > 0)
				{
					LinkedList cardn = new LinkedList();
					for ( int i = 0; i < 10; i++ ) cardn.addLast(cardgen(new Long(start++),dis));
					if ( addcard(id, cardn) )
						number -= 10;
				}
			}
			dis.close();
		}
		catch(Exception e) { e.printStackTrace(System.out); }

	}
}

class AddShortCardRecordTask implements Runnable
{

	final private java.text.Format datefmt = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	private String  code;
	private int     number;
	private Integer price;
	private Integer rate;
	private Integer pointcard;
	private Integer exchangepoint;
	private Date    endtime;
	private String  creator;
	private Integer func;
	private Integer funcparm;
	private Integer score;


	private static String cardgen(Long no, DataInputStream dis) throws Exception
	{
		String card = "000000000" + no.toString();

		String passwd = "000000" + Integer.toString(Math.abs(dis .readInt()));
		return "S"+card.substring(card.length()-9) + passwd.substring(passwd.length()-6)+"         ";
	}

	private static boolean addcard(Integer id, LinkedList cardn)
	{
		try
		{
			cardn.addFirst(new Integer(1)); //active each card
			cardn.addFirst(id);
			return application.procedure.handler.get("addcard").execute( cardn.toArray(), "auth0" ) == 0;
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return false;
	}

	AddShortCardRecordTask (String code, Integer number, Integer price, Integer rate, Integer pointcard, Integer exchangepoint, Date endtime, String creator, Integer func, Integer funcparm, Integer score)
	{
		String s = "00000000" + new String(code);
		this.code          = s.substring(s.length()-9);
		this.number        = number.intValue();
		this.price         = new Integer(price.intValue());
		this.rate          = new Integer(rate.intValue());
		this.pointcard     = new Integer(pointcard.intValue());
		this.exchangepoint = new Integer(exchangepoint.intValue());
		this.endtime       = (Date)endtime.clone();
		this.creator       = new String(creator);
		this.func          = new Integer(func.intValue());
		this.funcparm      = new Integer(funcparm.intValue());
		this.score         = new Integer(score.intValue());
	}

	public void run()
	{
		
		try
		{
			long start = Long.parseLong(code);
			DataInputStream dis = new DataInputStream(new FileInputStream("/dev/urandom"));
			Object[] o;
			application.procedure.handler.get("addcardrecord").execute(o = new Object[] {
				"S"+code, price, rate, pointcard, exchangepoint, datefmt.format(endtime),
				creator, func, funcparm, score, new Integer(0) }, "auth0");
			Integer id = (Integer)o[10];
			if (id.intValue() > -1) {
				while(number > 0)
				{
					LinkedList cardn = new LinkedList();
					for ( int i = 0; i < 10; i++ ) cardn.addLast(cardgen(new Long(start++),dis));
					if ( addcard(id, cardn) )
						number -= 10;
				}
			}
			dis.close();
		}
		catch(Exception e) { e.printStackTrace(System.out); }

	}
}
public class storage
{
	public static boolean validatepasswd(String name, String passwd)
	{
		try
		{
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(name.getBytes());
			md.update(passwd.getBytes());
			byte[] b0 = md.digest();

			Object[] o = application.query.handler.get("getpasswd").select("getpasswd").execute( new Object[] { name }, "auth0");
			byte[] b1 = (byte[])((Object[])o[0])[0];
			
			return Arrays.equals(b0, b1);
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return false;
	}

	public static void addCardRecord(String code, Integer number, Integer price, Integer rate, Integer pointcard, Integer exchangepoint, Date endtime, String creator, Integer func, Integer funcparm, Integer score )
	{
		Thread thread = new Thread ( 
			new addCardRecordTask(code, number, price, rate, pointcard, 
				exchangepoint, endtime, creator, func, funcparm, score)
		);
		thread.setDaemon(true);
		thread.start();
	}

	public static void addCardRecord(String code, Integer number, Integer price, Integer rate, Integer pointcard, Integer exchangepoint, Date endtime, String creator, Integer func, Integer funcparm, Integer score, boolean isShortCard )
	{
		//short card
		if (isShortCard) {
			Thread thread = new Thread ( 
				new AddShortCardRecordTask(code, number, price, rate, pointcard, 
					exchangepoint, endtime, creator, func, funcparm, score)
			);
			thread.setDaemon(true);
			thread.start();
		} else { // realcard
			Thread thread = new Thread ( 
				new addCardRecordTask(code, number, price, rate, pointcard, 
					exchangepoint, endtime, creator, func, funcparm, score)
			);
			thread.setDaemon(true);
			thread.start();
		}
		
	}

	public static void auditCardRecord(Integer id, String auditor)
	{
		try
		{
			application.procedure.handler.get("auditcard").execute( new Object[] { id, auditor, new Integer(1)}, "auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
	}

	public static void deleteCardRecord(Integer id)
	{
		try
		{
			application.procedure.handler.get("deletecard").execute( new Object[] { id }, "auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
	}

	public static Object[] getCardRecordIDS()
	{
		try
		{
			return application.query.handler.get("cardrecordids").select("all").execute( new Object[] { }, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return new Object[0];
	}

	public static Object[] getCardRecordNewIDS()
	{
		try
		{
			return application.query.handler.get("cardrecordids").select("new").execute( new Object[] { }, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return new Object[0];
	}

	public static Object[] getCardRecordAuditedIDS()
	{
		try
		{
			return application.query.handler.get("cardrecordids").select("audited").execute( new Object[] { }, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return new Object[0];
	}

	public static Object[] getCardRecordInfo(Integer id)
	{
		try
		{
			Object[] rows = (Object[]) application.query.handler.get("cardrecordinfo").select("item").
				execute( new Object[] { id }, "auth0" );
			return (Object[]) rows[0];
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}

	public static int getCardStatus( String fromnumber, String tonumber, Integer status, String op )
	{
		try
		{
			if( fromnumber.length() != 10 || tonumber.length() != 10 )
				return -3;

			long from = Long.parseLong(fromnumber);
			long to = Long.parseLong(tonumber);
			to ++;
			tonumber = new String(""+to);

			if( to - from > 30000 )
				return -4;

			Object[] parameters = new Object[] { fromnumber, tonumber, status, op, new Integer(0) };
			if( application.procedure.handler.get("getcardstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[4]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int checkCardStatus( String fromnumber, String tonumber, Integer status, String op )
	{
		try
		{
			if( fromnumber.length() != 10 || tonumber.length() != 10 )
				return -3;

			long from = Long.parseLong(fromnumber);
			long to = Long.parseLong(tonumber);
			to ++;
			tonumber = new String(""+to);

			if( to - from > 30000 )
				return -4;

			Object[] parameters = new Object[] { fromnumber, tonumber, status, op, new Integer(0) };
			if( application.procedure.handler.get("checkcardstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[4]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int changeCardStatus( String fromnumber, String tonumber, Integer fromstatus, Integer tostatus, String op )
	{
		try
		{
			if( fromnumber.length() != 10 || tonumber.length() != 10 )
				return -3;

			long from = Long.parseLong(fromnumber);
			long to = Long.parseLong(tonumber);
			to ++;
			tonumber = new String(""+to);

			if( to - from > 30000 )
				return -4;

			Object[] parameters = new Object[] { fromnumber, tonumber, fromstatus, tostatus, op, new Integer(0) };
			if( application.procedure.handler.get("changecardstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[5]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int confirmCard( String fromnumber, String tonumber, String op )
	{
		try
		{
			Object[] parameters = new Object[] { fromnumber, tonumber, new Integer(0), new Integer(1), op, new Integer(0) };
			if( application.procedure.handler.get("changecardstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[5]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int discardCard( String fromnumber, String tonumber, String op )
	{
		try
		{
			Object[] parameters = new Object[] { fromnumber, tonumber, new Integer(1), new Integer(2), op, new Integer(0) };
			if( application.procedure.handler.get("changecardstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[5]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static Object[] downloadCard(Integer id)
	{
		try
		{
			Object[] result=application.query.handler.get("downloadcard").select("all").execute( new Object[] { id }, "auth0" );
			System.out.println("========= Download Card id="+id+"  result="+result.length+" =================");
			return result;
		}
		catch (Exception e) { e.printStackTrace(); }
		return new String[0];
	}
	
	/*
	public static Object[] downloadCard(Integer id, boolean isShortCard)
	{
		try
		{
			Object[] result=application.query.handler.get("downloadcard").select("all").execute( new Object[] { id }, "auth0" );
			if (isShortCard) {
				for (Object res: result) {
					res = ((String)res).substring(1);
				}
			}
			System.out.println("========= Download Card id="+id+"  result="+result.length+" =================");
			return result;
		}
		catch (Exception e) { e.printStackTrace(); }
		return new String[0];
	}
	*/
	static
	{
		try
		{
			// parser.parse(storage.class.getResourceAsStream("/table.xml"));
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public static void main(String[] args)
	{
	}
}
