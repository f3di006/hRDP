#pragma once
#include"Globals.h"
namespace Patcher {
	void startPatch(BYTE* dll, size_t sz);
	void prepKeys();
	void* memmem(const char* haystack, size_t haystack_len,
		const void* const needle, const size_t needle_len);
	
}