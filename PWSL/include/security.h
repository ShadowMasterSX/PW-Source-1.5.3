/*
 *	用于协议网络的加密模块，NetSession中包含了加密和解密的相关模块
 *	作者：未知
 *	修改：2009-07 杨延昭 增加了用于分组压缩加密的加密类
 *	公司：完美时空
 */


#ifndef __ONLINEGAME_SECURITY_H__
#define __ONLINEGAME_SECURITY_H__

#include <map>
#include <assert.h>
#include "octets.h"
#include "marshal.h"

namespace GNET
{
enum { 
	RANDOM = 0, 
	NULLSECURITY = 1, 
	ARCFOURSECURITY = 2, 
	MD5HASH = 3, 
	HMAC_MD5HASH = 4, 
	COMPRESS_MPPC = 5,			//MPPC不带历史状态的
	UNCOMPRESS_MPPC = 6,
	SHA1HASH = 7,
	COMPRESS_ZLIB = 8,			//ZLIB非流式的
	UNCOMPRESS_ZLIB = 9,
	COMPRESS_STREAM_MPPC = 12,		//MPPC流式带历史
	UNCOMPRESS_STREAM_MPPC = 13,
	RC4_MPPC_STREAM_COMPRESS = 14,		//已经封装好的rc4 +压缩 二级加密类
	RC4_MPPC_STREAM_UNCOMPRESS = 15,
	TWOLEVEL_SECURITY = 16,			//内部使用
	CC_SECURITY 	= 17,			//内部使用
	UNCC_SECURITY 	= 18,			//内部使用
	AES_CTR_SECURITY = 19,

	COMPRESSARCFOURSECURITY = 14,
	DECOMPRESSARCFOURSECURITY = 15,
};

class Octets;
class TwoLevelSecurity;
class Security
{
public:
	typedef unsigned int Type;
protected:
	Type type;
	virtual ~Security() { }
	Security(Type t) : type(t)
	{
	}
public:
	Security(const Security &rhs): type(rhs.type)
	{
	}
	inline Type GetType() const { return type;}
	virtual void SetParameter(const Octets &) { }
	virtual void GetParameter(Octets &) { }
	virtual bool SetExraParameter(int type, const Octets &) { return true; }		//设置额外的参数，目前只有AES_CTR用到了，对于AES_CTR:type 0 应传入16字节counter, type 1:应传入16字节iv 不调用则从密码中生成
	virtual Octets& Update(Octets &) = 0;
	virtual void Update(void *o, size_t o_size) { assert(false); }
	virtual Octets& Final(Octets &o) { return o; }
	virtual void Final (void *o, size_t o_size) { }
	virtual Security *Clone() const = 0;
	virtual void Destroy() { delete this; }
	static Security* Create(Type type);

	static Security * MakeCompressCrypt(Type compress, Type crypt);
	static Security * MakeUnCompressDeCrypt(Type decompress, Type decrypt);

	static Security * MakeStreamCompressCrypt(Type compress, Type crypt);
	static Security * MakeUnStreamCompressDeCrypt(Type decompress, Type decrypt);

	static Security * LinkSecurity(Security * first, Security * second, bool encode_or_decode/*true:encode, false:decode*/);	//创建一个适合分组压缩和分组加密的 二级加密对象
	static Security * StreamLinkSecurity(Security * first, Security * second);							//创建一个适合流压缩和流加密的 二级加密对象

	static float GetCompressRatio(uint64_t *src_len = NULL, uint64_t *dest_len = NULL);	//获取压缩率

	//Security的Save& Load仅限内部使用，因为这些代码不是异常安全的，所以要在高层保证正确性
	void Save(Marshal::OctetsStream & os)
	{
		os << type;
		OnSave(os);
	}
	static Security * Load(Marshal::OctetsStream & os) 
	{
		Type type;
		os >> type;
		Security * sec = Create(type);
		sec->OnLoad(os);
		return sec;
	}

	virtual void OnSave(Marshal::OctetsStream & os) const =0;
	virtual void OnLoad(Marshal::OctetsStream & os) = 0;

};

class NullSecurity : public Security
{
	Security *Clone() const { return new NullSecurity(*this); }
public:
	NullSecurity():Security(NULLSECURITY){ }
	NullSecurity(Type type) : Security(type) { }
	NullSecurity(const NullSecurity &rhs) : Security(rhs) { }
	inline Octets& Update(Octets &o) { return o; }
	virtual void OnSave(Marshal::OctetsStream & os) const {}
	virtual void OnLoad(Marshal::OctetsStream & os) {}
};


class TwoLevelSecurity : public Security
{
protected:
	Security *_first;
	Security *_second;

	Security * Clone() const{ return new TwoLevelSecurity(*this);}
public:
	TwoLevelSecurity(Type t):Security(t),_first(NULL),_second(NULL) {}	//用于串行化的创建，内部使用了
	TwoLevelSecurity(Type t1, Type t2);
	TwoLevelSecurity(const TwoLevelSecurity& rhs):Security(rhs),_first(rhs._first->Clone()), _second(rhs._second->Clone()) {}
	TwoLevelSecurity(Security* t1, Security* t2):Security(TWOLEVEL_SECURITY) { _first = t1; _second = t2;}
	virtual ~TwoLevelSecurity()
	{
		_first->Destroy();
		_second->Destroy();
	}
	virtual Octets& Update(Octets &o)
	{
		//只有t1,t2都满足流式算法的要求，才可这样调用
		_first->Update(o);
		_first->Final(o);

		_second->Update(o);
		_second->Final(o);
		return o;
	}
	void SetParameter(const Octets &param)
	{
		_first->SetParameter(param);
		_second->SetParameter(param);
	}
	virtual void OnSave(Marshal::OctetsStream & os) const 
	{
		_first->Save(os);
		_second->Save(os);
	}
	virtual void OnLoad(Marshal::OctetsStream & os) 
	{
		ASSERT(_first ==NULL && _second == NULL);
		_first = Load(os);
		_second = Load(os);
	}
};

}

#endif

