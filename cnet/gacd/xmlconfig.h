#ifndef __GNET_XMLCONFIG_H
#define __GNET_XMLCONFIG_H

#include <libxml/xmlreader.h>
#include <string>
#include <map>
#include <vector>

namespace GNET
{

class XmlConfig
{
public:
	typedef std::map<std::string,std::string> Attrs;
	struct Node;
	typedef std::vector<Node*> Nodes;
	struct Node
	{
		friend class XmlConfig;
		std::string m_name;
		std::string m_content;
		Attrs m_attrs;
		Nodes m_children;
		bool HasName(std::string name) const { return m_name == name; }
		const Nodes GetChildren() const { return m_children; }
		const Nodes GetChildren(std::string name) const;
		const Node* GetFirstChild() const;
		const Node* GetFirstChild(std::string name) const;
		const Attrs GetAttrs() const { return m_attrs; }
		const std::string GetAttr(std::string name) const { 
			Attrs::const_iterator it = m_attrs.find(name);
			if( it != m_attrs.end() )
				return (*it).second;
			return std::string();
		}
		const bool GetStrAttr(std::string name, std::string &str) const {
			std::string tmpstr = GetAttr(name);
			if( !tmpstr.empty() )
			{
				str = tmpstr;
				return true;
			}
			return false;
		}
		const bool GetIntAttr(std::string name, void* pInt ) const {
			std::string tmpstr = GetAttr(name);
			if( !tmpstr.empty() )
			{
				*static_cast<int*>(pInt) = atoi(tmpstr.c_str());
				return true;
			}
			return false;
		}
		const bool GetBoolAttr(std::string name, void* pBool ) const {
			std::string tmpstr = GetAttr(name);
			if( !tmpstr.empty() )
			{
				*static_cast<bool*>(pBool) = ( atoi(tmpstr.c_str()) != 0 );
				return true;
			}
			return false;
		}
		const bool GetUIntAttr(std::string name, void *pUInt ) const {
			std::string tmpstr = GetAttr(name);
			if( !tmpstr.empty() )
			{
				*static_cast<unsigned int*>(pUInt) = atoi(tmpstr.c_str());
				return true;
			}
			return false;
		}
		void Dump();
	private:
		void Destroy();
	};
private:
	Node* m_pRoot;
	std::string m_fileName;

	Node* Build(xmlDoc *pDoc, xmlNode* pXmlNode);

	void Load(std::string fileName);
	void Destroy() { if( m_pRoot ) { m_pRoot->Destroy(); } m_pRoot = NULL; }
public:
	XmlConfig(std::string fileName) : m_pRoot(NULL) { Reload(fileName); }
	XmlConfig() : m_pRoot(NULL) { }

	void Reload(std::string fileName);
	void Reload();
	void Reload(const char *szBuffer, size_t uSize);
	void Load(const char *szBuffer, size_t uSize);
	const Node* Root() const { return m_pRoot; }
	bool IsEmpty() const { return m_pRoot == NULL; }
	void Dump() const { if( m_pRoot != NULL ) m_pRoot->Dump(); }
	~XmlConfig() { Destroy(); }
};

}
#endif
