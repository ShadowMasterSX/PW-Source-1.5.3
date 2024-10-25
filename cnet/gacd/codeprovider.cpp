
#include "codeprovider.hpp"

namespace GNET
{

SendingCode::SendingCode(int iCodeID, CodeProvider *pProvider)
        : m_iCodeID(iCodeID), m_pProvider(pProvider) {  }

bool SendingCode::GetCode(int iUserID, int iSeq, CodePieceVector &cpv, CodeResChecker *&pChecker)
{
    if( m_pProvider == NULL ) return false;
    return m_pProvider->GetCodePieceVector(iUserID, m_iCodeID, cpv, iSeq, pChecker);
}

};

