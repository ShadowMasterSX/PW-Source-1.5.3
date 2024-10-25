// PWPasswordCheck.h: interface for the PWPasswordCheck class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PWPASSWORDCHECK_H__9BB4BEDE_0172_4D4A_BDEB_F44D98B52098__INCLUDED_)
#define AFX_PWPASSWORDCHECK_H__9BB4BEDE_0172_4D4A_BDEB_F44D98B52098__INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "security.h"

class CPWPasswordCheck  
{
	class MD5Hash
	{
	public:
		static void Digest( const unsigned char* pData, int nCount, char* pOut)
		{
			GNET::MD5Hash 	sec;
			GNET::Octets	oct( pData, nCount); 
			sec.Update( oct);
			sec.Final( oct);
			memcpy( pOut, oct.begin(), oct.size());
		}
	};
	enum
	{
		SIZE_MD5VALUE		= 16,
		SIZE_BUFFERSTIRNG	= 32,
	};
	
	inline static unsigned short MAKEWORD( char a, char b)
	{
		return ((unsigned short)(((unsigned char)(b)) | ((unsigned short)((unsigned char)(a))) << 8));
	}

	inline static int GetMergeByte( char a, char b, char c, char d)
	{
		return ((MAKEWORD( a, b) << 16) | MAKEWORD( c, d));
        }
	inline static int GetMergeByteAbs( char a, char b, char c, char d)
	{
		return abs( GetMergeByte( a, b, c, d));
	}

	static int TransformKey0( int nCode, int nKey)
	{
		char    szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];

		sprintf( szBuffer, "%d:%d", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);
		
		int nLastValue = 0;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByteAbs( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);
			nTemp = nTemp / 2 + nLastValue / 2;
			nLastValue ^= ~nTemp;
		}
		return abs( nLastValue);
	}
	static int TransformKey1( int nCode, int nKey)
	{
		char	szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];
		
		sprintf( szBuffer, "%d==%d", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);
		
		int nLastValue = 0;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByteAbs( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);

			nLastValue = nTemp / 2 + nLastValue / 2;
			nLastValue ^= nTemp;
		}
		return abs( nLastValue);
	}
	static int TransformKey2( int nCode, int nKey)
	{
		char	szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];
		
		sprintf( szBuffer, "%d*%d", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);
	
		int nLastValue = 0;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByteAbs( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);

			nTemp ^= nLastValue;
			nLastValue += nTemp / 4;
		}
			
		return abs( nLastValue);
	}
	static int TransformKey3( int nCode, int nKey)
	{
		char	szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];
		
		sprintf( szBuffer, "%d-%d", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);
		
		int nLastValue = 0;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByteAbs( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);
			nTemp ^= nLastValue;
			nLastValue += nTemp;
		}
		return abs( nLastValue);
	}
	static int TransformKey4( int nCode, int nKey)
	{
		char	szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];
		
		sprintf( szBuffer, "[%d][%d]", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);

		int nLastValue = nKey;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByte( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);
			nLastValue = nTemp / 2 + nLastValue / 2;
			nLastValue ^= nTemp;
		}
		
		return abs( nLastValue);
	}
	static int TransformKey5( int nCode, int nKey)
	{
		char	szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];
		
		sprintf( szBuffer, "%d&%d", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);
		
		int nLastValue = 0;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByte( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);
			nLastValue += nTemp / 4;
			nLastValue ^= nTemp;
		}
		
		return abs( nLastValue);
	}
	static int TransformKey6( int nCode, int nKey)
	{
		char	szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];
		
		sprintf( szBuffer, "{%d}{%d}", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);
		
		int nLastValue = 0;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByteAbs( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);
			nLastValue += nTemp / 4;
		}
		
		return abs( nLastValue);
	}
	static int TransformKey7( int nCode, int nKey)
	{
		char	szBuffer[SIZE_BUFFERSTIRNG];
		char	bbValue[SIZE_MD5VALUE];
		
		sprintf( szBuffer, "[%d]-[%d]", nCode, nKey);
		MD5Hash::Digest( (const unsigned char *)szBuffer, strlen( szBuffer), bbValue);
		
		int nLastValue = 0;
		for( int i = 0; i < ( SIZE_MD5VALUE / 4); i ++)
		{
			int nTemp = GetMergeByte( bbValue[i*4], bbValue[i*4 + 1], bbValue[i*4 + 2], bbValue[i*4 + 3]);
			nLastValue ^= nTemp;
		}
		
		return abs( nLastValue);
	}	
	class TransformKey
	{
	private:
		TransformKey()
		{
			m_pFuncs[0] = TransformKey0;
			m_pFuncs[1] = TransformKey1;
			m_pFuncs[2] = TransformKey2;
			m_pFuncs[3] = TransformKey3;
			m_pFuncs[4] = TransformKey4;
			m_pFuncs[5] = TransformKey5;
			m_pFuncs[6] = TransformKey6;
			m_pFuncs[7] = TransformKey7;
		}
		virtual ~TransformKey()
		{
		}
	private:
		enum
		{
			COUNT_TRANSFORMKEY	= 8,
		};
		typedef int (*PTKF)( int nCode, int nKey);
		PTKF	m_pFuncs[COUNT_TRANSFORMKEY];
	public:
		static bool IsMethodExist( int nMethod)
		{
			return nMethod >= 0 && nMethod < COUNT_TRANSFORMKEY;
		}
		int Transform( int nCode, int nKey, int nMethod)
		{
			return (m_pFuncs[nMethod])( nCode, nKey);
		}
		static TransformKey& GetInstance()
		{
			static TransformKey tkClass;
			return tkClass;
		}
	};
private:
	CPWPasswordCheck()
	{
	}
	virtual ~CPWPasswordCheck()
	{
	}
public:
	enum
	{
		ERROR_OK			= 0,
		ERROR_LOST_METHOD	= -1,
		ERROR_UNMATCH		= -2,
	};
	static int check( int nRandom, int nKey, int nMethod, int nResult)
	{
		if( !TransformKey::IsMethodExist( nMethod))
			return ERROR_LOST_METHOD;
		
		TransformKey& tk = TransformKey::GetInstance();
		if( tk.Transform( nRandom, nKey, nMethod) == nResult)
			return ERROR_OK;
		else	
			return ERROR_UNMATCH;
	}
};

#endif // !defined(AFX_PWPASSWORDCHECK_H__9BB4BEDE_0172_4D4A_BDEB_F44D98B52098__INCLUDED_)
