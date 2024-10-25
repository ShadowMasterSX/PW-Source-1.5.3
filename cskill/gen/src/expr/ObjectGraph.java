package expr;
import java.lang.*;
import java.util.*;
class OGNode
{
	public int type;
	public int name;
	public OGNode()
	{
		type=Constants.INVALID_IDENTIFIER;
		name=Constants.INVALID_IDENTIFIER;
	}
	public OGNode(int _t,int _n)
	{
		type=_t;
		name=_n;
	}
}
class TypeNode
{
	public int 			type;
	public ArrayList 	member_list;
	public TypeNode()
	{
		type = Constants.INVALID_IDENTIFIER;
		member_list = new ArrayList();
	}
	public TypeNode(int _type)
   	{ 
		type = _type;
		member_list = new ArrayList();
	}
}
public class ObjectGraph
{
	public static final Map reserved_type = new HashMap();
	public static final int MAX_RESERVED_TYPE = 30;
	static {
		reserved_type.put("bool",new Integer(1));
		reserved_type.put("char",new Integer(2));
		reserved_type.put("int",new Integer(3));
		reserved_type.put("long",new Integer(4));
		reserved_type.put("float",new Integer(5));
		reserved_type.put("double",new Integer(6));
	}
	/*
	 *@ Identifier to id map
   	 */	 
	private HashMap str2int;
	/*
	 *@ id to identifier map
   	 */	 
	private TreeMap int2str;
	/*
	 *@ typenode list, the structure of ObjectGraph is stored here 
   	 */	 
	private TreeMap typelist;
	/*
	 *@ record the maxium identifier id
   	 */	 
	private int identifier_id;
	/*
	 *@ record the current typenode, to which a member node is added to
   	 */	 
	static private TypeNode current_typenode;
	/*
	 *@ constructor of ObjectGraph
	 */	 
	public ObjectGraph()
	{
		identifier_id=MAX_RESERVED_TYPE;
		str2int = new HashMap();
		int2str = new TreeMap();
		typelist = new TreeMap();
	}
	/*
	 *@ check whether a identifier is exist
   	 */		 
	private boolean IsIdentifierExist(String identifier)
	{
		return str2int.containsKey(identifier);
	}
	/*
	 *@ Add a new identifier to map(both str2int and int2str)
     */	 
	private int AddNewIdentifier(String identifier)
	{
		if (IsIdentifierExist(identifier))
			return Constants.INVALID_IDENTIFIER;
		str2int.put(identifier,new Integer(identifier_id));
		int2str.put(new Integer(identifier_id),identifier);
		return identifier_id++;	
	}	
	/*
	 *	@Find member id of a typenode
	 */
	public int FindMemberNode(int typenode_id,int member_name_id)
	{
		if (typenode_id == Constants.INVALID_IDENTIFIER || member_name_id==Constants.INVALID_IDENTIFIER)
			return Constants.INVALID_IDENTIFIER;
		if ( typelist.containsKey(new Integer(typenode_id)))
		{
			TypeNode tn=(TypeNode)typelist.get(new Integer(typenode_id));
			//int i;
			for (int i=0;i<tn.member_list.size();i++)
			{
				if (((OGNode)tn.member_list.get(i)).name == member_name_id)
					return ((OGNode)tn.member_list.get(i)).type;
			}
			return Constants.INVALID_IDENTIFIER;
		}
		else
			return Constants.INVALID_IDENTIFIER;
	}
	/*
	 *@ get identifier's id by its name
   	 */	 
	public int GetIdentifier(String identifier)
	{
		if (str2int.containsKey(identifier))
			return ((Integer)str2int.get(identifier)).intValue();
		else
			return Constants.INVALID_IDENTIFIER;
	}
	/*
	 *@ get type's id by its name
   	 */
	public int GetTypeID(String identifier)
	{
		int id=GetIdentifier(identifier);
		if (id==Constants.INVALID_IDENTIFIER) return Constants.INVALID_IDENTIFIER;
		TypeNode t=(TypeNode)typelist.get(new Integer(id));
		if (t==null) return Constants.INVALID_IDENTIFIER;
		return t.type;
	}	
	public boolean AddTypeNode(String _type)
	{
		//the identity does not appear before
		if (!IsIdentifierExist(_type))
		{
			if (reserved_type.containsKey(_type))
			{
				Integer _type_id = (Integer)reserved_type.get(_type);
				str2int.put(_type,_type_id);
				int2str.put(_type_id,_type);		
				current_typenode=new TypeNode(_type_id.intValue());
			}
			else	
				current_typenode=new TypeNode(AddNewIdentifier(_type));
			typelist.put(new Integer(current_typenode.type),current_typenode);
			return true;
		}
		else
		{
			int id=GetIdentifier(_type);
			if (typelist.containsKey(new Integer(id)))
			{
				System.out.println("ERROR: AddTypeNode::identity typenode exist.");
				return false;
			}
			else
			{
				current_typenode=new TypeNode(id);
				typelist.put(new Integer(id),current_typenode);
				return true;
			}
		}
	}
	public boolean AddMemberNode(String _type,String _name)
	{       
		if (current_typenode.type == Constants.INVALID_IDENTIFIER) return false;
		int type_id,name_id;
		if (IsIdentifierExist(_type))
		{
			type_id=GetIdentifier(_type);
		}
		else
		{
			type_id=AddNewIdentifier(_type);
		}

		if (IsIdentifierExist(_name))
		{
			name_id=GetIdentifier(_name);
		}
		else
		{
			name_id=AddNewIdentifier(_name);
		}
		current_typenode.member_list.add(new OGNode(type_id,name_id));
		return true;
	}	
	public void Display()
	{
		Iterator it=typelist.values().iterator();
		TypeNode typenode;
		OGNode ognode;
		while (it.hasNext())
		{
			typenode=(TypeNode)it.next();
			System.out.println("type:("+typenode.type+")"+(String)int2str.get(new Integer(typenode.type)));
			for (int i=0;i<typenode.member_list.size();i++)
			{
				ognode = (OGNode)typenode.member_list.get(i);
				System.out.println("\ttype=("+ognode.type+")"+(String)int2str.get(new Integer(ognode.type))+"\tname=("+ognode.name+")"+(String)int2str.get(new Integer(ognode.name)));
			}
		}				
	}	
}
