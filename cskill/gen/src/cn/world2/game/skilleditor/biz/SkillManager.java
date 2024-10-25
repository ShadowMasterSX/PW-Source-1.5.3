/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.biz;

import java.util.ArrayList;
import java.util.List;

import cn.world2.game.skilleditor.DB.DAO.ISkillDAO;
import cn.world2.game.skilleditor.DB.DAO.ISkillOtherDAO;
import cn.world2.game.skilleditor.DB.DAO.JDBC.SkillDAOImp;
import cn.world2.game.skilleditor.DB.DAO.JDBC.SkillOtherDAOImp;
import cn.world2.game.skilleditor.vo.HitStatusParam;
import cn.world2.game.skilleditor.vo.Skill;
import cn.world2.game.skilleditor.vo.SkillOtherVO;
import cn.world2.game.skilleditor.vo.data.AttackWay;
import cn.world2.game.skilleditor.vo.data.SkillLevelVO;

/**
 * @author linxiaojun
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class SkillManager {
	private ISkillDAO sd;

	public SkillManager() {
		sd = new SkillDAOImp();
	}

	/**
	 * 插入一条技能
	 * 
	 * @param skill
	 * @return 返回插入技能的id
	 */
	public String insertOneSkill(Skill skill) {
		return sd.insertSkill(skill);
	}

	public List getSkills(String roleid) {
		return sd.getSkill(" where roleid='" + roleid + "'");
	}

	public List getSkillsShort(String roleid) {
		return sd.getSkillsShort(" where roleid='" + roleid + "'");
	}

	public void deleteSkills(List skillidlist) {
		this.sd.deleteSkill(skillidlist);
	}

	public boolean updateSkillLevel(SkillLevelVO slvo, String skillid) {
		boolean ret = this.sd.updateSkillLevel(slvo, skillid);
		return ret;
	}

	public SkillLevelVO getSkillLevel(String skillid) {
		SkillLevelVO slvo = this.sd.getSkillLevel(skillid);
		if (slvo == null)
			slvo = new SkillLevelVO();
		return slvo;
	}

	public boolean updateSkillBasic(Skill skill) {
		boolean ret = this.sd.updateSkillBasic(skill);
		return ret;
	}

	public Skill getBasicSkill(String skillid) {
		Skill skill = this.sd.getSkillBasic(skillid);
		if (skill == null)
			skill = new Skill();
		return skill;
	}

	public boolean saveSkillProcedure(List list, String skillid) {
		return this.sd.updateSkillProcedure(list, skillid);
	}

	public List getSkillProcedure(String skillid) {
		return this.sd.getSkillProcedure(skillid);
	}

	public boolean updateCooltime(String skillid, String cooltime) {
		String sqlStr = "update skill set cooltime='" + cooltime
				+ "' where id='" + skillid + "'";
		return this.sd.updateSkillByQuery(sqlStr);
	}

	public boolean updateOthersOfProcedurePage(String skillid, String cooltime,String executetime,
			String distance, String condition, String attack,ArrayList<HitStatusParam> hsplist,AttackWay attackway,String hatedegree, String releaseeffect, String usehpcondition,
			String combosk_preskill, String combosk_interval, String combosk_extracond, String combosk_endaction, String combosk_nobreak) {
		String sqlStr = "update skill set cooltime='" + cooltime
			    + "',executetime='" + executetime
				+ "',distance='" + distance + "',usecondition='" + condition
				+ "',attack='" + attack + "',hatedegree='"+hatedegree+ "',releaseeffect='"+releaseeffect
				+ "',usehpcondition='" + usehpcondition
				+ "',combosk_preskill='" + combosk_preskill
				+ "',combosk_interval='" + combosk_interval
				+ "',combosk_extracond='" + combosk_extracond
				+ "',combosk_endaction='" + combosk_endaction
				+ "',combosk_nobreak='" + combosk_nobreak
				+"' where id='" + skillid + "'";
		//System.out.println(sqlStr);
		this.sd.updateBlob(hsplist,"hitstatus",skillid);
		this.sd.updateBlob(attackway,"attackway",skillid);
		return this.sd.updateSkillByQuery(sqlStr);
	}

	public String getCooltime(String skillid) {
		Skill skill = this.sd.getSkillBasic(skillid);
		return skill.getCooltime();
	}

	public Object[] getOtherDataOfProcedurePage(String skillid) {
		Skill skill = this.sd.getSkillBasic(skillid);
		Object[] data = new Object[15];
		data[0] = skill.getCooltime();
		data[1] = skill.getDistance();
		data[2] = skill.getCondition();
		data[3] = skill.getAttack();
		data[4] = skill.getExecutetime();
		ArrayList<HitStatusParam> hsplist=null;
		AttackWay attackway = null;
		try{
			hsplist = (ArrayList<HitStatusParam>)this.sd.getBlob("hitstatus",skillid);
			attackway = (AttackWay)this.sd.getBlob("attackway",skillid);
		}catch(Exception e){
			
		}
		if(hsplist==null) 
		{
			hsplist = new ArrayList<HitStatusParam>();
			hsplist.add(new HitStatusParam());
			hsplist.add(new HitStatusParam());
			hsplist.add(new HitStatusParam());
			hsplist.add(new HitStatusParam());
			hsplist.add(new HitStatusParam());
		}
		while(hsplist.size()<5) 
		{
			hsplist.add(new HitStatusParam());
		}
		data[5] = hsplist;
		data[6] = attackway;
		data[7] = skill.getHatedegree();
		data[8] = skill.getReleaseeffect();
		data[9] = skill.getUsehpcondition();
		data[10] = skill.getCombosk_preskill();
		data[11] = skill.getCombosk_interval();
		data[12] = skill.getCombosk_extracond();
		data[13] = skill.getCombosk_endaction();
		data[14] = skill.getCombosk_nobreak();
		return data;
	}

	public void copySkill(String oldskillid, String newSkillid,String oldrole) {
		String sqlStr = "insert into `skill`" + "( `roleid`," + "`code`,"
				+ "`name`," + "`icon`," + "`skilldesc`," + "`releaseeffect`,"
				+ "`continueeffect`," + "`upgradeeffect`," + "`passiveStatus`," + "`initskilluse`,"+ "`changeweaponskilluse`,"
				+ "`ridable`," + "`weaponlimitation`," + "`preconditionskill`,"
				+ "`reviselevel`," + "`fenmingtime`," + "`attackway`,"
				+ "`needgoal`," + "`cooltime`," + "`executetime`," + "`distance`,"
				+ "`usecondition`," + "`attack`," + "`maxlevel`," + "`levelinfo`,"
				+ "`skillprocedure`," + "`othervaluestate`,"
				+ "`fly_effect_proportion`," + "`fly_effect_parameter`,"
				+ "`displayorder`," + "`hitstatus`,"
				+ "`isonland`," + "`isinsky`," + "`isinwater`," + "`bianshenhou`," + "`futihou`," + "`isattackskill`," + "`cannotusetogether`,"
				+ "`mainskillproperty`," + "`isautosearchDest`," + "`autosearchDestExp`," + "`searchcenter`," + "`intonateactionname`,"
				+ "`exeactionname`," + "`exeactionnameselect`," + "`exeactionweaponselect`," + "`effectfilename`," + "`effectfiletype`,"+ "`hitinjureactionname`,"
				+ "`guide_dict`)" + "(select" + "`roleid`," + "`code`,"
				+ "`name`," + "`icon`," + "`skilldesc`," + "`releaseeffect`,"
				+ "`continueeffect`," + "`upgradeeffect`," + "`passiveStatus`," + "`initskilluse`,"+ "`changeweaponskilluse`,"
				+ "`ridable`," + "`weaponlimitation`," + "`preconditionskill`,"
				+ "`reviselevel`," + "`fenmingtime`," + "`attackway`,"
				+ "`needgoal`," + "`cooltime`," + "`executetime`," + "`distance`,"
				+ "`usecondition`," + "`attack`," + "`maxlevel`," + "`levelinfo`,"
				+ "`skillprocedure`," + "`othervaluestate`,"
				+ "`fly_effect_proportion`," + "`fly_effect_parameter`,"
				+ "`displayorder`," + "`hitstatus`,"
				+ "`isonland`," + "`isinsky`," + "`isinwater`," + "`bianshenhou`," + "`futihou`," + "`isattackskill`," + "`cannotusetogether`,"
				+ "`mainskillproperty`," + "`isautosearchDest`," + "`autosearchDestExp`," + "`searchcenter`," + "`intonateactionname`,"
				+ "`exeactionname`," + "`exeactionnameselect`," + "`exeactionweaponselect`," + "`effectfilename`," + "`effectfiletype`," + "`hitinjureactionname`,"
				+ "`guide_dict`" + "from `skill` where id='"+oldskillid+"');";
		String LAST_INSERT_ID = this.sd.copySkillByQuery(sqlStr,oldrole);
		//System.out.println(LAST_INSERT_ID);
		
	}
	public SkillOtherVO getSkillOther(String skillid){
		ISkillOtherDAO sodao = new SkillOtherDAOImp();
		return sodao.getSkillOtherVOById(skillid);
	}
	public boolean updateSkillOther(SkillOtherVO sov){
		ISkillOtherDAO sodao = new SkillOtherDAOImp();
		return sodao.updateSkillOtherVO(sov);
	}
}
