/*
 * Created on 2005-1-18
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
import java.util.ArrayList;
import java.util.List;

import cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO;
import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.Dictionary;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class DictionaryDAOImp implements IDictionaryDAO {
	private IDictionaryDAO adaptee;
	public DictionaryDAOImp(){
		
	}
	public DictionaryDAOImp(IDictionaryDAO adaptee){
		this.adaptee=adaptee;
	}
	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#getDitionary(java.lang.String)
	 */
	public Dictionary getDitionary(String key) {
		// TODO Auto-generated method stub
		return null;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#getAllDictionary()
	 */
	public List getAllDictionary() {
//		if(adaptee!=null) return adaptee.getAllDictionary();
//		else return null;
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement("select file from dictionary where id='0'");
			ResultSet rs= ps.executeQuery();
			while(rs.next()){
				InputStream in = rs.getBinaryStream("file");
				ObjectInputStream ois = new ObjectInputStream(in);
				Object obj = ois.readObject();
				ps.close();
				return (List)obj;
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
		return new ArrayList();		
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#insertDictionary(cn.world2.game.skilleditor.vo.Dictionary)
	 */
	public boolean insertDictionary(Dictionary d) {
		// TODO Auto-generated method stub
		return false;
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#updateDictionary(java.util.List)
	 */
	public boolean updateDictionary(List list) {
//		if(adaptee!=null) return adaptee.updateDictionary(list);
//		else return false;
		ByteArrayOutputStream bos1 = new ByteArrayOutputStream();
		ObjectOutputStream oos1=null;
		try {
			oos1 = new ObjectOutputStream(bos1);
			oos1.writeObject(list);
		} catch (IOException e1) {
			e1.printStackTrace();
			return false;
		}		
		InputStream in1 = new ByteArrayInputStream(bos1.toByteArray());
		String sqlUpdate="update dictionary set file=? where id='0'";
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

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#getSkillPrivateDict(java.lang.String)
	 */
	public List getSkillPrivateDict(String skillid) {
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement("select guide_dict from skill where id='"+skillid+"'");
			ResultSet rs= ps.executeQuery();
			while(rs.next()){
				InputStream in = rs.getBinaryStream("guide_dict");
				ObjectInputStream ois = new ObjectInputStream(in);
				Object obj = ois.readObject();
				ps.close();
				return (List)obj;
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
		return new ArrayList();
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#updateSkillPrivateDict(java.util.List, java.lang.String)
	 */
	public boolean updateSkillPrivateDict(List list, String skillid) {
		ByteArrayOutputStream bos1 = new ByteArrayOutputStream();
		ObjectOutputStream oos1=null;
		try {
			oos1 = new ObjectOutputStream(bos1);
			oos1.writeObject(list);
		} catch (IOException e1) {
			e1.printStackTrace();
			return false;
		}		
		InputStream in1 = new ByteArrayInputStream(bos1.toByteArray());
		String sqlUpdate="update skill set guide_dict=? where id='"+skillid+"'";
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

}
