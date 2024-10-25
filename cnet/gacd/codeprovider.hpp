#ifndef __GNET_CODEPROVIDER_HPP
#define __GNET_CODEPROVIDER_HPP

#include "codepiece.hpp"
#include "codereschecker.hpp"
#include "xmlconfig.h"
#include <deque>

namespace GNET
{

class CodeProvider;

struct SendingCode
{
	int m_iCodeID;
	CodeProvider* m_pProvider;
	SendingCode(int iCodeID = 0, CodeProvider *pProvider = NULL ); 

	bool GetCode(int iUserID, int iSeq, CodePieceVector &cpv, CodeResChecker *&pChecker);
};

typedef std::deque<SendingCode> SendingQueue;

class CodeProvider
{
public:

	virtual ~CodeProvider() { }

	virtual bool GetCodePieceVector(int iUserID, int iID, CodePieceVector &cpv, 
		int iSeq, CodeResChecker *&pChecker) = 0;
	virtual void UpdateSendingQueue(SendingQueue &queue) const = 0;
	virtual void OnUpdateConfig(const XmlConfig::Node *pRoot) = 0;
};

};


#endif

