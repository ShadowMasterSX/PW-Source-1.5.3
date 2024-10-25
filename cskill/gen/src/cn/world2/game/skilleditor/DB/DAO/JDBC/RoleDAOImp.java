/*
 * Created on 2004-12-21
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

import cn.world2.game.skilleditor.DB.DAO.IRoleDAO;
import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.Role;
import cn.world2.game.skilleditor.vo.Skill;

/**
 * @author linxiaojun
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class RoleDAOImp implements IRoleDAO {

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.IRoleDAO#insertOneRole(cn.world2.game.skilleditor.vo.Role)
	 */
	public boolean insertOneRole(Role role) {
		String sqlStr="insert into role (name) values('"+role.getName()+"')";
		System.out.println(sqlStr);
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt  = conn.createStatement();
			stmt.execute(sqlStr);
			return true;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.IRoleDAO#deleteOneRole(cn.world2.game.skilleditor.vo.Role)
	 */
	public boolean deleteOneRole(Role role) {
		// TODO Auto-generated method stub
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.IRoleDAO#updateOneRole(cn.world2.game.skilleditor.vo.Role)
	 */
	public int updateOneRole(Role newRole) {
		// TODO Auto-generated method stub
		return 0;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.IRoleDAO#getRoles(java.lang.String)
	 */
	public List<Role> getRoles(String condition) {
		String sqlWhere = "";
		if (condition != null) {
			sqlWhere = sqlWhere + condition;
		}
		String sqlStr = "select * from role " + sqlWhere + "order by id";
		Connection conn = ConnectionService.getConnection();
		Statement stmt;
		try {
			stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			List<Role> list = new ArrayList<Role>();
			while(rs.next()){
				Role role =new Role();
				role.setId(rs.getString("id"));
				role.setName(rs.getString("name"));
				list.add(role);
			}
			return list;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IRoleDAO#retrieveRole(java.lang.String)
	 */
	public Role retrieveRole(String id) {
		String sqlStr = "select * from role where id='"+id+"' order by id";
		Role role = null;
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			while(rs.next()){
				role = new Role();
				role.setId(rs.getString("id"));
				role.setName(rs.getString("name"));
				return role;
			}			
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IRoleDAO#getRolesWithSkills(java.lang.String)
	 */
	public List getRolesWithSkills(String condition) {
		if(condition==null) condition="";
		String sqlRole="select * from role "+condition + " order by id";
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlRole);
			List<Role> list = new ArrayList<Role>();
			while(rs.next()){
				Role role = new Role();
				String roleid = rs.getString("id");
				String rolename=rs.getString("name");
				role.setId(roleid);//(1) roleid
				role.setName(rolename);//(2) rolename
				String sqlSkill = "select id,name from skill where roleid='"+roleid+"' order by id";
				Statement stmt2 = conn.createStatement();				
				ResultSet rs2=stmt2.executeQuery(sqlSkill);
				List<Skill> skills=new ArrayList<Skill>();
				while(rs2.next()){
					Skill skill =new Skill();
					String skillid=rs2.getString("id");
					String skillname=rs2.getString("name");
					skill.setId(skillid);
					skill.setName(skillname);
					skills.add(skill);
				}
				role.setSkills(skills);
				list.add(role);
			}
			return list;
		} catch (SQLException e) {
			e.printStackTrace();
			return null;
		} finally {
			ConnectionService.closeConnection(conn);
		}
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IRoleDAO#deleteRoles(java.util.List)
	 */
	public int deleteRoles(List idlist) {
		if (idlist == null || idlist.size() == 0)
			return 0;
		Connection conn = ConnectionService.getConnection();
		try {
			conn.setAutoCommit(false);
			conn.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);
			Statement stmt = conn.createStatement();
			for (int k = 0; k < idlist.size(); k++) {
				String id = (String) idlist.get(k);
				String sqlStr = "delete from role where id='" + id + "'";
				stmt.addBatch(sqlStr);
			}
			int[] i = stmt.executeBatch();
			conn.commit();
			return i.length;
		} catch (SQLException e) {
			try {
				conn.rollback();
			} catch (SQLException e1) {
				e1.printStackTrace();
			}
			return -1;
		} finally {
			ConnectionService.closeConnection(conn);
		}
	}
}
