#ifndef __SKILL_EXPR_H
#define __SKILL_EXPR_H

#include "expr.h"

namespace GNET
{

class Skill;
class PlayerWrapper;

class SkillExpr
{
public:
	static void UpdateExpr( const std::string & key, const std::string & expr )
	{
		if( key.length() > 0 && expr.length() > 0 )
			Expr::GetInstance().UpdateExpr( key, expr, "Skill" );
	}

	static int  intValue( const std::string & key, Skill * skill )
	{
		SkillCO	sco(skill);
		try {
			return Expr::GetInstance().intValue( key, &sco );
		} catch ( ... ) { }
		return 0;
	}

	static float floatValue( const std::string & key, Skill * skill )
	{
		SkillCO	sco(skill);
		try {
			return Expr::GetInstance().floatValue( key, &sco );
		} catch ( ... ) { }
		return 0;
	}

	static bool boolValue( const std::string & key, Skill * skill )
	{
		SkillCO	sco(skill);
		try {
			return Expr::GetInstance().boolValue( key, &sco );
		} catch ( ... ) { }
		return false;
	}

	static void Transact( const std::string & key, Skill * skill )
	{
		SkillCO	sco(skill);
		try {
			Expr::GetInstance().Transact( key, &sco );
		} catch ( ... ) { }
	}
};

};

#endif

