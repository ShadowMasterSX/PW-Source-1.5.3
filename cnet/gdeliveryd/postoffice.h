/* @file: postoffice.h
 * @description: establish a mailbox for each ROLE. postoffice is the data center and management center for
 *               entire mail system
 *
 * @Date: 2005-10-26
 * @Last-Modified: 2005-10-26
 * @Author: liping
 *
 */ 
#ifndef __GNET_POSTOFFICE_H
#define __GNET_POSTOFFICE_H

#include <map>
#include "mutex.h"
#include "mailbox.h"
namespace GNET
{
	class PostOffice
	{
		struct link_info_t
		{
			unsigned int link_sid;
			unsigned int localsid;
			link_info_t() : link_sid(0),localsid(0) { }
			link_info_t(unsigned int _lnksid,unsigned int _localsid) : 
				link_sid(_lnksid),localsid(_localsid)
			{ }
		};
		typedef std::map<int,MailBox>     MailMap;     //map roleid to mailbox
		typedef std::map<int,link_info_t> LinkInfoMap; //map roleid to link_info_t
		PostOffice() : m_locker_("PostOffice::lock PlayerMailbox map") { }
	public:
		static PostOffice& GetInstance() { static PostOffice instance; return instance; }
		// player Event handler
		void OnRoleOnline( int roleid,unsigned int link_sid,unsigned int localsid );
		void OnRoleOffline( int roleid );
		// DB Event handler
		int HaveNewMail( int roleid , unsigned char & present_type); //-1 means no new mail. otherwise return remain-time of the fastest-arrive mail
		bool IsMailBoxValid( int roleid );
		bool UpdateMailList(int roleid, const GMailHeaderVector& maillist);
		bool GetMailList( int roleid,GMailHeaderVector& maillist);
		bool AddNewMail(int roleid,const GMailHeader& header);
		bool DeleteMail(int roleid,unsigned char mail_id);
		bool DeleteMail(int roleid,const IntVector& maillist);
		bool GetMail( int roleid,unsigned char mail_id, GMailHeader& mail );
		int  GetMailBoxSize( int roleid );
		bool IsMailExist(int roleid,unsigned char mail_id);
		bool MarkReadMail( int roleid,unsigned char mail_id);
		bool MarkGetAttachment( int roleid,unsigned char mail_id,unsigned char attach_type);
		bool PreserveMail(int roleid,unsigned char mail_id,bool blPreserve);
		bool CheckExpireMail( int roleid,GMailIDVector& maillist );
		bool FindMail(int roleid, IntVector& maillist ,int type,int special_sender, int except_sender);
		bool CheckSpecialTitle(int roleid,const IntVector& maillist, int arg);
	private:
		//protocol handler
		MailBox* GetMailBox( int roleid );
	private:
		Thread::Mutex   m_locker_;
		MailMap         m_mapPlayerMailbox_;	
		LinkInfoMap     m_mapPlayerLinkInfo_;	
	};
}

#endif
