/*
 * Created on 2004-12-21
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO.JDBC;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import cn.world2.game.skilleditor.DB.DAO.ISkillDAO;
import cn.world2.game.skilleditor.DB.DAO.ISkillOtherDAO;
import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.HitStatusParam;
import cn.world2.game.skilleditor.vo.Role;
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
public class SkillDAOImp implements ISkillDAO {
	private boolean updateSkillweaponlimitation(Skill skill,String id){
		ByteArrayOutputStream bos1 = new ByteArrayOutputStream();
		ObjectOutputStream oos1=null;
		try {
			oos1 = new ObjectOutputStream(bos1);
			oos1.writeObject(skill.getArmlimitation());
		} catch (IOException e1) {
			e1.printStackTrace();
			return false;
		}		
		InputStream in1 = new ByteArrayInputStream(bos1.toByteArray());
		String sqlUpdate="update skill set weaponlimitation=? where id='"+id+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement(sqlUpdate);
			ps.setBinaryStream(1,in1,in1.available());
			ps.executeUpdate();
			ps.close();
			return true;
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return false;
	}
	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#insertSkill(cn.world2.game.skilleditor.vo.Skill)
	 */
	public String insertSkill(Skill skill) {
//		ByteArrayOutputStream bos1 = new ByteArrayOutputStream();
//		ObjectOutputStream oos1=null;
//		try {
//			oos1 = new ObjectOutputStream(bos1);
//			oos1.writeObject(skill.getArmlimitation());
//		} catch (IOException e1) {
//			e1.printStackTrace();
//			return false;
//		}		
//		InputStream in1 = new ByteArrayInputStream(bos1.toByteArray());
//		
		String sqlStr = "insert into skill (roleid,name) values('"+skill.getRole().getId()+"','"+skill.getName()+"')";
		String sqlSelect = "select id from skill where roleid='"+skill.getRole().getId()+"' and name='"+skill.getName()+"'";
		String id=null;
		//System.out.println(sqlStr);
		Connection conn = ConnectionService.getConnection();
		PreparedStatement ps;
		try {
			ps = conn.prepareStatement(sqlStr);
			//ps.setString(1,skill.getRole().getId());
			//ps.setString(2,skill.getName());
			//ps.setBinaryStream(3,in1,in1.available());
			ps.executeUpdate(sqlStr);
			ResultSet rs = ps.executeQuery(sqlSelect);
			while(rs.next()){
				id=rs.getString("id");
			}
			ps.close();
			//this.updateSkillweaponlimitation(skill,id);
			return id;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#deleteSkill(cn.world2.game.skilleditor.vo.Skill)
	 */
	public int deleteSkill(Skill skill) {
		// TODO Auto-generated method stub
		return 0;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#updateSkill(cn.world2.game.skilleditor.vo.Skill)
	 */
	public boolean updateSkill(Skill skill) {
		// TODO Auto-generated method stub
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#getSkill(java.lang.String)
	 */
	public List<Skill> getSkill(String condtion) {
		String sqlStr = "select * from skill " + condtion + "order by id";
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			List<Skill> list = new ArrayList<Skill>();
			while (rs.next()) {
				Skill s = new Skill();
				
				s.setId(rs.getString("id"));
				
				Role role = new Role();
				role.setId(rs.getString("roleid"));
				s.setRole(role);
			
				s.setCode(rs.getString("code"));
				s.setName(rs.getString("name"));
				s.setReleaseeffect(rs.getString("releaseeffect"));
				s.setContinueeffect(rs.getString("continueeffect"));
				s.setUpgradeeffect(rs.getString("upgradeeffect"));
				list.add(s);
			}
			return list;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#deleteSkill(java.util.List)
	 */
	public int deleteSkill(List idlist) {
		if (idlist == null || idlist.size() == 0)
			return 0;
		Connection conn = ConnectionService.getConnection();
		try {
			conn.setAutoCommit(false);
			conn.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);
			Statement stmt = conn.createStatement();
			for (int k = 0; k < idlist.size(); k++) {
				String id = (String) idlist.get(k);
				String sqlStr = "delete from skill where id='" + id + "'";
				stmt.addBatch(sqlStr);
			}
			int[] i = stmt.executeBatch();
			conn.commit();
			return i.length;
		} catch (SQLException e) {
			try {
				conn.rollback();
			} catch (SQLException e1) {
				e1.printStackTrace();
			}
			return -1;
		} finally {
			ConnectionService.closeConnection(conn);
		}
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#getSkillsShort(java.lang.String)
	 */
	public List getSkillsShort(String condition) {
		String sqlStr = "select id,roleid,code,name from skill " + condition + " order by id";
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			List<Skill> list = new ArrayList<Skill>();
			while (rs.next()) {
				Skill s = new Skill();
				
				s.setId(rs.getString("id"));
				
				Role role = new Role();
				role.setId(rs.getString("roleid"));
				s.setRole(role);
			
				s.setCode(rs.getString("code"));
				s.setName(rs.getString("name"));
				list.add(s);
			}
			return list;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#updateSkillLevel(cn.world2.game.skilleditor.vo.data.SkillLevelVO)
	 */
	public boolean updateSkillLevel(SkillLevelVO slvo,String skillid) {
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		ObjectOutputStream oos=null;
		try {
			oos = new ObjectOutputStream(bos);
			oos.writeObject(slvo);		
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}		
		ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());

		Connection conn = ConnectionService.getConnection();
		PreparedStatement ps=null;
		try {
			ps = conn.prepareStatement("update skill set levelinfo=? where id='"+skillid+"'");
			ps.setBinaryStream(1,bis,bis.available());
			ps.executeUpdate();
			ps.close();
			return true;
		} catch (SQLException e1) {			
			e1.printStackTrace();
			return false;
		} finally {
			ConnectionService.closeConnection(conn);
		}
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#getSkillLevel(java.lang.String)
	 */
	public SkillLevelVO getSkillLevel(String skillid) {
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement("select levelinfo from skill where id='"+skillid+"'");
			ResultSet rs= ps.executeQuery();
			while(rs.next()){
				InputStream in = rs.getBinaryStream("levelinfo");
				ObjectInputStream ois = new ObjectInputStream(in);
				Object obj = ois.readObject();
				ps.close();
				return (SkillLevelVO)obj;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (IOException e) {
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		} catch (NullPointerException e){
			
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#updateSkillBasic(cn.world2.game.skilleditor.vo.Skill)
	 */
	public boolean updateSkillBasic(Skill skill) {
		ByteArrayOutputStream bos1 = new ByteArrayOutputStream();
		ObjectOutputStream oos1=null;
		try {
			oos1 = new ObjectOutputStream(bos1);
			oos1.writeObject(skill.getArmlimitation());
		} catch (IOException e1) {
			e1.printStackTrace();
			return false;
		}		
		InputStream in1 = new ByteArrayInputStream(bos1.toByteArray());
		
		ByteArrayOutputStream bos2 = new ByteArrayOutputStream();
		ObjectOutputStream oos2 = null;
		try {
			oos2=new ObjectOutputStream(bos2);
			oos2.writeObject(skill.getAttackway());
		} catch (IOException e3) {
			e3.printStackTrace();
			return false;
		}
		
		InputStream in2=new ByteArrayInputStream(bos2.toByteArray());
		
		String skillid=skill.getId();
		Connection conn = ConnectionService.getConnection();
		String sqlStr="update skill set code=?, name=?, continueeffect=?, upgradeeffect=?, weaponlimitation=?, preconditionskill=?, passiveStatus=?, needgoal=?, maxlevel=?,skilldesc=?,reviselevel=?,levelonebook=?,initskilluse=?,changeweaponskilluse=?,inherent=?,movingcast=?,icon=? where id='"+skill.getId()+"'";
		String sqlWithoutIcon = "update skill set code=?, name=?, continueeffect=?, upgradeeffect=?, weaponlimitation=?, preconditionskill=?, passiveStatus=?, needgoal=?, maxlevel=?, skilldesc=?,reviselevel=?,levelonebook=?,initskilluse=?,changeweaponskilluse=?,inherent=?,movingcast=? where id='"+skill.getId()+"'";
		
		try {
			PreparedStatement ps;
			if(skill.getIcon()==null){
				ps = conn.prepareStatement(sqlWithoutIcon);
			}else{
				ps = conn.prepareStatement(sqlStr);
				ps.setString(17,skill.getIcon());
			}
			ps.setString(1,skill.getCode());
			ps.setString(2,skill.getName());
			ps.setString(3,skill.getContinueeffect());
			ps.setString(4,skill.getUpgradeeffect());
			try {
				ps.setBinaryStream(5,in1,in1.available());
			} catch (IOException e2) {
				ps.close();
				e2.printStackTrace();
				return false;
			}
			ps.setString(6,skill.getPreconditonSkill());
			ps.setString(7,skill.getPassiveStatus());
			ps.setString(8,skill.getNeedgoal());
			ps.setString(9,skill.getMaxLevel());
			ps.setString(10,skill.getSkilldesc());
			ps.setString(11,skill.getReviselevel());
			ps.setString(12,skill.getLevelonebook());
			
			ps.setString(13,skill.getInitskilluse());
			ps.setString(14,skill.getChangeweaponskilluse());
			ps.setString(15,skill.getInherent());
			ps.setString(16,skill.getMovingcast());
			
			ps.executeUpdate();
			ps.close();
			return true;
		} catch (SQLException e) {
			e.printStackTrace();
			return false;
		} finally {
			ConnectionService.closeConnection(conn);
		}
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#getSkillBasic(java.lang.String)
	 */
	public Skill getSkillBasic(String skillid) {
		String sqlStr="select code, name, icon, releaseeffect, continueeffect, upgradeeffect, weaponlimitation, preconditionskill, passiveStatus, attackway, needgoal, cooltime,executetime,maxlevel,skilldesc,reviselevel,levelonebook,distance,usecondition,attack,initskilluse,changeweaponskilluse,hatedegree,fenmingtime,displayorder,usehpcondition,combosk_preskill,combosk_interval,combosk_extracond,combosk_endaction,inherent,combosk_nobreak,movingcast from skill where id='"+skillid+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement(sqlStr);
			ResultSet rs = ps.executeQuery();
			Skill skill=null;
			while(rs.next()){
				skill = new Skill();
				skill.setCode(rs.getString("code"));
				skill.setName(rs.getString("name"));
				skill.setIcon(rs.getString("icon"));
				skill.setReleaseeffect(rs.getString("releaseeffect"));
				skill.setContinueeffect(rs.getString("continueeffect"));
				skill.setUpgradeeffect(rs.getString("upgradeeffect"));
				
				InputStream in1 = rs.getBinaryStream("weaponlimitation");
				try {
					ObjectInputStream ois1=new ObjectInputStream(in1);
					Object obj = ois1.readObject();
					skill.setArmlimitation((String[])obj);
				} catch (IOException e1) {
					e1.printStackTrace();
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
				} catch(NullPointerException ne){

				}
				
				skill.setPreconditonSkill(rs.getString("preconditionskill"));
				skill.setPassiveStatus(rs.getString("passiveStatus"));
				
				InputStream in2 = rs.getBinaryStream("attackway");
				try {
					ObjectInputStream ois2=new ObjectInputStream(in2);
					Object obj2=ois2.readObject();
					skill.setAttackway((AttackWay)obj2);
				} catch (IOException e2) {
					e2.printStackTrace();
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
				} catch (NullPointerException ne){
					
				}
				
				skill.setNeedgoal(rs.getString("needgoal"));
				skill.setCooltime(rs.getString("cooltime"));
				skill.setExecutetime(rs.getString("executetime"));
				skill.setMaxLevel(rs.getString("maxlevel"));
				skill.setSkilldesc(rs.getString("skilldesc"));
				skill.setReviselevel(rs.getString("reviselevel"));
				skill.setLevelonebook(rs.getString("levelonebook"));				
				
				skill.setDistance(rs.getString("distance"));
				skill.setCondition(rs.getString("usecondition"));
				skill.setAttack(rs.getString("attack"));
				
				skill.setInitskilluse(rs.getString("initskilluse"));
				skill.setChangeweaponskilluse(rs.getString("changeweaponskilluse"));
				skill.setHatedegree(rs.getString("hatedegree"));
				skill.setFenmingtime(rs.getString("fenmingtime"));
				skill.setDisplayorder(rs.getString("displayorder"));
				skill.setUsehpcondition(rs.getString("usehpcondition"));
				skill.setCombosk_preskill(rs.getString("combosk_preskill"));
				skill.setCombosk_interval(rs.getString("combosk_interval"));
				skill.setCombosk_extracond(rs.getString("combosk_extracond"));
				skill.setCombosk_endaction(rs.getString("combosk_endaction"));
				skill.setInherent(rs.getString("inherent"));
				skill.setCombosk_nobreak(rs.getString("combosk_nobreak"));
				skill.setMovingcast(rs.getString("movingcast"));
				
				ps.close();
				return skill;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#updateSkillProcedure(cn.world2.game.skilleditor.vo.data.ProcedureVO)
	 */
	public boolean updateSkillProcedure(List list,String skillid) {
		String sqlStr="update skill set skillprocedure=? where id='"+skillid+"'";
		System.out.println(sqlStr);
		InputStream in = null;
		try {
			ByteArrayOutputStream bos = new ByteArrayOutputStream();
			ObjectOutputStream oos = new ObjectOutputStream(bos);
			oos.writeObject(list);
			in = new ByteArrayInputStream(bos.toByteArray());
		} catch (IOException e1) {
			e1.printStackTrace();
		}
		
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement(sqlStr);
			ps.setBinaryStream(1,in,in.available());
			ps.executeUpdate();
			ps.close();
			return true;
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return false;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#getSkillProcedure(java.lang.String)
	 */
	public List getSkillProcedure(String skillid) {
		String sqlStr="select skillprocedure from skill where id='"+skillid+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			while(rs.next()){
				InputStream in = rs.getBinaryStream("skillprocedure");
				ObjectInputStream ois = new ObjectInputStream(in);
				Object obj = ois.readObject();
				return (List)obj;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		} catch (NullPointerException ne){
			
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#updateSkillByQuery(java.lang.String)
	 */
	public boolean updateSkillByQuery(String query) {
		Connection conn = ConnectionService.getConnection();
		try {
//			Statement stmt = conn.createStatement();
//			stmt.executeUpdate(query);
			PreparedStatement ps = conn.prepareStatement(query);
			ps.executeUpdate();
			ps.close();
//			stmt.close();
			return true;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return false;
	}
	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#copySkillByQuery(java.lang.String)
	 */
	public String copySkillByQuery(String query,String oldrole) {
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement(query);
			ps.executeUpdate();
			String LAST_INSERT_ID=null;
			ResultSet rs = ps.executeQuery("select LAST_INSERT_ID() from skill");
			while(rs.next()){
				LAST_INSERT_ID = rs.getString("LAST_INSERT_ID()");
			}
			ps.executeUpdate("update skill set roleid='"+oldrole+"' where id='"+LAST_INSERT_ID+"'");
			ps.close();
			return LAST_INSERT_ID;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}
	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#updateBlob(java.io.Serializable, java.lang.String)
	 */
	public void updateBlob(Serializable s, String field,String skillid) {
		ByteArrayOutputStream bos1 = new ByteArrayOutputStream();
		ObjectOutputStream oos1=null;
		try {
			oos1 = new ObjectOutputStream(bos1);
			oos1.writeObject(s);
		} catch (IOException e1) {
			e1.printStackTrace();
			return;
		}		
		InputStream in1 = new ByteArrayInputStream(bos1.toByteArray());
		String sqlUpdate="update skill set "+field+"=? where id='"+skillid+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement(sqlUpdate);
			ps.setBinaryStream(1,in1,in1.available());
			ps.executeUpdate();
			ps.close();
			return;
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return;		
	}
	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#getBlob(java.lang.String, java.lang.String)
	 */
	public Serializable getBlob(String field, String skillid) {
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement("select "+field+" from skill where id='"+skillid+"'");
			ResultSet rs= ps.executeQuery();
			while(rs.next()){
				InputStream in = rs.getBinaryStream(field);
				ObjectInputStream ois = new ObjectInputStream(in);
				Object obj = ois.readObject();
				ps.close();
				return (Serializable)obj;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (IOException e) {
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		} catch (NullPointerException e){
			
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}
	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillDAO#getSkillById(java.lang.String)
	 */
	public Skill getSkillById(String skillid) {
		Skill skill = this.getSkillBasic(skillid);
		skill.setId(skillid);
		SkillLevelVO levelinfo = this.getSkillLevel(skill.getId());		
		skill.setLevelinfo( levelinfo );
		
		List procedures = this.getSkillProcedure(skill.getId());
        skill.setProcedures( procedures );
        
		ISkillOtherDAO sodao = new SkillOtherDAOImp();
		SkillOtherVO otherinfo = sodao.getSkillOtherVOById(skillid);
        skill.setOtherinfo( otherinfo );	
        
        ArrayList<HitStatusParam> hsplist=null;
        try{
            hsplist = (ArrayList<HitStatusParam>)this.getBlob("hitstatus",skillid);
        }catch(Exception e){ }
        skill.setHitstatus( hsplist );
        
		return skill;
	}
}
