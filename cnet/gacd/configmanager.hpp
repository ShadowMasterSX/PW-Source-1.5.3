#ifndef __GNET_CONFIGMANAGER_HPP
#define __GNET_CONFIGMANAGER_HPP

#include "xmlconfig.h"

namespace GNET
{

class ConfigManager
{
public:

	void Init(const char *szFileName);
	void Init(const char *szBuffer, size_t uSize);
	void Reload(const char *szBuffer, size_t uSize);
	void Reload(const char *szFileName);

	static ConfigManager* GetInstance() { return &s_instance; }
	
	std::string GetVersion() const { return m_version; }
 
private:
	
	template<typename T>	
	void UpdateConfigInstance()
	{
		T::GetInstance()->OnUpdateConfig(m_config.Root());
	}
	template<typename T>	
	void UpdateConfigClass()
	{
		T::OnUpdateConfig(m_config.Root());
	}

	std::string m_version;
	XmlConfig m_config;

	static ConfigManager s_instance;
	void UpdateConfig();
};

};

#endif
