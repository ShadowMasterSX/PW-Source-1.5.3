/*
 * Created on 2005-2-26
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.vo;

import java.io.Serializable;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class HitStatus implements Serializable{
	/**
	 * Comment for <code>serialVersionUID</code>
	 */
	private static final long serialVersionUID = 1L;
	private String id;
	private String name;
	private String iconname;
	private String effectfilename;
	private boolean need_probability;//需要概率
	private boolean need_time;//需要时间
	private boolean need_proportion;//需要比例
	private boolean need_increasement;//需要增量
	private boolean need_total;//需要总量
	private boolean need_value;//需要数值
	private String execute;//执行方法

	private String reserved1; // 保留
	private String reserved2; // 保留
	public HitStatus(){
	}
	public HitStatus(String id){
		this.id=id;
	}
	/**
	 * @return Returns the effectfilename.
	 */
	public String getEffectfilename() {
		return effectfilename;
	}
	/**
	 * @param effectfilename The effectfilename to set.
	 */
	public void setEffectfilename(String effectfilename) {
		this.effectfilename = effectfilename;
	}
	/**
	 * @return Returns the iconname.
	 */
	public String getIconname() {
		return iconname;
	}
	/**
	 * @param iconname The iconname to set.
	 */
	public void setIconname(String iconname) {
		this.iconname = iconname;
	}
	/**
	 * @return Returns the need_increasement.
	 */
	public boolean isNeed_increasement() {
		return need_increasement;
	}
	/**
	 * @param need_increasement The need_increasement to set.
	 */
	public void setNeed_increasement(boolean need_increasement) {
		this.need_increasement = need_increasement;
	}
	/**
	 * @return Returns the need_probability.
	 */
	public boolean isNeed_probability() {
		return need_probability;
	}
	/**
	 * @param need_probability The need_probability to set.
	 */
	public void setNeed_probability(boolean need_probability) {
		this.need_probability = need_probability;
	}
	/**
	 * @return Returns the need_proportion.
	 */
	public boolean isNeed_proportion() {
		return need_proportion;
	}
	/**
	 * @param need_proportion The need_proportion to set.
	 */
	public void setNeed_proportion(boolean need_proportion) {
		this.need_proportion = need_proportion;
	}
	/**
	 * @return Returns the need_time.
	 */
	public boolean isNeed_time() {
		return need_time;
	}
	/**
	 * @param need_time The need_time to set.
	 */
	public void setNeed_time(boolean need_time) {
		this.need_time = need_time;
	}
	/**
	 * @return Returns the need_total.
	 */
	public boolean isNeed_total() {
		return need_total;
	}
	/**
	 * @param need_total The need_total to set.
	 */
	public void setNeed_total(boolean need_total) {
		this.need_total = need_total;
	}
	/**
	 * @return Returns the need_value.
	 */
	public boolean isNeed_value() {
		return need_value;
	}
	/**
	 * @param need_value The need_value to set.
	 */
	public void setNeed_value(boolean need_value) {
		this.need_value = need_value;
	}
	/**
	 * @return Returns the execute.
	 */
	public String getExecute() {
		return execute;
	}
	/**
	 * @param execute The execute to set.
	 */
	public void setExecute(String execute) {
		this.execute = execute;
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
	 * @return Returns the name.
	 */
	public String getName() {
		return name;
	}
	/**
	 * @param name The name to set.
	 */
	public void setName(String name) {
		this.name = name;
	}
}
