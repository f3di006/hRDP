#pragma once
#include"Globals.h"
namespace Utils {
	BOOL TakeOwnership(LPTSTR lpszOwnFile);
	bool ChangePerm(const wchar_t* filePath);
	BOOL isPatched();
	BOOL IsProcessElevated();
	std::wstring getName();
}