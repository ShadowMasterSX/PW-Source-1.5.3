#ifndef __MY_XMLDBOBJECT
#define __MY_XMLDBOBJECT

#include "xmlobject.h"

class XmlDB
{
public:
	class Exception { };
protected:
	virtual std::string GetFileName() const = 0;
	virtual XmlObject WriteObject() const = 0;
	virtual void LoadObject(const XmlObject &_obj) = 0;
	virtual ~XmlDB() { }
public:
	void Load();
	void Save() const;
private:
	void CheckDir() const;
};

#endif
