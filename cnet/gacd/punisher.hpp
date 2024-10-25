#ifndef __GNET_PUNISHER_HPP
#define __GNET_PUNISHER_HPP

#include <string>


namespace GNET
{

class Punisher
{
protected:

	int m_iUserID;
	int m_iDelayTime;

	Punisher() : m_iUserID(0), m_iDelayTime(0) { }
	Punisher(int iUserID) : m_iUserID(iUserID), m_iDelayTime(0) { }
	Punisher(int iUserID, int iDelayTime) : m_iUserID(iUserID), m_iDelayTime(iDelayTime) { }
	Punisher(const Punisher &p) : m_iUserID(p.m_iUserID), m_iDelayTime(p.m_iDelayTime) { }

public:

	virtual	~Punisher() { }

	int GetUserID()    const { return m_iUserID;    }
	int GetDelayTime() const { return m_iDelayTime; }

	virtual void Process() const = 0; 

};

class KickPunisher : public Punisher
{
protected:
	bool m_bForbid;
    int  m_iForbidTime;
    std::string m_reason;

public:

    KickPunisher(int iUserID, bool bForbid, int iForbidTime, std::string reason, int iDelayTime = 0)
        : Punisher(iUserID, iDelayTime), m_bForbid(bForbid), m_iForbidTime(iForbidTime), m_reason(reason) { }

    virtual ~KickPunisher() { }
    int GetForbidTime() const { return m_iForbidTime; }
    std::string GetReason() const { return m_reason; }
    virtual void Process() const;
};


};

#endif
