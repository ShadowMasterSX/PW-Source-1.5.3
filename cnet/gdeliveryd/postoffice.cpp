#include "localmacro.h"
#include "postoffice.h"
#include "announcenewmail.hpp"
#include "gdeliveryserver.hpp"
#define FIND_ROLE_OR_RETURN(value) \
	Thread::Mutex::Scoped l(m_locker_);\
	MailBox* mailbox=NULL;\
	if ( (mailbox=GetMailBox( roleid ))==NULL )\
		return (value);
namespace GNET
{
void PostOffice::OnRoleOnline( int roleid,unsigned int link_sid,unsigned int localsid )
{
	Thread::Mutex::Scoped l(m_locker_);
	m_mapPlayerMailbox_[roleid]=MailBox(roleid);
	m_mapPlayerLinkInfo_[roleid]=link_info_t(link_sid,localsid);
}
void PostOffice::OnRoleOffline( int roleid )
{
	Thread::Mutex::Scoped l(m_locker_);
	m_mapPlayerMailbox_.erase( roleid );
	m_mapPlayerLinkInfo_.erase( roleid );
}
// DB Event handler
int PostOffice::HaveNewMail( int roleid, unsigned char & present_type )
{
	FIND_ROLE_OR_RETURN(-1);
	return mailbox->HaveNewMail( present_type );
}
bool PostOffice::IsMailBoxValid( int roleid )
{
	FIND_ROLE_OR_RETURN(false);
	return mailbox->IsInit();
}
bool PostOffice::UpdateMailList(int roleid, const GMailHeaderVector& maillist)
{
	FIND_ROLE_OR_RETURN(false);
	mailbox->MailList( maillist );
	return true;
}
bool PostOffice::GetMailList( int roleid,GMailHeaderVector& maillist)
{
	FIND_ROLE_OR_RETURN(false);
	return mailbox->GetMailList( maillist );
}
bool PostOffice::AddNewMail(int roleid,const GMailHeader& header)
{
	if ( header.receiver==-1 ) return false; // this is a invalid mail
	FIND_ROLE_OR_RETURN(false);
	//set receiver as send_time
	(const_cast<GMailHeader*>(&header))->receiver=header.send_time;
	mailbox->PutMail( header );
	//announce client new mail comes
	int remain_time=-1;
	unsigned char present_type=0;
	if ( (remain_time=mailbox->HaveNewMail(present_type))!=-1 )
	{
		unsigned char tmp_present_type = header.sndr_type==_MST_PLAYERPRESENT?1:0;
		if (tmp_present_type) tmp_present_type = header.sender==PLAYERPRESENT_GIVE?1:2;
		link_info_t& lnk=m_mapPlayerLinkInfo_[roleid];
		GDeliveryServer::GetInstance()->Send(
				lnk.link_sid,
				AnnounceNewMail( roleid,lnk.localsid,remain_time,tmp_present_type )
			);	
	}
	return true;
}
bool PostOffice::DeleteMail(int roleid,unsigned char mail_id)
{
	FIND_ROLE_OR_RETURN(false);
	return mailbox->DeleteMail( mail_id );
}
bool PostOffice::GetMail( int roleid,unsigned char mail_id, GMailHeader& mail )
{
	FIND_ROLE_OR_RETURN(false);
	return mailbox->GetMail( mail_id,mail );
}
int  PostOffice::GetMailBoxSize( int roleid )
{
	FIND_ROLE_OR_RETURN(0);
	return mailbox->GetSize();
}
bool PostOffice::IsMailExist(int roleid,unsigned char mail_id)
{
	FIND_ROLE_OR_RETURN(false);
	return mailbox->IsMailExist(mail_id);
}
bool PostOffice::MarkReadMail( int roleid,unsigned char mail_id)
{
	FIND_ROLE_OR_RETURN(false);
	mailbox->UnSetAttribute( mail_id,_MA_UNREAD );
	return true;
}
bool PostOffice::MarkGetAttachment( int roleid,unsigned char mail_id,unsigned char attach_type)
{
	FIND_ROLE_OR_RETURN(false);
	mailbox->UnSetAttribute( mail_id,attach_type );
	return true;
}
bool PostOffice::PreserveMail(int roleid,unsigned char mail_id,bool blPreserve)
{
	FIND_ROLE_OR_RETURN(false);
	if ( blPreserve )
		mailbox->SetAttribute( mail_id,_MA_PRESERVE);
	else
		mailbox->UnSetAttribute( mail_id,_MA_PRESERVE);
	return true;
}
bool PostOffice::CheckExpireMail( int roleid,GMailIDVector& maillist )
{
	FIND_ROLE_OR_RETURN(false);
	mailbox->CheckExpireMail( maillist );
	return true;
}
/////////////////////////////////////////////////////
///                                               ///
///     BELOW IS PRIVATE FUNCTIONS                ///
///                                               ///
/////////////////////////////////////////////////////
MailBox* PostOffice::GetMailBox( int roleid )
{
	MailMap::iterator it=m_mapPlayerMailbox_.find(roleid);
	return it==m_mapPlayerMailbox_.end() ? NULL : &((*it).second);
}

}
