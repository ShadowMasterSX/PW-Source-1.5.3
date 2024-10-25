#ifndef _GNET_MEMFUNC_H
#define _GNET_MEMFUNC_H
namespace GNET
{
class abstract_function_t
{
public:
	virtual ~abstract_function_t() {}
};
typedef abstract_function_t* memfunc_pointer_t;

template<typename _Res, typename _Tp>
class unary_function_t : public abstract_function_t
{
private:
	_Res (_Tp::* __M_f)() const;
public:
	unary_function_t(_Res (_Tp::* _pf)() const) :  __M_f(_pf) { }
	~unary_function_t() {}
	_Res operator()(const _Tp* _tp)
   	{ return (_tp->*__M_f)(); }
};

template<typename _Res, typename _Arg, typename _Tp>
class binary_function_t : public abstract_function_t
{
private:
	_Res (_Tp::* __M_f)(_Arg);
public:
	binary_function_t(_Res (_Tp::* _pf)(_Arg)) :  __M_f(_pf) { }
	~binary_function_t() {}
	_Res operator()(_Tp* _tp,_Arg _arg)
   	{ return (_tp->*__M_f)(_arg); }
};

template<typename _Res, typename _Arg, typename _Tp>
class const_binary_function_t : public abstract_function_t
{
private:
	_Res (_Tp::* __M_f)(_Arg) const;
public:
	const_binary_function_t(_Res (_Tp::* _pf)(_Arg) const) :  __M_f(_pf) { }
	~const_binary_function_t() {}
	_Res operator()(_Tp* _tp,_Arg _arg) const
   	{ return (_tp->*__M_f)(_arg); }
};

template <typename _Res,typename _Tp>
inline unary_function_t<_Res,_Tp> mem_fun_r(_Res (_Tp::* _pf)() const)
{
	return unary_function_t<_Res,_Tp>(_pf);
}

template <typename _Res,typename _Tp>
inline unary_function_t<_Res,_Tp>* mem_fun_p(_Res (_Tp::* _pf)() const)
{
	return new unary_function_t<_Res,_Tp>(_pf);
}

template <typename _Res,typename _Arg,typename _Tp>
inline binary_function_t<_Res,_Arg,_Tp> mem_fun_r(_Res (_Tp::* _pf)(_Arg))
{
	return binary_function_t<_Res,_Arg,_Tp>(_pf);
}

template <typename _Res,typename _Arg,typename _Tp>
inline binary_function_t<_Res,_Arg,_Tp>* mem_fun_p(_Res (_Tp::* _pf)(_Arg))
{
	return new binary_function_t<_Res,_Arg,_Tp>(_pf);
}

template <typename _Res,typename _Arg,typename _Tp>
inline const_binary_function_t<_Res,_Arg,_Tp> mem_fun_r(_Res (_Tp::* _pf)(_Arg) const)
{
	return const_binary_function_t<_Res,_Arg,_Tp>(_pf);
}

template <typename _Res,typename _Arg,typename _Tp>
inline const_binary_function_t<_Res,_Arg,_Tp>* mem_fun_p(_Res (_Tp::* _pf)(_Arg) const)
{
	return new const_binary_function_t<_Res,_Arg,_Tp>(_pf);
}

}; //end of namespace
#endif
