/*
 * Created on 2005-1-8
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO;

import java.util.List;

import cn.world2.game.skilleditor.vo.Dictionary;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public interface IDictionaryDAO {
	Dictionary getDitionary(String key);
	/**
	 * 获得公共字典
	 * @return
	 */
	List getAllDictionary();
	boolean insertDictionary(Dictionary d);
	/**
	 * 更新公共字典
	 * @param list
	 * @return
	 */
	boolean updateDictionary(List list);
	/**
	 * 获得该技能表达式私有的字典
	 * @return
	 */
	List getSkillPrivateDict(String skillid);
	/**
	 * 获得该技能表达式公共的字典
	 * @param list
	 * @return
	 */
	boolean updateSkillPrivateDict(List list,String skillid);	
}
