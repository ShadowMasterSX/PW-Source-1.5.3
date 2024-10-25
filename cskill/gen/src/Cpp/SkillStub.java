
package Cpp;

import java.io.PrintStream;
import java.util.*;

import cn.world2.game.skilleditor.biz.*;
import cn.world2.game.skilleditor.vo.*;
import cn.world2.game.skilleditor.vo.data.*;
import expr.*;

public class SkillStub extends CppClass
{
	private Skill skill;
	private List skills;
	private List weapons;
	private List allhitstatus;
	private ArrayList states       = new ArrayList();
	private boolean dynamic;
	private int errno;

	String GetWeaponId( String weaponname )
	{
		if( null == weaponname || weaponname.length() == 0 )
			return "0";
		for (Iterator it = this.weapons.iterator(); it.hasNext(); )
		{
			Weapon weapon = (Weapon)it.next();
			if( weaponname.equals( weapon.getWeaponname() ) )
				return weapon.getWeaponid();
		}
		return "0";
	}

	String GetPracticeLevel( String levelname )
	{
		if( null == levelname )
			return "-1";
		if( levelname.equals("元婴") )
			return "1";
		if( levelname.equals("渡劫") )
			return "2";
		if( levelname.equals("大乘") )
			return "3";
		if( levelname.equals("地仙") )
			return "4";
		if( levelname.equals("狂魔") )
			return "5";
		if( levelname.equals("上仙") )
			return "6";
		if( levelname.equals("魔尊") )
			return "7";
		if( levelname.equals("神") )
			return "8";
		return "-1";
	}

	private String ESCExpr(String s)
	{
		if( null == s )
			s = "";
		s.trim();
		if( s.length() > 0 && s.charAt(s.length()-1) != ';' )
			s += ";";
		return s;
	}

	private String GenExpr(String source, String resdefault, String contextclass, String prefix )
	{
		try
		{
			if( null == source || source.length() == 0 )
				return resdefault;
			// System.out.print( source + " " );
			String result = (new CodeGen()).GenExpr(ESCExpr(source),contextclass,prefix);
			result = result.substring(0,result.length()-1);
			if( null == result || result.length() == 0 )
				result = resdefault;
			return result;
		}
		catch (SyntaxErrorException e)
		{ 
			System.out.println("catch SyntaxErrorException.source="+source+";resdefault="+resdefault+";contextclass="+contextclass+";prefix="+prefix); 
			this.errno = 1;
			System.out.println(skill.getName());
		}
		catch (AnalyzerException e)
		{ 
			System.out.println("catch SyntaxErrorException.source="+source+";resdefault="+resdefault+";contextclass="+contextclass+";prefix="+prefix); 
			this.errno = 1;
			System.out.println(skill.getName());
		}
		return resdefault;
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

	String GetSelfEffect( List list )
	{
		if( null == list || list.size()<5 )
			return "";
		HitStatusParam hsp = (HitStatusParam)list.get(4);
		if( hsp.getId().equals("-1") )
			return "";

		HitStatus hss = GetHitStatusStatic(hsp.getId());
		if( null == hss )
			return "";

		String exp = "";
		if( hss.isNeed_probability() )
			exp += "victim.probability=1.0*" + ESCExpr(hsp.getProbability());
		if( hss.isNeed_time() )
			exp += "victim.time=" + ESCExpr(hsp.getTime());
		if( hss.isNeed_proportion() )
			exp += "victim.ratio=" + ESCExpr(hsp.getProportion());
		if( hss.isNeed_increasement() )
		{
			String str = hsp.getIncreasement();
			if(str==null||str.length()==0)
				str = "0";
			exp += "victim.showicon=" + ESCExpr(str);
		}
		if( hss.isNeed_total() )
			exp += "victim.amount=" + ESCExpr(hsp.getTotal());
		if( hss.isNeed_value() )
			exp += "victim.value=" + ESCExpr(hsp.getValue());

		if( null != hss.getExecute() && hss.getExecute().length() > 0 )
			exp += "victim." + hss.getExecute() + "=" + "1;";

		return exp;
	}
	String GetStateAttacks( List list )
	{
		String result = "";
		if( null == list )
			return "";
		int i = 0;

		for (Iterator it = list.iterator();it.hasNext() && i<4;i++)
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
			{
				String str = hsp.getIncreasement();
				if(str==null||str.length()==0)
					str = "0";
				exp += "victim.showicon=" + ESCExpr(str);
			}
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

	public void AddSkilldescMethod( Skill skill )
	{
		String src = skill.getSkilldesc();
		if( null == src || src.length() <= 0 )
			return;

		CppBlock block = new CppBlock();
		String[] items = src.split("[$]");

		for( int i=0; i<items.length; i++ )
		{
			items[i] = items[i].replaceAll("[\r\n]", "");
			if( 1 == (i % 2) )
			{
				String temp = items[i];
				items[i] = GenExpr(temp,"0","Skill","skill->");
				block.add( "s << " + items[i] + ";" ); 
			}
			else
			{
				block.add( "s << L\"" + items[i] + "\";" ); 
			}
		}

		add ( (new CppMethod("void", "GetExplanation", false, true, 2))
				.add((new CppParameterList())
					.add(new CppParameter("Skill*","skill"))
					.add(new CppParameter("wsstream&","s")))
				.add(block));
	}

	public SkillStub( Skill skill, List skills, List weapons, List allhitstatus, boolean dynamic )
	{
		super( "Skill" + skill.getId() + "Stub", "SkillStub" );
		this.skill = skill;
		this.skills = skills;
		this.weapons = weapons;
		this.allhitstatus = allhitstatus;
		this.dynamic = dynamic;
		this.errno = 0;

		SkillOtherVO other = skill.getOtherinfo();
		AttackWay range = other.getAttackway();
		if( null == range )	range = new AttackWay();
		SkillLevelVO levelinfo = skill.getLevelinfo();
		if( null == levelinfo ) levelinfo = new SkillLevelVO();

		String stateattacks = ESCExpr( GetStateAttacks( skill.getHitstatus() ) );
		String selfeffect   = ESCExpr( GetSelfEffect( skill.getHitstatus() ) );

		int i;

		List procedures = skill.getProcedures();
		if( null != procedures )
		{
			int stateid = 0;
			for(Iterator itp=procedures.iterator(); itp.hasNext(); )
			{
				ProcedureVO p = (ProcedureVO)itp.next();

				String name = p.getName();
				String time = GenExpr(p.getContinuetime(),"0","Skill","skill->");
				String quit = GenExpr(p.getQuitCondition(),"false","Skill","skill->");
				String loop = GenExpr(p.getLoopCondition(),"false","Skill","skill->");
				String bypass = GenExpr(p.getBypassCondition(),"false","Skill","skill->");
				String calculate = GenExpr(p.getComputeResult(),"","Skill","skill->");
				String interrupt = GenExpr(p.getAsynchronousTerminate(),"false","Skill","skill->");
				String cancel = GenExpr(p.getKequxiao(),"false","Skill","skill->");
				String skip = GenExpr(p.getXuli(),"false","Skill","skill->");

				if( null != name && name.length() > 0 )
				{
					stateid ++;
					SkillState s = SkillState.newState(stateid, name, time, quit, loop,
							bypass, calculate, interrupt, cancel, skip );
					s.time_exp = p.getContinuetime();
					s.quit_exp = p.getQuitCondition();
					s.loop_exp = p.getLoopCondition();
					s.bypass_exp = p.getBypassCondition();
					s.calculate_exp = p.getComputeResult();
					s.interrupt_exp = p.getAsynchronousTerminate();
					s.cancel_exp = p.getKequxiao();
					s.skip_exp = p.getXuli();
					add( s );
				}
			}
		}

		CppBlock block = new CppBlock();
		block.add( new CppEvaluate("cls",skill.getRole().getId(),"0",false) );
		block.add( "name = L\"" + skill.getName() + "\";" );
		block.add( "nativename = \"" + skill.getName() + "\";" );
		String icon = skill.getIcon();
		block.add( new CppEvaluate("icon",icon,"",true) );
		block.add( new CppEvaluate("max_level",skill.getMaxLevel(),"MAX_LEVEL",false) );
		block.add( new CppEvaluate("type",skill.getPassiveStatus(),"1",false) );
		block.add( new CppEvaluate("apcost",other.getMainskillproperty(),"0",false) );
		block.add( new CppEvaluate("arrowcost",skill.getReleaseeffect(),"0",false) );
		block.add( new CppEvaluate("apgain",other.getHitEnjureActionName(),"0",false) );
		block.add( new CppEvaluate("attr",other.getIsAutoSearchDestination(),"0",false) );
		block.add( new CppEvaluate("rank",skill.getReviselevel(),"-1",false) );
		block.add( new CppEvaluate("eventflag",skill.getNeedgoal(),"0",false) );

		//block.add( new CppEvaluate("pre_id",skill.getPreconditonSkill(),"0",false) );
		//block.add( new CppEvaluate("pre_level",skill.getCode(),"1",false) );
		String tmp = skill.getContinueeffect();
		if(tmp!=null && tmp.equals("1"))
		{
			block.add( new CppEvaluate("is_senior","1","0",false) );
		}
		tmp = other.getEffectfiletype();
		if(tmp!=null && !tmp.equals("0"))
		{
			block.add( new CppEvaluate("posdouble",tmp,"0",false) );
		}
		tmp = skill.getUpgradeeffect();
		if(tmp!=null && !tmp.equals("0"))
		{
			block.add( new CppEvaluate("clslimit",tmp,"0",false) );
		}
		block.add( new CppEvaluate("time_type",other.getIntonateActionName(),"0",false) );
		block.add( new CppEvaluate("showorder",other.getDisplayorder(),"0",false) );

		block.add( new CppEvaluate("allow_land",other.getIsonland(),"1",false) );
		block.add( new CppEvaluate("allow_air",other.getIsinsky(),"0",false) );
		block.add( new CppEvaluate("allow_water",other.getIsinwater(),"0",false) );
		block.add( new CppEvaluate("allow_ride",other.getRidable(),"0",false) );
		tmp = other.getAutosearchDestExp();
		if(tmp!=null && !tmp.equals("0"))
		{
			block.add( new CppEvaluate("notuse_in_combat",tmp,"0",false) );
		}
		block.add( new CppEvaluate("auto_attack",other.getFutihou(),"0",false) );
		block.add( new CppEvaluate("long_range",other.getIsAttackSkill(),"0",false) );
		block.add( new CppEvaluate("restrict_corpse",other.getContinuoushit(),"0",false) );

		// allow_changestatus
		String allow_changestatus = other.getBianshenhou();
		int allow_changestatus_i = 0;
		if( null != allow_changestatus )
		{
			String[] a = allow_changestatus.split(";");
			for( i=0; i<a.length; i++ )
				if(a[i].length()>0) allow_changestatus_i += Integer.parseInt(a[i]);
		}
		block.add( new CppEvaluate("allow_forms",(new Integer(allow_changestatus_i)).toString(),"0",false) );

		// restrict_weapons
		String[] arms = skill.getArmlimitation();
		for(i=0; null!=arms && i<arms.length; i++ )
			block.add( (new CppBlock()).add("restrict_weapons.push_back("+GetWeaponId(arms[i])+");") );

		// effect
		block.add( new CppEvaluate("effect",other.getEffectFileName(),"",true) );
		//block.add( new CppEvaluate("aerial_effect",other.getHitEnjureActionName(),"",true) );

		// range
		block.add( new CppEvaluate("range.type",range.getApproach(),"0",false) );
		String range_radius_exp = range.getRadious01();
		if( range.getApproach()!=null && range.getApproach().equals("1") )
			range_radius_exp = range.getRadious00();
		if( range.getApproach()!=null && range.getApproach().equals("4") )
			range_radius_exp = range.getRadious02();

		block.add ( new CppEvaluate("doenchant", (stateattacks.length()<=0)?"false":"true", "false", false) );
		block.add ( new CppEvaluate("dobless", (selfeffect.length()<=0)?"false":"true", "false", false) );

		//公共冷却
		String commoncooldown = other.getSearchcenter();
		int commoncooldown_mask = 0;
		if( null != commoncooldown )
		{
			String[] a = commoncooldown.split(";");
			for( i=0; i<a.length; i++ )
				if(a[i].length()>0) commoncooldown_mask += (1 << Integer.parseInt(a[i]));
		}
		block.add( new CppEvaluate("commoncooldown",(new Integer(commoncooldown_mask)).toString(),"0",false) );
		block.add( new CppEvaluate("commoncooldowntime",other.getExeactionnameselect(),"0",false) );
	
		//消耗物品
		tmp = skill.getAttack();
		if(tmp!=null && tmp.length()!=0 && !tmp.equals("0"))
		{
			block.add( new CppEvaluate("itemcost",tmp,"0",false) );
		}

		//前提技能
		String preskill_str = skill.getPreconditonSkill();
		String preskilllevel_str = skill.getCode(); 
		if(preskill_str != null && preskilllevel_str != null)
		{
			String[] preskill_list = preskill_str.split(";");
			String[] preskilllevel_list = preskilllevel_str.split(";");
			if(preskill_list != null && preskilllevel_list != null && preskill_list.length == preskilllevel_list.length)
			{
				for(i=0; i<preskill_list.length; i++)
				{
					if(preskill_list[i] != null && preskill_list[i].length() > 0 && preskilllevel_list[i] != null && preskilllevel_list[i].length() > 0)
					{
						block.add("pre_skills.push_back(std::pair<ID, int>("+preskill_list[i]+","+preskilllevel_list[i]+"));");
					}
				}
			}
		}
		
		//连续技
		tmp = skill.getCombosk_preskill();
		String tmp2 = skill.getCombosk_interval();
		if(tmp!=null && tmp.length()!=0 && !tmp.equals("0")
				&& tmp2!=null && tmp2.length()!=0 && !tmp2.equals("0"))
		{
			block.add( new CppEvaluate("combosk_preskill",tmp,"0",false) );
			block.add( new CppEvaluate("combosk_interval",tmp2,"0",false) );
		}
		tmp = skill.getCombosk_nobreak();
		if(tmp!=null && tmp.length()!=0 && !tmp.equals("0"))
		{
			block.add( new CppEvaluate("combosk_nobreak",tmp,"0",false) );
		}
		//天生技能
		tmp = skill.getInherent();
		if(tmp!=null && tmp.length()!=0 && tmp.equals("1"))
		{
			block.add( new CppEvaluate("is_inherent","true","false",false) );
		}
		//移动释放
		tmp = skill.getMovingcast();
		if(tmp!=null && tmp.length()!=0 && tmp.equals("1"))
		{
			block.add( new CppEvaluate("is_movingcast","true","false",false) );
		}

		if( dynamic )
		{
			/* Version dynamic */
			for(Iterator its=this.states.iterator(); its.hasNext(); )
			{
				SkillState s = (SkillState)its.next();
				block.add( "statestub.push_back(new State("+s.stateid+",\""+s.name+"\",\""+ESCExpr(s.time_exp)+"\",\""+ESCExpr(s.quit_exp)+"\",\""+ESCExpr(s.loop_exp)+"\",\""+ESCExpr(s.bypass_exp)+"\",\""+ESCExpr(s.calculate_exp)+"\",\""+ESCExpr(s.interrupt_exp)+"\",\""+ESCExpr(s.cancel_exp)+"\",\""+ESCExpr(s.skip_exp)+"\"));");
			}

			block.add ( new CppEvaluate("learncond_exp[1]",ESCExpr(levelinfo.getLevel1Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[2]",ESCExpr(levelinfo.getLevel2Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[3]",ESCExpr(levelinfo.getLevel3Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[4]",ESCExpr(levelinfo.getLevel4Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[5]",ESCExpr(levelinfo.getLevel5Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[6]",ESCExpr(levelinfo.getLevel6Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[7]",ESCExpr(levelinfo.getLevel7Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[8]",ESCExpr(levelinfo.getLevel8Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[9]",ESCExpr(levelinfo.getLevel9Learning()),"1;",true) );
			block.add ( new CppEvaluate("learncond_exp[10]",ESCExpr(levelinfo.getLevel10Learning()),"1;",true) );

			block.add ( new CppEvaluate("learn_exp[1]",ESCExpr(levelinfo.getLevel1Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[2]",ESCExpr(levelinfo.getLevel2Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[3]",ESCExpr(levelinfo.getLevel3Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[4]",ESCExpr(levelinfo.getLevel4Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[5]",ESCExpr(levelinfo.getLevel5Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[6]",ESCExpr(levelinfo.getLevel6Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[7]",ESCExpr(levelinfo.getLevel7Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[8]",ESCExpr(levelinfo.getLevel8Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[9]",ESCExpr(levelinfo.getLevel9Cost()), "1;", true) );
			block.add ( new CppEvaluate("learn_exp[10]",ESCExpr(levelinfo.getLevel10Cost()), "1;", true) );

			block.add ( new CppEvaluate("mpcost_exp",ESCExpr(skill.getCondition()), "0;", true) );
			block.add ( new CppEvaluate("executetime_exp",ESCExpr(skill.getExecutetime()), "0;", true) );
			block.add ( new CppEvaluate("coolingtime_exp",ESCExpr(skill.getCooltime()), "0;", true) );
			block.add ( new CppEvaluate("stateattack_exp",stateattacks, "", true) );
			block.add ( new CppEvaluate("enmity_exp",ESCExpr(skill.getHatedegree()), "0;", true) );
			block.add ( new CppEvaluate("takeeffect_exp",ESCExpr(skill.getInitskilluse()), "", true) );
			block.add ( new CppEvaluate("requiredlevel_exp",ESCExpr(levelinfo.getLevel_requirement()), "0;", true) );
			block.add ( new CppEvaluate("requiredsp_exp",ESCExpr(levelinfo.getSkill_point_requirement()), "0;", true) );

			block.add ( new CppEvaluate("range.radius_exp",ESCExpr(range_radius_exp), "0;", true) );
			block.add ( new CppEvaluate("range.attackdistance_exp",ESCExpr(range.getAttackdistance01()), "0;", true) );
			block.add ( new CppEvaluate("range.angle_exp","1-0.0111111*("+ESCExpr(range.getAngle())+")", "0;", true) );
			block.add ( new CppEvaluate("range.praydistance_exp",ESCExpr(skill.getDistance()), "0;", true) );
			block.add ( new CppEvaluate("range.effectdistance_exp",ESCExpr(range.getAttackdistance00()), "0;", true) );
		}
		else
		{
			/* Version static */
			block.add("#ifdef _SKILL_SERVER");
			for(Iterator its=this.states.iterator(); its.hasNext(); )
				block.add( "statestub.push_back(new "+((SkillState)its.next()).getName()+"());");

			block.add("#endif");
			CppDefineScope scope = new CppDefineScope("_SKILL_SERVER");
			scope.add ( (new CppMethod("int", "GetEnmity", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return "+GenExpr(skill.getHatedegree(),"0","Skill","skill->")+";")));
			if(stateattacks.length()>0)
				scope.add ( (new CppMethod("bool", "StateAttack", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr(stateattacks,"","Skill","skill->")+";return true;")));
			if(selfeffect.length()>0)
				scope.add ( (new CppMethod("bool", "BlessMe", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr(selfeffect,"","Skill","skill->")+";return true;")));
			scope.add ( (new CppMethod("bool", "TakeEffect", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr(((null!=skill.getInitskilluse()&&skill.getInitskilluse().length()>0)?skill.getInitskilluse():skill.getChangeweaponskilluse()),"","Skill","skill->")+";return true;")));
			scope.add ( (new CppMethod("float", "GetEffectdistance", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (float)("+GenExpr(range.getAttackdistance00(),"0","Skill","skill->")+");")));
			String tmpstr = other.getFenmingtime();
			if(tmpstr!=null && tmpstr.length()!=0)
			{
				int speed = Integer.parseInt(tmpstr)/50;
				if(speed>256)
					speed = 256;

				scope.add ( (new CppMethod("int", "GetAttackspeed", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return " + speed +";")));
			}

			scope.add ( (new CppMethod("float", "GetHitrate", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (float)("+GenExpr(other.getExeactionname(),"1.0","Skill","skill->")+");")));

			tmpstr = other.getHit_effect_proportion();
			if(tmpstr!=null && tmpstr.length()!=0 && !tmpstr.equals("0"))
			{
				scope.add ( (new CppMethod("float", "GetTalent0", false, true, 0)).add((new CppParameterList()).add(new CppParameter("PlayerWrapper*","player"))).add((new CppBlock()).add("return (float)("+GenExpr(tmpstr,"0","PlayerWrapper","player->")+");")));
			}
			tmpstr = other.getFly_effect_behave();
			if(tmpstr!=null && tmpstr.length()!=0 && !tmpstr.equals("0"))
			{
				scope.add ( (new CppMethod("float", "GetTalent1", false, true, 0)).add((new CppParameterList()).add(new CppParameter("PlayerWrapper*","player"))).add((new CppBlock()).add("return (float)("+GenExpr(tmpstr,"0","PlayerWrapper","player->")+");")));
			}
			tmpstr = other.getCannotusetogether();
			if(tmpstr!=null && tmpstr.length()!=0 && !tmpstr.equals("0"))
			{
				scope.add ( (new CppMethod("float", "GetTalent2", false, true, 0)).add((new CppParameterList()).add(new CppParameter("PlayerWrapper*","player"))).add((new CppBlock()).add("return (float)("+GenExpr(tmpstr,"0","PlayerWrapper","player->")+");")));
			}
			tmpstr = skill.getCombosk_endaction();
			if(tmpstr!=null && tmpstr.length()!=0 && !tmpstr.equals("0"))
			{
				scope.add ( (new CppMethod("void", "ComboSkEndAction", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr(tmpstr,"","Skill","skill->") + ";")));
			}
			add(scope);


			add ( (new CppMethod("float", "GetMpcost", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (float)( "+GenExpr(skill.getCondition(),"0","Skill","skill->")+");")));
			add ( (new CppMethod("int", "GetExecutetime", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return "+GenExpr(skill.getExecutetime(),"0","Skill","skill->")+";")));
			add ( (new CppMethod("int", "GetCoolingtime", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return "+GenExpr(skill.getCooltime(),"0","Skill","skill->")+";")));
			tmpstr = levelinfo.getLevel_requirement();
			if(tmpstr!=null && tmpstr.length()!=0)
			{
				add ( (new CppMethod("int", "GetRequiredLevel", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr("static int array[10]={"+tmpstr+"}","","Skill","skill->")+";return array[skill->GetLevel()-1];")));
			}
			tmpstr = levelinfo.getSkill_point_requirement();
			if(tmpstr!=null && tmpstr.length()!=0)
			{
				add ( (new CppMethod("int", "GetRequiredSp", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr("static int array[10]={"+tmpstr+"}","","Skill","skill->")+";return array[skill->GetLevel()-1];")));
			}
			tmpstr = other.getFly_effect_proportion();
			if(tmpstr!=null && tmpstr.length()!=0)
			{
				add ( (new CppMethod("int", "GetRequiredItem", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr("static int array[10]={"+tmpstr+"}","","Skill","skill->")+";return array[skill->GetLevel()-1];")));
			}
			tmpstr = other.getFly_effect_parameter();
			if(tmpstr!=null && tmpstr.length()!=0)
			{
				add ( (new CppMethod("int", "GetRequiredMoney", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr("static int array[10]={"+tmpstr+"}","","Skill","skill->")+";return array[skill->GetLevel()-1];")));
			}
			tmpstr = other.getFly_effect_time();
			if(tmpstr!=null && tmpstr.length()!=0)
			{
				add ( (new CppMethod("int", "GetMaxAbility", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr("static int array[10]={"+tmpstr+"}","","Skill","skill->")+";return array[skill->GetLevel()-1];")));
			}
			tmpstr = other.getExeactionweaponselect();
			if(tmpstr!=null && tmpstr.length()!=0)
			{
				add ( (new CppMethod("int", "GetRequiredRealmLevel", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add(GenExpr("static int array[10]={"+tmpstr+"}","","Skill","skill->")+";return array[skill->GetLevel()-1];")));
			}
			add ( (new CppMethod("float", "GetRadius", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (float)("+GenExpr(range_radius_exp,"0","Skill","skill->")+");")));
			add ( (new CppMethod("float", "GetAttackdistance", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (float)("+GenExpr(range.getAttackdistance01(),"0","Skill","skill->")+");")));
			add ( (new CppMethod("float", "GetAngle", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (float)(1-0.0111111*("+GenExpr(range.getAngle(),"0","Skill","skill->")+"));")));
			add ( (new CppMethod("float", "GetPraydistance", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (float)("+GenExpr(skill.getDistance(),"0","Skill","skill->")+");")));
			//增加技能施放时hp的限制
			tmpstr = skill.getUsehpcondition();
			if(tmpstr!= null && tmpstr.length() > 0)
			{
				String[] a = tmpstr.split(",");
				if(a.length == 2)
				{
					add ( (new CppMethod("bool", "CheckHpCondition", false, true, 0)).add((new CppParameterList()).add(new CppParameter("int","hp")).add(new CppParameter("int","max_hp"))).add((new CppBlock()).add("return hp>=max_hp/100.f*("+Integer.parseInt(a[0].trim())+") && hp<=max_hp/100.f*("+Integer.parseInt(a[1].trim())+");")));
				}
			}
			tmpstr = skill.getCombosk_extracond();
			if(tmpstr!=null && tmpstr.length()!=0 && !tmpstr.equals("0"))
			{
				add ( (new CppMethod("bool", "CheckComboSkExtraCondition", false, true, 0)).add((new CppParameterList()).add(new CppParameter("Skill*","skill"))).add((new CppBlock()).add("return (bool)("+GenExpr(tmpstr,"false","Skill","skill->")+");")));
			}

		}

		//AddSkilldescMethod( skill );
		GenerateStrings(skill);

		add ( (new CppConstructor(this)).
				add(new CppParameterList()).
				add((new CppMemberInitList()).
					add(new CppMemberInit("SkillStub", skill.getId()))).
				add(block) );

		add ( (new CppDestructor(this)).add(new CppBlock()) );
	}
	public void GenerateStrings(Skill skill)
	{
		String name = skill.getName();
		String desc = skill.getSkilldesc();
		int id = Integer.parseInt(skill.getId());
		if(desc==null)
			desc = "";
		desc  = desc.replaceAll("[\r\n ]+$", "");

		String[] items = desc.split("[$]");
		String format = "";
		CppBlock block = new CppBlock();
		if(items.length>1)
		{
			block.add("\n\treturn _snwprintf(buffer, length, format,");
			for( int i=0; i<items.length; i++ )
			{
				if( 1 == (i % 2) )
				{
					String temp = items[i];
					if(temp.indexOf(".")!=-1)
						format += "%.1f";
					else
						format += "%d";
					items[i] = GenExpr(temp,"0","Skill","skill->");
					if(i>=items.length-2)
						block.add("\t\t"+items[i]);
					else
						block.add("\t\t"+items[i]+",");
				}
				else
				{
					format += items[i];
				}
			}
			block.add("\t);\n");
		}
		else
		{
			format += desc;
			block.add("\n\treturn _snwprintf(buffer, length, format);\n");
		}
		PrintStream ups = Generator.GetStream();
		PrintStream gps = Generator.GbkStream();
		ups.print(id*10);
		gps.print(id*10);
		ups.print("  \"" + name + "\"\r\n");
		gps.print("  \"" + name + "\"\r\n");
		ups.print(id*10+1);
		gps.print(id*10+1);
		ups.print("  \"" + format + "\"\r\n");
		gps.print("  \"" + format + "\"\r\n");

		add ( (new CppMethod("int", "GetIntroduction", false, true, 2))
				.add((new CppParameterList())
					.add(new CppParameter("Skill*","skill"))
					.add(new CppParameter("wchar_t*","buffer"))
					.add(new CppParameter("int","length"))
					.add(new CppParameter("wchar_t*","format")))
				.add(block));
	}

	public void write(PrintStream ps)
	{
		super.write(ps);
	}
	public int getErrno()
	{
		return this.errno;
	}

	public CppComponent add(CppComponent component)
	{
		if (component instanceof SkillState )
		{
			this.states.add(component);
			if( dynamic )	return this;
		}
		return super.add(component);
	}
}

