#ifndef __CPPGEN_GNET_SKILL1016
#define __CPPGEN_GNET_SKILL1016
namespace GNET
{
#ifdef _SKILL_SERVER
    class Skill1016:public Skill
    {
      public:
        enum
        { SKILL_ID = 1016 };
          Skill1016 ():Skill (SKILL_ID)
        {
        }
    };
#endif
    class Skill1016Stub:public SkillStub
    {
      public:
#ifdef _SKILL_SERVER
        class State1:public SkillStub::State
        {
          public:
            int GetTime (Skill * skill) const
            {
                return 0;
            }
            bool Quit (Skill * skill) const
            {
                return false;
            }
            bool Loop (Skill * skill) const
            {
                return false;
            }
            bool Bypass (Skill * skill) const
            {
                return false;
            }
            void Calculate (Skill * skill) const
            {
                skill->GetPlayer ()->SetDecelfmp (55 + (skill->GetLevel () - 1) * 2);
                skill->GetPlayer ()->SetDecelfap (300 + skill->GetLevel () * 60);
                skill->GetPlayer ()->SetPerform (1);
            }
            bool Interrupt (Skill * skill) const
            {
                return false;
            }
            bool Cancel (Skill * skill) const
            {
                return 0;
            }
            bool Skip (Skill * skill) const
            {
                return 0;
            }
        };
#endif
      Skill1016Stub ():SkillStub (1016)
        {
            cls = 258;
            name = L"气缚咒";
            nativename = "气缚咒";
            icon = "气缚咒.dds";
            max_level = 10;
            type = 3;
            apcost = 360060;
            arrowcost = 0;
            apgain = 0;
            attr = 7;
            rank = 0;
            eventflag = 0;
            time_type = 1;
            showorder = 0;
            allow_land = 1;
            allow_air = 1;
            allow_water = 1;
            allow_ride = 0;
            auto_attack = 0;
            long_range = 0;
            restrict_corpse = 0;
            allow_forms = 1;
            effect = "气缚咒.sgc";
            range.type = 0;
            doenchant = true;
            dobless = false;
            commoncooldown = 0;
            commoncooldowntime = 0;
            pre_skills.push_back (std::pair < ID, int >(0, 0));
#ifdef _SKILL_SERVER
            statestub.push_back (new State1 ());
#endif
        }
        virtual ~ Skill1016Stub ()
        {
        }
        float GetMpcost (Skill * skill) const
        {
            return (float) (55 + (skill->GetLevel () - 1) * 2);
        }
        int GetExecutetime (Skill * skill) const
        {
            return 0;
        }
        int GetCoolingtime (Skill * skill) const
        {
            return 60000;
        }
        int GetRequiredLevel (Skill * skill) const
        {
            static int array[10] = { 2010222, 2010227, 2010232, 2010237, 2010242, 2010247, 2010252, 2010257, 2010262, 2010267 };
            return array[skill->GetLevel () - 1];
        }
        int GetRequiredSp (Skill * skill) const
        {
            static int array[10] = { 1280, 2000, 2980, 4300, 6000, 8280, 11320, 15380, 20800, 28400 };
            return array[skill->GetLevel () - 1];
        }
        float GetRadius (Skill * skill) const
        {
            return (float) (0);
        }
        float GetAttackdistance (Skill * skill) const
        {
            return (float) (0);
        }
        float GetAngle (Skill * skill) const
        {
            return (float) (1 - 0.0111111 * (0));
        }
        float GetPraydistance (Skill * skill) const
        {
            return (float) (5);
        }
#ifdef _SKILL_CLIENT
        int GetIntroduction (Skill * skill, wchar_t * buffer, int length, wchar_t * format) const
        {
            return _snwprintf (buffer, length, format,
                               skill->GetLevel (), 55 + (skill->GetLevel () - 1) * 2, 300 + 60 * skill->GetLevel (), 15 + skill->GetLevel ());

        }
#endif
#ifdef _SKILL_SERVER
        int GetEnmity (Skill * skill) const
        {
            return 10;
        }
        bool StateAttack (Skill * skill) const
        {
            skill->GetVictim ()->SetProbability (1.0 * 15 + skill->GetLevel () + skill->GetT1 () * 0.25);
            skill->GetVictim ()->SetTime (4000);
            skill->GetVictim ()->SetSealed (1);
            return true;
        }
        bool TakeEffect (Skill * skill) const
        {;
            return true;
        }
        float GetEffectdistance (Skill * skill) const
        {
            return (float) (6);
        }
        int GetAttackspeed (Skill * skill) const
        {
            return 20;
        }
        float GetHitrate (Skill * skill) const
        {
            return (float) (1.0);
        }
        float GetTalent0 (PlayerWrapper * player) const
        {
            return (float) (player->GetElfstr ());
        }
        float GetTalent1 (PlayerWrapper * player) const
        {
            return (float) (player->GetElfagi ());
        }
#endif
    };
}
#endif
