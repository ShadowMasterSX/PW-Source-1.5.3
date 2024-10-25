#ifndef __GNET_USBKEY_H
#define __GNET_USBKEY_H

namespace GNET 
{
	class UsbKey
	{
		Octets	seed;//
		Octets 	pin;//size can only be 0 or 4
		int		rtime;//divide au's currenttime by 32(when use cache,consider the time difference between au and delivery)
	public:
		UsbKey() {}	
		UsbKey(Octets& _seed,Octets& _pin,int _rtime):seed(_seed),pin(_pin),rtime(_rtime) {}
		UsbKey(const UsbKey& r):seed(r.seed),pin(r.pin),rtime(r.rtime) {}

		void GetElecNumber(unsigned char ret[6]/*ret[0] is the first elec number*/)
		{
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
		}

		void CalcResponse(Octets& account,Octets& challenge,Octets& response,Octets& used_elec_number/*output*/)
		{
			if(seed.size() != 16)
			{
				Log::log(LOG_ERR,"USBKEY2's seed.size invalid,size=%d", seed.size());
				return;
			}

			//pin can contain number and letter,so is different from elec number
			if(pin.size() != 4 && pin.size() != 0)
			{
				Log::log(LOG_ERR,"USBKEY2's pin.size invalid,size=%d", pin.size());
				return;
			}

			Octets temp;
			if(pin.size() == 4)
			{
				temp.insert(temp.end(),pin.begin(),pin.size());
			}
			unsigned char ret[6];
			GetElecNumber(ret);
			for(int i = 0;i < 6;++i)
			{
				ret[i] += '0';
			}
			used_elec_number.replace(ret,sizeof(ret));
			temp.insert(temp.end(),ret,sizeof(ret));
			/*
			 * 1. first use MD5Hash
			 * 2. second use HMAC_MD5Hash
			 */
			MD5Hash md5;
			Octets passwd;
			md5.Update(account);
			md5.Update(temp);
			md5.Final(passwd);

			HMAC_MD5Hash hash;
			hash.SetParameter(passwd);
			hash.Update(challenge);
			hash.Final(response);
		}
	};
}

#endif
