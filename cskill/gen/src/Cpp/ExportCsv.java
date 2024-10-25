
package Cpp;

import java.io.PrintStream;
import java.util.*;

import cn.world2.game.skilleditor.biz.*;
import cn.world2.game.skilleditor.vo.*;
import cn.world2.game.skilleditor.vo.data.*;
import expr.*;

public class ExportCsv
{
	private Skill skill;
	private List skills;
	private List weapons;
	private List allhitstatus;

	private String ESCExpr(String s)
	{
		if( null == s )
			s = "";
		s.trim();
		if( s.length() > 0 && s.charAt(s.length()-1) != ';' )
			s += ";";
		return s;
	}

	HitStatus GetHitStatusStatic( String id )
	{
		if( null == id )
			return null;
		for (Iterator it = this.allhitstatus.iterator(); it.hasNext(); )
		{
			HitStatus hss = (HitStatus)it.next();
			if( id.equals( hss.getId() ) )
				return hss;
		}
		return null;
	}

	String GetStateAttacks( List list )
	{
		String result = "";
		if( null == list )
			return "";

		for (Iterator it = list.iterator(); it.hasNext(); )
		{
			HitStatusParam hsp = (HitStatusParam)it.next();
			if( hsp.getId().equals("-1") )
				continue;

			HitStatus hss = GetHitStatusStatic(hsp.getId());
			if( null == hss )
			{
				System.out.println( "\tError GetHitStatusStatic: id="+hsp.getId() );
				continue;
			}

			String exp = "";
			if( hss.isNeed_probability() )
				exp += "victim.probability=1.0*" + ESCExpr(hsp.getProbability());
			if( hss.isNeed_time() )
				exp += "victim.time=" + ESCExpr(hsp.getTime());
			if( hss.isNeed_proportion() )
				exp += "victim.ratio=" + ESCExpr(hsp.getProportion());
			if( hss.isNeed_increasement() )
				exp += "victim.showicon=" + ESCExpr(hsp.getIncreasement());
			if( hss.isNeed_total() )
				exp += "victim.amount=" + ESCExpr(hsp.getTotal());
			if( hss.isNeed_value() )
				exp += "victim.value=" + ESCExpr(hsp.getValue());

			if( null != hss.getExecute() && hss.getExecute().length() > 0 )
				exp += "victim." + hss.getExecute() + "=" + "1;";

			if( exp.length() > 0 )
				result += exp;
		}
		return result;
	}

	public ExportCsv( Skill skill, List skills, List weapons, List allhitstatus )
	{
		this.skill = skill;
		this.skills = skills;
		this.weapons = weapons;
		this.allhitstatus = allhitstatus;
	}

	static public void writeheader(PrintStream ps)
	{
		ps.print( "角色ID," );
		ps.print( "技能名称," );
		ps.print( "图标," );
		ps.print( "技能显示顺序," );
		ps.print( "技能说明," );
		ps.print( "前提技能," );
		ps.print( "修真级别," );
		ps.print( "技能类型," );
		ps.print( "最大级别," );
		ps.print( "主技能属性," );

		ps.print( "是否需要目标," );
		ps.print( "陆地是否有效," );
		ps.print( "空中是否有效," );
		ps.print( "水中是否有效," );
		ps.print( "骑乘是否有效," );
		ps.print( "是否瞬发技能," );
		ps.print( "使用后自动攻击," );
		ps.print( "目标为尸体," );
		ps.print( "变身状态限制," );
		ps.print( "武器限制," );
		ps.print( "不能同时起作用的技能," );

		int count = 0;
		while( count ++ < 3 )
			ps.print( "过程名,时间,退出条件,循环条件,旁路条件,结果计算,中断概率,可取消,蓄力," );

		ps.print( "效果文件名," );
		ps.print( "击中受伤动作名," );
		ps.print( "伤害延时," );
		ps.print( "物理攻击准确度修正倍数," );

		ps.print( "攻击范围," );
		ps.print( "最大有效距离," );
		ps.print( "圆柱长度," );
		ps.print( "圆柱半径," );
		ps.print( "球半径," );
		ps.print( "扇形半径," );
		ps.print( "扇形半张角," );

		ps.print( "消耗MP," );
		ps.print( "吟唱距离," );
		ps.print( "执行时间," );
		ps.print( "冷却时间," );
		ps.print( "对怪物仇恨度的增加," );
		ps.print( "击中结果状态," );
		ps.print( "被动技能效果," );

		ps.print( "学习条件1," );
		ps.print( "学习条件2," );
		ps.print( "学习条件3," );
		ps.print( "学习条件4," );
		ps.print( "学习条件5," );
		ps.print( "学习条件6," );
		ps.print( "学习条件7," );
		ps.print( "学习条件8," );
		ps.print( "学习条件9," );
		ps.print( "学习条件10," );

		ps.print( "学习结果计算1," );
		ps.print( "学习结果计算2," );
		ps.print( "学习结果计算3," );
		ps.print( "学习结果计算4," );
		ps.print( "学习结果计算5," );
		ps.print( "学习结果计算6," );
		ps.print( "学习结果计算7," );
		ps.print( "学习结果计算8," );
		ps.print( "学习结果计算9," );
		ps.print( "学习结果计算10," );

		ps.print( "级别要求," );
		ps.print( "花费元神," );
		ps.print( "学习需要物品," );
		ps.print( "学习花费金钱," );
		ps.print( "升级需要熟练度," );

		ps.print("\n");
	}

	public void write(PrintStream ps)
	{
		SkillOtherVO other = skill.getOtherinfo();
		AttackWay range = other.getAttackway();
		if( null == range )	range = new AttackWay();
		SkillLevelVO levelinfo = skill.getLevelinfo();
		if( null == levelinfo ) levelinfo = new SkillLevelVO();

		String stateattacks = //ESCExpr( skill.getAttack() ) + 
			ESCExpr( GetStateAttacks( skill.getHitstatus() ) );

		int i;

		ps.print( skill.getRole().getId() + "," );
		ps.print( skill.getName() + "," );
		ps.print( skill.getIcon() + "," );
		ps.print( other.getDisplayorder() + "," );
		ps.print( skill.getSkilldesc() + "," );
		ps.print( skill.getPreconditonSkill() + "," );
		ps.print( skill.getReviselevel() + "," );
		ps.print( skill.getPassiveStatus() + "," );
		ps.print( skill.getMaxLevel() + "," );
		ps.print( other.getMainskillproperty() + "," );

		ps.print( skill.getNeedgoal() + "," );
		ps.print( other.getIsonland() + "," );
		ps.print( other.getIsinsky() + "," );
		ps.print( other.getIsinwater() + "," );
		ps.print( other.getRidable() + "," );
		ps.print( other.getIntonateActionName() + "," );
		ps.print( other.getFutihou() + "," );
		ps.print( other.getContinuoushit() + "," );
		ps.print( other.getBianshenhou() + "," );
		String strArmlimitation = new String();
		String[] arms = skill.getArmlimitation();
		for( i = 0; null!= arms && i<arms.length; i++ )
			strArmlimitation += (arms[i] + ";");
		ps.print( strArmlimitation + "," );
		ps.print( other.getCannotusetogether() + "," );

		int count = 0;
		List procedures = skill.getProcedures();
		if( null != procedures )
		{
			int stateid = 0;
			for(Iterator itp=procedures.iterator(); itp.hasNext(); )
			{
				ProcedureVO p = (ProcedureVO)itp.next();

				ps.print( p.getName() + "," );
				ps.print( p.getContinuetime() + "," );
				ps.print( p.getQuitCondition() + "," );
				ps.print( p.getLoopCondition() + "," );
				ps.print( p.getBypassCondition() + "," );
				ps.print( p.getComputeResult() + "," );	// calculate
				ps.print( p.getAsynchronousTerminate() + "," );	// interrupt
				ps.print( p.getKequxiao() + "," );	// cancel
				ps.print( p.getXuli() + "," );	// skip
				count ++;
			}
		}
		while( count ++ < 3 )	ps.print( ",,,,,,,,," );

		ps.print( other.getEffectFileName() + "," );
		ps.print( other.getHitEnjureActionName() + "," );
		ps.print( other.getFenmingtime() + "," );
		ps.print( other.getExeactionname() + "," );

		ps.print( range.getApproach() + "," );
		ps.print( range.getAttackdistance00() + "," );
		ps.print( range.getAttackdistance01() + "," );
		ps.print( range.getRadious00() + "," );
		ps.print( range.getRadious01() + "," );
		ps.print( range.getRadious02() + "," );
		ps.print( range.getAngle() + "," );

		ps.print( skill.getCondition() + "," );
		ps.print( skill.getDistance() + "," );
		ps.print( skill.getExecutetime() + "," );
		ps.print( skill.getCooltime() + "," );
		ps.print( skill.getHatedegree() + "," );
		ps.print( stateattacks + "," );
		ps.print( (null!=skill.getInitskilluse()&&skill.getInitskilluse().length()>0)?skill.getInitskilluse():skill.getChangeweaponskilluse() + "," );

		ps.print( levelinfo.getLevel1Learning() + "," );
		ps.print( levelinfo.getLevel2Learning() + "," );
		ps.print( levelinfo.getLevel3Learning() + "," );
		ps.print( levelinfo.getLevel4Learning() + "," );
		ps.print( levelinfo.getLevel5Learning() + "," );
		ps.print( levelinfo.getLevel6Learning() + "," );
		ps.print( levelinfo.getLevel7Learning() + "," );
		ps.print( levelinfo.getLevel8Learning() + "," );
		ps.print( levelinfo.getLevel9Learning() + "," );
		ps.print( levelinfo.getLevel10Learning() + "," );

		ps.print( levelinfo.getLevel1Cost() + "," );
		ps.print( levelinfo.getLevel2Cost() + "," );
		ps.print( levelinfo.getLevel3Cost() + "," );
		ps.print( levelinfo.getLevel4Cost() + "," );
		ps.print( levelinfo.getLevel5Cost() + "," );
		ps.print( levelinfo.getLevel6Cost() + "," );
		ps.print( levelinfo.getLevel7Cost() + "," );
		ps.print( levelinfo.getLevel8Cost() + "," );
		ps.print( levelinfo.getLevel9Cost() + "," );
		ps.print( levelinfo.getLevel10Cost() + "," );

		ps.print( "\"" + levelinfo.getLevel_requirement() + "\"," );
		ps.print( "\"" + levelinfo.getSkill_point_requirement() + "\"," );
		ps.print( "\"" + other.getFly_effect_proportion() + "\"," );
		ps.print( "\"" + other.getFly_effect_parameter() + "\"," );
		ps.print( "\"" + other.getFly_effect_time() + "\"," );

		ps.print("\n");
	}

}

