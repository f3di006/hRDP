
#include"Config.h"
#include<vector>
#include <string>
#include"Globals.h"
namespace Patcher {
	#define sigHrdpLen 5
	const char* needed_keys[] = { "LocalOnlyOffset","LocalOnlyCode","DefPolicyOffset","DefPolicyCode"}; //needed patch from rdpwrap project
	char sig_hrdp64[] = { "\x40\x12\xf0\x04\xc0" };
	char sig_hrdp86[] = { "\xff\x12\xf0\x04\xc0" };
	
	char hrdpatch64[] = {"\x01\x00\x00\x00\x31\xc0\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"};
	
	char hrdpatch86[] = { "\xc7\x85\xd8\xfe\xff\xff\x01\x00\x00\x00\x31\xc0\x90\xb9\xe8\x47\x01\x10\x90\x90\x90\x90\x90" };
	char hrdpatch86_sig[] = { "xx????xxxxxxx?????xxxxx" };
	std::vector<std::string>keys;
	
	void prepKeys() {
		keys.clear();
		for (int i = 0; i < _countof(needed_keys); i++) {
			std::string newk(needed_keys[i]);
			if (Config::is64) {
				newk = newk + (".x64");
			}
			else {
				newk = newk + (".x86");
			}
			keys.push_back(newk);

		}
	}
	
	int rva2offset(char* buffer, DWORD rva) {

		PIMAGE_DOS_HEADER dosHeader;
		PIMAGE_NT_HEADERS peHeader;
		PIMAGE_SECTION_HEADER sectionHeader;



		dosHeader = (PIMAGE_DOS_HEADER)buffer;

		if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
		{
			peHeader = (PIMAGE_NT_HEADERS)((u_char*)dosHeader + dosHeader->e_lfanew);

			if (peHeader->Signature == IMAGE_NT_SIGNATURE)
			{
				sectionHeader = IMAGE_FIRST_SECTION(peHeader);
				UINT nSectionCount = peHeader->FileHeader.NumberOfSections;
				UINT i = 0;
				//check in which section the address belongs
				for (i = 0; i <= nSectionCount; ++i, ++sectionHeader)
				{
					if ((sectionHeader->VirtualAddress) > rva)
					{
						sectionHeader--;
						break;
					}
				}

				if (i > nSectionCount)
				{
					sectionHeader = IMAGE_FIRST_SECTION(peHeader);
					UINT nSectionCount = peHeader->FileHeader.NumberOfSections;
					for (i = 0; i < nSectionCount - 1; ++i, ++sectionHeader);
				}

				//once the correct section is found below formula gives the actual disk offset 
				DWORD retAddr = rva - (sectionHeader->VirtualAddress) +
					(sectionHeader->PointerToRawData);
				return retAddr;


			}
			return -1;
		}
		else
		{
			return -1;
		}

	}
	void* memmem(const char* haystack, size_t haystack_len,
		const void* const needle, const size_t needle_len)
	{
		if (haystack == NULL) return NULL;
		if (haystack_len == 0) return NULL;
		if (needle == NULL) return NULL;
		if (needle_len == 0) return NULL;

		for (const char* h = haystack;
			haystack_len >= needle_len;
			++h, --haystack_len) {
			if (!memcmp(h, needle, needle_len)) {
				return (void*)h;
			}
		}
		return NULL;
	}

	BOOL patcherx86(BYTE* dll, size_t sz) {
		if (sizeof(hrdpatch86) != sizeof(hrdpatch86_sig)) { std::cout << "Wrong sig \n"; return FALSE; }
		void* p = memmem((char*)dll, sz, sig_hrdp86, sigHrdpLen);
		int patchOff=34;
		if (p == NULL) { return FALSE; }

		
		unsigned char* patchbg = (unsigned char*)p - patchOff;
		//check before patch
		if (patchbg[0] != 0x83 || patchbg[1] != 0xa5 || patchbg[6] != 0x00) { std::cout << "x86 patch : not supported version\n"; return FALSE; }
		//patch
		for (int i = 0; i < sizeof(hrdpatch86) - 1; i++) {
			if (hrdpatch86_sig[i] == 'x') {
				patchbg[i] = hrdpatch86[i];
			}
			//else { }
		}


		return TRUE;
	}

	BOOL hRDP_Patcher(BYTE* dll, size_t sz) {
		int patchOff ;
		int patch_sz;
		
		if (!Config::is64) { return patcherx86(dll,sz); }
		//x64
		patchOff = 28; 
		patch_sz = sizeof(hrdpatch64) - 1;

		void* p = memmem((char*)dll, sz, sig_hrdp64, sigHrdpLen);
		if (p == NULL) { return FALSE; }
		
		char* patchStart = (char*)p - patchOff;
		if (patchStart[0] != 0x00) { return FALSE; }
		
		if (memcpy_s(patchStart, sz, hrdpatch64, patch_sz)) {
			std::cout << "Error patching \n";
			return FALSE;
		}
		
		return TRUE;


	}
	void startPatch(BYTE* dll, size_t sz) {
		char buffConfig[1000];
		char patchb[1000];
		//hRDP Patch
		if (!hRDP_Patcher(dll, sz)) { std::cout << "[-] hRDP Patch Failed . this version is not well supported .RDP may not work . continuing ...\n"; }
		else { std::cout << "[+] hRDP Patch OK\n"; }

		for (int i = 0; i < keys.size(); i+=2) {
			if (!Config::getKeyVal(Config::iniConf_path, Config::Ver.c_str(), keys[i].c_str(), buffConfig, _countof(buffConfig))) {
				std::cout << "[-] Failed to get key : " << keys[i].c_str() << "\n";
				continue;
			}
			//should contains offset
			unsigned int rva;
			try {
				rva = std::stoi(buffConfig, nullptr,16);
				
			}
			catch (...) { std::cout << "[-] Failed to get offset for key : " << keys[i].c_str() << "\n"; continue; }


			if (!Config::getKeyVal(Config::iniConf_path, Config::Ver.c_str(), keys[i+1].c_str(), buffConfig, _countof(buffConfig))) {
				std::cout << "[-] Failed to get key : " << keys[i+1].c_str() << "\n";
				continue;
			}

			//offset and bytes needed
			if (!Config::getKeyVal(Config::iniConf_path, "PatchCodes", buffConfig, patchb, _countof(patchb))) {
				std::cout << "[-] Failed to get key : " << buffConfig << "\n";
				continue;
			}

			
			//toh
			size_t patchSize;
			BYTE* ptch = Config::hstoB(patchb, &patchSize);
			if (!ptch) {
				std::cout << "[-] patch error convert for key : "<< keys[i].c_str()<<"\n";
				continue;
			}
			int offset = rva2offset((char*)dll, rva);
			if (offset == -1) { return ; }
			//printf("offset: %p patch : %s\n", offset, patchb);
			memcpy_s(&dll[offset], sz, ptch, patchSize);
			free(ptch);

		}

		return ;
	}


}