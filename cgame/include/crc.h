#ifndef __CRC32_H__
#define __CRC32_H__

#ifdef __cplusplus
extern "C" {
#endif

unsigned  crc32(const char *s, int len);
/*返回值才是真正的ctc， 传入的参数的最初值应为0xFFFFFFFF*/
unsigned  custom_crc32(unsigned int * crc_data,const char *s, int len);	

unsigned  short crc16(const unsigned char *s, int len);

#ifdef __cplusplus
};
#endif
#endif



