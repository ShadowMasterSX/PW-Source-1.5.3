#ifndef __ONLINEGAME_GS_OBJ_PROPERTY_AR_H__
#define __ONLINEGAME_GS_OBJ_PROPERTY_AR_H__ 
#include "property.h"
#include <common/packetwrapper.h>

inline archive & operator >>(archive & wrapper, basic_prop &rhs)
{
	return wrapper.pop_back(&rhs,sizeof(rhs));
}


inline archive & operator <<(archive & wrapper, basic_prop &rhs)
{
	return wrapper.push_back(&rhs,sizeof(rhs));
}

inline archive & operator >>(archive & wrapper, extend_prop &rhs)
{
	return wrapper.pop_back(&rhs,sizeof(rhs));
}

inline archive & operator <<(archive & wrapper, const extend_prop &rhs)
{
	return wrapper.push_back(&rhs,sizeof(rhs));
}

inline archive & operator >>(archive & wrapper,item_prop &rhs)
{
	return wrapper.pop_back(&rhs,sizeof(rhs));
}
inline archive & operator <<(archive & wrapper,item_prop &rhs)
{
	return wrapper.push_back(&rhs,sizeof(rhs));
}

inline archive & operator >>(archive & wrapper,enhanced_param &rhs)
{
	return wrapper.pop_back(&rhs,sizeof(rhs));
}
inline archive & operator <<(archive & wrapper,enhanced_param &rhs)
{
	return wrapper.push_back(&rhs,sizeof(rhs));
}

inline archive & operator >>(archive & wrapper,scale_enhanced_param &rhs)
{
	return wrapper.pop_back(&rhs,sizeof(rhs));
}
inline archive & operator <<(archive & wrapper,scale_enhanced_param &rhs)
{
	return wrapper.push_back(&rhs,sizeof(rhs));
}

inline archive & operator >>(archive & wrapper,property_rune &rhs)
{
	return wrapper.pop_back(&rhs,sizeof(rhs));
}
inline archive & operator <<(archive & wrapper,property_rune &rhs)
{
	return wrapper.push_back(&rhs,sizeof(rhs));
}
#endif


