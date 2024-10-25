/*
 * Created on 2005-1-13
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.config;

import java.io.UnsupportedEncodingException;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ArmlimitationConfig {
	public static String[] getArmlimitationConfig(){
		String a1=new String("µ¶½£");
		String a2=new String("³¤±ø");
		String a3=new String("¸«´¸");
		String a4=new String("È­Ì×");
		String a5=new String("³¤±Þ");
		String a6=new String("¹­¼ý");
		String a7=new String("·¨Æ÷");
		String[] armlimitation=new String[7];
		try {
			byte[] b1=a1.getBytes("gb2312");
			byte[] b2=a2.getBytes("gb2312");
			byte[] b3=a3.getBytes("gb2312");
			byte[] b4=a4.getBytes("gb2312");
			byte[] b5=a5.getBytes("gb2312");
			byte[] b6=a6.getBytes("gb2312");
			byte[] b7=a7.getBytes("gb2312");
			String aa1=new String(b1);
			String aa2=new String(b2);
			String aa3=new String(b3);
			String aa4=new String(b4);
			String aa5=new String(b5);
			String aa6=new String(b6);
			String aa7=new String(b7);
			armlimitation[0]=aa1;
			armlimitation[1]=aa2;
			armlimitation[2]=aa3;
			armlimitation[3]=aa4;
			armlimitation[4]=aa5;
			armlimitation[5]=aa6;
			armlimitation[6]=aa7;
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		
		return armlimitation;
	}
}
