#include "serverattr.h"
namespace GNET
{
	void ServerAttr::SetLoad(unsigned char nload) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.load=nload;
	}
	unsigned char ServerAttr::GetLoad() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.load;
	}
	void ServerAttr::SetLambda(unsigned char nlambda) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.lambda=nlambda;
	}
	unsigned char ServerAttr::GetLambda() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.lambda;
	}
	unsigned char ServerAttr::GetDoubleExp() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.doubleExp;
	}
	void ServerAttr::SetDoubleExp( unsigned char doubleExp ) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.doubleExp=(unsigned char)!!doubleExp;
	}
	unsigned char ServerAttr::GetExpRate()
	{
		Thread::Mutex::Scoped l(m_locker_);
		return ExpRate;
	}
	void ServerAttr::SetExpRate(unsigned char exp)
	{
		Thread::Mutex::Scoped l(m_locker_);
		ExpRate = exp;
	}
	unsigned char ServerAttr::GetPVP() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.pvp;
	}
	void ServerAttr::SetPVP( unsigned char _pvp ) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.pvp=(unsigned char)!!_pvp;
	}

	unsigned char ServerAttr::GetDoubleMoney() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.doubleMoney;
	}	
	void ServerAttr::SetDoubleMoney( unsigned char doubleMoney ) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.doubleMoney=doubleMoney;
	}
	
	unsigned char ServerAttr::GetDoubleObject() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.doubleObject;
	}
	void ServerAttr::SetDoubleObject( unsigned char doubleObj ) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.doubleObject=doubleObj;
	}

	unsigned char ServerAttr::GetDoubleSP() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.doubleSP;
	}
	void ServerAttr::SetDoubleSP( unsigned char doubleSP ) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.doubleSP=doubleSP;
	}
    unsigned char ServerAttr::GetFreeZone() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.freeZone;
	}
    void ServerAttr::SetFreeZone( unsigned char freeZone) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.freeZone=freeZone;
	}
    unsigned char ServerAttr::GetBattle() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.bBattle;
	}
    void ServerAttr::SetBattle( unsigned char battle) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.bBattle=battle;
	}
    unsigned char ServerAttr::GetSellpoint() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_.bSellpoint;
	}
    void ServerAttr::SetSellpoint( unsigned char sellpoint) {
		Thread::Mutex::Scoped l(m_locker_);
		m_Attr_.bSellpoint=sellpoint;
	}
	unsigned int ServerAttr::GetAttr() {
		Thread::Mutex::Scoped l(m_locker_);
		return m_Attr_._attr;
	}

}
