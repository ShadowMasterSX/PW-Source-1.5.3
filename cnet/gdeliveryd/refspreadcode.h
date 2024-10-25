#ifndef _REF_SPREAD_CODE_H_
#define _REF_SPREAD_CODE_H_

namespace GNET
{
	class Octets;
	class RefSpreadCode
	{
	public:
		static void Encode(int userid, int district_id, int roleid, Octets &code);
		static bool Decode(const Octets &code, int &userid, int &district_id, int &roleid);
	};
};
#endif
