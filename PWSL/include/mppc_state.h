/*
 *	带有历史的mppc压缩类, 区分流式和非流式两种.  其中非流式是标准的MPPC压缩算法。流式由于加入了结束符，因此是非标准算法。
 *	作者：杨延昭
 *	时间：2009-06
 *	公司：完美时空
*/

#ifndef __MPPC_STATE_H
#define __MPPC_STATE_H

#include <stdint.h>
#include "byteorder.h" 
#include "octets.h"
#include "marshal.h"

namespace GNET
{
class mppc_state
{
public:
	mppc_state(bool is_compress);
	mppc_state(const mppc_state& rhs);

	bool StreamSave(Marshal::OctetsStream &os) const;
	bool StreamLoad(Marshal::OctetsStream &os);
	

	/** 
	* @brief 估算压缩一定长度字节的东西最坏情况下所需字节。用在非流式的压缩。
	* 
	* @param sourcelen	输入长度
	* 
	* @return 　最坏情况下的输出长度
	*/
	static size_t compressBound( size_t sourcelen )
	{
		return (((sourcelen*9)/8)+1)+2+3;
	}
	enum ECODE {			//非流式压缩时的错误码
		E_OK, 			//正确
		E_UNKNOWN = -1, 	//未知
		E_DST_NO_ROOM = -2,	//目标缓冲不足
		E_HISTORY = -3,		//引用到了被清除的历史（或不存在的历史)
	};

	/** 
	* @brief 	非流式压缩或解压
	* 
	* @param dest		目标缓冲
	* @param destLen	存放目标缓冲的长度。压缩时可以用compressBound来估计。
	* @param source		源
	* @param sourceLen	源的长度
	* 
	* @return 	错误码ECODE. 压缩后的长度会存放在destLen中
	*/
	int Update(unsigned char *dest, int *destLen, const unsigned char *source, int sourceLen);

	/** 
	* @brief 	流式压缩或解压
	* 
	* @param in	输入
	* 
	* @return 	输出会被放入in中并返回
	*/
	Octets & StreamUpdate(Octets& in);
private:
	enum {CTRL_OFF_EOB =0, MAX_HIST_LEN = 8192, MAX_HASH_SIZE = 8192};
	unsigned char _hist[MAX_HIST_LEN];
	unsigned short _hash[MAX_HASH_SIZE];
	unsigned short _hist_len;
	bool _compress;

	unsigned char *AppendHistory(const unsigned char *buf, size_t size);
	void AppendHistoryRoll(const unsigned char *buf, size_t size);
	static inline unsigned short HASH( const unsigned char *x){ 
		return 	(((40543*(((((x)[0]<<4)^(x)[1])<<4)^(x)[2]))>>4) & (MAX_HASH_SIZE-1));
	}
	inline unsigned char * GetPredecitAddr (const unsigned char *s)
	{
		unsigned short idx = HASH(s);
		unsigned char *p= _hist + _hash[idx];
		_hash[idx] = (unsigned short) (s - &_hist[0]);
		return p;
	}
	static int CompressBuffer(mppc_state *state, const unsigned char *ibuf, unsigned char *obuf, int isize, int osize);
	static int DecompressBuffer(mppc_state* state, const unsigned char *ibuf, unsigned char *obuf, int isize, int osize);

	static inline void passbits(const unsigned int n, unsigned int& l, unsigned int& blen);
	static inline unsigned int fetch(const unsigned char *&buf, unsigned int& l);

	//流式压缩所需要的
	unsigned char *_histptr;
	unsigned int _legacy_in_size;

	unsigned char* compress_block( unsigned char *obuf, size_t isize );
	Octets& CompressUpdate(Octets &in);
	Octets& CompressFinal(Octets &in);

	//解压流式所需要的
	unsigned int _l, _adjust_l;
	Octets   _legacy_in;

	unsigned char *_rptr;
	unsigned char *_adjust_rptr;
	unsigned int _blen, _blen_total;
	bool passbits(const unsigned int n);
	unsigned int fetch();
	Octets& DecompressUpdate(Octets &in);
};
}
#endif
