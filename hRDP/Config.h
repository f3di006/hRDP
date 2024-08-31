#pragma once
#include<iostream>
#include"Globals.h"
#pragma comment(lib,"Version.lib")
namespace Config {
	BOOL getKeyVal(const char* filepath, const char* sect, const char* key, _Out_ char* buff, int buff_len);
	BYTE* hstoB(const char* hexString, _Out_ size_t* sz);
	bool GetVer(BYTE* buffer, DWORD bufferSize);
	extern std::string Ver;
	extern BOOL is64;
	extern const char* iniConf;
	extern const char* urlConf;
	extern char iniConf_path[MAX_PATH + 1];
	bool init();
}