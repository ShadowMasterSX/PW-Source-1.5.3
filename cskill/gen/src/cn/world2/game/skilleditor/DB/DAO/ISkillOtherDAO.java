/*
 * Created on 2005-1-25
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO;

import cn.world2.game.skilleditor.vo.SkillOtherVO;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public interface ISkillOtherDAO {
	SkillOtherVO getSkillOtherVOById(String skillid);
	boolean insertSkillOtherVO(SkillOtherVO sov);
	boolean updateSkillOtherVO(SkillOtherVO sov);
}
