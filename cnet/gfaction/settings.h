#ifndef __GNET_SETTINGS_H
#define __GNET_SETTINGS_H

namespace GNET
{
	class FactionConfig{
	public:
		enum{
			MAX_LEVEL        = 10,
			MAX_TERRITORY    = 43,
		};
		static int UpgradeCost(int level);
		static int MaxMember(int level);
		static int MaxRole(int role);
	};


}
#endif
