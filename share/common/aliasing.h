

namespace __aliasing_cast_helper
{

template<typename U, typename V>
union TypeAliasing
{
	U u;
	V v;
	TypeAliasing(V _v) : v(_v) { }
	operator U () const { return u; }
};

}

template<typename U, typename V>
inline U aliasing_cast(V v)
{
	__aliasing_cast_helper::TypeAliasing<U, V> data(v);
	return data;
}

