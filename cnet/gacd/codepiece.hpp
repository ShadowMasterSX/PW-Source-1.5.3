#ifndef __GNET_CODEPIECE_HPP
#define __GNET_CODEPIECE_HPP

#include <vector>
#include <string>
#include "octets.h"

namespace GNET
{

class CodePiece
{
	Octets m_data;

	struct Header
	{
		unsigned short m_usSize;
		short m_sID;
		short m_sType;
	};

	Header* GetHeader() { return (Header*)m_data.begin(); }
	void* InnerDataBegin() { return ((char*)m_data.begin())+sizeof(Header); }
	const Header* GetHeader() const { return (const Header*)m_data.begin(); }
public:
	enum CodePieceType { CPT_DATA = 0, CPT_RUN = 2, CPT_RUN_IN_THREAD = 3 };
public:
	CodePiece() { }
	CodePiece(Octets os) { SetData(os); }
	CodePiece(const std::string &str) { SetData(str); }
	CodePiece(short sID, const std::string &str, bool bHex = false) { SetData(sID, str, bHex); }
	CodePiece(const CodePiece &cp) : m_data(cp.m_data) { }
	~CodePiece() { }

	CodePiece& operator=(const CodePiece &cp) { m_data = cp.m_data; return *this; }
	bool operator==(const CodePiece &cp) const { return this == &cp || m_data==cp.m_data; }

	void SetData(const Octets &os); 
	void SetData(const std::string &str); 
	void SetData(short sID, const std::string &str, bool bHex = false);
	void MakeIntDataPiece(short sID, int iValue);

	CodePiece& ChangeID(short sNewID);
	CodePiece& PatchInt(int iPos, int iValue);
	CodePiece& PatchShort(int iPos, short sValue);
	CodePiece& PatchData(int iPos, const void *pValue, unsigned short usSize);

	short GetID() const { return GetHeader()->m_sID; }
	short GetType() const { return GetHeader()->m_sType; }
	bool IsRunType() const { return GetType() == CPT_RUN || GetType() == CPT_RUN_IN_THREAD; }
	size_t GetSize() const { return m_data.size(); }
	Octets GetData() const { return m_data; }
	std::string GetStr() const;

	bool IsEmpty() const { return m_data.size() == 0; }
	void Clear() { m_data.clear(); }
};

typedef std::vector<CodePiece> CodePieceVector;

};

#endif
