/*
 * Created on 2005-1-8
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package cn.world2.game.skilleditor.DB.DAO.FILE;

import java.util.List;

import cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO;
import cn.world2.game.skilleditor.vo.Dictionary;

/**
 * @author linxiaojun
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class DictionaryDAOFileImp implements IDictionaryDAO{
	private String path;
	private IDictionaryDAO adaptee;
	public DictionaryDAOFileImp(){	
	}
	public DictionaryDAOFileImp(IDictionaryDAO adaptee,String path){
		this.adaptee=adaptee;
		this.path=path;
	}
	public DictionaryDAOFileImp(String path){
		this.path=path;
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
		return this.adaptee.getAllDictionary();
//		File f = new File(this.path+"/Dictionary.out");
//		if(!f.exists()||f.length()==0) return null;
//		try {
//			FileInputStream fis = new FileInputStream(f);
//			ObjectInputStream ois = new ObjectInputStream(fis);
//			Object obj = ois.readObject();
//			fis.close();
//			ois.close();
//			return (List)obj;
//		} catch (FileNotFoundException e) {
//			e.printStackTrace();
//		} catch (IOException e) {
//			e.printStackTrace();
//		} catch (ClassNotFoundException e) {
//			e.printStackTrace();
//		}
//		return null;
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
		return this.adaptee.updateDictionary(list);
//		File f = new File(this.path+"/Dictionary.out");
//		if(!f.exists()){
//			try {
//				f.createNewFile();
//			} catch (IOException e1) {
//				e1.printStackTrace();
//			}
//		}
//		try {
//			FileOutputStream fos = new FileOutputStream(f);
//			ObjectOutputStream oos = new ObjectOutputStream(fos);
//			oos.writeObject(list);
//			fos.close();
//			oos.close();
//			return true;
//		} catch (FileNotFoundException e) {
//			e.printStackTrace();
//		} catch (IOException e) {
//			e.printStackTrace();
//		}
//		return false;
	}
	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#getSkillPrivateDict(java.lang.String)
	 */
	public List getSkillPrivateDict(String skillid) {
		if(this.adaptee!=null) return adaptee.getSkillPrivateDict(skillid);
		else return null;
	}
	/* (non-Javadoc)
	 * @see cn.world2.game.skilleditor.DB.DAO.IDictionaryDAO#updateSkillPrivateDict(java.util.List, java.lang.String)
	 */
	public boolean updateSkillPrivateDict(List list, String skillid) {
		if(this.adaptee!=null) return adaptee.updateSkillPrivateDict(list,skillid);
		else return false;
	}

}
