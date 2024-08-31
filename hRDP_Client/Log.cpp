#include"Globals.h"
#include <stdio.h>
#include"net.h"
namespace Log {

	void SaveLog(wchar_t* txt){
		//printf("%ls\n", txt);
		int i = (wcslen(txt)*sizeof(wchar_t))+2;
		net::pktSend(net::s, (char*)txt, i, 0x04);
	}
}