#include "rkpacket.hpp"
#include <string.h>
#include <stdio.h>

rkpacket::rkpacket(int id,rk_aiq_gray_mode_t mode,int ret ,unsigned int data_size ) {
	unsigned int hash = 0,packetsize = PKSIZE;
	packetinfo = NULL;
	offset = 0;
	packetinfo = new char[PKSIZE];
	packetinfo[0] = 'R';
	packetinfo[1] = 'K';
	offset += 2;
	parsedata((int)packetsize);
	parsedata(id);
	parsedata(ret);
	parsedata((int)data_size);
	parsedata((int)mode);
	hash = MurMurHash((const void *)&mode,data_size);
	parsedata((int)hash);
}

rkpacket::~rkpacket(void) {
	if ( packetinfo != NULL )
		delete packetinfo;
}

int rkpacket::printinfo(void) {
	int _offset = 10;
	char * _packetinfo = packetinfo + _offset;
	int ret = (_packetinfo[0] & 0xff) | ((_packetinfo[1] & 0xff) << 8) | ((_packetinfo[2] & 0xff) << 16) | ((_packetinfo[3] & 0xff) << 24);
	printf("rkaiq cmd result : %d\r\n",ret);
	_offset += 8;
	_packetinfo = packetinfo + _offset;
	int data = (_packetinfo[0] & 0xff) | ((_packetinfo[1] & 0xff) << 8) | ((_packetinfo[2] & 0xff) << 16) | ((_packetinfo[3] & 0xff) << 24);
	printf("rkaiq return data : %d\r\n",data);
	return data;
}

void rkpacket::parsedata(int data) {
	int pdata = data;
	char * _packetinfo;
	_packetinfo = packetinfo + offset;
	_packetinfo[0] = pdata & 0xff;
	_packetinfo[1] = ( pdata >> 8 ) & 0xff;
	_packetinfo[2] = ( pdata >> 16 ) & 0xff;
	_packetinfo[3] = ( pdata >> 24 ) & 0xff;
	offset += 4;
}

unsigned int rkpacket::MurMurHash(const void* key, int len)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    const int seed = 97;
    unsigned int h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char* data = (const unsigned char*)key;
    while(len >= 4)
    {
        unsigned int k = *(unsigned int*)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch(len)
    {
        case 3:
            h ^= data[2] << 16;
        case 2:
            h ^= data[1] << 8;
        case 1:
            h ^= data[0];
            h *= m;
    };
    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

