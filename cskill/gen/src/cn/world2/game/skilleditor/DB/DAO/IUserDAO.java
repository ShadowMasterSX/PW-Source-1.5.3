/*
 * Created on 2005-1-22
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO;

import cn.world2.game.skilleditor.vo.User;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public interface IUserDAO {
	User getUser(String userid);
	User getUser(String account,String password);
	User saveWorkpath(String userid,String wp);
}
