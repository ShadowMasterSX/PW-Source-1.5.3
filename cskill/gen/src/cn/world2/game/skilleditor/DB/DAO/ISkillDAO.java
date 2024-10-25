/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO;

import java.io.Serializable;
import java.util.List;

import cn.world2.game.skilleditor.vo.Skill;
import cn.world2.game.skilleditor.vo.data.SkillLevelVO;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public interface ISkillDAO {
	/**
	 * 按照技能id取得某个技能所有。
	 * @param skillid
	 * @return
	 */
	Skill getSkillById(String skillid);
	/**
	 * 插入技能
	 * @param skill
	 * @return
	 */
	String insertSkill(Skill skill);
	/**
	 * 删除技能
	 * @param skill
	 * @return
	 */
	int deleteSkill(Skill skill);
	/**
	 * 删除技能
	 * @param idlist
	 * @return
	 */
	int deleteSkill(List idlist);
	/**
	 * 更新技能
	 * @param skill
	 * @return
	 */
	boolean updateSkill(Skill skill);
	/**
	 * 获得技能列表
	 * @param condtion
	 * @return
	 */
	List getSkill(String condtion);
	/**
	 * 获得所有技能简短列表
	 * （只包含id和名字）
	 * @param condition
	 * @return
	 */
	List getSkillsShort(String condition);
	/**
	 * 更新skill级别
	 * @param slvo
	 * @return
	 */
	boolean updateSkillLevel(SkillLevelVO slvo,String skillid);
	/**
	 * 获得skill级别
	 * @param skillid
	 * @return
	 */
	SkillLevelVO getSkillLevel(String skillid);
	/**
	 * 更新技能基本编辑
	 * @param skill
	 * @return
	 */
	boolean updateSkillBasic(Skill skill);
	/**
	 * 获得基本技能的数据
	 * @param skillid
	 * @return
	 */
	Skill getSkillBasic(String skillid);
	/**
	 * 保存Skill的过程
	 * @param p
	 * @return
	 */
	boolean updateSkillProcedure(List list,String skillid);
	/**
	 * 获得SKILL的过程
	 * @param skillid
	 * @return
	 */
	List getSkillProcedure(String skillid);
	/**
	 * 通过查询更新技能，可以执行任意INSERT,UPDATE,DELETE
	 * @param query
	 * @return
	 */
	boolean updateSkillByQuery(String query);
	/**
	 * 拷贝技能用
	 * @param query
	 * @return
	 */
	String copySkillByQuery(String query,String oldrole);
	void updateBlob(Serializable s,String field,String skillid);
	Serializable getBlob(String field,String skillid);
}
