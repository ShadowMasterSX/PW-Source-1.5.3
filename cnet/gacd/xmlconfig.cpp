#include "xmlconfig.h"
#include "acconstants.h"

#include <iostream>
#include <algorithm>

namespace GNET
{

void XmlConfig::Reload(std::string fileName)
{
	Destroy();
	Load(fileName);
}

void XmlConfig::Reload()
{
	Reload(m_fileName);
}

void XmlConfig::Reload(const char *szBuffer, size_t uSize)
{
	Destroy();
	Load(szBuffer, uSize);
}

void XmlConfig::Node::Destroy()
{
	//std::for_each(children.begin(), children.end(), std::mem_fun(&XmlConfig::Node::Destroy));
	for(Nodes::iterator it = m_children.begin(), ie = m_children.end(); it != ie ; ++it)
	{
		if( (*it) != NULL )
			(*it)->Destroy();
	}
	delete this;
}

void XmlConfig::Node::Dump()
{
	std::cout<<"node name: "<<m_name<<std::endl;
	std::cout<<"node content: "<<m_content<<std::endl;
	for(Attrs::const_iterator it = m_attrs.begin(), ie = m_attrs.end(); it != ie ; ++it)
	{
		std::cout<<"\t"<<(*it).first<<"\t=\t"<<(*it).second<<std::endl;
	}
	for(Nodes::const_iterator it = m_children.begin(), ie = m_children.end(); it != ie ; ++it)
	{
		(*it)->Dump();
	}
}

const XmlConfig::Nodes XmlConfig::Node::GetChildren(std::string name) const
{
	Nodes nodes;
	for(Nodes::const_iterator it = m_children.begin(), ie = m_children.end(); it != ie ; ++it)
	{
		if( (*it)->HasName(name) )
			nodes.push_back((*it));
	}
	return nodes;
}

const XmlConfig::Node* XmlConfig::Node::GetFirstChild() const
{
	return m_children.empty() ? NULL : *(m_children.begin());
}

const XmlConfig::Node* XmlConfig::Node::GetFirstChild(std::string name) const
{
	Nodes::const_iterator it = std::find_if(m_children.begin(), m_children.end(), 
		std::bind2nd(std::mem_fun(&XmlConfig::Node::HasName), name));
	return it == m_children.end() ? NULL : *it;
}

XmlConfig::Node* XmlConfig::Build(xmlDoc *pDoc, xmlNode* pXmlNode)
{
	Node *pNode = new Node();
	pNode->m_name = std::string((char*)pXmlNode->name);
	xmlChar *pXmlContent = xmlNodeListGetString(pDoc, pXmlNode->xmlChildrenNode, 1);
	if( pXmlContent != NULL )
	{
		pNode->m_content = std::string((char*)pXmlContent);
		xmlFree(pXmlContent);	
	}

	for(xmlAttr *pCurAttr = pXmlNode->properties; pCurAttr != NULL ; pCurAttr = pCurAttr->next)
	{
		const xmlChar *pAttrName = pCurAttr->name;
		xmlChar *pAttrValue = xmlGetProp(pXmlNode, pAttrName);
		if( pAttrValue != NULL )
		{
			pNode->m_attrs[std::string((char*)pAttrName)] = std::string((char*)pAttrValue);
			xmlFree(pAttrValue);
		}
	}

	for(xmlNode *pCurNode = pXmlNode->children; pCurNode != NULL ; pCurNode = pCurNode->next)
	{
		if( pCurNode->type == XML_ELEMENT_NODE )
			pNode->m_children.push_back(Build(pDoc, pCurNode));
	}
	
	return pNode;
}

void XmlConfig::Load(const char *szBuffer, size_t uSize)
{
	xmlDoc *pDoc = xmlParseMemory(szBuffer, uSize);
	m_pRoot = Build(pDoc, xmlDocGetRootElement(pDoc));
	if( pDoc != NULL )
		xmlFreeDoc(pDoc);
	xmlCleanupParser();
    xmlMemoryDump();
}

void XmlConfig::Load(std::string fileName)
{
	if( fileName.empty() ) return;
	m_fileName = fileName;
	xmlDoc *pDoc = xmlReadFile(fileName.c_str(), NULL, 0);

	m_pRoot = Build(pDoc, xmlDocGetRootElement(pDoc));

	if( pDoc != NULL )
		xmlFreeDoc(pDoc);
	xmlCleanupParser();
    xmlMemoryDump();
}

}
