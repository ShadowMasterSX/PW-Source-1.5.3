#ifndef __GNET_PROTOSTAT
#define __GNET_PROTOSTAT

namespace GNET
{

struct ProtoStat
{
	int gamedatasend;
	short keepalive;
	short acreport;

	ProtoStat() : gamedatasend(0), keepalive(0), acreport(0) { }
	ProtoStat(int _gamedatasend, short _keepalive, short _acreport )
		: gamedatasend(_gamedatasend), keepalive(_keepalive), acreport(_acreport) { }
	bool operator==(const ProtoStat &ps) const { return this == &ps ||
		gamedatasend == ps.gamedatasend && keepalive == ps.keepalive && acreport == ps.acreport; }
};

};

#endif
