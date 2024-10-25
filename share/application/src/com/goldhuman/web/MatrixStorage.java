package com.goldhuman.web;

import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Date;



class addMatrixRecordTask implements Runnable
{
	private final static int MATRIX_LENGTH = 80;
	final private java.text.Format datefmt = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

	private String  code;
	private int     number;
	private Integer price;
	private Integer rate;
	private Date expiredtime;
	private String  creator;
	
	
	private static byte[] matrixCardGenerate(DataInputStream dis) throws IOException 
	{
		byte[] matrix = new byte[MATRIX_LENGTH];

		try {
			if ( dis.read(matrix) != MATRIX_LENGTH){
				throw new IOException();
			}
		} catch (IOException e) {
			throw e;
		}
		for (int i = 0; i <MATRIX_LENGTH; i++) {
			matrix[i] = (byte)((matrix[i]&0x7F)%100);
		}
		
		if ( matrix[MATRIX_LENGTH - 1] == 0) {
			matrix[MATRIX_LENGTH - 1] = 99;  //for sybase database
		}
	
		return matrix;
	}

	private static void addMatrixCard(Integer recordid, String matrixId, byte[] matrix) throws Exception
	{
		if ((matrixId == null) || (matrixId.length() >12)){
			return;
		}
		String matrixId12 = matrixId;
		if (matrixId12.length() < 12){
			matrixId12 = "000000000000".substring(0,12-matrixId12.length()) + matrixId12;
		}
		
		
		application.procedure.handler.get("addnewmatrix").execute( 
					new Object[]{recordid, matrixId12, matrix}, "auth0" );

	}

	addMatrixRecordTask (String code, Integer number, Integer price, Integer rate, Date expiredtime, String creator )
	{
		this.code          = new String(code);
		this.number        = number.intValue();
		this.price         = new Integer(price.intValue());
		this.rate          = new Integer(rate.intValue());
		this.expiredtime   = expiredtime;
		this.creator       = creator;
		
	}

	public void run()
	{
		try
		{
			long start = Long.parseLong(code);
			DataInputStream dis = new DataInputStream(new FileInputStream("/dev/urandom"));
			//DataInputStream dis = new DataInputStream(addMatrixRecordTask.class.getResourceAsStream("/kaspersky.rar"));
			Object[] o;
			// create add matrix record
			int ret = application.procedure.handler.get("addmatrixrecord").execute(
						(o = new Object[] {code, number, price, rate, datefmt.format(expiredtime), creator, new Integer(0)}),
						"auth0");
			if (ret != 0){
				System.out.println("addmatrixrecord failed , code = " + code);
				return; 
			}

			byte[] matrixNew;
			while(number > 0)
			{
				matrixNew = matrixCardGenerate(dis);
				
				addMatrixCard((Integer)o[6], String.valueOf(start), matrixNew);
				start++;
				number --;
			}
			dis.close();
			System.out.println("addmatrixrecord finished , code = " + code);
		}
		catch(Exception e) { e.printStackTrace(System.out); }
	}
}

public class MatrixStorage {
	
	/**
	 * add specified number of new matrix from code.
	 * @param code begin id of new matrixs.lenght of code should be 12. for example '123456000000'
	 * @param number count of new matrixs should be added. 
	 * @param price 
	 * @param rate 
	 * @param expiredtime overdue time
	 * @param creator people who create a batch of new matrixs.
	 */
	public static void addMatrix(String code, Integer number, Integer price, Integer rate, Date expiredtime, String creator )
	{
		if ((code == null) || (code.length() >12)){
			return;
		}
		if (code.length() < 12){
			code = "000000000000".substring(0,12-code.length()) + code;
		}
		if (!"000000".equals(code.substring(6))){
			return;
		}
		
		Thread thread = new Thread ( 
			new addMatrixRecordTask(code, number, price, rate, expiredtime, creator)
		);
		thread.setDaemon(true);
		thread.start();
	}
	
	/**
	 * 
	 * @param id
	 * @param auditor
	 */
	public static int auditMatrixRecord(Integer id, String auditor)
	{
		try
		{
			return application.procedure.handler.get("auditmatrix").execute(
					new Object[] { id, auditor, new Integer(1)}, "auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	/**
	 * delete MatrixRecord and also its related new matrixs. 
	 * only make effect when value of status is zero. 
	 * @param id
	 * @return count of new matrixs be deleted when success, value negative when failure.  
	 */
	public static int deleteMatrixRecord(Integer id)
	{
		try
		{
			return application.procedure.handler.get("deletematrix").execute( new Object[] { id }, "auth0");
		}
		catch(Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	
	public static Object[] getMatrixRecordIDS()
	{
		try
		{
			return application.query.handler.get("matrixrecordids").select("all").execute( new Object[] { }, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return new Object[0];
	}
	
	/**
	 * get id array of MatrixRecord when its status is 0.
	 * @return
	 */
	public static Object[] getMatrixRecordNewIDS()
	{
		try
		{
			return application.query.handler.get("matrixrecordids").select("new").execute( new Object[] { }, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return new Object[0];
	}

	/**
	 * get id array of MatrixRecord when its status is 1.
	 * @return
	 */
	public static Object[] getMatrixRecordAuditedIDS()
	{
		try
		{
			return application.query.handler.get("matrixrecordids").select("audited").execute( new Object[] { }, "auth0" );
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return new Object[0];
	}

	/**
	 * get matrix record information according to id.
	 * @param id
	 * @return matric record information when success, value null when failure.
	 */
	public static Object[] getMatrixRecordInfo(Integer id) {
		try
		{
			Object[] rows = (Object[]) application.query.handler.get("matrixrecordinfo").select("item").
				execute( new Object[] { id }, "auth0" );
			if (rows.length == 0) {
				return null;
			} else {
				return (Object[]) rows[0];
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}
	
	/*
	public static int checkMatrixOverdueStatus( String fromnumber, String tonumber, Integer status, String op )
	{
		try
		{
			if( tonumber.length() > 0 )
			{
				long x = Long.parseLong(tonumber);
				x ++;
				tonumber = new String(""+x);
			}

			Object[] parameters = new Object[] { fromnumber, tonumber, status, op, new Integer(0) };
			if( application.procedure.handler.get("checkcardstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[4]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	*/
	

	/**
	 * change matrix record to any status.
	 * @deprecated 
	 * @see auditMatrixRecord
	 * @see confirmMatrix
	 * @see discardmatirx
	 */
	public static int changeMatrixStatus( Integer id, Integer tostatus ) {
		try
		{

			Object[] parameters = new Object[] { id, tostatus, new Integer(0) };
			if( application.procedure.handler.get("changematrixstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[2]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	
	/**
	 * make a group of matrix available(status =1)
	 * @param id 
	 * @return value count of matrixnew table rows affected when success, value -1 when failure 
	 */
	public static int confirmMatrix( Integer id ) {
		try
		{
			Object[] parameters = new Object[] { id, new Integer(1), new Integer(0) };
			if( application.procedure.handler.get("changematrixstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[2]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	/**
	 * make a group of matrix overdue(status =2)
	 * @param id
	 * @return value count of matrixnew table rows affected when success, value -1 when failure 
	 */
	public static int discardMatrix( Integer id ) {
		try
		{
			Object[] parameters = new Object[] { id, new Integer(2), new Integer(0) };
			if( application.procedure.handler.get("changematrixstatus").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[2]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	/**
	 * only when status = 2 matrixnew can be cleard
	 * @deprecated
	 * @param id
	 * @return
	 */
	public static int clearMatrix( Integer id )	{
		
		try
		{
			Object[] parameters = new Object[] { id, new Integer(0) };
			if( application.procedure.handler.get("clearmatrix").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[1]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	/**
	 * get a batch of newmatrix according to id.
	 * @param id
	 * @return all rows of newmatrix hold recordid=id.
	 */
	public static Object[] downloadMatrix(Integer id){
		
		try
		{
			Object[] result=application.query.handler.get("downloadmatrix").select("all").execute( new Object[] { id }, "auth0" );
			System.out.println("========= Download Card id="+id+"  result="+result.length+" =================");
			return result;
		}
		catch (Exception e) { e.printStackTrace(); }
		return new String[0];
	}
	
	/**
	 * get id of matrixrecod by code 
	 * @param code code of matrixrecord. for example "123456000000"
	 * @return value id when success, value negative when failure 
	 */
	public static int getMatrixRecordIdByCode(String code) {
		try
		{
			Object[] rows  ;
			rows = 
				application.query.handler.get("querymatrixrecordid").select("id").execute( (new Object[]{ code }), "auth0");
			if (rows.length == 0) {
				return -1;
			} else {
				return ((Integer)((Object[])rows[0])[0]).intValue();
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
		
	}

	/**
	 * get new matrix information by matrixId 
	 * @param matrixId Id of new matrix
	 * @return value null if no matrix be found; value Object[] contained new matrixinfo when success.
	 *         Object[0]:recordid  Object[1]|:matrixId Object[2]:matrix
	 */
	/*	
	public static Object[] getNewMatrixById(String matrixId) {
		try
		{
			Object[] rows  ;
			rows = 
				application.query.handler.get("querynewmatrix").select("id").execute( (new Object[]{ matrixId }), "auth0");
			if (rows.length == 0) {
				return null;
			} else {
				return (Object[])rows[0];
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
		
	}*/

	public static Object[] getNewMatrixById(String matrixId) {
		try
		{
			Object[] parameters = new Object[] { new Integer(0), matrixId, new byte[80],new Integer(0) };
			if (application.procedure.handler.get("querymatrixnewbyid").execute(parameters, "auth0") == 0){
				if ( 0 == (Integer)parameters[3]){
					return parameters;
				} 
			}
			return null;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;		
	}
	
	/**
	 * get used matrix information by matrixId 
	 * @param matrixId Id of new matrix
	 * @return value null if no matrix be found; value Object[] contained used matrixinfo when success.
	 *         Object[0]:id  Object[1]|:matrix Object[2]:userid Object[3]:begindate Object[4]:enddate Object[5]:ip 
	 */
	public static Object[] getMatrixUsedById(String matrixId) {
		try
		{
			Object[] rows  ;
			rows = 
				application.query.handler.get("querymatrixused").select("id").execute( (new Object[]{ matrixId }), "auth0");
			if (rows.length == 0) {
				return null;
			} else {
				return (Object[])rows[0];
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
		
	}
	
	/**
	 * get used matrix information by userId 
	 * @param matrixId Id of new matrix
	 * @return value null if no matrix be found; value Object[] contained all rows of relative usedmatrixinfo when success.
	 *         Object[][0]:id  Object[][1]|:matrix Object[][2]:userid Object[][3]:begindate Object[4]:enddate Object[5]:ip 
	 */
	public static Object[] getMatrixUsedByUserId(Integer userId) {
		try
		{
			Object[] rows  ;
			rows = 
				application.query.handler.get("querymatrixused").select("userid").execute( (new Object[]{ userId }), "auth0");
			if (rows.length == 0) {
				return null;
			} else {
				return rows;
			}
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
		
	}
	

	/**
	 * 
	 * @param matrixId
	 * @param coord
	 * @return -1.matrixId invalid, -2 coordinates invalid
	 */
	/*
	public static int checkNewMatrix(String matrixId, int[] coord, String verifyCode ) {
		if ((matrixId == null) || (matrixId.length() != 12)) {
			return -1;
		}
		
		if ((coord == null) || (coord.length < 4)) {
			return -2;
		}
		
		if ((verifyCode == null) || (verifyCode.length != 4)) {
			return -3;
		}
		
		int fisrt;
		int second;
		
		int retcode = 0;
		byte[] matrix;
		Object[] rows = getNewMatrixById(matrixId)  ;
		if (rows == null) {
			retcode = -4;//not found
		} else {
			matrix = (byte[])(((Object[])rows[0])[2]);
			int posi1 = (coord[0] << 3 + coord[1]);
			int posi2 = (coord[2] << 3 + coord[3]);
			matrix[posi1] = ;
			matrix[posi2];
		}
		return retcode;
	}*/

	public static int bindMatrix( Integer userId, String matrixId, Integer IP)	{
		
		try
		{
			Object[] parameters = new Object[] { userId, matrixId, IP };
			return application.procedure.handler.get("bindmatrix").execute(parameters, "auth0");
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}
	
	public static int unbindMatrix( Integer userId, String matrixId, Integer IP)	{
		
		try
		{
			Object[] parameters = new Object[] { userId, matrixId, IP };
			return application.procedure.handler.get("unbindmatrix").execute(parameters, "auth0"); 
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int rebindMatrix( Integer userId, String matrixIdold, String matrixId, Integer IP)	{
		
		try
		{
			Object[] parameters = new Object[] { userId, matrixIdold, matrixId, IP };
			return application.procedure.handler.get("rebindmatrix").execute(parameters, "auth0"); 
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	public static int queryMatrixUsedCountByRecordId( Integer id )	{
		try
		{
			Object[] parameters = new Object[] { id, new Integer(0) };
			if( application.procedure.handler.get("querymatrixusedcount").execute(parameters, "auth0") == 0 )
				return ((Integer)parameters[1]).intValue();
			return -1;
		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return -1;
	}

	/**
	 * get used matrix information by userId 
	 * @param userId Id of user
	 * @return value null if no matrix be found; value Object[] contained matrixinfo of this user when success.
	 *         Object[0]:id  Object[1]:recordid Object[2]:matrixid Object[3]:matrix  
	 */
	public static Object[] getMatrixByUserId(Integer userId) {
		try
		{
			Object[] parameters = new Object[]{userId, new Integer(0) , new String(""), new byte[80] };
                        if (application.procedure.handler.get("querymatrixbyid").execute(parameters, "auth0") == 0) {
                                return parameters;
                        } else {
                                return null;
                        }

		}
		catch (Exception e) { e.printStackTrace(System.out); }
		return null;
	}	
	
	public static void main(String[] args)
	{
	}
}
