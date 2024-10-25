
package Cpp;

import java.io.PrintStream;
import java.util.*;

public class SkillState extends CppClass
{
	public int stateid;
	public String name;
	public String time;
	public String time_exp;
	public String quit;
	public String quit_exp;
	public String loop;
	public String loop_exp;
	public String bypass;
	public String bypass_exp;
	public String calculate;
	public String calculate_exp;
	public String interrupt;
	public String interrupt_exp;
	public String cancel;
	public String cancel_exp;
	public String skip;
	public String skip_exp;

	public SkillState(int stateid, String name, String time, String quit, String loop, String bypass, String calculate, String interrupt, String cancel, String skip)
	{
		super( "State"+stateid, "SkillStub::State", true);

		this.stateid = stateid;
		this.name = name;

		this.time = time;
		this.quit = quit;
		this.loop = loop;
		this.bypass = bypass;
		this.calculate = calculate;
		this.interrupt = interrupt;
		this.cancel = cancel;
		this.skip = skip;
	}

	public static SkillState newState(int stateid, String name, String time, String quit, String loop, String bypass, String calculate, String interrupt, String cancel, String skip)
	{
		if( null == time || time.length() == 0 )			time = "1";
		if( null == quit || quit.length() == 0 )			quit = "false";
		if( null == loop || loop.length() == 0 )			loop = "false";
		if( null == bypass || bypass.length() == 0 )		bypass = "false";
		if( null == interrupt || interrupt.length() == 0 )	interrupt = "false";
		if( null == cancel || cancel.length() == 0 )		cancel = "false";
		if( null == skip || skip.length() == 0 )			skip = "false";

		return (SkillState)new SkillState(stateid,name,time,quit,loop,bypass,calculate,interrupt,cancel,skip).
			add(new CppMethod("int", "GetTime", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add("return " + time + ";"))).
			add(new CppMethod("bool", "Quit", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add("return " + quit + ";"))).
			add(new CppMethod("bool", "Loop", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add("return " + loop + ";"))).
			add(new CppMethod("bool", "Bypass", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add("return " + bypass + ";"))).
			add(new CppMethod("void", "Calculate", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add(calculate.length()>0?calculate+";":""))).
			add(new CppMethod("bool", "Interrupt", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add("return " + interrupt + ";"))).
			add(new CppMethod("bool", "Cancel", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add("return " + cancel + ";"))).
			add(new CppMethod("bool", "Skip", false, true, 0).
				add( (new CppParameterList()).
					add(new CppParameter("Skill *", "skill"))).
				add( (new CppBlock()).add("return " + skip + ";")));
	}


}

