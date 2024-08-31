#include"Globals.h"

#include <iostream>
#include"Config.h"
#include"Patcher.h"
#include"net.h"
#include"ClientProc.h"
#include <string>
const char* banner = "\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x23\x23\x23\x23\x23\x23\x20\x20\x23\x23\x23\x23\x23\x23\x23\x23\x20\x20\x23\x23\x23\x23\x23\x23\x23\x23\x20\x20\x0D\x0A\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x0D\x0A\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x0D\x0A\x23\x23\x23\x23\x23\x23\x23\x23\x23\x20\x23\x23\x23\x23\x23\x23\x23\x23\x20\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x23\x23\x23\x23\x23\x23\x20\x20\x0D\x0A\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x23\x23\x20\x20\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x20\x20\x20\x0D\x0A\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x23\x23\x20\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x20\x20\x20\x0D\x0A\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x20\x20\x20\x20\x20\x23\x23\x20\x23\x23\x23\x23\x23\x23\x23\x23\x20\x20\x23\x23\x20\x20\x20\x20\x20\x20\x20\x20";
#pragma comment(lib,"Urlmon")
int main(int argc,char *argv[]) {
	std::cout << banner << "\n\n";
	if (!Config::init()) { return -2; }
	if (!net::init()) { return -3; }
	if (argc > 1) {
		if (!strcmp(argv[1], "-u")){DeleteFileA(Config::iniConf); }
		else {
			try {
				SOCKET s = (SOCKET)std::stoi(argv[1]);
				ClientProc::ClientStart(s);
				return 0;
			}
			catch (...) { return -2; }
			
		}
	}
	FILE* fp;
	fopen_s(&fp, Config::iniConf, "r");
	if (!fp) {

		if (URLDownloadToFileA(NULL, Config::urlConf, Config::iniConf, 0, NULL) != S_OK) {
			std::cout << "error downloading config .\n";
			return -1;
		}
	}
	else {
		std::cout << "found config file \nstart with -u to update it\n";
		fclose(fp);
	}
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION hJb = { 0 };
	hJb.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	HANDLE hjob = CreateJobObject(NULL, NULL);
	if (hjob == NULL) { return -9; }
	if (!SetInformationJobObject(hjob, JobObjectExtendedLimitInformation, &hJb, sizeof(hJb))) {
		Sleep(3000); return -10;
	}
	AssignProcessToJobObject(hjob, GetCurrentProcess());

	
	std::cout << "STARTING SERVER...\n";
	SOCKET srv = net::SrvStart(7050);
	if (srv == SOCKET_ERROR) { std::cout << "ERROR STARTING SERVER : "<<GetLastError(); return -1; }
	std::cout << "SERVER STARTED\n";
	SYSTEMTIME st;
	while (1) {
		SOCKET client = accept(srv, NULL, NULL);
		if (client == INVALID_SOCKET) {
			continue;
		}
		std::cout << "Accepted Client : ";
		GetLocalTime(&st);
		std::cout << st.wYear << "-"
			<< (st.wMonth < 10 ? "0" : "") << st.wMonth << "-"
			<< (st.wDay < 10 ? "0" : "") << st.wDay << " "
			<< (st.wHour < 10 ? "0" : "") << st.wHour << ":"
			<< (st.wMinute < 10 ? "0" : "") << st.wMinute << ":"
			<< (st.wSecond < 10 ? "0" : "") << st.wSecond << "."
			<< std::endl;
		ClientProc::spawnProcClient(&client);
		Sleep(1000);
	}
	
	

}
//hRDP project https://github.com/f3di006/hRDP f3di006@gmail.com