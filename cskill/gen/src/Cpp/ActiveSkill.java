
package Cpp;

import java.io.PrintStream;
import java.util.*;

import cn.world2.game.skilleditor.biz.*;
import cn.world2.game.skilleditor.vo.*;
import cn.world2.game.skilleditor.vo.data.*;

public class ActiveSkill extends CppClass
{
	protected Skill skill;

	public ActiveSkill( Skill skill )
	{
		super( "Skill" + skill.getId(), "ActiveSkill" );
		this.skill = skill;

		add ( new CppField("enum","{ SKILL_ID = "+ skill.getId() + " }",false) );

		add ( (new CppConstructor(this)).
			add(new CppParameterList()).
			add((new CppMemberInitList()).
				add(new CppMemberInit("Skill", "SKILL_ID"))).
				//add(new CppMemberInit("ActiveSkill", "SKILL_ID"))).
			add(new CppBlock()) );
		/*

		add ( (new CppConstructor(this)).
			add((new CppParameterList()).
				add(new CppParameter("const "+super.name+" &","rhs"))).
			add((new CppMemberInitList()).
				add(new CppMemberInit("ActiveSkill", "rhs"))).
			add(new CppBlock()) );

		add ( (new CppMethod("Skill *", "Clone", false, true, false)).add(new CppParameterList()).add((new CppBlock()).add("return new "+name+"(*this);")));
		*/
	}

	public void write(PrintStream ps)
	{
		super.write(ps);
	}

	public CppComponent add(CppComponent component)
	{
		return super.add(component);
	}
}

