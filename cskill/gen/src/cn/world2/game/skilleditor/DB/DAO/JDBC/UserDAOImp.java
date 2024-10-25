/*
 * Created on 2005-1-22
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO.JDBC;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import cn.world2.game.skilleditor.DB.DAO.IUserDAO;
import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.User;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class UserDAOImp implements IUserDAO{

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IUserDAO#getUser(java.lang.String)
	 */
	public User getUser(String userid) {
		String sqlStr="select id,account,password,workpath from user_ where id='"+userid+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			while(rs.next()){
				User u = new User();
				String id=rs.getString("id");
				String account=rs.getString("account");
				String password=rs.getString("password");
				String wp = rs.getString("workpath");
				u.setId(id);
				u.setAccount(account);
				u.setPassword(password);
				u.setWorkpath(wp);
				stmt.close();
				return u;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IUserDAO#getUser(java.lang.String, java.lang.String)
	 */
	public User getUser(String account, String password) {
		String sqlStr="select id,account,password,workpath from user_ where account='"+account+"' and password='"+password+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs=stmt.executeQuery(sqlStr);
			while(rs.next()){
				User u = new User();
				String id=rs.getString("id");
//				String account=rs.getString("account");
//				String password=rs.getString("password");
				String wp = rs.getString("workpath");
				u.setId(id);
				u.setAccount(account);
				u.setPassword(password);
				u.setWorkpath(wp);
				stmt.close();
				return u;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IUserDAO#saveWorkpath(java.lang.String, java.lang.String)
	 */
	public User saveWorkpath(String userid, String wp) {
		String sqlStr="update user_ set workpath=? where id=?";
		//System.out.println(sqlStr);
		String sqlStr2="select id,account,password,workpath from user_ where id='"+userid+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement(sqlStr);
			Statement stmt = conn.createStatement();
			ps.setString(1,wp);
			ps.setString(2,userid);
			ps.executeUpdate();
			ps.close();
			ResultSet rs=stmt.executeQuery(sqlStr2);
			while(rs.next()){
				User u = new User();
				String id=rs.getString("id");
				String account=rs.getString("account");
				String password=rs.getString("password");
//				String wp = rs.getString("workpath");
				u.setId(id);
				u.setAccount(account);
				u.setPassword(password);
				u.setWorkpath(wp);
				stmt.close();
				return u;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}
}
