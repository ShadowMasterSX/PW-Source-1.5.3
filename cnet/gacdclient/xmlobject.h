#ifndef __MY_XMLOBJECT
#define __MY_XMLOBJECT


#include <list>
#include <map>
#include <string>

#include <libxml/xmlreader.h>

#include "stringcaster.h"
/**
 
 */


struct XmlObject
{
	class Exception { };
	static std::string nl;
	static std::string tab;
	static std::string space;
	typedef std::map<std::string, std::string> Attrs;
	typedef std::list<XmlObject> Children;

	std::string name;
	std::string content;
	Attrs attrs;
	Children children;

	XmlObject() { }
	XmlObject(std::string _name) : name(_name) { }
	bool operator==(const XmlObject &_obj) const;
	XmlObject(const XmlObject &_obj);
	XmlObject& operator=(const XmlObject &_obj);
	~XmlObject() { }

	Children GetChildren() { return children; }
	Children GetChildren(std::string _name);
	void RemoveChildren(std::string _name);

	XmlObject& AddChild(const XmlObject &_obj);
	XmlObject& RemoveChild(XmlObject _obj);
	XmlObject& SetAttrStr(std::string _attr_name, const std::string &_attr_value);
	const std::string& GetAttrStr(std::string _attr_name) const;

	template<class T>
	XmlObject& SetAttr(std::string _attr_name, const T& _t)	
	{
		return SetAttrStr(_attr_name, StringCaster<T>::ToString(_t));
	}

	template<class T>
	T GetAttr(std::string _attr_name, const T& _t ) const
	{
		return StringCaster<T>::FromString(GetAttrStr(_attr_name));
	}

	std::string GetAttr(std::string _attr_name) const { return GetAttrStr(_attr_name); }

	bool HasAttr(std::string _attr_name) const;
	
	std::string ToString(size_t _lvl = 0) const;
	//std::string ToString() const { return ToString(0); }
	XmlObject& FromString(const std::string &str);

	XmlObject Build(xmlDoc *doc, xmlNode* xmlnode);
	bool IsEmptyString(const std::string &str) const;
};


#endif
