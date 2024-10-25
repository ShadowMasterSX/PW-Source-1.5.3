#include <netinet/in.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdint.h>

#include "hashtab.h"
#include "astring.h"
#include "base64.h"
#include "strfunc.h"

#include "cert.h"
#pragma pack(1)
struct certificate_header
{
	int32_t	stime;
	int32_t etime;
	char 	key[cert::KEY_LENGTH];
};
struct cerificate_vstring
{
	int16_t len;
	char 	buf[0];
};
#pragma pack()

class certXImp
{
	typedef abase::hashtab<abase::string,abase::string,abase::_hash_function> 	table_type;

	certXImp():_start(0),_end(0),_author(10),_acceptor(10){
		memset(_key,0,sizeof(_key));
	}

	time_t 	_start;
	time_t 	_end;
	char 	_key[cert::KEY_LENGTH + 1];
	table_type _author;
	table_type _acceptor;

	int  _M_parseAuthorVString(char * buf, int len)
	{
		_author.clear();
		return _M_parseVString(_author,buf,len);
	}
	int _M_parseAcceptorVString(char * buf, int len)
	{
		_acceptor.clear();
		return _M_parseVString(_acceptor,buf,len);
	}
	int _M_dumpAuthorVString(char * buf, int len)
	{
		return _M_dumpVString(_author,buf,len);
	}
	int _M_dumpAcceptorVString(char *buf, int len)
	{
		return _M_dumpVString(_acceptor,buf,len);
	}
	
	int _M_parseVString(table_type & tab, char * buf, int len);
	int _M_dumpVString(table_type & tab, char * buf, int len);
	struct dumper{
		char *  buf;
		int 	len;
		int 	maxsize;
		int 	state;
		dumper(char * buf,int size):buf(buf),maxsize(size){
			len = 0;
			state = 0;
		}
		int operator() (const char * key, const char * val){
			if(state < 0) return -2;
			if(len + (int)strlen(key) + (int)strlen(val) + 1 >= maxsize) {
				state = -1;
				return -1;
			}
			char * tmp = buf + len;
			strcpy(tmp,key);
			strcat(tmp,"=");
			strcat(tmp,val);
			len += strlen(tmp) + 1;
			return 0;
		}
		
	};
	friend class cert;
};

int certXImp::_M_parseVString(certXImp::table_type &tab, char * buf, int length)
{
	int len;
	char *str;
	str = buf;
	while((len = strlen(str))>0){
		char* tmp = strchr(str,'=');
		if(tmp == NULL){
			return -2;
		}
		*tmp = 0;
		tab.put(str,tmp+1);
		str += len + 1;
	}
	return 0;
}

int certXImp::_M_dumpVString(certXImp::table_type &tab, char * buf, int length)
{
	dumper dp(buf,length);
	tab.enum_element(dp);
	if(dp.state < 0) return -2;
	return dp.len;
}

cert::cert()
{
	_imp = new certXImp;
}

cert::~cert()
{
	delete _imp;
}

int cert::setStart(time_t __start)
{
	_imp->_start = __start;
	return 0;
}

int cert::setEnd(time_t __end)
{
	_imp->_end = __end;
	return 0;
}

int cert::genKey()
{
	char *char_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_";
	int  len = strlen(char_table);
	unsigned char key[KEY_LENGTH];
	RAND_bytes(key,KEY_LENGTH);
	for(int i = 0; i< KEY_LENGTH;i++)
	{
		_imp->_key[i] = char_table[key[i] % len];
	}
	return 0;
}

int cert::setKey(const char *__key)
{
	strncpy(_imp->_key,__key,KEY_LENGTH);
	return 0;
}

int cert::setAuthor(const char * __key, const char * __value)
{
	abase::pair<abase::string *,bool> p = _imp->_author.get(__key);
	if(!p.second){
		_imp->_author.put((char*)__key,(char*)__value);
	}
	else
	{
		*p.first = (char*)__value;
	}
	return 0;
}

int cert::setAcceptor(const char *__key, const char * __value)
{

	abase::pair<abase::string *,bool> p = _imp->_acceptor.get(__key);
	if(!p.second){
		_imp->_acceptor.put((char*)__key,(char*)__value);
	}
	else
	{
		*p.first = (char*)__value;
	}
	return 0;
}

time_t cert::getStart()
{
	return _imp -> _start;
}

time_t cert::getEnd()
{
	return _imp -> _end;
}

const char * cert::getKey()
{
	return _imp->_key;
}

const char * cert::getAuthor(const char *__key)
{
	abase::pair<abase::string *,bool> p = _imp->_author.get(__key);
	if(p.second){
		return *p.first;
	}

	return NULL;
}

const char * cert::getAcceptor(const char *__key)
{
	abase::pair<abase::string *,bool> p = _imp->_acceptor.get(__key);
	if(p.second){
		return *p.first;
	}

	return NULL;
}

int cert::enumAuthor(enumcb & cb)
{
	_imp->_author.enum_element(cb);
	return 0;
}

int cert::enumAcceptor(enumcb & cb)
{
	_imp->_acceptor.enum_element(cb);
	return 0;
}

int cert::load(const char * __buf, int __size)
{
	int rst;
	certificate_header * header = (certificate_header*) __buf;
	_imp->_start = ntohl(header->stime);
	_imp->_end = ntohl(header->etime);
	memcpy(_imp->_key,header->key,KEY_LENGTH);

	char * tmp = (char*)__buf + sizeof(certificate_header);
	cerificate_vstring *vstr = (cerificate_vstring *)tmp;
	int size = ntohs(vstr->len);
	rst = _imp->_M_parseAuthorVString(vstr->buf,size);
	if(rst) return rst;
	tmp += size + sizeof(vstr->len);
	vstr = (cerificate_vstring *)tmp;
	size = ntohs(vstr->len);
	rst = _imp->_M_parseAcceptorVString(vstr->buf,size);
	if(rst) return rst;
	return 0;
}

int cert::save(char * __buf, int __size)
{
	int rst;
	certificate_header * header = (certificate_header*) __buf;
	header->stime = htonl(_imp->_start);
	header->etime = htonl(_imp->_end);
	memcpy(header->key,_imp->_key,KEY_LENGTH);

	char * tmp = (char*)__buf + sizeof(certificate_header);
	cerificate_vstring *vstr = (cerificate_vstring *)tmp;
	int size = __size - (tmp - __buf) - sizeof(vstr->len);
	rst = _imp->_M_dumpAuthorVString(vstr->buf,size-1);
	if(rst < 0) return rst;
	vstr->buf[rst] = 0;
	rst ++;
	vstr->len = htons(rst);
	tmp += rst + sizeof(vstr->len);
	vstr = (cerificate_vstring *)tmp;
	size = __size - (tmp - __buf) - sizeof(vstr->len);
	rst = _imp->_M_dumpAcceptorVString(vstr->buf,size-1);
	if(rst < 0) return rst;
	vstr->buf[rst] = 0;
	rst ++;
	vstr->len = htons(rst);
	tmp += rst + sizeof(vstr->len);
	return tmp - __buf;
}

int certfile::loadFile(const char * filename)
{
	ASSERT(_cert);

//read data from file
	FILE * file = fopen(filename,"r");
	if(NULL == file) return -5;
	char linebuf[128];
	char buf[4096];
	int len = 0;
	buf[0] = 0;
	int state = 0;
	fgets(linebuf,sizeof(linebuf),file);	//ignore first line
	do
	{
		fgets(linebuf,sizeof(linebuf),file);
		if(feof(file) || ferror(file)) {
			state = -1;
			break;
		}
		if(*linebuf <=32 || *linebuf == '-'){
			break;
		}
		trimright(linebuf);
		int tmp = strlen(linebuf);
		if(len + tmp >= (int)sizeof(buf)){
			state = -2;
			break;
		}
		strcat(buf+len,linebuf);
		len += tmp;
	}while(1);
	fclose(file);
	if(state < 0) return state;
	if(len % 4 != 0) return -3;

//read file complete, base64 decode
	char buf2[4096];
	int rst = base64_decode(buf,len,(unsigned char *)buf2);
//verify data and signature size only , not verify signature here  
	int datlen = ntohs(*(uint16_t *)buf2);
	char *dat = buf2 + sizeof(uint16_t);
	if(datlen == 0 || datlen > rst - 2*(int)sizeof(uint16_t)) return -4;
	int siglen = ntohs(*(uint16_t*)(buf2 + datlen + sizeof(uint16_t)));
	char *sig = buf2 + sizeof(uint16_t) + datlen + sizeof(uint16_t); 
	if(datlen + siglen + 2*(int)sizeof(uint16_t) != rst) return -5;
//store sig and data
	setSignature(sig,siglen);
	if(_datbuf) delete _datbuf;
	_datbuf = new char[datlen];
	memcpy(_datbuf,dat,datlen);
	_datsize = datlen;
//load data to cert object,_cert->load will change dat  parameter
	if((rst = _cert->load(dat,datlen)) < 0) {
		return -6;
	}
	return 0;
}

int certfile::saveFile(const char * filename)
{	
	if(!_sigbuf || !_datbuf) return -1;
	FILE *file = fopen(filename,"w");
	if(!file) return -2;
	char buf[MAX_SIZE];
	int  offset = 0;
//handle data
	memcpy(buf + sizeof(uint16_t),_datbuf,_datsize);
	offset = _datsize; 
	*(uint16_t*)(buf) = htons(offset);
	offset += sizeof(uint16_t);
//handle sig
	*(uint16_t*)(buf + offset) = htons(_sigsize);
	memcpy(buf + offset + sizeof(uint16_t),_sigbuf,_sigsize);
	offset += _sigsize + sizeof(uint16_t);
//base64 encode
	char buf2[MAX_SIZE];
	int rst = base64_encode((unsigned char *)buf, offset,buf2);
	fprintf(file,"------ Epie BEGIN CERTIFICATE ------\n");
	offset = 0;
	do
	{
		char linebuf[128];
		int len = 64;
		if(len > rst) len = rst;
		memcpy(linebuf,buf2+offset,len);
		linebuf[len] = 0;
		fprintf(file,"%s\n",linebuf);
		offset += len;
		rst -= len;
	}while(rst > 0);
	fprintf(file,"------ Epie END CERTIFICATE ------\n");
	fclose(file);
	return 0;
}


int certfile::refreshData()
{
	if(_datbuf) delete _datbuf;
	_datbuf = new char[MAX_SIZE];
	_datsize = _cert->save(_datbuf,MAX_SIZE);
	if(_datsize < 0){
		delete _datbuf;
		_datbuf = NULL;
		return _datsize;
	}
	return 0;
}

int certfile::storeData()
{
	int rst;
	char buf[4096];
	if((unsigned int )_datsize > sizeof(buf)) return -1;
	memcpy(buf,_datbuf,_datsize); //cert的load会改变参数内容
	if((rst = _cert->load(buf,_datsize)) < 0){
		return -2;
	}
	return 0;
}

int certfile::sign()
{
	if(!_prikey) return -1;
	EVP_MD_CTX     md_ctx;
	EVP_SignInit   (&md_ctx, EVP_sha1());  
	EVP_SignUpdate (&md_ctx, _datbuf, _datsize);  
	char buf[MAX_SIGNATURE_SIZE];
	unsigned int len = sizeof(buf);
	int err = EVP_SignFinal (&md_ctx, (unsigned char *)buf, &len, _prikey);
	if(err != 1){
		return -2;
	}
	setSignature(buf,len);
	return 0;
}

int certfile::verify()
{
	ASSERT(_sigbuf);
	ASSERT(_datbuf);
	if(!_pubkey) return -1;

	EVP_MD_CTX     md_ctx;
	EVP_VerifyInit (&md_ctx, EVP_sha1());  
	EVP_VerifyUpdate (&md_ctx, _datbuf, _datsize);  
	int err = EVP_VerifyFinal (&md_ctx, (unsigned char *)_sigbuf, _sigsize, _pubkey);
	if(err != 1){
		return 1;
	}
	return 0;
}

int certfile::verify(const char * data, int size)
{
	if(size % 4) return -1;	//不符合base64编码大小
	char buf2[4096];
	if((unsigned int)(size*3/4) >= sizeof(buf2)) return -2;
	int rst = base64_decode((char *)data,size,(unsigned char *)buf2);

//verify data and signature size 
	int datlen = ntohs(*(uint16_t *)buf2);
	char *dat = buf2 + sizeof(uint16_t);
	if(datlen == 0 || datlen > rst - 2*(int)sizeof(uint16_t)) return -4;
	int siglen = ntohs(*(uint16_t*)(buf2 + datlen + sizeof(uint16_t)));
	char *sig = buf2 + sizeof(uint16_t) + datlen + sizeof(uint16_t); 
	if(datlen + siglen + 2*(int)sizeof(uint16_t) != rst) return -5;
//store sig and data
	setSignature(sig,siglen);
	if(_datbuf) delete _datbuf;
	_datbuf = new char[datlen];
	memcpy(_datbuf,dat,datlen);
	_datsize = datlen;

//verify
	return verify();
}

certfile::~certfile()
{
	if(_sigbuf) delete _sigbuf;
	if(_datbuf) delete _datbuf;
}

