#include"net.h"
#include "Globals.h"
#include"Config.h"
#include"Patcher.h"
#include <string>

namespace ClientProc {

	void spawnProcClient(SOCKET *s) {
        char szFileName[MAX_PATH];
		std::string arg = " ";
		arg=arg+std::to_string((DWORD)*s).c_str();
        GetModuleFileNameA(NULL, szFileName, MAX_PATH);
        STARTUPINFOA inf = { sizeof(inf) };
        PROCESS_INFORMATION pi;
        inf.cb = sizeof(inf);
        inf.dwFlags = STARTF_USESHOWWINDOW;
        inf.wShowWindow = SW_SHOWNORMAL;
        if (!CreateProcessA(szFileName, (LPSTR)arg.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &inf, &pi)) { return; }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        closesocket(*s);

	}

	BOOL verifyClient(SOCKET client) {
		const char* magik = "\x66\x33\x64\x69\x30\x30\x36\x2F\x68\x52\x44\x50\x00";
		const char http_denied[] = { "\x48\x54\x54\x50\x2F\x31\x2E\x31\x20\x32\x30\x30\x20\x4F\x4B\x0D\x0A\x43\x6F\x6E\x74\x65\x6E\x74\x2D\x54\x79\x70\x65\x3A\x20\x74\x65\x78\x74\x2F\x68\x74\x6D\x6C\x0D\x0A\x43\x6F\x6E\x74\x65\x6E\x74\x2D\x4C\x65\x6E\x67\x74\x68\x3A\x20\x35\x37\x30\x0D\x0A\x0D\x0A\x3C\x68\x74\x6D\x6C\x3E\x3C\x74\x69\x74\x6C\x65\x3E\x68\x52\x44\x50\x20\x53\x65\x72\x76\x65\x72\x3C\x2F\x74\x69\x74\x6C\x65\x3E\x3C\x62\x6F\x64\x79\x20\x73\x74\x79\x6C\x65\x3D\x22\x62\x61\x63\x6B\x67\x72\x6F\x75\x6E\x64\x2D\x63\x6F\x6C\x6F\x72\x3A\x62\x6C\x61\x63\x6B\x3B\x22\x3E\x3C\x63\x65\x6E\x74\x65\x72\x3E\x3C\x68\x31\x20\x73\x74\x79\x6C\x65\x3D\x22\x63\x6F\x6C\x6F\x72\x3A\x72\x65\x64\x22\x3E\x68\x52\x44\x50\x20\x53\x65\x72\x76\x65\x72\x3C\x2F\x68\x31\x3E\x3C\x62\x72\x3E\x3C\x68\x33\x20\x73\x74\x79\x6C\x65\x3D\x22\x63\x6F\x6C\x6F\x72\x3A\x67\x72\x65\x65\x6E\x22\x3E\x68\x52\x44\x50\x20\x70\x72\x6F\x6A\x65\x63\x74\x20\x3A\x20\x68\x74\x74\x70\x73\x3A\x2F\x2F\x67\x69\x74\x68\x75\x62\x2E\x63\x6F\x6D\x2F\x66\x33\x64\x69\x30\x30\x36\x2F\x68\x52\x44\x50\x3C\x2F\x68\x33\x3E\x3C\x62\x72\x3E\x3C\x68\x33\x20\x73\x74\x79\x6C\x65\x3D\x22\x63\x6F\x6C\x6F\x72\x3A\x67\x72\x65\x65\x6E\x22\x3E\x43\x6F\x6E\x74\x61\x63\x74\x3A\x20\x3C\x61\x20\x68\x72\x65\x66\x3D\x22\x23\x22\x20\x69\x64\x3D\x22\x65\x6D\x61\x69\x6C\x4C\x69\x6E\x6B\x22\x3E\x3C\x2F\x61\x3E\x20\x2C\x20\x74\x6D\x3A\x20\x40\x66\x33\x64\x69\x30\x30\x36\x3C\x2F\x68\x33\x3E\x3C\x62\x72\x3E\x3C\x2F\x63\x65\x6E\x74\x65\x72\x3E\x3C\x2F\x62\x6F\x64\x79\x3E\x3C\x73\x63\x72\x69\x70\x74\x20\x74\x79\x70\x65\x3D\x22\x74\x65\x78\x74\x2F\x6A\x61\x76\x61\x73\x63\x72\x69\x70\x74\x22\x3E\x76\x61\x72\x20\x75\x73\x65\x72\x20\x3D\x20\x22\x66\x33\x64\x69\x30\x30\x36\x22\x3B\x76\x61\x72\x20\x64\x6F\x6D\x61\x69\x6E\x20\x3D\x20\x22\x67\x6D\x61\x69\x6C\x22\x3B\x76\x61\x72\x20\x74\x6C\x64\x20\x3D\x20\x22\x63\x6F\x6D\x22\x3B\x76\x61\x72\x20\x65\x6D\x61\x69\x6C\x20\x3D\x20\x75\x73\x65\x72\x20\x2B\x20\x22\x40\x22\x20\x2B\x20\x64\x6F\x6D\x61\x69\x6E\x20\x2B\x20\x22\x2E\x22\x20\x2B\x20\x74\x6C\x64\x3B\x76\x61\x72\x20\x65\x6D\x61\x69\x6C\x4C\x69\x6E\x6B\x20\x3D\x20\x64\x6F\x63\x75\x6D\x65\x6E\x74\x2E\x67\x65\x74\x45\x6C\x65\x6D\x65\x6E\x74\x42\x79\x49\x64\x28\x22\x65\x6D\x61\x69\x6C\x4C\x69\x6E\x6B\x22\x29\x3B\x65\x6D\x61\x69\x6C\x4C\x69\x6E\x6B\x2E\x68\x72\x65\x66\x20\x3D\x20\x22\x6D\x61\x69\x6C\x74\x6F\x3A\x22\x20\x2B\x20\x65\x6D\x61\x69\x6C\x3B\x65\x6D\x61\x69\x6C\x4C\x69\x6E\x6B\x2E\x74\x65\x78\x74\x43\x6F\x6E\x74\x65\x6E\x74\x20\x3D\x20\x65\x6D\x61\x69\x6C\x3B\x3C\x2F\x73\x63\x72\x69\x70\x74\x3E\x3C\x2F\x68\x74\x6D\x6C\x3E" };
		struct timeval timeout;
		timeout.tv_sec = 5;  // 5 seconds
		timeout.tv_usec = 0; // 0 microseconds
		DWORD result = setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
		if (result == SOCKET_ERROR) {
			return FALSE;
		}
		char buffer[1000];
		int totalBytesReceived=0;
		int bytesLeft = 13;

		while (totalBytesReceived < 13) {
			result = recv(client, buffer + totalBytesReceived, bytesLeft, 0);
			if (result == SOCKET_ERROR || result==0) {
				return FALSE;
				
			}
			else {
				totalBytesReceived += result;
				bytesLeft -= result;
			}
		}
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		result = setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
		if (result == SOCKET_ERROR) {
			return FALSE;
		}
		if (Patcher::memmem(buffer, 13, magik, 13)) { return TRUE; }
		if (Patcher::memmem(buffer, 13, "GET", 3)) {
			send(client, http_denied, sizeof(http_denied)-1, 0);
			std::cout << "HTTP request detected .";
			Sleep(2000);
		}
		
		return FALSE;
	}

	void EndProc(SOCKET s) {
		std::cout << "Client disconnected !\n"; closesocket(s); Sleep(5000);
		ExitProcess(0);
	}
    void ClientStart(SOCKET client) {

		if (!verifyClient(client)) { closesocket(client); ExitProcess(0); }
		char* buffer = (char*)malloc(MAXPACK);
		if (buffer == NULL) { std::cout << "ERROR malloc\n"; Sleep(5000); ExitProcess(-5); }
		int p_sz;
		while (1) {
			if (net::recvp(client, buffer, MAXPACK, &p_sz) == FALSE) { EndProc(client); }
			switch (buffer[0])
			{
			case 0x02:
				//dll patch
				if (!Config::GetVer((BYTE*)&buffer[1], p_sz)) { std::cout << "[-] error getting version\n"; break; }
				std::cout << "[+] Version : " << Config::Ver << "\n";
				Patcher::prepKeys();
				Patcher::startPatch((BYTE*)&buffer[1], p_sz);
				if (net::DllSend(client, &buffer[1], p_sz) == SOCKET_ERROR) { EndProc(client); }
				break;
			case 0x04:
				printf("\tClient Log : %ls\n", (wchar_t*)&buffer[1]);
				break;
			case 0x03:
				net::RDPServer(client);
				EndProc(client);
				break;
			case 0x09:
				std::cout << "error connecting to RDP , this version may not be supported . Exit...\n";
				Sleep(5000);
				ExitProcess(0);
			default:
				break;
			}

		}

    }

}