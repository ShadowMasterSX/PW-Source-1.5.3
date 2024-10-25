/*
 * Created on 2004-12-29
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.vo.data;

import java.io.Serializable;

/**
 * @author linxiaojun
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class ProcedureVO implements Serializable {
	private String skillid;

	/**
	 * @return Returns the skillid.
	 */
	public String getSkillid() {
		return skillid;
	}

	/**
	 * @param skillid
	 *            The skillid to set.
	 */
	public void setSkillid(String skillid) {
		this.skillid = skillid;
	}

	private String name;

	private String continuetime;//持续时间

	private Object effectObj;//效果对象

	private String loopCondition;//循环条件

	private String quitCondition;//运行条件

	private String bypassCondition;//旁路条件

	private String computeResult;//计算结果

	private String asynchronousTerminate;//异步中断条件
	
	private String kequxiao;//可取消
	private String xuli;//蓄力

	private String reserved1;//保留
	private String reserved2;//保留
	
	
	/**
	 * @return Returns the bypassCondition.
	 */
	public String getBypassCondition() {
		return bypassCondition;
	}

	/**
	 * @param bypassCondition
	 *            The bypassCondition to set.
	 */
	public void setBypassCondition(String bypassCondition) {
		this.bypassCondition = bypassCondition;
	}

	/**
	 * @return Returns the computeResult.
	 */
	public String getComputeResult() {
		return computeResult;
	}

	/**
	 * @param computeResult
	 *            The computeResult to set.
	 */
	public void setComputeResult(String computeResult) {
		this.computeResult = computeResult;
	}

	/**
	 * @return Returns the continuetime.
	 */
	public String getContinuetime() {
		return continuetime;
	}

	/**
	 * @param continuetime
	 *            The continuetime to set.
	 */
	public void setContinuetime(String continuetime) {
		this.continuetime = continuetime;
	}

	/**
	 * @return Returns the effectObj.
	 */
	public Object getEffectObj() {
		return effectObj;
	}

	/**
	 * @param effectObj
	 *            The effectObj to set.
	 */
	public void setEffectObj(Object effectObj) {
		this.effectObj = effectObj;
	}

	/**
	 * @return Returns the loopCondition.
	 */
	public String getLoopCondition() {
		return loopCondition;
	}

	/**
	 * @param loopCondition
	 *            The loopCondition to set.
	 */
	public void setLoopCondition(String loopCondition) {
		this.loopCondition = loopCondition;
	}

	/**
	 * @return Returns the name.
	 */
	public String getName() {
		return name;
	}

	/**
	 * @param name
	 *            The name to set.
	 */
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * @return Returns the quitCondition.
	 */
	public String getQuitCondition() {
		return quitCondition;
	}

	/**
	 * @param quitCondition
	 *            The quitCondition to set.
	 */
	public void setQuitCondition(String quitCondition) {
		this.quitCondition = quitCondition;
	}

	/**
	 * @return Returns the asynchronousTerminate.
	 */
	public String getAsynchronousTerminate() {
		return asynchronousTerminate;
	}

	/**
	 * @param asynchronousTerminate
	 *            The asynchronousTerminate to set.
	 */
	public void setAsynchronousTerminate(String asynchronousTerminate) {
		this.asynchronousTerminate = asynchronousTerminate;
	}
	/**
	 * @return Returns the kequxiao.
	 */
	public String getKequxiao() {
		return kequxiao;
	}
	/**
	 * @param kequxiao The kequxiao to set.
	 */
	public void setKequxiao(String kequxiao) {
		this.kequxiao = kequxiao;
	}
	/**
	 * @return Returns the xuli.
	 */
	public String getXuli() {
		return xuli;
	}
	/**
	 * @param xuli The xuli to set.
	 */
	public void setXuli(String xuli) {
		this.xuli = xuli;
	}
}
