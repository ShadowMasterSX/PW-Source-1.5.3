/*
 * Created on 2005-2-26
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
import java.util.LinkedList;

import cn.world2.game.skilleditor.DB.DAO.IHitStatusDAO;
import cn.world2.game.skilleditor.service.ConnectionService;
import cn.world2.game.skilleditor.vo.HitStatus;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class HitStatusDAOImp implements IHitStatusDAO {

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IHitStatusDAO#retrieveAllHitStatus()
	 */
	public LinkedList<HitStatus> retrieveAllHitStatus() {
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement("select file from dictionary where id='2'");
			ResultSet rs= ps.executeQuery();
			Object obj=null;
			while(rs.next()){
				InputStream in = rs.getBinaryStream("file");
				System.out.println(in.available());
				ObjectInputStream ois = new ObjectInputStream(in);
				obj = ois.readObject();
			}
			ps.close();
			return (LinkedList<HitStatus>)obj;
			
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		} catch (NullPointerException e){
		} finally {
			ConnectionService.closeConnection(conn);
		}
		return new LinkedList<HitStatus>();		
	}

	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IHitStatusDAO#saveAllHitStatus(java.util.List)
	 */
	public boolean saveAllHitStatus(LinkedList<HitStatus> list) {
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
		String sqlUpdate="update dictionary set file=? where id='2'";
		Connection conn = ConnectionService.getConnection();
		try {
			PreparedStatement ps = conn.prepareStatement(sqlUpdate);
			System.out.println(in1.available());
			
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
