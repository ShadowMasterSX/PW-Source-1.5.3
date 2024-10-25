/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO;

import java.util.List;

import cn.world2.game.skilleditor.vo.Role;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public interface IRoleDAO {
	/**
	 * 插入一角色
	 * @param role
	 * @return
	 */
	boolean insertOneRole(Role role);
	/**
	 * 删除一角色
	 * @param role
	 * @return
	 */
	boolean deleteOneRole(Role role);
	/**
	 * 更新一角色
	 * @param newRole
	 * @return
	 */
	int updateOneRole(Role newRole);
	/**
	 * 按照条件获得较色，
	 * 如果输入null则返回所有角色
	 * @param condition
	 * @return
	 */
	List getRoles(String condition);
	/**
	 * 返回含有skills的Role列表
	 * 如果输入null则返回所有角色
	 * @param condition
	 * @return
	 */
	List getRolesWithSkills(String condition);
	/**
	 * 取回一个Role
	 * @param id
	 * @return
	 */
	Role retrieveRole(String id);
	/**
	 * 删除一序列role
	 * @param idlist
	 */
	int deleteRoles(List idlist);
}
