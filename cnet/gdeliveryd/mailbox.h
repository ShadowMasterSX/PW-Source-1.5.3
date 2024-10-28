/* @file: mailbox.h
 * @description: MailBox class management a mail list of one role.
 * 
 * @Date: 2005-10-26
 * @Last-Modified: 2005-10-26
 * @Author: liping
 */ 
#ifndef  __GNET_MAILBOX_H
#define  __GNET_MAILBOX_H
#include "macros.h"
#include "gmailid"
#include "gmailheader"

#define MAIL_UNREAD_DELETE 2592000 // 30 days
#define MAIL_READ_DELETE   259200  // 3 days
#define MAIL_ONTHEWAY_TIME 3600    // 3600 seconds
namespace GNET
{
	class MailBox
	{
	public:	
		MailBox(int roleid=-1) : m_blInit_(false) { m_roleid_=roleid; }
		MailBox(int roleid,const GMailHeaderVector& mc ) : 
			m_blInit_(false),
			mail_container_(mc)
		{ m_roleid_=roleid; }
		~MailBox() { mail_container_.clear(); }
		void  MailList(const GMailHeaderVector& mc)
		{
			m_blInit_=true;
			mail_container_=mc; 
		}
		//set mail attribute
		void SetAttribute(unsigned char mail_id,unsigned char mail_attr);
		void UnSetAttribute(unsigned char mail_id,unsigned char mail_attr);
		bool HasAttribute(unsigned char mail_id,unsigned char mail_attr);
		//mail box operation
		bool IsInit() { return m_blInit_; }
		int HaveNewMail(unsigned char & present_type); //return the remain-time of the fastest-arrive mail, -1 means no new mail
		bool IsMailExist(unsigned char mail_id);
		int  GetSize();//return all mails in the mailbox, including In-The-Way mail
		bool GetMail( unsigned char mail_id,GMailHeader& mail );
		bool GetMailList( GMailHeaderVector& maillist ); // return all available mails for player,exclude In-The-Way mail
		void PutMail( const GMailHeader& mail );
		bool DeleteMail( unsigned char mail_id );
		bool DeleteMail( const IntVector& maillist );
		void CheckExpireMail( GMailIDVector& maillist );
		void FindMail(IntVector& maillist ,int type, int special_sender,int except_sender);
		bool CheckSpecialTitle(const IntVector& maillist, int arg);
	private:
		bool DeletePolicy( const GMailHeader& mail,time_t now );	
		int DelayPolicy( const GMailHeader& mail,time_t now ); //return remain-time, 0 means no need to delay
		bool TitlePolicy( const GMailHeader& mail,int arg);
		//define bit operations
		void SetBit( unsigned char& data,unsigned char pos );
		void UnsetBit( unsigned char& data,unsigned char pos );
		bool HasBit( unsigned char data,unsigned char pos );
	private:	
		int                m_roleid_;
		bool               m_blInit_;
		GMailHeaderVector  mail_container_;
	};

}
#endif
