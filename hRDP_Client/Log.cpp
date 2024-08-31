#include"Globals.h"
#include <stdio.h>
#include"net.h"
namespace Log {

	void SaveLog(wchar_t* txt){
		//printf("%ls\n", txt);
		int i = (wcslen(txt)*sizeof(wchar_t))+2;
		send(net::s, "\x04", 1, 0);
		net::sendall(net::s, (char*)&i, 4);
		net::sendall(net::s, (char*)txt, i);
	}
}