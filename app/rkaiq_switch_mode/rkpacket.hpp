#ifndef __RKPACKET_
#define __RKPACKET_

#include "rktype.hpp"

// "RK" + packet_size + cmdid + cmdret + data_size + data + hash
// 2 + sizeof(unsigned int) + sizeof(int) + sizeof(int) + sizeof(unsigned int) + sizeof(unsigned int) + sizeof(int)
#define PKSIZE 26


class rkpacket {
public:
	char * packetinfo;
	rkpacket(int id,rk_aiq_gray_mode_t mode,int ret = 0,unsigned int data_size = 4);
	~rkpacket();
	int printinfo(void);
private:
	int offset;
	unsigned int MurMurHash(const void* key, int len);
	void parsedata(int data);
};


#endif
