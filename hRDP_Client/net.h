#pragma once

#include"Globals.h"


#pragma comment(lib,"Ws2_32")
#define MAXPACK 10000000 // 10MB
namespace net {
	BOOL connect(char* srv, unsigned short port,SOCKET *s);
	BOOL init();
	int sendall(SOCKET s, char* buf, int len);
	BOOL sendDll();
	BOOL recvp(char* buffer, int maxsz, _Out_ int* p_size);
	int pktSend(SOCKET s, char* buff, int sz, char type);
	extern SOCKET s;
}