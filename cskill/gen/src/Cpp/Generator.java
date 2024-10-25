
package Cpp;

import java.io.PrintStream;
import java.util.*;
import java.io.*;

import cn.world2.game.skilleditor.DB.DAO.JDBC.*;
import cn.world2.game.skilleditor.biz.*;
import cn.world2.game.skilleditor.vo.*;
import cn.world2.game.skilleditor.vo.data.*;
import expr.*;

public class Generator
{
	public Generator( ) { }
	static PrintStream strtable;
	static PrintStream gbktable;
	static int retcode = 0;
	public static PrintStream GetStream() { return strtable; }
	public static PrintStream GbkStream() { return gbktable; }

	public static void genSkills( boolean dynamic ) throws Exception
	{
		ArrayList allskills      = new ArrayList();
		strtable = new PrintStream(new FileOutputStream("skills/skillstr.txt",false),false,"UTF-16LE");
		gbktable = new PrintStream(new FileOutputStream("skills/skillgbk.txt",false),false,"GBK");
		byte header[] = { (byte)0xFF, (byte)0xFE };
		strtable.write(header, 0, 2); 
		strtable.print("#_index\r\n#_begin\r\n\r\n");
		gbktable.print("#_index\r\n#_begin\r\n\r\n");

		RoleManager rm = new RoleManager();
		SkillDAOImp sm = new SkillDAOImp();
		WeaponDAOImp wm = new WeaponDAOImp();

		List roles = rm.getAllRolesWithSkills();
		List weapons = wm.getAvailableWeapon();
		for (Iterator itr = roles.iterator(); itr.hasNext(); )
		{
			Role role = (Role)itr.next();
			List roleskills = role.getSkills();

			for (Iterator its = roleskills.iterator(); its.hasNext(); )
			{
				Skill sk = (Skill)its.next();
				String sid = sk.getId();

				Skill skill = sm.getSkillById(sid);
				skill.setRole(role);

				allskills.add( skill );
			}
		}

		int n = 1;
		int index = 0;
		Iterator its = allskills.listIterator();
		while(its.hasNext())
		{
			int counter;
			CppCppFile cfstubs = new CppCppFile("GNET");
			cfstubs.add( "#include \"skill.h\"" );

			// ×´Ì¬¹¥»÷
			HitStatusDAOImp hsd = new HitStatusDAOImp();
			List allhitstatus = hsd.retrieveAllHitStatus();
			for (counter=0, its=allskills.listIterator(index); counter<256 && its.hasNext(); counter++)
			{
				Skill skill = (Skill)its.next();
				//System.out.println("include " + skill.getId());

				if( genSkill( skill, allskills, weapons, allhitstatus, dynamic ) )
				{
					cfstubs.add( "#include \"skill"+skill.getId()+".h\"" );
					if( dynamic )
					{
						cfstubs.add(new CppField("Skill"+skill.getId()+"Stub*", "__stub_Skill"+skill.getId()+
									"Stub = new Skill"+skill.getId()+"Stub()",false));
					}
					else
					{
						cfstubs.add(new CppField("Skill"+skill.getId()+"Stub", "__stub_Skill"+skill.getId()+"Stub",false));
					}
				}
			}
			//System.out.println("sum " + counter);

			cfstubs.add(new CppComment("#ifdef _SKILL_SERVER"));
			for (counter=0, its=allskills.listIterator(index); counter<256 && its.hasNext(); counter++)
			{
				Skill skill = (Skill)its.next();

				//System.out.println("stub " + skill.getId());
				if( dynamic )
				{
					cfstubs.add(new CppField("Skill"+skill.getId()+"*", "\t__stub_Skill"+skill.getId()+
								" = new Skill"+skill.getId()+"()",false));
				}
				else
				{
					cfstubs.add(new CppField("Skill"+skill.getId(), "\t__stub_Skill"+skill.getId(),false));
				}
			}
			//System.out.println("sum " + counter);
			cfstubs.add(new CppComment("#endif"));
			cfstubs.write( "skills/stubs" + n + ".cpp" );
			n++;
			index+=256;
		}
		strtable.close();       
		gbktable.close();  
	}

	public static boolean genSkill(Skill skill, List skills, List weapons, List allhitstatus,boolean dynamic)
	{
		System.out.println( "generate Skill(roleid=" + skill.getRole().getId()
							+ ", skillid=" + skill.getId()
							+ ", skillname=" + skill.getName()  );
		
		CppClass as = null;
		String type = skill.getPassiveStatus();
		if( null == type )	type = new String();
		type.trim();

		as = new SkillRoot(skill);

		SkillStub ss = new SkillStub( skill, skills, weapons, allhitstatus, dynamic );
		if(ss.getErrno()!=0)
			retcode = ss.getErrno();

		CppHeaderFile cf = new CppHeaderFile("skill"+skill.getId(), "GNET");
		cf.add( as );
		cf.add( ss );
		cf.write( "skills/skill" + skill.getId() + ".h" );
		return true;
	}

	public static void genSample()
	{
		CppClass cc = new CppClass("NineBoxingStub", "SkillStub");

		cc.add (SkillState.newState(1,"State1","2000","true","false","false","","rand()<(100-5*skill->GetLevel())","true","true"));
		cc.add (SkillState.newState(2,"State2","1000","true","false","false","skill->GetPlayer()->SetMP(skill->GetPlayer()->GetMP() - 25 - skill->GetLevel() )","false","false","false"));

		cc.add ( (new CppConstructor(cc)).
			add(new CppParameterList()).
			add((new CppMemberInitList()).
				add(new CppMemberInit("SkillStub", "NineBoxing::SKILL_ID"))).
			add(new CppBlock()) );
		
		cc.add ( (new CppMethod("int", "GetCoolingTime", false, true, 0)).add(new CppParameterList()).add((new CppBlock()).add("return 30;")));

		cc.add ( (new CppDestructor(cc)).add(new CppBlock()) );
		cc.write(System.out);
	}

	public static void genSkillName()
	{
		RoleManager rm = new RoleManager();
		SkillManager sm = new SkillManager();
		WeaponDAOImp wm = new WeaponDAOImp();
		byte[] header = new byte[] { (byte)0xFF, (byte)0xFE };
		PrintStream ps = null;

		try{
			ps = new PrintStream(new FileOutputStream("skills/skill.txt",false),false,"gbk");
			ps.write(header, 0, 2);
			ps.print("#_index\r\n");
			ps.print("#_begin\r\n");

			List roles = rm.getAllRolesWithSkills();
			List weapons = wm.getAvailableWeapon();
			for (Iterator itr = roles.iterator(); itr.hasNext(); )
			{
				Role role = (Role)itr.next();
				List roleskills = role.getSkills();

				for (Iterator its = roleskills.iterator(); its.hasNext(); )
				{
					Skill sk = (Skill)its.next();
					String sid = sk.getId();

					Skill skill = sm.getBasicSkill(sid);
					ps.print(sid + " \"");
					ps.print(skill.getName());
					ps.print("\"\r\n");
				}
			}
			ps.close();
		}catch(Exception e)
		{
		}

	}

	public static void exportCsv( )
	{
		ArrayList allskills      = new ArrayList();

		RoleManager rm = new RoleManager();
		SkillDAOImp sm = new SkillDAOImp();
		WeaponDAOImp wm = new WeaponDAOImp();

		List roles = rm.getAllRolesWithSkills();
		List weapons = wm.getAvailableWeapon();
		for (Iterator itr = roles.iterator(); itr.hasNext(); )
		{
			Role role = (Role)itr.next();
			List roleskills = role.getSkills();

			for (Iterator its = roleskills.iterator(); its.hasNext(); )
			{
				Skill sk = (Skill)its.next();
				String sid = sk.getId();

				Skill skill = sm.getSkillById(sid);
				skill.setRole(role);

				allskills.add( skill );
			}
		}

		HitStatusDAOImp hsd = new HitStatusDAOImp();
		List allhitstatus = hsd.retrieveAllHitStatus();

		ExportCsv.writeheader( System.err );
		for (Iterator its = allskills.iterator(); its.hasNext(); )
		{
			Skill skill = (Skill)its.next();
			ExportCsv csv = new ExportCsv( skill, allskills, weapons, allhitstatus );
			csv.write( System.err );
		}
	}

	public static void main(String[] args)
	{
		System.setProperty ( "file.encoding", "GBK" );

		if(args.length<1)
		{
			System.err.println("Usage: java -jar gen.jar <OG file> [dynamic|name|export]");
			return;
		}
		new CodeGen(args[0]);
		boolean dynamic = false;
		if( args.length > 1)
		{
			if(args[1].equals("dynamic") )
				dynamic = true;
			else if(args[1].equals("name"))
			{
				genSkillName();
				return;
			}
			else if(args[1].equals("export"))
			{
				exportCsv();
				return;
			}
		}
		try
		{
			genSkills( dynamic );
		}catch(Exception e)
		{
			e.printStackTrace();
		}
		if(retcode!=0)
			System.exit(retcode);
	}
}

