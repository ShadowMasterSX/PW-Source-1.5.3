#include "mailbox.h"
#include "localmacro.h"
#define MAX_SINT_VALUE 0x7fffffff
namespace GNET
{
void _patch_mail(GMailHeader& mail)
{
	if ( mail.sndr_type==_MST_WEB ) mail.sndr_type=_MST_PLAYER;
}	
void MailBox::SetAttribute(unsigned char mail_id,unsigned char mail_attr)
{
	if ( !m_blInit_ || mail_attr>=_MA_TYPE_NUM ) return;
    for ( size_t i=0;i<mail_container_.size();++i )
        if ( mail_container_[i].id==mail_id )
		{
			if ( mail_attr==_MA_ATTACH_BOTH )
			{
				SetBit( mail_container_[i].attribute,_MA_ATTACH_OBJ );
				SetBit( mail_container_[i].attribute,_MA_ATTACH_MONEY );
			}
			else	
				SetBit( mail_container_[i].attribute,mail_attr );
			break;
		}
}
void MailBox::UnSetAttribute(unsigned char mail_id,unsigned char mail_attr)
{
	if ( !m_blInit_ || mail_attr>=_MA_TYPE_NUM ) return;
	for ( size_t i=0;i<mail_container_.size();++i )
        if ( mail_container_[i].id==mail_id )
		{
			if ( mail_attr==_MA_ATTACH_BOTH )
			{
				UnsetBit( mail_container_[i].attribute,_MA_ATTACH_OBJ );
				UnsetBit( mail_container_[i].attribute,_MA_ATTACH_MONEY );
			}
			else	
				UnsetBit( mail_container_[i].attribute,mail_attr );
			//if no _MA_PRESERVE,_MA_ATTACH_OBJ,_MA_ATTACH_MONEY flags, set receiver as now
			if ( (mail_attr & MAIL_RESERVED)==0 ) 
				mail_container_[i].receiver=time(NULL); //if mail is read, set receiver to read time
			break;
		}

}
bool MailBox::HasAttribute(unsigned char mail_id,unsigned char mail_attr)
{
	if ( !m_blInit_ || mail_attr>=_MA_TYPE_NUM ) return false;
	for ( size_t i=0;i<mail_container_.size();++i )
        if ( mail_container_[i].id==mail_id )
		{
			if ( mail_attr==_MA_ATTACH_BOTH )
			{
				return ( HasBit(mail_container_[i].attribute,_MA_ATTACH_OBJ) ) &&
					   ( HasBit(mail_container_[i].attribute,_MA_ATTACH_MONEY) );
			}
			else
				return HasBit(mail_container_[i].attribute,mail_attr);
		}
	return false;
}
int MailBox::HaveNewMail(unsigned char & present_type)
{
	if ( !m_blInit_ ) return -1;
	//需要先遍历邮箱把赠送和索取邮件的标志位提取出来发给客户端 gxw20130307
	for ( size_t i=0;i<mail_container_.size();++i )
	{
		if ( HasBit(mail_container_[i].attribute,_MA_UNREAD) )
		{
			if ( mail_container_[i].sndr_type == _MST_PLAYERPRESENT)
			{
				if ((present_type & 0x01) == 0 && mail_container_[i].sender == PLAYERPRESENT_GIVE)
					present_type |= 0x01;
				else if ((present_type & 0x02) == 0 && mail_container_[i].sender == PLAYERPRESENT_ASK)
					present_type |= 0x02;
			}
			else if ( mail_container_[i].sndr_type == _MST_FRIENDCALLBACK)
			{
				if(mail_container_[i].sender == PLAYERREQUITE_CALL)
					present_type |= 0x04;
				else if(mail_container_[i].sender ==PLAYERREQUITE_ANSWER)
					present_type |= 0x08;
			}
		}
	}
	int now=time(NULL);
	int remain_time=MAX_SINT_VALUE,tmp_delay=0;
	for ( size_t i=0;i<mail_container_.size();++i )
		if ( HasBit(mail_container_[i].attribute,_MA_UNREAD) )
		{
			//mail-package must delay 1 hour to send to receive
			if ((tmp_delay=DelayPolicy(mail_container_[i],now)))
			{
				if ( tmp_delay<remain_time ) remain_time=tmp_delay;
				continue;
			}
			return 0;
		}
	return ( remain_time<MAX_SINT_VALUE ) ? remain_time : -1; // -1 means no new mail
}
bool MailBox::IsMailExist(unsigned char mail_id)
{
	if ( !m_blInit_ ) return false;
	for ( size_t i=0;i<mail_container_.size();++i )
		if ( mail_container_[i].id==mail_id ) return true;
	return false;
}
int  MailBox::GetSize()
{
	if ( !m_blInit_ ) return 0;
	return mail_container_.size();
}
bool MailBox::GetMail( unsigned char mail_id,GMailHeader& mail )
{
	if ( !m_blInit_ ) return false;
	for ( size_t i=0;i<mail_container_.size();++i )
		if ( mail_container_[i].id==mail_id )
		{
			mail=mail_container_[i];
			_patch_mail(mail);	
			return true;
		}
	return false;
}
bool MailBox::GetMailList( GMailHeaderVector& maillist )
{
	if ( !m_blInit_ ) return false;
	time_t now=time(NULL);
	for ( size_t i=0;i<mail_container_.size();++i )
	{   
		if ( DelayPolicy(mail_container_[i],now) || 
			 DeletePolicy( mail_container_[i],now )	
		   )
			continue;	
		maillist.push_back( mail_container_[i] );
	}
	for ( size_t i=0;i<maillist.size();++i )
		_patch_mail( maillist[i] );
	return true;
}
void MailBox::PutMail( const GMailHeader& mail )
{
	if ( !m_blInit_ ) return;
	mail_container_.push_back( mail );
}	
bool MailBox::DeleteMail( unsigned char mail_id )
{
	if ( !m_blInit_ ) return false;
	GMailHeaderVector::iterator it=mail_container_.begin(),ite=mail_container_.end();
	for ( ;it!=ite;++it )
		if ( (*it).id==mail_id )
		{
			mail_container_.erase(it);
			return true;
		}
	return false;
}
bool MailBox::DeleteMail(const IntVector& maillist)
{
	if ( !m_blInit_ ) return false;
	IntVector::const_iterator idit=maillist.begin(),idite=maillist.end();
	for( ; idit!=idite;++idit )
	{
		GMailHeaderVector::iterator it=mail_container_.begin(),ite=mail_container_.end();
		for ( ;it!=ite;++it )
		{
			if ( (*it).id == (*idit) )
			{
				mail_container_.erase(it);
				break;
			}
		}
	}
	return true;
}

void MailBox::CheckExpireMail(GMailIDVector& maillist )
{
	if ( !m_blInit_ ) return;
	time_t now=time(NULL);
    GMailHeaderVector::iterator it=mail_container_.begin(),ite=mail_container_.end();
    for ( ;it!=ite;++it )
		if ( DeletePolicy( *it,now ) )
		{
			maillist.add( GMailID(m_roleid_,(*it).id) );
		}
}
/////////////////////////////////////////////////
///                                           ///
///    Below is private functions             ///
///                                           ///
/////////////////////////////////////////////////
bool MailBox::DeletePolicy( const GMailHeader& mail,time_t now )
{
	if ( HasBit(mail.attribute,_MA_ATTACH_OBJ) || 
		 HasBit(mail.attribute,_MA_ATTACH_MONEY) || 
		 HasBit(mail.attribute,_MA_PRESERVE) )
		return false;
	if ( HasBit(mail.attribute,_MA_UNREAD) && 
		 now-mail.receiver > MAIL_UNREAD_DELETE ) //receiver is 'read-time'
		return true;
	if ( !HasBit(mail.attribute,_MA_UNREAD) &&
		 now-mail.receiver > MAIL_READ_DELETE )   //receiver is 'read-time'
		return true;
	return false;	
}
int MailBox::DelayPolicy( const GMailHeader& mail,time_t now )
{
	return ( mail.sndr_type == _MST_PLAYER &&
			 HasBit(mail.attribute,_MA_ATTACH_OBJ) && 
			 (now-mail.send_time < MAIL_ONTHEWAY_TIME)
		   ) ? MAIL_ONTHEWAY_TIME-(now-mail.send_time) : 0;
}
//define bit operations
void MailBox::SetBit( unsigned char& data,unsigned char pos )
{
	if ( pos > 7 ) return;
	data |= (1<<pos);
}
void MailBox::UnsetBit( unsigned char& data,unsigned char pos )
{
	if ( pos > 7 ) return;
	data &= ~(1<<pos);
}
bool MailBox::HasBit( unsigned char data,unsigned char pos )
{
	if ( pos > 7 ) return false;
	return data & (1<<pos);
}

void MailBox::FindMail(IntVector& maillist ,int type,int special_sender, int except_sender)
{
	if ( !m_blInit_ ) return;
    GMailHeaderVector::iterator it=mail_container_.begin(),ite=mail_container_.end();
    for ( ;it!=ite;++it )
	{
		if((*it).sndr_type != type) continue;
		if(-1 != special_sender && (*it).sender != special_sender) continue;
		if(-1 != except_sender && (*it).sender == except_sender) continue;
		maillist.add((*it).id);
	}

}

bool MailBox::TitlePolicy( const GMailHeader& mail,int arg)
{
	switch(mail.sndr_type)
	{
		case _MST_FRIENDCALLBACK:
		{
			Marshal::OctetsStream os(mail.title);
			int roleid;
			os >> roleid;
			return roleid == arg;
		}break;
	}
	
	return false;
}

bool MailBox::CheckSpecialTitle(const IntVector& maillist, int arg)
{
	if ( !m_blInit_ ) return false;
	IntVector::const_iterator idit=maillist.begin(),idite=maillist.end();
	for( ; idit!=idite;++idit )
	{
		GMailHeaderVector::iterator it=mail_container_.begin(),ite=mail_container_.end();
		for ( ;it!=ite;++it )
		{
			if ( (*it).id == (*idit) )
			{
				if(TitlePolicy(*it,arg)) 
					return true;
			}
		}
	}
	return false;
}



}
