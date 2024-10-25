/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.vo;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class User {
	private String id;
	private String account;
	private String password;
	private String workpath;
	/**
	 * @return Returns the password.
	 */
	public String getPassword() {
		return password;
	}
	/**
	 * @param password The password to set.
	 */
	public void setPassword(String password) {
		this.password = password;
	}
	/**
	 * @return Returns the account.
	 */
	public String getAccount() {
		return account;
	}
	/**
	 * @param account The account to set.
	 */
	public void setAccount(String account) {
		this.account = account;
	}
	/**
	 * @return Returns the id.
	 */
	public String getId() {
		return id;
	}
	/**
	 * @param id The id to set.
	 */
	public void setId(String id) {
		this.id = id;
	}
	/**
	 * @return Returns the workpath.
	 */
	public String getWorkpath() {
		return workpath;
	}
	/**
	 * @param workpath The workpath to set.
	 */
	public void setWorkpath(String workpath) {
		this.workpath = workpath;
	}
}
