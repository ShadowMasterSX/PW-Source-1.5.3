package RandomGen;

import java.util.Random;
import java.lang.Math.*;
import java.lang.*;
import java.io.*;
public class StrGenerator
{
	private Random rdgen;

	private DataInputStream dis;

	private static String genpasswd(int len,DataInputStream dis) throws Exception
	{
		if( len > 20 || len <= 0 )	throw new Exception("len is invalid");

		String passwd = "00000000000000000000" 
			+ Long.toString(Math.abs(dis.readLong())) 
			+ Integer.toString(Math.abs(dis.readInt()));

		return passwd.substring(passwd.length()-len);
	}

	private StrGenerator() throws Exception
	{
		rdgen=new Random();
		dis = new DataInputStream(new FileInputStream("/dev/urandom"));
	}
	public String Generate_Mix(int _size)
	{
		byte[] res= new byte[_size];
		byte achar=0;
		for (int i=0;i<_size;i++)
		{
			do
			{
				achar=(byte)((rdgen.nextInt()^0x5c4e68c7) % 64 +48);	
			}while ( !(
				(achar>='3' && achar<='9' && achar!='5' && achar!='8')
				|| (achar>='a' && achar<='z' && achar!='o' && achar!='i' && achar!='l' && achar!='z' && achar!='s' && achar!='b')
			) );
			res[i]=achar;
		}
		return new String(res);
	}
	public String Generate_Num(int _size)
	{
		byte[] res= new byte[_size];
		byte achar=0;
		for (int i=0;i<_size;i++)
		{
			do
			{
				achar=(byte)(rdgen.nextInt() % 64 +48);	
			}while ( !((achar>=48 && achar<=57)) );
			res[i]=achar;
		}
		return new String(res);
	}
	public String Generate_Num(int _size,int value) throws Exception
	{
		if( _size > 20 || _size <= 0 )	throw new Exception("_size is invalid");

		String name = "00000000000000000000" + value;
		return name.substring(name.length()-_size);
	}
	public static StrGenerator GetInstance() throws Exception
	{
		StrGenerator instance=new StrGenerator();
		return instance;
	}	
}

