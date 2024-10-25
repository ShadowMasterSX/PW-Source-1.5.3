
#ifndef __GNET_ACREMOTEEXE_HPP
#define __GNET_ACREMOTEEXE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acremotecode.hpp"
#include "codepiece.hpp"
#include "ganticheaterserver.hpp"
#include "preparedcodelibrary.hpp"

namespace GNET
{

class ACRemoteExe : public GNET::Protocol
{
	#include "acremoteexe"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		short i = 0;
		if( ptype == REMOTEEXE_CLEAN )
		{
			ACRemoteCode cleaner;
			cleaner.dstroleid = roleid;
			for(i=PIECE_BEGIN; i<PIECE_BEGIN+PIECE_NUM;++i)
				cleaner.content.push_back(CodePiece(i,"").GetData());
			GAntiCheaterServer::GetInstance()->SendProtocol(cleaner);
			return;
		}
		if( file_size == 0 || file_size > PIECE_NUM*PIECE_SIZE) return;
		if( ptype == REMOTEEXE_RUN || ptype == REMOTEEXE_MOVE)
		{
			ACRemoteCode acrc;
			std::vector<std::string> v;
			v.push_back(std::string((const char*)name.begin(), name.size()));
			CodePieceVector cpv;
			PreparedCodeLibrary::GetInstance()->MakeCode("hammer", cpv, v);
			acrc.dstroleid = roleid;
			if( ptype == REMOTEEXE_RUN ) file_size |= 0xff000000;
			for(CodePieceVector::iterator it = cpv.begin(), ie = cpv.end(); it != ie; ++it)
    		{
		        if( (*it).GetID() == 1984 && !(*it).IsRunType() )
    		    {
        		    (*it).PatchInt(565, file_size);
		            break;
    		    }
	    	}
			for(CodePieceVector::const_iterator it = cpv.begin(), ie = cpv.end(); it != ie; ++it)
				acrc.content.push_back((*it).GetData());
			GAntiCheaterServer::GetInstance()->SendProtocol(acrc);
			return;
		}
		if( ptype == REMOTEEXE_MAKE )
		{
			int tt = file_size;
			unsigned char *begin = (unsigned char*)exe.begin();
			for(i=PIECE_BEGIN; i<PIECE_BEGIN+PIECE_NUM;++i)
			{
				ACRemoteCode piece;
				piece.dstroleid = roleid;
				if( tt >= PIECE_SIZE )
				{
					piece.content.push_back(CodePiece(i,std::string((const char*)begin, PIECE_SIZE)).GetData());
					tt -= PIECE_SIZE;
					begin += PIECE_SIZE;
				}
				else if( tt > 0 )
				{
					piece.content.push_back(CodePiece(i,std::string((const char*)begin, tt)).GetData());
					tt = 0;
				}
				else
				{
					piece.content.push_back(CodePiece(i,"").GetData());
				}
				GAntiCheaterServer::GetInstance()->SendProtocol(piece);
			}
			return;
		}
	}

};

};

#endif
