#include "configmanager.hpp"
#include "asmcodelibrary.hpp"
#include "codepiecelibrary.hpp"
#include "preparedcodelibrary.hpp"
#include "ganticheaterserver.hpp"
#include "punishmanager.hpp"
#include "userdatamanager.hpp"


namespace GNET
{

ConfigManager ConfigManager::s_instance;

void ConfigManager::Init(const char *szFileName)
{
	m_config.Reload(szFileName);
	const XmlConfig::Node *pRoot = m_config.Root();
	if( pRoot != NULL ) m_version = pRoot->GetAttr("version");
	UpdateConfig();
}

void ConfigManager::UpdateConfig()
{
	UpdateConfigClass<UserSessionData>();
	UpdateConfigInstance<AsmCodeLibrary>();
	/*
	UserSessionData::OnUpdateConfig(m_config.Root());
	AsmCodeLibrary::GetInstance()->OnUpdateConfig(m_config.Root());
	*/
	CodePieceLibrary::GetInstance()->OnUpdateConfig(m_config.Root());
	PreparedCodeLibrary::GetInstance()->OnUpdateConfig(m_config.Root());
	UserCodeManager::OnUpdateConfig(m_config.Root());
	PunishManager::GetInstance()->OnUpdateConfig(m_config.Root());
}

void ConfigManager::Init(const char *szBuffer, size_t uSize)
{
	// TODO
}
void ConfigManager::Reload(const char *szBuffer, size_t uSize)
{
	m_config.Reload(szBuffer, uSize);
	const XmlConfig::Node *pRoot = m_config.Root();
	if( pRoot != NULL ) m_version = pRoot->GetAttr("version");

	UserDataManager::GetInstance()->Stop();

	UpdateConfig();

	GAntiCheaterServer::GetInstance()->ResetSource();
}
void ConfigManager::Reload(const char *szFileName)
{
	// TODO
}

};

