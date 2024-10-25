#ifndef __LOGIN_H
#define __LOGIN_H

#include "security.h"
#include "protocol.h"

namespace GNET
{

#define PROTOCOL_CHALLENGE	10
#define PROTOCOL_RESPONSE	11

class Challenge : public Protocol
{
	Octets time_stamp;
	Protocol *Clone() const { return new Challenge(*this); }
public:
	Challenge(Type type) : Protocol(type) { }
	Challenge(const Challenge &rhs) : Protocol(rhs) { }
	void Setup(size_t size)
	{
		Security *random = Security::Create(RANDOM);
		random->Update(time_stamp.resize(size));
		random->Destroy();
	}
	OctetsStream& marshal(OctetsStream &os) const { return os << time_stamp; }
	const OctetsStream& unmarshal(const OctetsStream &os) { return os >> time_stamp; }
	operator const Octets& () const { return time_stamp; }
	operator Octets& () { return time_stamp; }
	int PriorPolicy() { return 1; }
	void Process(Manager *manager, Manager::Session::ID sid);
};

class Response : public Protocol
{
	Octets identity;
	Octets response;
	Protocol *Clone() const { return new Response(*this); }
public:
	Response(Type type) : Protocol(type) { }
	Response(const Challenge &rhs) : Protocol(rhs) { }
	OctetsStream& marshal(OctetsStream &os) const { return os << identity << response; }
	const OctetsStream& unmarshal(const OctetsStream &os) { return os >> identity >> response; }
	void Setup(const char *name, const char *passwd, Challenge* challenge)
	{
		HMAC_MD5Hash hash;
		hash.SetParameter(Octets(passwd, strlen(passwd)));
		hash.Update(*challenge);
		hash.Final(response);
		identity.replace(name, strlen(name));
	}
	int PriorPolicy() { return 1; }
	void Process(Manager *manager, Manager::Session::ID sid);
};

};

#endif
