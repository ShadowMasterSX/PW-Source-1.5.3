/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.service;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;
import java.util.*;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.sql.DataSource;

/**
 * @author linxiaojun
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class ConnectionService {
	private static Properties pr = null;
	private static LinkedList m_Connections = new LinkedList();
	private static int max_conn = 20;
	private static int cur_conns = 0;

	static {
		pr = new Properties();
		pr.put("useUnicode", "TRUE");
		pr.put("characterEncoding", "gbk");

		try {
			Class.forName("com.mysql.jdbc.Driver");
		} catch (ClassNotFoundException e1) {
			e1.printStackTrace();
		}
	}

	public static synchronized Connection getConnection() {
		Connection connection = null;
		while(m_Connections.size()!=0) {
			try {
				connection = (Connection) m_Connections.removeFirst();
				if (connection.isClosed()) {
					cur_conns -= 1;
					continue;
				}

				return connection;
			} catch (Exception e) {
				e.printStackTrace();
				return null;
			}
		}

		try {
			if(cur_conns >= max_conn) {
				return null;
			}
			connection = DriverManager.getConnection("jdbc:mysql://10.68.16.55:3306/cskill?user=root", pr);
			cur_conns += 1;
		} catch (SQLException e2) {
			e2.printStackTrace();
		}
		return connection;
	}

	public static synchronized void closeConnection(Connection conn) {
		try {
			if (conn != null) {
				m_Connections.add(conn);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
}
