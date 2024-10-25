#ifndef _GNET_OBJECTGRAPH_H
#define _GNET_OBJECTGRAPH_H
#include <string>
#include <map>
#include <algorithm>
#include <vector>

#define INVALID_IDENTIFIER	0
#define MAX_RESERVED_TYPE	30
//define reserved types
#define BOOL_T		1
#define CHAR_T		2
#define INT_T		3
#define FLOAT_T		4
#define DOUBLE_T	5
namespace GNET
{
class ObjectGraph
{
	struct stringcompare
	{
		bool operator()(const std::string& _s1,const std::string& _s2)
		{ 
			return _s1.compare(_s2.c_str())<0;
		}
	};

	typedef std::map<std::string,unsigned int,stringcompare> String2IntMap;
	String2IntMap str2int;

	typedef std::map<unsigned int,std::string> Int2StringMap;
	Int2StringMap int2str;

	static String2IntMap reserved_types;
	static bool init_reserved;

	struct OGNode
	{
		unsigned int type;
		unsigned int name;
		OGNode() : type(INVALID_IDENTIFIER),name(INVALID_IDENTIFIER) {};
		OGNode(unsigned int _t,unsigned int _n) : type(_t),name(_n) { }
	};
	struct TypeNode
	{
		unsigned int type;
		std::vector<OGNode> member_list;
		TypeNode() : type(INVALID_IDENTIFIER) { }
		TypeNode(unsigned int _type) : type(_type) { }
		bool operator==(const TypeNode& rhs) { return type==rhs.type; }
	};
	typedef std::vector<TypeNode> TypeList;
	TypeList typelist;

	unsigned int Identifier_ID; //IDs of identifiers
	TypeNode current_typenode;
	
private:
	bool IsIdentifierExist(const std::string& identifier)
	{
		return str2int.find(identifier) != str2int.end();
	}
	unsigned int AddNewIdentifier(const std::string& identifier)
	{
		if (IsIdentifierExist(identifier)) return INVALID_IDENTIFIER;
		String2IntMap::iterator it=reserved_types.find(identifier);
		if (it==reserved_types.end())
		{
			str2int[identifier]=Identifier_ID;
			int2str[Identifier_ID]=identifier;
			return Identifier_ID++;
		}
		else
		{
			str2int[identifier]=(*it).second;
			int2str[(*it).second]=identifier;
			return (*it).second;
		}
	}
public:
	ObjectGraph() : Identifier_ID(MAX_RESERVED_TYPE+1) 
	{
		init_reserved=false;
		if (! init_reserved)
		{
			reserved_types["bool"]=BOOL_T;
			reserved_types["char"]=CHAR_T;
			reserved_types["int"]=INT_T;
			reserved_types["float"]=FLOAT_T;
			reserved_types["double"]=DOUBLE_T;
			init_reserved = true;
		}
	}
	~ObjectGraph() { typelist.clear(); str2int.clear(); int2str.clear(); }
	void Reset()
	{
		str2int.clear();
		int2str.clear();
		typelist.clear();
	}
	static bool IsReservedType(unsigned int type_id)
	{
		return type_id<=MAX_RESERVED_TYPE;
	}
	unsigned int FindMemberNode(unsigned int typenode_id,unsigned int member_name_id)
	{
		if (typenode_id == INVALID_IDENTIFIER || member_name_id==INVALID_IDENTIFIER) return INVALID_IDENTIFIER;
		TypeList::iterator it=std::find(typelist.begin(),typelist.end(),TypeNode(typenode_id));
		if (it == typelist.end()) return INVALID_IDENTIFIER;
		
		for (size_t i=0;i<(*it).member_list.size();i++)
			if ((*it).member_list[i].name==member_name_id) return (*it).member_list[i].type;
		return INVALID_IDENTIFIER;
	}

	unsigned int GetIdentifier(const std::string& identifier)	
	{
		String2IntMap::iterator it=str2int.find(identifier);
		if (it == str2int.end()) return INVALID_IDENTIFIER;
		return (*it).second;
	}
	unsigned int GetTypeID(const std::string& type_name)
	{
		unsigned int type_id=GetIdentifier(type_name);
		if (type_id==INVALID_IDENTIFIER) return INVALID_IDENTIFIER;
		TypeList::iterator it=std::find(typelist.begin(),typelist.end(),TypeNode(type_id));
		if (it == typelist.end()) return INVALID_IDENTIFIER;
		return type_id;
	}
	bool AddTypeNode(const std::string& _type) 
	{
		//the identity does not appear before
		if (!IsIdentifierExist(_type))
		{
			current_typenode=TypeNode(AddNewIdentifier(_type));
			typelist.push_back(current_typenode);
			return true;
		}
		else
		{
			unsigned int id=GetIdentifier(_type);
			if (std::find(typelist.begin(),typelist.end(),TypeNode(id))!=typelist.end())
			{
				printf("ERROR: AddTypeNode::identity typenode exist.\n");
			   	return false;
			}
			else
			{
				current_typenode=TypeNode(id);
				typelist.push_back(current_typenode);
				return true;
			}
		}
	}
	bool AddMemberNode(const std::string& _type,const std::string& _name)
	{
		if (current_typenode.type == INVALID_IDENTIFIER) return false;
		unsigned int type_id,name_id;
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
		
		TypeList::iterator it=std::find(typelist.begin(),typelist.end(),current_typenode);
		(*it).member_list.push_back(OGNode(type_id,name_id));
		return true;
	}
	bool FindMember(const std::string& type_name,const std::string& member_name,std::string& member_type)
	{
		unsigned int member_type_id=FindMemberNode(GetIdentifier(type_name),GetIdentifier(member_name));
		if (member_type_id!=INVALID_IDENTIFIER)
		{
			member_type=int2str[member_type_id];
			return true;
		}
		else
			return false;
	}
	void Display()
	{
		for (size_t i=0;i<typelist.size();i++)
		{
			printf("type:(%d)%s\n",typelist[i].type,int2str[typelist[i].type].c_str());
			for (size_t j=0;j<typelist[i].member_list.size();j++)
			{
				OGNode& node=typelist[i].member_list[j];
				printf("\ttype=(%d)%s\t\tname=(%d)%s\n",node.type,int2str[node.type].c_str(),node.name,int2str[node.name].c_str());
			}
		}
	}
};	
};
#endif
