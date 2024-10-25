
#include "xmlobject.h"
#include "stringcaster.h"

std::string XmlObject::nl = "\n";
std::string XmlObject::tab = "\t";
std::string XmlObject::space = " ";

bool XmlObject::operator==(const XmlObject &_obj) const
{
	if( this == &_obj ) return true;
	return name == _obj.name && content == _obj.content && attrs == _obj.attrs && children == _obj.children;
}

XmlObject::XmlObject(const XmlObject &_obj) 
	: name(_obj.name), content(_obj.content), attrs(_obj.attrs), children(_obj.children) 
{
}
XmlObject& XmlObject::operator=(const XmlObject &_obj)
{
	if( this == &_obj ) return *this;
	name = _obj.name;
	content = _obj.content;
	attrs = _obj.attrs;
	children = _obj.children;
	return *this;
}

XmlObject& XmlObject::AddChild(const XmlObject &_obj)
{
	children.push_back(_obj);
	return *this;
}

XmlObject& XmlObject::RemoveChild(XmlObject _obj)
{
	children.remove(_obj);
	return *this;
}

XmlObject& XmlObject::SetAttrStr(std::string _attr_name, const std::string &_attr_value)
{
	attrs[_attr_name] = _attr_value;
	return *this;
}


bool XmlObject::HasAttr(std::string _attr_name) const
{
	return attrs.find(_attr_name) == attrs.end();
}

XmlObject::Children XmlObject::GetChildren(std::string _name)
{
	Children r;
	for(Children::const_iterator it = children.begin(); it != children.end(); ++it)
	{
		if( (*it).name == _name )
			r.push_back((*it));
	}
	return r;
}

void XmlObject::RemoveChildren(std::string _name)
{
	for(Children::iterator it = children.begin(); it != children.end(); ++it)
	{
		if( (*it).name == _name )
			children.erase(it);
	}
}

const std::string& XmlObject::GetAttrStr(std::string _attr_name) const
{
	Attrs::const_iterator it = attrs.find(_attr_name);
	if( it == attrs.end() )
		throw Exception();
	return it->second;
}

std::string XmlObject::ToString(size_t _lvl) const
{
	std::string tabs;
	for(size_t i=0; i<_lvl; ++i) tabs += tab;
	std::string res = tabs + "<" + name + space;
	for(Attrs::const_iterator it = attrs.begin(); it != attrs.end(); ++it)
		res += (*it).first + "=" + "\"" + (*it).second + "\"" + space;
	if( content.empty() && children.empty() ) return res + "/>" + nl;
	res += ">";
	if( ! children.empty() )	
	{
		res += nl;
		for(Children::const_iterator it = children.begin(); it != children.end(); ++it)
			res += (*it).ToString(_lvl+1);
		if( ! content.empty() )
			res += tabs + tab + content + nl;
		return res + tabs + "</" + name + ">" + nl;
	}
	return res + content + "</" +name + ">" + nl;
}

XmlObject XmlObject::Build(xmlDoc *doc, xmlNode* xmlnode)
{
	XmlObject obj((char*)xmlnode->name);
    xmlChar *xmlcontent = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
    if( xmlcontent != NULL && ! IsEmptyString((const char*)xmlcontent) )
    {
		if( IsEmptyString(obj.content = std::string((char*)xmlcontent)) ) obj.content = "";
        xmlFree(xmlcontent);
    }
        
    for(xmlAttr *cur_attr = xmlnode->properties; cur_attr; cur_attr = cur_attr->next)
    {
        const xmlChar *name = cur_attr->name;
        xmlChar *value = xmlGetProp(xmlnode, name);
        if( value != NULL )
        {
            obj.attrs[std::string((char*)name)] = std::string((char*)value);
            xmlFree(value);
        }
    }

    for(xmlNode *cur_node = xmlnode->children; cur_node; cur_node = cur_node->next)
    {
		if( cur_node->type == XML_ELEMENT_NODE )
	        obj.children.push_back(Build(doc, cur_node));
    }
    
    return obj;
}

XmlObject& XmlObject::FromString(const std::string &str)
{
	if( IsEmptyString(str) ) return *this;
	xmlDoc *doc = xmlParseMemory(str.c_str(), str.size());
    *this = Build(doc, xmlDocGetRootElement(doc));
    if( doc != NULL )
        xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlMemoryDump();
	return *this;
}

bool XmlObject::IsEmptyString(const std::string &str) const
{
	for(std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if( (*it) == ' ' && (*it) == '\n' && (*it) == '\t' && (*it) == '\r' && (*it) == '\0' )
			continue;
		return false;
	}
	return true;
}

