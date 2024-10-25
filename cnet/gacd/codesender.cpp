
#include "preparedcodelibrary.hpp"
#include "codesender.hpp"

namespace GNET
{

void CodeSender::operator() (UserSessionData *data)
{
	CodePieceVector cpv;
	int iCodeID = PreparedCodeLibrary::GetInstance()->MakeCode(m_codeName, cpv, m_params);
	data->GetCodeManager().SendCode(iCodeID, iCodeID, cpv, m_pChecker->Clone());
}

};

