/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.biz;

import java.util.List;

import cn.world2.game.skilleditor.DB.DAO.IRoleDAO;
import cn.world2.game.skilleditor.DB.DAO.JDBC.RoleDAOImp;
import cn.world2.game.skilleditor.vo.Role;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class RoleManager {
	IRoleDAO rd;
	public RoleManager(){
		rd = new RoleDAOImp();
	}
	public void insertOneRole(Role role){
		rd.insertOneRole(role);
	}
	public List getAllRoles(){
		return rd.getRoles(null);
	}
	public Role getRoleById(String roleid){
		return rd.retrieveRole(roleid);
	}
	public List getAllRolesWithSkills(){
		List list = null;
		list = rd.getRolesWithSkills(null);
		return list;
	}
	public void deleteRole(List roleidlist){
		this.rd.deleteRoles(roleidlist);
	}
}
