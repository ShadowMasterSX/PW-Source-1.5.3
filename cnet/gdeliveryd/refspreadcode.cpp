#include "refspreadcode.h"
#include "../common/octets.h"
#include <string.h>
#include <stdio.h>

namespace GNET
{
	static char str_platform[] = "WG";

	static int GetStrSum(const char *str, size_t sz)
	{
		int sum = 0;
		for (size_t i = 0; i < sz; ++i)
			sum += str[i];
		return sum;
	}

	//生成验证码
	static void GetCheckSum(const Octets &oct_id, Octets &oct_checksum)
	{
		const char *str_id = (const char *)oct_id.begin();
		int SA = GetStrSum(str_id, 2);
		str_id += 2;
		int SB = GetStrSum(str_id, 10);
		str_id += 10;
		int SC = GetStrSum(str_id, 2);
		str_id += 2;
		int SD = GetStrSum(str_id, 10);

		char str_checksum[16] = {0};
		snprintf(str_checksum, 16, "%x", SB^SD);
		size_t len = strlen(str_checksum);
		snprintf(str_checksum+len, 16-len, "%x", SA^SC);
		len = strlen(str_checksum);
		snprintf(str_checksum+len, 16-len, "%x", SA|SB);
		len = strlen(str_checksum);
		snprintf(str_checksum+len, 16-len, "%x", SC&SD);
		len = strlen(str_checksum);
		if (len < 6)
		{
			for (int i = len; i < 6; ++i)
				str_checksum[i] = '0';
		}
		oct_checksum.reserve(6);
		oct_checksum.insert(oct_checksum.begin(), str_checksum, 6);
	}

	#define SWAP_CHAR(A, B) (void)((A)=(A)^(B), (B)=(A)^(B), (A)=(A)^(B))
	static void encrypt(char *begin, size_t len)
	{
		for (size_t i = 0; i < len/2; i+=2)
			SWAP_CHAR(begin[i], begin[len-i-1]);

		for (size_t i = 0; i < len-5; i+=6)
		{
			SWAP_CHAR(begin[i], begin[i+3]);
			SWAP_CHAR(begin[i+1], begin[i+4]);
			SWAP_CHAR(begin[i+2], begin[i+5]);
		}

		for (size_t i = 0; i < len-3; i+=4)
		{
			SWAP_CHAR(begin[i], begin[i+3]);
			SWAP_CHAR(begin[i+1], begin[i+2]);
		}
	}

	static void decrypt(char *begin, size_t len)
	{
		for (size_t i = 0; i < len-3; i+=4)
		{
			SWAP_CHAR(begin[i], begin[i+3]);
			SWAP_CHAR(begin[i+1], begin[i+2]);
		}

		for (size_t i = 0; i < len-5; i+=6)
		{
			SWAP_CHAR(begin[i], begin[i+3]);
			SWAP_CHAR(begin[i+1], begin[i+4]);
			SWAP_CHAR(begin[i+2], begin[i+5]);
		}

		for (size_t i = 0; i < len/2; i+=2)
			SWAP_CHAR(begin[i], begin[len-i-1]);
	}

	void RefSpreadCode::Encode(int userid, int district_id, int roleid, Octets &code)
	{
		char str_userid[11] = {0};
		snprintf(str_userid, 11, "%d", userid);
		size_t len = strlen(str_userid);
		if (len < 10)
		{
			int i = 9, j = len-1;
			for (; j>=0; --i, --j)
				str_userid[i] = str_userid[j];
			for (; i>=0; --i)
				str_userid[i] = '0';
		}

		char str_distid[3] = {0};
		snprintf(str_distid, 3, "%d", district_id);
		len = strlen(str_distid);
		if (len < 2)
		{
			int i = 1, j = len-1;
			for (; j>=0; --i, --j)
				str_distid[i] = str_distid[j];
			for (; i>=0; --i)
				str_distid[i] = '0';
		}

		char str_roleid[11] = {0};
		snprintf(str_roleid, 11, "%d", roleid);
		len = strlen(str_roleid);
		if (len < 10)
		{
			int i = 9, j = len-1;
			for (; j>=0; --i, --j)
				str_roleid[i] = str_roleid[j];
			for (; i>=0; --i)
				str_roleid[i] = '0';
		}

		code.clear();
		code.reserve(30);
		code.insert(code.end(), str_platform, strlen(str_platform));
		code.insert(code.end(), str_userid, strlen(str_userid));
		code.insert(code.end(), str_distid, strlen(str_distid));
		code.insert(code.end(), str_roleid, strlen(str_roleid));
		Octets oct_checksum;
		GetCheckSum(code, oct_checksum);
		code.insert(code.end(), oct_checksum.begin(), oct_checksum.size());
		encrypt((char *)code.begin(), 24);
	}

	bool RefSpreadCode::Decode(const Octets &spread_code, int &userid, int &district_id, int &roleid)
	{
		Octets code = spread_code;
		if (code.size() != 30) return false;
		decrypt((char *)code.begin(), 24);
		if (strncmp((char *)code.begin(), str_platform, 2) != 0)
			return false;
		Octets oct_checksum;
		GetCheckSum(code, oct_checksum);
		if (strncmp((char *)oct_checksum.begin(), (char *)code.begin()+24, 6) != 0)
			return false;

		char id[11] = {0};
		const char *begin = (const char *)code.begin();
		begin += 2;
		strncpy(id, begin, 10);
		id[10] = 0;
		userid = atoi(id);
		begin += 10;
		strncpy(id, begin, 2);
		id[2]= 0;
		district_id = atoi(id);
		begin += 2;
		strncpy(id, begin, 10);
		id[10] = 0;
		roleid = atoi(id);
		return true;
	}
};

#ifdef __TEST__
using namespace GNET;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char **argv)
{
	if (argc!=4 && argc!=3)
	{
		printf("Usage:\t %s userid distid roleid\n", argv[0]);
		printf("\t %s -d refcode\n", argv[0]);
		return 1;
	}
	if (argc==3)
	{
		if (strncmp(argv[1], "-d", 2) != 0)
		{
			printf("Usage: %s -d refcode\n", argv[0]);
			return 1;
		}
	}

	if (argc == 4)
	{
		Octets code;
		int userid = atoi(argv[1]);
		int distid = atoi(argv[2]);
		int roleid = atoi(argv[3]);
		RefSpreadCode::Encode(userid, distid, roleid, code);
		printf("%s\n", (char *)code.begin());
	}
	else
	{
		Octets code(argv[2], strlen(argv[2]));
		int userid, distid, roleid;
		if (RefSpreadCode::Decode(code, userid, distid, roleid))
		{
			printf("userid=%d, districtid=%d, roleid=%d\n", userid, distid, roleid);
		}
		else
		{
			printf("Invalid refcode\n");
		}
	}
	return 0;
}
#endif
