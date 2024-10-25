/*
 * Created on 2005-1-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO.JDBC;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import cn.world2.game.skilleditor.DB.DAO.IWeaponDAO;
import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.Weapon;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class WeaponDAOImp implements IWeaponDAO{

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IWeaponDAO#getAvailableWeapon()
	 */
	public List<Weapon> getAvailableWeapon() {
		String sqlStr="select id,name from weapon";
		Connection conn=ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			List<Weapon> list = new ArrayList<Weapon>();
			while(rs.next()){
				Weapon w = new Weapon();
				String id=rs.getString("id");
				String name=rs.getString("name");
				w.setWeaponid(id);
				w.setWeaponname(name);
				list.add(w);
			}
			stmt.close();
			return list;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

}
