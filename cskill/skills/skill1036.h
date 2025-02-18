#ifndef __CPPGEN_GNET_SKILL1036
#define __CPPGEN_GNET_SKILL1036
namespace GNET
{
#ifdef _SKILL_SERVER
    class Skill1036:public Skill
    {
      public:
        enum
        { SKILL_ID = 1036 };
          Skill1036 ():Skill (SKILL_ID)
        {
        }
    };
#endif
    class Skill1036Stub:public SkillStub
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
                skill->GetPlayer ()->SetDecelfmp (125 + (skill->GetLevel () - 1) * 3);
                skill->GetPlayer ()->SetDecelfap (801 + 99 * (skill->GetLevel () - 1));
                skill->SetWaterdamage ((78 + (skill->GetLevel () - 1) * 54 + skill->GetT1 () * 2) * 6.7);
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
      Skill1036Stub ():SkillStub (1036)
        {
            cls = 258;
            name = L"寒冰领域";
            nativename = "寒冰领域";
            icon = "寒冰领域.dds";
            max_level = 10;
            type = 1;
            apcost = 801099;
            arrowcost = 0;
            apgain = 0;
            attr = 4;
            rank = 0;
            eventflag = 0;
            time_type = 1;
            showorder = 0;
            allow_land = 0;
            allow_air = 0;
            allow_water = 1;
            allow_ride = 0;
            auto_attack = 0;
            long_range = 0;
            restrict_corpse = 0;
            allow_forms = 1;
            effect = "寒冰领域.sgc";
            range.type = 2;
            doenchant = true;
            dobless = false;
            commoncooldown = 0;
            commoncooldowntime = 0;
            pre_skills.push_back (std::pair < ID, int >(0, 0));
#ifdef _SKILL_SERVER
            statestub.push_back (new State1 ());
#endif
        }
        virtual ~ Skill1036Stub ()
        {
        }
        float GetMpcost (Skill * skill) const
        {
            return (float) (125 + (skill->GetLevel () - 1) * 3);
        }
        int GetExecutetime (Skill * skill) const
        {
            return 0;
        }
        int GetCoolingtime (Skill * skill) const
        {
            return 20000;
        }
        int GetRequiredLevel (Skill * skill) const
        {
            static int array[10] = { 60552, 60557, 60562, 60567, 60572, 60577, 60582, 60587, 60592, 60597 };
            return array[skill->GetLevel () - 1];
        }
        int GetRequiredSp (Skill * skill) const
        {
            static int array[10] = { 11320, 15380, 20800, 28400, 38400, 65400, 106600, 169400, 263600, 359000 };
            return array[skill->GetLevel () - 1];
        }
        float GetRadius (Skill * skill) const
        {
            return (float) (20);
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
            return (float) (18);
        }
#ifdef _SKILL_CLIENT
        int GetIntroduction (Skill * skill, wchar_t * buffer, int length, wchar_t * format) const
        {
            return _snwprintf (buffer, length, format,
                               skill->GetLevel (),
                               125 + (skill->GetLevel () - 1) * 3,
                               801 + 99 * (skill->GetLevel () - 1), (78 + 54 * (skill->GetLevel () - 1)) * 6.7, 20 + skill->GetLevel () * 3);

        }
#endif
#ifdef _SKILL_SERVER
        int GetEnmity (Skill * skill) const
        {
            return 0;
        }
        bool StateAttack (Skill * skill) const
        {
            skill->GetVictim ()->SetProbability (1.0 * 60);
            skill->GetVictim ()->SetTime (3000);
            skill->GetVictim ()->SetFix (1);
            skill->GetVictim ()->SetProbability (1.0 * 60 + skill->GetT1 () * 0.2);
            skill->GetVictim ()->SetTime (3000);
            skill->GetVictim ()->SetRatio (0.2 + skill->GetLevel () * 0.03 + skill->GetT1 () * 0.0025);
            skill->GetVictim ()->SetSlowattack (1);
            skill->GetVictim ()->SetProbability (1.0 * 60 + skill->GetT1 () * 0.2);
            skill->GetVictim ()->SetTime (3000);
            skill->GetVictim ()->SetRatio (0.2 + skill->GetLevel () * 0.03 + skill->GetT1 () * 0.0025);
            skill->GetVictim ()->SetSlowpray (1);
            return true;
        }
        bool TakeEffect (Skill * skill) const
        {;
            return true;
        }
        float GetEffectdistance (Skill * skill) const
        {
            return (float) (21);
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
