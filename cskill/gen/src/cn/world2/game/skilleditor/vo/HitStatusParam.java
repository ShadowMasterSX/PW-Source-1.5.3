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
public class HitStatusParam implements Serializable {
	/**
	 * Comment for <code>serialVersionUID</code>
	 */
	private static final long serialVersionUID = 1L;
	private String id;
	private String name;
	private String probability;//需要概率
	private String time;//需要时间
	private String proportion;//需要比例
	private String increasement;//需要增量
	private String total;//需要总量
	private String value;//需要数值
	private String execute;//执行方法

	private String reserved1; // 保留
	private String reserved2; // 保留
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
	/**
	 * @return Returns the increasement.
	 */
	public String getIncreasement() {
		return increasement;
	}
	/**
	 * @param increasement The increasement to set.
	 */
	public void setIncreasement(String increasement) {
		this.increasement = increasement;
	}
	/**
	 * @return Returns the probability.
	 */
	public String getProbability() {
		return probability;
	}
	/**
	 * @param probability The probability to set.
	 */
	public void setProbability(String probability) {
		this.probability = probability;
	}
	/**
	 * @return Returns the proportion.
	 */
	public String getProportion() {
		return proportion;
	}
	/**
	 * @param proportion The proportion to set.
	 */
	public void setProportion(String proportion) {
		this.proportion = proportion;
	}
	/**
	 * @return Returns the time.
	 */
	public String getTime() {
		return time;
	}
	/**
	 * @param time The time to set.
	 */
	public void setTime(String time) {
		this.time = time;
	}
	/**
	 * @return Returns the total.
	 */
	public String getTotal() {
		return total;
	}
	/**
	 * @param total The total to set.
	 */
	public void setTotal(String total) {
		this.total = total;
	}
	/**
	 * @return Returns the value.
	 */
	public String getValue() {
		return value;
	}
	/**
	 * @param value The value to set.
	 */
	public void setValue(String value) {
		this.value = value;
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
}
