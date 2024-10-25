
package Cpp;

import java.io.PrintStream;
import java.util.*;

import cn.world2.game.skilleditor.biz.*;
import cn.world2.game.skilleditor.vo.*;
import cn.world2.game.skilleditor.vo.data.*;

public class SkillRoot extends CppClass
{
	protected Skill skill;

	public SkillRoot( Skill skill )
	{
		super( "Skill" + skill.getId(), "Skill" , true);
		this.skill = skill;

		add ( new CppField("enum","{ SKILL_ID = "+ skill.getId() + " }",false) );

		add ( (new CppConstructor(this)).
			add(new CppParameterList()).
			add((new CppMemberInitList()).
				add(new CppMemberInit("Skill", "SKILL_ID"))).
			add(new CppBlock()) );
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

