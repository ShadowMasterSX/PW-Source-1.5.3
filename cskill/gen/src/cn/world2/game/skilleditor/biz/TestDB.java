/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.biz;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.User;

/**
 * @author linxiaojun
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class TestDB {
	public List getUsers() {
		Connection conn = ConnectionService.getConnection();
		Statement stmt;
		try {
			stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery("select * from user");
			List<User> list = new ArrayList<User>();
			while(rs.next()){
				User u = new User();
				u.setId(rs.getString("id"));
				u.setAccount(rs.getString("account"));
				list.add(u);
			}
			return list;
			
		} catch (SQLException e) {
			e.printStackTrace();
			return null;
		} finally {
			ConnectionService.closeConnection(conn);
		}
	}
}
