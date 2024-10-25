#ifndef __GNET_PHONETOKENCHECKER_H
#define __GNET_PHONETOKENCHECKER_H

#include "matrixchecker.h"

namespace GNET
{

	class PhoneTokenChecker : public MatrixChecker
	{
		Octets	seed;
		int		rtime;//when use cache,consider the time-diff between au and delivery
		Octets	used_elec_number;
	public:
		PhoneTokenChecker() {}	
		PhoneTokenChecker(const PhoneTokenChecker& r) : MatrixChecker(r), seed(r.seed), rtime(r.rtime) {}
		PhoneTokenChecker(unsigned int uid, unsigned int ip, GNET::Octets& _seed, int _rtime) : MatrixChecker(uid,ip), seed(_seed), rtime(_rtime)
		{
		}

		unsigned int GetElecNumber()
		{
			char ret[6];/*ret[0] is the first elec number*/
			unsigned char text[8];
			text[0] = text[4] = rtime & 0xFF;  
			text[1] = text[5] = (rtime>>8) & 0xFF;
			text[2] = text[6] = (rtime>>16) & 0xFF;
			text[3] = text[7] = (rtime>>24) & 0xFF;

			Octets re(text,8);
			HMAC_MD5Hash hash;
			Octets os;
			hash.SetParameter(seed);
			hash.Update(re); 
			hash.Final(os);

			const unsigned char *p = (const unsigned char*)os.begin();
			unsigned char pos = p[15] & 0xf;
			if(pos > 11)
				pos = 11;
			const unsigned char *p2 = p + pos;
			for(int i = 0; i < 3 ; ++i)
			{
				unsigned char c = p2[i];
				unsigned char c1 = (c & 0xf) & 7;
				if( p[14] & (1 << (2*i)) ) ++c1;
				if( p[13] & (1 << (2*i)) ) ++c1;
				unsigned char c2 = ((c >> 4) & 0xf) & 7;
				if( p[14] & (1 << (2*i + 1)) ) ++c2;
				if( p[13] & (1 << (2*i + 1)) ) ++c2;

				ret[i*2] = c1;
				ret[i*2+1] = c2;
			}
			return ret[0]*100000+ret[1]*10000+ret[2]*1000+ret[3]*100+ret[4]*10+ret[5];
		}

		virtual unsigned int Challenge()
		{
			return 0;
		}

		virtual bool Verify( unsigned int response )
		{
			if(seed.size() != 16)
			{
				Log::log(LOG_ERR,"PhoneToken's seed.size invalid,size=%d",seed.size());
				return false;
			}

			unsigned int num = GetElecNumber();
			if(response != num)
			{
				this->rtime -= 1;
				num = GetElecNumber();
				if(response != num)
				{
					this->rtime += 2;
					num = GetElecNumber();
					if(response != num)
					{
						this->rtime -= 3;
						num = GetElecNumber();
						if(response != num)
						{
							this->rtime += 4;
							num = GetElecNumber();
							if(response != num)
								return false;
						}
					}
				}
			}
			ElecNumberConvert(num, used_elec_number);
			return true;
		}

		virtual bool GetUsedElecNumber(Octets & num) 
		{
			if(used_elec_number.size())
			{
				num = used_elec_number;
				return true; 
			}
			else
			{
				return false;
			}
		}

	private:
		void ElecNumberConvert(unsigned int in, Octets & out)
		{
			char num[6];
			int i = 0,j = 100000;
			while(i < 6)
			{
				num[i] = in / j + '0';
				in = (in % j);
				i++;
				j /= 10;
			}
			out.replace(num,sizeof(num));
		}
		virtual int GetType(){ return MC_TYPE_PHONETOKEN; }
	};

}

#endif
