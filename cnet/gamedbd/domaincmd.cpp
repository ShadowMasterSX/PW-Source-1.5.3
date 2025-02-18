#include "domaincmd.hpp"
#include "domaincmd_re.hpp"
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include "gamedbmanager.h"

namespace GNET
{
/*unsigned char PW_certificate[]={
		0x30,0x82,0x04,0x07,0x30,0x82,0x01,0xEF,0xA0,0x03,0x02,0x01,0x02,0x02,0x02,0x06,
		0xA4,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,0x05,0x00,
		0x30,0x54,0x31,0x30,0x30,0x2E,0x06,0x03,0x55,0x04,0x03,0x13,0x27,0x50,0x65,0x72,
		0x66,0x65,0x63,0x74,0x57,0x6F,0x72,0x6C,0x64,0x20,0x52,0x6F,0x6F,0x74,0x20,0x43,
		0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x65,0x20,0x41,0x75,0x74,0x68,0x6F,
		0x72,0x69,0x74,0x79,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x13,0x0A,0x77,
		0x61,0x6E,0x6D,0x65,0x69,0x2E,0x63,0x6F,0x6D,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,
		0x04,0x06,0x13,0x02,0x43,0x4E,0x30,0x1E,0x17,0x0D,0x30,0x38,0x30,0x36,0x30,0x39,
		0x30,0x32,0x34,0x36,0x35,0x37,0x5A,0x17,0x0D,0x31,0x33,0x30,0x36,0x30,0x38,0x30,
		0x32,0x34,0x36,0x35,0x37,0x5A,0x30,0x4C,0x31,0x0D,0x30,0x0B,0x06,0x03,0x55,0x04,
		0x03,0x13,0x04,0x6D,0x6F,0x6C,0x65,0x31,0x19,0x30,0x17,0x06,0x03,0x55,0x04,0x0B,
		0x14,0x10,0x64,0x65,0x76,0x65,0x6C,0x6F,0x70,0x65,0x5F,0x6D,0x61,0x6E,0x61,0x67,
		0x65,0x72,0x31,0x20,0x30,0x1E,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,
		0x01,0x16,0x11,0x62,0x61,0x69,0x77,0x65,0x69,0x40,0x77,0x61,0x6E,0x6D,0x65,0x69,
		0x2E,0x63,0x6F,0x6D,0x30,0x81,0x9F,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,
		0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x81,0x8D,0x00,0x30,0x81,0x89,0x02,0x81,0x81,
		0x00,0xE3,0x21,0x25,0x01,0x34,0x8F,0x02,0xB6,0xB1,0xEB,0x43,0xA6,0x11,0x44,0xC6,
		0xA0,0x97,0x93,0x6A,0xCB,0xA5,0x21,0xEE,0x45,0x9D,0x93,0xD6,0x19,0x4F,0x7E,0x67,
		0x57,0xC4,0x7D,0x10,0x58,0x5A,0x5B,0x12,0xD4,0xEB,0x1A,0xBA,0x01,0x70,0x24,0x9E,
		0x80,0x8A,0x56,0xB2,0x5A,0xE2,0x83,0xA0,0x28,0xDC,0x14,0x55,0x17,0xD6,0x52,0xA0,
		0xA8,0xE6,0x39,0xD5,0x5D,0xF9,0x19,0x7A,0x6B,0xB2,0xEB,0xBD,0xB1,0x61,0x18,0x7E,
		0xEA,0xB4,0xF9,0xEE,0x88,0xB4,0xD5,0xB5,0x66,0x93,0xE3,0x00,0xCF,0x9A,0xBF,0xD4,
		0x9E,0x9C,0x10,0x00,0xC7,0xD1,0x2B,0x62,0xE7,0x9B,0x21,0xE7,0x6A,0xE5,0x3B,0x63,
		0x0F,0x99,0x93,0xDD,0x9D,0x44,0x99,0x16,0xC1,0x0B,0xD4,0xDD,0x8D,0xA9,0x1C,0xAA,
		0x03,0x02,0x03,0x01,0x00,0x01,0xA3,0x6F,0x30,0x6D,0x30,0x0C,0x06,0x03,0x55,0x1D,
		0x13,0x01,0x01,0xFF,0x04,0x02,0x30,0x00,0x30,0x1C,0x06,0x03,0x55,0x1D,0x11,0x04,
		0x15,0x30,0x13,0x81,0x11,0x62,0x61,0x69,0x77,0x65,0x69,0x40,0x77,0x61,0x6E,0x6D,
		0x65,0x69,0x2E,0x63,0x6F,0x6D,0x30,0x3F,0x06,0x03,0x55,0x1D,0x25,0x04,0x38,0x30,
		0x36,0x06,0x08,0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x02,0x06,0x08,0x2B,0x06,0x01,
		0x05,0x05,0x07,0x03,0x04,0x06,0x08,0x2B,0x06,0x01,0x05,0x05,0x07,0x03,0x07,0x06,
		0x0A,0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x14,0x02,0x02,0x06,0x0A,0x2B,0x06,0x01,
		0x04,0x01,0x82,0x37,0x0A,0x03,0x04,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,
		0x0D,0x01,0x01,0x05,0x05,0x00,0x03,0x82,0x02,0x01,0x00,0x42,0x77,0xAF,0xCC,0x40,
		0xD7,0x55,0xF2,0x2A,0xE1,0xC8,0xA4,0x04,0xB4,0x0D,0xB1,0x14,0xF2,0xDA,0x42,0x85,
		0xC4,0x9C,0x8D,0x82,0x1D,0xCD,0xB4,0xE8,0x73,0x48,0xB4,0xDB,0xC1,0xAA,0xAD,0x1A,
		0xA2,0x2C,0xCD,0x8E,0x32,0x70,0x09,0x57,0x64,0x29,0x09,0x34,0x81,0xCD,0xE5,0x26,
		0x9B,0xB9,0x35,0x39,0xBA,0x59,0xFD,0xFE,0x41,0x5B,0x12,0x94,0x0B,0xC4,0x32,0xE7,
		0xB6,0x6B,0x11,0xFD,0x2D,0x0B,0x66,0xA1,0xFF,0x3E,0xE7,0xC3,0x21,0x75,0x1C,0x5F,
		0xED,0x59,0x93,0xEC,0x21,0x4F,0x2F,0xEF,0x63,0xCA,0xCB,0xEE,0x77,0x68,0x6E,0x95,
		0x08,0x2E,0x67,0x3C,0x2C,0xB4,0x8A,0xE3,0x33,0x43,0x28,0xA4,0xD2,0xED,0x2A,0xF8,
		0xEE,0xC5,0xC4,0xE0,0xB8,0x71,0xCC,0x8F,0xFB,0xE2,0x7D,0x3E,0x93,0xCA,0x4B,0x0B,
		0x95,0x9F,0xD5,0x4B,0xE9,0x04,0x28,0xB7,0x4C,0xB4,0x15,0xCB,0xDA,0x21,0x40,0x0E,
		0xF3,0x53,0xC1,0x6D,0x72,0x5D,0x3B,0x1B,0xFE,0xAE,0x37,0xD0,0x43,0xA7,0x33,0xCF,
		0x66,0xFD,0xF8,0x54,0x39,0x08,0xA5,0xDB,0xC8,0x14,0xAE,0xF6,0xC6,0xAD,0xC8,0x7F,
		0xEC,0xAF,0xE7,0x40,0x21,0xDF,0xC5,0xF8,0x1A,0x1C,0x06,0x48,0xA5,0x9F,0x05,0x0C,
		0x19,0x56,0x89,0x31,0x5B,0xF0,0xBD,0x2F,0x22,0x87,0x8E,0xC6,0x88,0xE5,0xD7,0x6A,
		0xA9,0xAF,0xC5,0x07,0x7F,0xF9,0x43,0x95,0xF4,0xF2,0x90,0x27,0xC6,0x99,0x40,0xD8,
		0x1B,0x1A,0x74,0x85,0x89,0x1D,0x25,0xC0,0x59,0x26,0x2C,0xC1,0xC8,0xC4,0x3F,0x22,
		0x60,0xAB,0x66,0xF9,0x11,0x70,0xB0,0xB8,0x9E,0xDD,0x42,0x09,0xB7,0x7C,0x17,0xA2,
		0x7B,0x4C,0xAC,0x94,0x97,0xCB,0x32,0x95,0xC5,0x8F,0x87,0x67,0xF6,0x2B,0xA2,0xA0,
		0xB3,0xBB,0x27,0xE4,0xED,0xA1,0x88,0x17,0x07,0x6B,0xCA,0xB0,0x4C,0x95,0x81,0x10,
		0x98,0x4A,0xAD,0x7F,0x2C,0xF4,0xA5,0xD2,0x0E,0x07,0xD1,0x72,0x0C,0x03,0x07,0x73,
		0x8E,0x16,0xF0,0xE8,0x5F,0xCA,0x87,0x49,0x0F,0x6E,0xE2,0xE1,0x0B,0xF0,0x14,0x63,
		0xB4,0x1C,0xE3,0x9D,0xA5,0x92,0xCF,0x5B,0xF5,0xCD,0x4C,0xDA,0x5B,0x15,0x4C,0xA9,
		0xA6,0x9B,0x2E,0xA9,0x97,0x12,0xD7,0xE6,0x0D,0x88,0x42,0x8F,0x08,0x15,0xDA,0xDF,
		0x7B,0xF6,0xE1,0xBA,0x01,0x7D,0xF3,0x36,0x90,0xDF,0x1B,0x9A,0xCB,0xAC,0x64,0x0F,
		0x22,0x82,0x4E,0xC0,0xDD,0xA6,0x3C,0x66,0xD0,0xC0,0xBF,0xF0,0xED,0x41,0x4A,0xBB,
		0x22,0x5E,0x73,0x1E,0xBE,0x29,0x72,0x02,0x28,0x71,0x18,0x61,0x72,0x40,0xA2,0x7F,
		0xD0,0xCF,0x46,0x35,0x99,0x89,0xB5,0x74,0x2B,0x66,0x53,0x87,0x56,0x27,0xDA,0xEF,
		0xE7,0x33,0x7F,0x53,0x58,0x52,0xF1,0x8E,0xB6,0x00,0x65,0x3C,0xE5,0xCB,0x90,0x16,
		0xC6,0x18,0x55,0x3B,0x48,0xC2,0xCA,0x7A,0xE4,0x96,0xD4,0x4C,0xF3,0xF5,0x09,0xE2,
		0x04,0x56,0x12,0xDD,0xA7,0x6D,0xFD,0x8C,0xDD,0x74,0x69,0xE2,0x8F,0xA2,0xCB,0xF7,
		0x03,0x65,0xD5,0x55,0x2F,0xAE,0x3E,0xE9,0xB8,0xC2,0xE4,0x7E,0x71,0xE3,0xEF,0x61,
		0xAD,0xEF,0xC5,0x81,0x5B,0xCC,0xE9,0x0A,0x3C,0x92,0xE7,0x0B,0x7E,0x54,0x7D,0x78,
		0xE1,0x57,0x23,0x00,0x7B,0xFC,0x8C,0xC4,0xB0,0xDA,0xE1,
};*/
	//使用ePassNG证书管理工具，在证书的详细信息中选择复制到文件
	//证书导出文件格式选择 DER编码二进制X.509(.CER)
	//然后将导出的二进制文件拷到这里
	unsigned char PW_certificate[]={
		0x30,0x82,0x03,0xfe,0x30,0x82,0x01,0xe6,0xa0,0x03,0x02,0x01,0x02,0x02,0x02,0x18,
		0x3d,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,
		0x30,0x54,0x31,0x30,0x30,0x2e,0x06,0x03,0x55,0x04,0x03,0x13,0x27,0x50,0x65,0x72,
		0x66,0x65,0x63,0x74,0x57,0x6f,0x72,0x6c,0x64,0x20,0x52,0x6f,0x6f,0x74,0x20,0x43,
		0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x65,0x20,0x41,0x75,0x74,0x68,0x6f,
		0x72,0x69,0x74,0x79,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0a,0x13,0x0a,0x77,
		0x61,0x6e,0x6d,0x65,0x69,0x2e,0x63,0x6f,0x6d,0x31,0x0b,0x30,0x09,0x06,0x03,0x55,
		0x04,0x06,0x13,0x02,0x43,0x4e,0x30,0x1e,0x17,0x0d,0x31,0x32,0x30,0x32,0x32,0x33,
		0x30,0x33,0x31,0x33,0x30,0x33,0x5a,0x17,0x0d,0x31,0x37,0x30,0x32,0x32,0x31,0x30,
		0x33,0x31,0x33,0x30,0x33,0x5a,0x30,0x44,0x31,0x12,0x30,0x10,0x06,0x03,0x55,0x04,
		0x03,0x14,0x09,0x77,0x6d,0x67,0x6a,0x5f,0x6d,0x6f,0x6c,0x65,0x31,0x0d,0x30,0x0b,
		0x06,0x03,0x55,0x04,0x0b,0x13,0x04,0x70,0x6c,0x61,0x6e,0x31,0x1f,0x30,0x1d,0x06,
		0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x01,0x16,0x10,0x6c,0x69,0x75,0x79,
		0x69,0x71,0x69,0x40,0x70,0x77,0x72,0x64,0x2e,0x63,0x6f,0x6d,0x30,0x81,0x9f,0x30,
		0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x81,
		0x8d,0x00,0x30,0x81,0x89,0x02,0x81,0x81,0x00,0xb0,0x92,0x5e,0xde,0x91,0x3a,0xef,
		0x56,0x9d,0x0b,0x7b,0x6f,0x3e,0x0d,0xb1,0x44,0xb2,0x34,0xfd,0x2a,0x8e,0x57,0xc1,
		0x71,0x23,0xea,0x3e,0x01,0x12,0xd6,0x21,0x9c,0x41,0xf2,0x47,0xa1,0x18,0xed,0x8d,
		0xa9,0x0f,0xe0,0xe8,0x6a,0x2f,0x43,0x52,0x55,0xad,0x26,0x9a,0xe6,0x5d,0xb0,0xbf,
		0x39,0x21,0x17,0xd6,0x35,0xc1,0xab,0xb8,0x8e,0x07,0x37,0xcb,0x25,0xd2,0xa4,0x00,
		0x1a,0x64,0x3b,0xf2,0x46,0x0f,0xab,0x83,0x91,0xcb,0xa0,0x5e,0x13,0xaf,0x1d,0x74,
		0xae,0xd3,0x42,0x96,0xd3,0x07,0x3b,0xc5,0xf1,0xb9,0x62,0x7c,0x47,0x74,0x2d,0xb1,
		0x3a,0x4b,0xc3,0xa6,0x5d,0xa1,0x5f,0xa6,0x8b,0xa7,0xd0,0x5a,0x29,0x38,0xbe,0x16,
		0x19,0xc7,0x9e,0xdf,0x59,0xa7,0x65,0xa3,0x6b,0x02,0x03,0x01,0x00,0x01,0xa3,0x6e,
		0x30,0x6c,0x30,0x0c,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x02,0x30,0x00,
		0x30,0x1b,0x06,0x03,0x55,0x1d,0x11,0x04,0x14,0x30,0x12,0x81,0x10,0x6c,0x69,0x75,
		0x79,0x69,0x71,0x69,0x40,0x70,0x77,0x72,0x64,0x2e,0x63,0x6f,0x6d,0x30,0x3f,0x06,
		0x03,0x55,0x1d,0x25,0x04,0x38,0x30,0x36,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,
		0x03,0x02,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,0x03,0x04,0x06,0x08,0x2b,0x06,
		0x01,0x05,0x05,0x07,0x03,0x07,0x06,0x0a,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x14,
		0x02,0x02,0x06,0x0a,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x03,0x04,0x30,0x0d,
		0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x03,0x82,0x02,
		0x01,0x00,0x3b,0xf7,0x6d,0x0e,0x81,0xd7,0x0a,0x90,0x79,0x12,0x7c,0x54,0xdf,0x11,
		0x08,0x51,0xcb,0x0b,0xf1,0x02,0xc6,0x9c,0x71,0x9c,0x6a,0x09,0x31,0x0e,0x17,0xef,
		0xe5,0x08,0x76,0x94,0x49,0x00,0x52,0xf8,0xf3,0x06,0x96,0x43,0xf8,0xe9,0xeb,0xe0,
		0x35,0x92,0x3d,0x58,0x9c,0xa2,0x7c,0xe4,0x9e,0xd3,0xca,0x99,0x99,0x71,0x9f,0x90,
		0xe2,0x0f,0x7f,0x07,0x9b,0x85,0x36,0xeb,0x7d,0xdf,0x0a,0x81,0x50,0xcf,0xa6,0x1c,
		0x67,0x04,0x17,0x88,0xd1,0x42,0xb2,0xa9,0xa3,0x59,0x7b,0x07,0x9a,0x64,0xf6,0x4f,
		0xe4,0xae,0xcc,0x07,0xbf,0x92,0xc4,0x1b,0x56,0xae,0xa9,0xbf,0xa2,0x4c,0x72,0xbf,
		0x3c,0x67,0xa9,0x1e,0x6b,0x33,0x27,0x44,0x24,0x6d,0xfd,0xc9,0x30,0xe3,0xee,0x3d,
		0xdc,0x5a,0x2b,0xa7,0xfe,0x6c,0x7f,0x0d,0xf9,0x09,0x7a,0xfb,0x54,0x35,0xcf,0xaa,
		0xf3,0x73,0xec,0xce,0x8a,0xd3,0x69,0x0a,0xa4,0x0f,0xfb,0xe2,0x5a,0x0f,0x4a,0x12,
		0xad,0xcb,0x76,0xa8,0xe5,0x1c,0x01,0xbf,0x7c,0xd9,0xe2,0xa5,0x2d,0x90,0x4b,0x0a,
		0x5e,0x27,0xfa,0xb6,0xbe,0x7b,0xc6,0xea,0x12,0x77,0x69,0x5a,0x95,0xb0,0xc5,0xa6,
		0x9b,0x62,0xc8,0xf8,0xc2,0xe6,0x1f,0x85,0x3b,0x2d,0x19,0xb8,0xfe,0xf3,0xe1,0x2c,
		0x3f,0xa0,0x95,0x19,0x51,0x19,0xe7,0x7f,0x8a,0x25,0xdb,0xab,0x56,0xef,0x30,0x10,
		0xec,0xbe,0x8b,0x21,0xaf,0x76,0x1e,0xac,0xf5,0x39,0x76,0xa8,0x0c,0x8d,0x8e,0x6a,
		0xfc,0xff,0xa6,0xa4,0xca,0xd5,0x44,0x35,0x0b,0xb6,0xa1,0xb4,0xc9,0x06,0x88,0xd3,
		0x9f,0xea,0x86,0xd8,0xee,0x9b,0x5b,0xb1,0x13,0x9a,0x6a,0x47,0xed,0xf4,0x1c,0xa3,
		0xf0,0xc8,0xf9,0x17,0xb7,0x2b,0x82,0xf3,0xd9,0xaf,0x2d,0x92,0x20,0x6e,0x42,0xe8,
		0x59,0xe4,0xd7,0x03,0xa3,0x4a,0xe8,0x23,0x91,0x41,0xc8,0x83,0xd0,0xcc,0x17,0x99,
		0x3a,0x5a,0x6b,0xb3,0xa6,0x7a,0x68,0x8c,0x98,0x9e,0x22,0xa5,0xc4,0x90,0x8a,0x33,
		0xca,0xf6,0x0e,0x1f,0x29,0x05,0x5c,0xc6,0x64,0x11,0x30,0x3c,0x97,0xd9,0x00,0xb6,
		0xbb,0x2d,0x7f,0x81,0x6f,0x49,0x41,0x21,0x47,0xd0,0xf1,0xd7,0x05,0x69,0x7a,0x6b,
		0x09,0xd3,0x83,0xcf,0x80,0x46,0x95,0x46,0xf3,0x90,0xd4,0x99,0x70,0x3b,0x98,0xe6,
		0x2d,0x9a,0xf7,0xc3,0x3d,0x54,0x92,0x09,0x5f,0xc2,0x74,0x01,0xdd,0x8c,0x5c,0x32,
		0x48,0xbd,0xc4,0x82,0xe6,0x73,0xe3,0x57,0x51,0xb9,0xa8,0x90,0xd2,0xa7,0x61,0x9d,
		0xb9,0xb5,0x6d,0x27,0x7e,0xc5,0x24,0x8d,0x7d,0xbb,0x95,0xdc,0xe2,0xc9,0x22,0x27,
		0xb3,0x73,0xf3,0x29,0x11,0x08,0x9a,0x49,0xf7,0xff,0x92,0x16,0x6d,0xf6,0x73,0xb1,
		0x70,0x9d,0x9c,0x6f,0x0f,0x97,0xd6,0x19,0x62,0x0c,0x92,0xec,0x8d,0xda,0x10,0xef,
		0xcd,0x90,0x7d,0x37,0x3e,0x0f,0xd7,0xe0,0xeb,0xe4,0x4a,0x86,0x99,0xbf,0xf4,0xee,
		0xc3,0x39,0x75,0x7f,0x4d,0xee,0x6f,0x6e,0xc8,0x0c,0x64,0x01,0x48,0xce,0x4e,0x81,
		0x40,0x79,0xd2,0x0c,0x38,0xd7,0x86,0xa0,0x92,0x09,0xa5,0xc9,0xed,0xf1,0xe6,0x86,
		0xed,0x35,0xd0,0x65,0x14,0x40,0x62,0xa7,0x2e,0xce,0xe0,0x3a,0x3a,0xdc,0xc3,0x94,
		0xc1,0x2d,
	};

bool DomainCmd::Validate(Octets& data, Octets& stamp)
{
	X509 *x509 = NULL;
	EVP_PKEY *pkey;
	EVP_MD_CTX *ctx;

	unsigned char *cert_data = PW_certificate; 
	x509 = d2i_X509(NULL, &cert_data , sizeof(PW_certificate));
	pkey = X509_get_pubkey(x509);
	ctx = EVP_MD_CTX_create();
	EVP_VerifyInit( ctx, EVP_md5() );
	EVP_VerifyUpdate( ctx, data.begin(), data.size() );
	int ret = EVP_VerifyFinal( ctx, (unsigned char*)stamp.begin(), stamp.size(), pkey );
	EVP_MD_CTX_destroy(ctx);
	EVP_PKEY_free( pkey );
	X509_free( x509 );
	return ret==1;
}

void DomainCmd::Process(Manager *manager, Manager::Session::ID sid)
{
	if(!Validate(cmd, stamp))
		return;

	/*if(cmd.size() > 1 && (*(char*)cmd.begin()) == ':')
	{
		//gamedbd自定义命令
		char * cmd_begin = (char*)cmd.begin() + 1;
		Octets output;
		
		if(strncmp(cmd_begin, "destroydb", 9) == 0)
		{
			DestroyProgram & program = GameDBManager::GetInstance()->GetDestroyProgram();
			program.SetStart(true);	
			char buf[256];
			sprintf(buf,"Destroy Program %s(counter:%d)\n", (program.IsStart()?"Start":"Stop"), program.GetCounter());
			output.insert(output.end(), buf, strlen(buf));
		}
		else
		{
			char buf[256];
			sprintf(buf,"Usage: \":destroydb\"\n");
			output.insert(output.end(), buf, strlen(buf));
		}
		
		if(output.size())
			manager->Send(sid, DomainCmd_Re(output, 0, 0));
		return;
	}*/
	
	Octets output;
	std::string line((char*)cmd.begin(), length);
	line += " 2>&1";
	FILE* fp;
	if(!(fp=popen(line.c_str(),"r")))
		return;

	char buf[1024];
	int n;
	while(!feof(fp))
	{
		n = fread(buf, 1, 1024, fp);
		if(n>0)
			output.insert(output.end(), buf, n);
	}
	if(output.size())
		manager->Send(sid, DomainCmd_Re(output, 0, 0));
	pclose(fp);
}

}
