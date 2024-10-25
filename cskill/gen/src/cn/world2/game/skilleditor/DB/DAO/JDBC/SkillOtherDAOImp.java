/*
 * Created on 2005-1-25
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
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import cn.world2.game.skilleditor.DB.DAO.ISkillOtherDAO;
import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.SkillOtherVO;
import cn.world2.game.skilleditor.vo.data.AttackWay;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class SkillOtherDAOImp implements ISkillOtherDAO {

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillOtherDAO#getSkillOtherVOById(java.lang.String)
	 */
	public SkillOtherVO getSkillOtherVOById(String skillid) {
		String sqlStr="select attackway,isonland,isinsky,isinwater,isattackskill,cannotusetogether,mainskillproperty,isautosearchDest,autosearchDestExp,searchcenter,continuoushit,intonateactionname,exeactionname,exeactionnameselect,effectfilename,hitinjureactionname,ridable,exeactionweaponselect,effectfiletype,bianshenhou,futihou,fly_effect_proportion,fly_effect_parameter,fly_effect_behave,fly_effect_time,hit_effect_proportion,fenmingtime,displayorder from skill where id='"+skillid+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sqlStr);
			SkillOtherVO sov=null;
			while(rs.next()){
				sov = new SkillOtherVO();
				
				InputStream in2 = rs.getBinaryStream("attackway");
				try {
					ObjectInputStream ois2=new ObjectInputStream(in2);
					Object obj2=ois2.readObject();
					sov.setAttackway((AttackWay)obj2);
				} catch (IOException e2) {
					e2.printStackTrace();
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
				} catch (NullPointerException ne){
					
				}
				
				String isonland = rs.getString("isonland");
				String isinsky = rs.getString("isinsky");
				String isinwater = rs.getString("isinwater");
				String isattackskill = rs.getString("isattackskill");
				String cannotusetogether = rs.getString("cannotusetogether");
				String mainskillproperty = rs.getString("mainskillproperty");
				String isautosearchDest = rs.getString("isautosearchDest");
				String autosearchDestExp = rs.getString("autosearchDestExp");
				String searchcenter = rs.getString("searchcenter");
				String continuoushit = rs.getString("continuoushit");
				String intonateactionname = rs.getString("intonateactionname");
				String exeactionname = rs.getString("exeactionname");
				String exeactionnameselect = rs.getString("exeactionnameselect");
				String effectfilename = rs.getString("effectfilename");
				String hitinjureactionname = rs.getString("hitinjureactionname");
				
				String ridable = rs.getString("ridable");
				String exeactionweaponselect = rs.getString("exeactionweaponselect");
				String effectfiletype = rs.getString("effectfiletype");
				
				String bianshenhou = rs.getString("bianshenhou");
				String futihou = rs.getString("futihou");
				
				String fly_effect_proportion=rs.getString("fly_effect_proportion");
				String fly_effect_parameter=rs.getString("fly_effect_parameter");
				String fly_effect_behave=rs.getString("fly_effect_behave");
				String fly_effect_time=rs.getString("fly_effect_time");
				String hit_effect_proportion=rs.getString("hit_effect_proportion");
				String fenmingtime = rs.getString("fenmingtime");
				String displayorder = rs.getString("displayorder");
				
				sov.setIsonland(isonland);
				sov.setIsinsky(isinsky);
				sov.setIsinwater(isinwater);
				sov.setIsAttackSkill(isattackskill);
				sov.setCannotusetogether(cannotusetogether);
				sov.setMainskillproperty(mainskillproperty);
				sov.setIsAutoSearchDestination(isautosearchDest);
				sov.setAutosearchDestExp(autosearchDestExp);
				sov.setSearchcenter(searchcenter);
				sov.setContinuoushit(continuoushit);
				sov.setIntonateActionName(intonateactionname);
				sov.setExeactionname(exeactionname);
				sov.setExeactionnameselect(exeactionnameselect);
				sov.setEffectFileName(effectfilename);
				sov.setHitEnjureActionName(hitinjureactionname);
				sov.setId(skillid);
				
				sov.setRidable(ridable);
				sov.setExeactionweaponselect(exeactionweaponselect);
				sov.setEffectfiletype(effectfiletype);
				
				sov.setBianshenhou(bianshenhou);
				sov.setFutihou(futihou);
				
				sov.setFly_effect_proportion(fly_effect_proportion);
				sov.setFly_effect_parameter(fly_effect_parameter);
				sov.setFly_effect_behave(fly_effect_behave);
				sov.setFly_effect_time(fly_effect_time);
				sov.setHit_effect_proportion(hit_effect_proportion);
				sov.setFenmingtime(fenmingtime);
				sov.setDisplayorder(displayorder);
				
			}
			stmt.close();
			return sov;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillOtherDAO#insertSkillOtherVO(cn.world2.game.skilleditor.vo.SkillOtherVO)
	 */
	public boolean insertSkillOtherVO(SkillOtherVO sov) {
//		String id = sov.getId();
//		String isonland = sov.getIsonland();
//		String isinsky = sov.getIsinsky();
//		String isinwater = sov.getIsinwater();
//		String isattackskill = sov.getIsAttackSkill();
//		String mainskillproperty = sov.getMainskillproperty();
//		String isautosearchDest = sov.getIsAutoSearchDestination();
//		String intonateactionname = sov.getIntonateActionName();
//		String effectfilename = sov.getEffectFileName();
//		String hitinjureactionname = sov.getHitEnjureActionName();
//		
//		String sqlStr="update skill set isonland='"+isonland+"',isinsky='"+isinsky+"',isinwater='"+isinwater+"',isattackskill='"+isattackskill+"',mainskillproperty='"+mainskillproperty+"',isautosearchDest='"+isautosearchDest+"',intonateactionname='"+intonateactionname+"',effectfilename='"+effectfilename+"',hitinjureactionname='"+hitinjureactionname+"' where id='"+id+"'";
//		
//		Connection conn = ConnectionService.getConnection();
//		try {
//			Statement stmt = conn.createStatement();
//			stmt.executeUpdate(sqlStr);
//			stmt.close();
//			conn.close();
//		} catch (SQLException e) {
//			try {
//				conn.close();
//			} catch (SQLException e1) {
//				e1.printStackTrace();
//			}
//			e.printStackTrace();
//		}		
		return false;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.ISkillOtherDAO#updateSkillOtherVO(cn.world2.game.skilleditor.vo.SkillOtherVO)
	 */
	public boolean updateSkillOtherVO(SkillOtherVO sov) {
		ByteArrayOutputStream bos2 = new ByteArrayOutputStream();
		ObjectOutputStream oos2 = null;
		try {
			oos2=new ObjectOutputStream(bos2);
			oos2.writeObject(sov.getAttackway());
		} catch (IOException e3) {
			e3.printStackTrace();
			return false;
		}
		
		InputStream in2=new ByteArrayInputStream(bos2.toByteArray());
		
		String id = sov.getId();
		String isonland = sov.getIsonland();
		String isinsky = sov.getIsinsky();
		String isinwater = sov.getIsinwater();
		String isattackskill = sov.getIsAttackSkill();
		String cannotusetogether = sov.getCannotusetogether();
		String mainskillproperty = sov.getMainskillproperty();
		String isautosearchDest = sov.getIsAutoSearchDestination();
		String autosearchDestExp = sov.getAutosearchDestExp();
		String searchcenter = sov.getSearchcenter();
		String continuoushit = sov.getContinuoushit();
		String intonateactionname = sov.getIntonateActionName();
		String exeactionname = sov.getExeactionname();
		String exeactionnameselect = sov.getExeactionnameselect();
		String effectfilename = sov.getEffectFileName();
		String hitinjureactionname = sov.getHitEnjureActionName();
		
		String ridable = sov.getRidable();
		String exeactionweaponselect = sov.getExeactionweaponselect();
		String effectfiletype = sov.getEffectfiletype();
		
		String bianshenhou = sov.getBianshenhou();
		String futihou = sov.getFutihou();
		String fenmingtime = sov.getFenmingtime();
		
		String fly_effect_proportion = sov.getFly_effect_proportion();
		String fly_effect_parameter = sov.getFly_effect_parameter();
		String fly_effect_behave = sov.getFly_effect_behave();
		String fly_effect_time = sov.getFly_effect_time();
		String hit_effect_proportion=sov.getHit_effect_proportion();
		String displayorder=sov.getDisplayorder();
		
		//String sqlStr="update skill set isonland='"+isonland+"',isinsky='"+isinsky+"',isinwater='"+isinwater+"',isattackskill='"+isattackskill+"',cannotusetogether='"+cannotusetogether+"',mainskillproperty='"+mainskillproperty+"',isautosearchDest='"+isautosearchDest+"',autosearchDestExp='"+autosearchDestExp+"',searchcenter='"+searchcenter+"',intonateactionname='"+intonateactionname+"',exeactionname='"+exeactionname+"',exeactionnameselect='"+exeactionnameselect+"',effectfilename='"+effectfilename+"',hitinjureactionname='"+hitinjureactionname+"',ridable='"+ridable+"',exeactionweaponselect='"+exeactionweaponselect+"',effectfiletype='"+effectfiletype+"' where id='"+id+"'";
		String sqlStr="update skill set isonland=?,isinsky=?,isinwater=?,isattackskill=?,cannotusetogether=?,mainskillproperty=?,isautosearchDest=?,autosearchDestExp=?,searchcenter=?,continuoushit=?,intonateactionname=?,exeactionname=?,exeactionnameselect=?,effectfilename=?,hitinjureactionname=?,ridable=?,exeactionweaponselect=?,effectfiletype=?,bianshenhou=?,futihou=?,fly_effect_proportion=?,fly_effect_parameter=?,fly_effect_behave=?,fly_effect_time=?,hit_effect_proportion=?,fenmingtime=?,displayorder=? where id='"+id+"'";
		Connection conn = ConnectionService.getConnection();
		try {
			//Statement stmt = conn.createStatement();
			PreparedStatement ps = conn.prepareStatement(sqlStr);
			ps.setString(1,isonland);
			ps.setString(2,isinsky);
			ps.setString(3,isinwater);			
			ps.setString(4,isattackskill);			
			ps.setString(5,cannotusetogether);			
			ps.setString(6,mainskillproperty);			
			ps.setString(7,isautosearchDest);			
			ps.setString(8,autosearchDestExp);			
			ps.setString(9,searchcenter);
			ps.setString(10,continuoushit);
			ps.setString(11,intonateactionname);			
			ps.setString(12,exeactionname);			
			ps.setString(13,exeactionnameselect);			
			ps.setString(14,effectfilename);			
			ps.setString(15,hitinjureactionname);			
			ps.setString(16,ridable);			
			ps.setString(17,exeactionweaponselect);			
			ps.setString(18,effectfiletype);			
//			ps.setBinaryStream(19,in2,in2.available());			
//			ps.setString(20,bianshenhou);
//			ps.setString(21,futihou);
			ps.setString(19,bianshenhou);
			ps.setString(20,futihou);
			ps.setString(21,fly_effect_proportion);
			ps.setString(22,fly_effect_parameter);
			ps.setString(23,fly_effect_behave);
			ps.setString(24,fly_effect_time);
			ps.setString(25,hit_effect_proportion);
			ps.setString(26,fenmingtime);
			ps.setString(27,displayorder);
			
			ps.executeUpdate();
			ps.close();
//			stmt.executeUpdate(sqlStr);
//			stmt.close();
			return true;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return false;
	}
}
