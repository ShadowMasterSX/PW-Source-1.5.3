/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.vo;

import java.util.List;

import cn.world2.game.skilleditor.vo.data.AttackWay;
import cn.world2.game.skilleditor.vo.data.OtherValueState;
import cn.world2.game.skilleditor.vo.data.SkillLevelVO;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class Skill {
	/**
	 * @return Returns the code.
	 */
	public String getCode() {
		return code;
	}
	/**
	 * @param code The code to set.
	 */
	public void setCode(String code) {
		this.code = code;
	}
	/**
	 * @return Returns the continueeffect.
	 */
	public String getContinueeffect() {
		return continueeffect;
	}
	/**
	 * @param continueeffect The continueeffect to set.
	 */
	public void setContinueeffect(String continueeffect) {
		this.continueeffect = continueeffect;
	}
	private String id;
	private Role role;
	private String code;
	private String name;
	private String continueeffect;
	private String upgradeeffect;
	private String releaseeffect;
	private String passiveStatus;//1主动攻击、2主动祝福、3主动诅咒、4召唤、5被动、6激活、7变身、8附体 八选一（默认主动攻击）//主被状态：1主动攻击、2主动辅助、3主动召唤、4主动状态、5主动其他、6被动、7激活，七选一（默认主动攻击）
	private String ridable;
	private String[] armlimitation;
	private String preconditonSkill;
	private AttackWay attackway;
	private String needgoal;
	private SkillLevelVO levelinfo;
	private OtherValueState otherValueState;	//未用
	private String normalAttackForce;			//未用
	private String maxAttackForce;				//未用
	private String hundredNormal;				//未用
	private String hundredEstimateForce;		//未用
	private String cooltime;
	private String icon;
	private List procedures;
	private String maxLevel;
	private String skilldesc;
	private String reviselevel;//修真级别
	private String levelonebook;
	private String executetime;//总执行时间
	private String distance;//吟唱距离 
	private String condition;
	private String attack;//击中结果
	private SkillOtherVO otherinfo;
	private String initskilluse;//初始化时被动技能作用
	private String changeweaponskilluse;//换武器时被动技能作用
	
	private String hatedegree;//对怪物仇恨度的增加
	private List<HitStatusParam> hitstatus;
	
	private String fenmingtime;//粉名时间
	private String displayorder;//技能显示顺序
	private String usehpcondition;	//释放时hp限制
	private String combosk_preskill;	//连续技:施放前提技能
	private String combosk_interval;	//连续技:施放前提技能间隔
	private String combosk_extracond;	//连续技:施放额外条件
	private String combosk_endaction;	//连续技:施放成功后动作
	private String combosk_nobreak;		//连续技:不被后续技能中断前置条件
	private String inherent;			//是否天生技能
	private String movingcast;			//是否可以移动施放
	/**
	 * @return Returns the changeweaponskilluse.
	 */
	public String getChangeweaponskilluse() {
		return changeweaponskilluse;
	}
	/**
	 * @param changeweaponskilluse The changeweaponskilluse to set.
	 */
	public void setChangeweaponskilluse(String changeweaponskilluse) {
		this.changeweaponskilluse = changeweaponskilluse;
	}
	/**
	 * @return Returns the initskilluse.
	 */
	public String getInitskilluse() {
		return initskilluse;
	}
	/**
	 * @param initskilluse The initskilluse to set.
	 */
	public void setInitskilluse(String initskilluse) {
		this.initskilluse = initskilluse;
	}
	/**
	 * @return Returns the attack.
	 */
	public String getAttack() {
		return attack;
	}
	/**
	 * @param attack The attack to set.
	 */
	public void setAttack(String attack) {
		this.attack = attack;
	}
	/**
	 * @return Returns the condition.
	 */
	public String getCondition() {
		return condition;
	}
	/**
	 * @param condition The condition to set.
	 */
	public void setCondition(String condition) {
		this.condition = condition;
	}
	/**
	 * @return Returns the distance.
	 */
	public String getDistance() {
		return distance;
	}
	/**
	 * @param distance The distance to set.
	 */
	public void setDistance(String distance) {
		this.distance = distance;
	}
	/**
	 * @return Returns the levelonebook.
	 */
	public String getLevelonebook() {
		return levelonebook;
	}
	/**
	 * @param levelonebook The levelonebook to set.
	 */
	public void setLevelonebook(String levelonebook) {
		this.levelonebook = levelonebook;
	}
	/**
	 * @return Returns the reviselevel.
	 */
	public String getReviselevel() {
		return reviselevel;
	}
	/**
	 * @param reviselevel The reviselevel to set.
	 */
	public void setReviselevel(String reviselevel) {
		this.reviselevel = reviselevel;
	}
	/**
	 * @return Returns the skilldesc.
	 */
	public String getSkilldesc() {
		return skilldesc;
	}
	/**
	 * @param skilldesc The skilldesc to set.
	 */
	public void setSkilldesc(String skilldesc) {
		this.skilldesc = skilldesc;
	}
	/**
	 * @return Returns the maxLevel.
	 */
	public String getMaxLevel() {
		return maxLevel;
	}
	/**
	 * @param maxLevel The maxLevel to set.
	 */
	public void setMaxLevel(String maxLevel) {
		this.maxLevel = maxLevel;
	}
	/**
	 * @return Returns the procedures.
	 */
	public List getProcedures() {
		return procedures;
	}
	/**
	 * @param procedures The procedures to set.
	 */
	public void setProcedures(List procedures) {
		this.procedures = procedures;
	}
	/**
	 * @return Returns the icon.
	 */
	public String getIcon() {
		return icon;
	}
	/**
	 * @param icon The icon to set.
	 */
	public void setIcon(String icon) {
		this.icon = icon;
	}
	/**
	 * @return Returns the cooltime.
	 */
	public String getCooltime() {
		return cooltime;
	}
	/**
	 * @param cooltime The cooltime to set.
	 */
	public void setCooltime(String cooltime) {
		this.cooltime = cooltime;
	}
	/**
	 * @return Returns the hundredEstimateForce.
	 */
	public String getHundredEstimateForce() {
		return hundredEstimateForce;
	}
	/**
	 * @param hundredEstimateForce The hundredEstimateForce to set.
	 */
	public void setHundredEstimateForce(String hundredEstimateForce) {
		this.hundredEstimateForce = hundredEstimateForce;
	}
	/**
	 * @return Returns the hundredNormal.
	 */
	public String getHundredNormal() {
		return hundredNormal;
	}
	/**
	 * @param hundredNormal The hundredNormal to set.
	 */
	public void setHundredNormal(String hundredNormal) {
		this.hundredNormal = hundredNormal;
	}
	/**
	 * @return Returns the levelinfo.
	 */
	public SkillLevelVO getLevelinfo() {
		return levelinfo;
	}
	/**
	 * @param levelinfo The levelinfo to set.
	 */
	public void setLevelinfo(SkillLevelVO levelinfo) {
		this.levelinfo = levelinfo;
	}
	/**
	 * @return Returns the maxAttackForce.
	 */
	public String getMaxAttackForce() {
		return maxAttackForce;
	}
	/**
	 * @param maxAttackForce The maxAttackForce to set.
	 */
	public void setMaxAttackForce(String maxAttackForce) {
		this.maxAttackForce = maxAttackForce;
	}
	/**
	 * @return Returns the normalAttackForce.
	 */
	public String getNormalAttackForce() {
		return normalAttackForce;
	}
	/**
	 * @param normalAttackForce The normalAttackForce to set.
	 */
	public void setNormalAttackForce(String normalAttackForce) {
		this.normalAttackForce = normalAttackForce;
	}
	/**
	 * @return Returns the otherValueState.
	 */
	public OtherValueState getOtherValueState() {
		return otherValueState;
	}
	/**
	 * @param otherValueState The otherValueState to set.
	 */
	public void setOtherValueState(OtherValueState otherValueState) {
		this.otherValueState = otherValueState;
	}
	/**
	 * @return Returns the armlimitation.
	 */
	public String[] getArmlimitation() {
		return armlimitation;
	}
	/**
	 * @param armlimitation The armlimitation to set.
	 */
	public void setArmlimitation(String[] armlimitation) {
		this.armlimitation = armlimitation;
	}
	/**
	 * @return Returns the attackway.
	 */
	public AttackWay getAttackway() {
		return attackway;
	}
	/**
	 * @param attackway The attackway to set.
	 */
	public void setAttackway(AttackWay attackway) {
		this.attackway = attackway;
	}
	/**
	 * @return Returns the needgoal.
	 */
	public String getNeedgoal() {
		return needgoal;
	}
	/**
	 * @param needgoal The needgoal to set.
	 */
	public void setNeedgoal(String needgoal) {
		this.needgoal = needgoal;
	}
	/**
	 * @return Returns the passiveStatus.
	 */
	public String getPassiveStatus() {
		return passiveStatus;
	}
	/**
	 * @param passiveStatus The passiveStatus to set.
	 */
	public void setPassiveStatus(String passiveStatus) {
		this.passiveStatus = passiveStatus;
	}
	/**
	 * @return Returns the preconditonSkill.
	 */
	public String getPreconditonSkill() {
		return preconditonSkill;
	}
	/**
	 * @param preconditonSkill The preconditonSkill to set.
	 */
	public void setPreconditonSkill(String preconditonSkill) {
		this.preconditonSkill = preconditonSkill;
	}
	/**
	 * @return Returns the ridable.
	 */
	public String getRidable() {
		return ridable;
	}
	/**
	 * @param ridable The ridable to set.
	 */
	public void setRidable(String ridable) {
		this.ridable = ridable;
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
	/**
	 * @return Returns the releaseeffect.
	 */
	public String getReleaseeffect() {
		return releaseeffect;
	}
	/**
	 * @param releaseeffect The releaseeffect to set.
	 */
	public void setReleaseeffect(String releaseeffect) {
		this.releaseeffect = releaseeffect;
	}
	/**
	 * @return Returns the role.
	 */
	public Role getRole() {
		return role;
	}
	/**
	 * @param role The role to set.
	 */
	public void setRole(Role role) {
		this.role = role;
	}
	/**
	 * @return Returns the upgradeeffect.
	 */
	public String getUpgradeeffect() {
		return upgradeeffect;
	}
	/**
	 * @param upgradeeffect The upgradeeffect to set.
	 */
	public void setUpgradeeffect(String upgradeeffect) {
		this.upgradeeffect = upgradeeffect;
	}
	/**
	 * @return Returns the otherinfo.
	 */
	public SkillOtherVO getOtherinfo() {
		return otherinfo;
	}
	/**
	 * @param otherinfo The otherinfo to set.
	 */
	public void setOtherinfo(SkillOtherVO otherinfo) {
		this.otherinfo = otherinfo;
	}
	/**
	 * @return Returns the executetime.
	 */
	public String getExecutetime() {
		return executetime;
	}
	/**
	 * @param executetime The executetime to set.
	 */
	public void setExecutetime(String executetime) {
		this.executetime = executetime;
	}
	
	/**
	 * @return Returns the hatedegree.
	 */
	public String getHatedegree() {
		return hatedegree;
	}
	/**
	 * @param hatedegree The hatedegree to set.
	 */
	public void setHatedegree(String hatedegree) {
		this.hatedegree = hatedegree;
	}
	/**
	 * @return Returns the hitstatus.
	 */
	public List<HitStatusParam> getHitstatus() {
		return hitstatus;
	}
	/**
	 * @param hitstatus The hitstatus to set.
	 */
	public void setHitstatus(List<HitStatusParam> hitstatus) {
		this.hitstatus = hitstatus;
	}
	/**
	 * @return Returns the displayorder.
	 */
	public String getDisplayorder() {
		return displayorder;
	}
	/**
	 * @param displayorder The displayorder to set.
	 */
	public void setDisplayorder(String displayorder) {
		this.displayorder = displayorder;
	}
	/**
	 * @return Returns the fenmingtime.
	 */
	public String getFenmingtime() {
		return fenmingtime;
	}
	/**
	 * @param fenmingtime The fenmingtime to set.
	 */
	public void setFenmingtime(String fenmingtime) {
		this.fenmingtime = fenmingtime;
	}

	public String getUsehpcondition()
	{
		return usehpcondition;
	}

	public String setUsehpcondition(String usehpcondition)
	{
		return this.usehpcondition = usehpcondition;
	}

	public String getCombosk_preskill()
	{
		return combosk_preskill;
	}

	public String setCombosk_preskill(String combosk_preskill)
	{
		return this.combosk_preskill = combosk_preskill;
	}

	public String getCombosk_interval()
	{
		return combosk_interval;
	}

	public String setCombosk_interval(String combosk_interval)
	{
		return this.combosk_interval = combosk_interval;
	}

	public String getCombosk_extracond()
	{
		return combosk_extracond;
	}

	public String setCombosk_extracond(String combosk_extracond)
	{
		return this.combosk_extracond = combosk_extracond;
	}

	public String getCombosk_endaction()
	{
		return combosk_endaction;
	}

	public String setCombosk_endaction(String combosk_endaction)
	{
		return this.combosk_endaction = combosk_endaction;
	}

	public String getCombosk_nobreak()
	{
		return combosk_nobreak;
	}

	public String setCombosk_nobreak(String combosk_nobreak)
	{
		return this.combosk_nobreak = combosk_nobreak;
	}

	public String getInherent()
	{
		return inherent;
	}

	public String setInherent(String inherent)
	{
		return this.inherent = inherent;
	}

	public String getMovingcast()
	{
		return movingcast;
	}

	public String setMovingcast(String movingcast)
	{
		return this.movingcast = movingcast;
	}

}
