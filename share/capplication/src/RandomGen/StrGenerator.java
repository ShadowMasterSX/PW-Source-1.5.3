package RandomGen;

import java.util.Random;
import java.lang.Math.*;
import java.lang.*;
import java.io.FileOutputStream;
public class StrGenerator
{
	private Random rdgen;
	private static StrGenerator instance=new StrGenerator();

	private StrGenerator()
	{
		rdgen=new Random();
	}
	public String Generate_Mix(int _size)
	{
		byte[] res= new byte[_size];
		byte achar=0;
		for (int i=0;i<_size;i++)
		{
			do
			{
				achar=(byte)(rdgen.nextInt() % 64 +48);	
			}while ( !((achar>=50 && achar<=57) || (achar>=65 && achar<=90 && achar!=73 && achar!=79))  );
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
	public static StrGenerator GetInstance()
	{
		return instance;
	}	
}

