/*
 * Created on 2005-1-25
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.vo;

import cn.world2.game.skilleditor.vo.data.AttackWay;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class SkillOtherVO {
	private String id;//skillid
	private String isonland;//陆地是否有效：  1是  0否 二选一（默认是） 
	private String isinsky;//空中是否有效：  1是  0否 二选一（默认否） 
	private String isinwater;//水中是否有效：  1是  0否 二选一（默认否） 
	private String isAttackSkill;//是否为攻击性技能:    1是  0否 二选一（默认是）
	private String cannotusetogether;//不能同时起作用的技能，以;分割	
	private String mainskillproperty;//主技能属性  1物理、 2金、 3木、 4水、 5火、6 土 六选一 默认（1物理）    
	private String isAutoSearchDestination;//是否自动寻找目标 1是、 0否二选一
	private String autosearchDestExp;//搜寻半径
	private String searchcenter;//1.以自身为中心2.以目标为中心
	private String continuoushit;//是否连击：1是0否
	private String intonateActionName;//吟唱动作名
	private String exeactionname;//执行动作名
	private String exeactionnameselect;//级别选择框 1级别相关 2级别无关
	private String exeactionweaponselect;//武器选择框 1武器相关 2武器无关
	private String effectFileName;//效果文件名
	private String effectfiletype;//效果文件类型：1直线，2抛物线，3香蕉线，4贝塞尔线
	private String fly_effect_proportion;//飞行效果大小比例系数
	private String fly_effect_parameter;//曲线附加参数
	private String fly_effect_behave;//飞行效果表现方式：1固定面伤害，2飞行子弹伤害
	private String fly_effect_time;//飞行时间
	private String hitEnjureActionName;//击中受伤动作名
	private String hit_effect_proportion;//击中效果大小比例系数
	
	private String ridable;//骑乘是否有效1有效0无效
	private AttackWay attackway;//攻击范围

	private String bianshenhou;//变身状态限制(多选)：用;分割。比如1;2;4表示1、2、4选中 1正常状态 2变身状态一 3变身状态二 4变身状态三//是否必须变身后：1是、0否
	private String futihou;//是否必须附体后：1是、0否

	private String fenmingtime;//粉名时间
	private String displayorder;//技能显示顺序

	private String reserved1;	// 保留
	private String reserved2;	// 保留
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
	 * @return Returns the effectFileName.
	 */
	public String getEffectFileName() {
		return effectFileName;
	}
	/**
	 * @param effectFileName The effectFileName to set.
	 */
	public void setEffectFileName(String effectFileName) {
		this.effectFileName = effectFileName;
	}
	/**
	 * @return Returns the hitEnjureActionName.
	 */
	public String getHitEnjureActionName() {
		return hitEnjureActionName;
	}
	/**
	 * @param hitEnjureActionName The hitEnjureActionName to set.
	 */
	public void setHitEnjureActionName(String hitEnjureActionName) {
		this.hitEnjureActionName = hitEnjureActionName;
	}
	/**
	 * @return Returns the intonateActionName.
	 */
	public String getIntonateActionName() {
		return intonateActionName;
	}
	/**
	 * @param intonateActionName The intonateActionName to set.
	 */
	public void setIntonateActionName(String intonateActionName) {
		this.intonateActionName = intonateActionName;
	}
	/**
	 * @return Returns the isAttackSkill.
	 */
	public String getIsAttackSkill() {
		return isAttackSkill;
	}
	/**
	 * @param isAttackSkill The isAttackSkill to set.
	 */
	public void setIsAttackSkill(String isAttackSkill) {
		this.isAttackSkill = isAttackSkill;
	}
	/**
	 * @return Returns the isAutoSearchDestination.
	 */
	public String getIsAutoSearchDestination() {
		return isAutoSearchDestination;
	}
	/**
	 * @param isAutoSearchDestination The isAutoSearchDestination to set.
	 */
	public void setIsAutoSearchDestination(String isAutoSearchDestination) {
		this.isAutoSearchDestination = isAutoSearchDestination;
	}
	/**
	 * @return Returns the isinsky.
	 */
	public String getIsinsky() {
		return isinsky;
	}
	/**
	 * @param isinsky The isinsky to set.
	 */
	public void setIsinsky(String isinsky) {
		this.isinsky = isinsky;
	}
	/**
	 * @return Returns the isinwater.
	 */
	public String getIsinwater() {
		return isinwater;
	}
	/**
	 * @param isinwater The isinwater to set.
	 */
	public void setIsinwater(String isinwater) {
		this.isinwater = isinwater;
	}
	/**
	 * @return Returns the isonland.
	 */
	public String getIsonland() {
		return isonland;
	}
	/**
	 * @param isonland The isonland to set.
	 */
	public void setIsonland(String isonland) {
		this.isonland = isonland;
	}
	/**
	 * @return Returns the mainskillproperty.
	 */
	public String getMainskillproperty() {
		return mainskillproperty;
	}
	/**
	 * @param mainskillproperty The mainskillproperty to set.
	 */
	public void setMainskillproperty(String mainskillproperty) {
		this.mainskillproperty = mainskillproperty;
	}
	/**
	 * @return Returns the autosearchDestExp.
	 */
	public String getAutosearchDestExp() {
		return autosearchDestExp;
	}
	/**
	 * @param autosearchDestExp The autosearchDestExp to set.
	 */
	public void setAutosearchDestExp(String autosearchDestExp) {
		this.autosearchDestExp = autosearchDestExp;
	}
	/**
	 * @return Returns the cannotusetogether.
	 */
	public String getCannotusetogether() {
		return cannotusetogether;
	}
	/**
	 * @param cannotusetogether The cannotusetogether to set.
	 */
	public void setCannotusetogether(String cannotusetogether) {
		this.cannotusetogether = cannotusetogether;
	}
	/**
	 * @return Returns the exeactionname.
	 */
	public String getExeactionname() {
		return exeactionname;
	}
	/**
	 * @param exeactionname The exeactionname to set.
	 */
	public void setExeactionname(String exeactionname) {
		this.exeactionname = exeactionname;
	}
	/**
	 * @return Returns the exeactionnameselect.
	 */
	public String getExeactionnameselect() {
		return exeactionnameselect;
	}
	/**
	 * @param exeactionnameselect The exeactionnameselect to set.
	 */
	public void setExeactionnameselect(String exeactionnameselect) {
		this.exeactionnameselect = exeactionnameselect;
	}
	/**
	 * @return Returns the searchcenter.
	 */
	public String getSearchcenter() {
		return searchcenter;
	}
	/**
	 * @param searchcenter The searchcenter to set.
	 */
	public void setSearchcenter(String searchcenter) {
		this.searchcenter = searchcenter;
	}
	/**
	 * @return Returns the continuoushit.
	 */
	public String getContinuoushit() {
		return continuoushit;
	}
	/**
	 * @param continuoushit The continuoushit to set.
	 */
	public void setContinuoushit(String continuoushit) {
		this.continuoushit = continuoushit;
	}
	/**
	 * @return Returns the exeactionweaponselect.
	 */
	public String getExeactionweaponselect() {
		return exeactionweaponselect;
	}
	/**
	 * @param exeactionweaponselect The exeactionweaponselect to set.
	 */
	public void setExeactionweaponselect(String exeactionweaponselect) {
		this.exeactionweaponselect = exeactionweaponselect;
	}
	/**
	 * @return Returns the effectfiletype.
	 */
	public String getEffectfiletype() {
		return effectfiletype;
	}
	/**
	 * @param effectfiletype The effectfiletype to set.
	 */
	public void setEffectfiletype(String effectfiletype) {
		this.effectfiletype = effectfiletype;
	}
	/**
	 * @return Returns the fly_effect_proportion.
	 */
	public String getFly_effect_proportion() {
		return fly_effect_proportion;
	}
	/**
	 * @param fly_effect_proportion The fly_effect_proportion to set.
	 */
	public void setFly_effect_proportion(String fly_effect_proportion) {
		this.fly_effect_proportion = fly_effect_proportion;
	}
	/**
	 * @return Returns the fly_effect_parameter.
	 */
	public String getFly_effect_parameter() {
		return fly_effect_parameter;
	}
	/**
	 * @param fly_effect_parameter The fly_effect_parameter to set.
	 */
	public void setFly_effect_parameter(String fly_effect_parameter) {
		this.fly_effect_parameter = fly_effect_parameter;
	}
	/**
	 * @return Returns the fly_effect_behave.
	 */
	public String getFly_effect_behave() {
		return fly_effect_behave;
	}
	/**
	 * @param fly_effect_behave The fly_effect_behave to set.
	 */
	public void setFly_effect_behave(String fly_effect_behave) {
		this.fly_effect_behave = fly_effect_behave;
	}
	/**
	 * @return Returns the fly_effect_time.
	 */
	public String getFly_effect_time() {
		return fly_effect_time;
	}
	/**
	 * @param fly_effect_time The fly_effect_time to set.
	 */
	public void setFly_effect_time(String fly_effect_time) {
		this.fly_effect_time = fly_effect_time;
	}
	/**
	 * @return Returns the hit_effect_proportion.
	 */
	public String getHit_effect_proportion() {
		return hit_effect_proportion;
	}
	/**
	 * @param hit_effect_proportion The hit_effect_proportion to set.
	 */
	public void setHit_effect_proportion(String hit_effect_proportion) {
		this.hit_effect_proportion = hit_effect_proportion;
	}
	/**
	 * @return Returns the bianshenhou.
	 */
	public String getBianshenhou() {
		return bianshenhou;
	}
	/**
	 * @param bianshenhou The bianshenhou to set.
	 */
	public void setBianshenhou(String bianshenhou) {
		this.bianshenhou = bianshenhou;
	}
	/**
	 * @return Returns the futihou.
	 */
	public String getFutihou() {
		return futihou;
	}
	/**
	 * @param futihou The futihou to set.
	 */
	public void setFutihou(String futihou) {
		this.futihou = futihou;
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
}
