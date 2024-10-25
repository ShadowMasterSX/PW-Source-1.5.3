#ifndef __GNET_BENCODE_V
#define __GNET_BENCODE_V 
#include "variant.h" 
namespace GNET
{
typedef variant_helper::ListNode ListNode;
typedef variant_helper::StringNode StringNode;
typedef variant_helper::DictNode DictNode;
typedef variant_helper::IntNode IntNode;
typedef ListNode::value_type list_type;
typedef DictNode::value_type dict_type;


template <typename T>
bool IsRightNode(variant v)
{
	T* t = dynamic_cast<T*>(v.GetObject());
	return t?true:false;
}
/*
template <typename T>
void* GetValue(variant v)
{
	return &((dynamic_cast<T*>(v.GetObject()))->value);
}
}*/
template <typename T>
typename T::value_type&
GetValue2(variant v)
{
	return (dynamic_cast<T*>(v.GetObject()))->value;
}

template <typename T,typename V>
V& GetValue(variant v)
{
	return (dynamic_cast<T*>(v.GetObject()))->value;
}
}

#endif
