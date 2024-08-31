#include"Globals.h"
#include <AclAPI.h>
#include <LM.h>
#include <cstdio>
#include"Globals.h"
#include <string>
#include "Log.h"
const wchar_t* dllService = L"C:\\Windows\\System32\\termsrv.dll";
namespace Utils {
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

    BOOL isPatched() {

        FILE* fp;
        _wfopen_s(&fp, dllService, L"rb");
        if (!fp) { return FALSE; }
        _fseeki64(fp, 0, SEEK_END);
        long long sz = _ftelli64(fp);
        rewind(fp);
        char* p = (char*)malloc(sz);
        if (!p) { return FALSE; }
        fread(p, 1, sz, fp);
        fclose(fp);
        BOOL i = FALSE;
        if (memmem(p, sz, "\x66\x33\x64\x69\x30\x30\x36", 7)) {
            i = TRUE;
        }
        free(p);
        return i;
    }
    BOOL SetPrivilege(
        HANDLE hToken,          // access token handle
        LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
        BOOL bEnablePrivilege   // to enable or disable privilege
    )
    {
        TOKEN_PRIVILEGES tp;
        LUID luid;

        if (!LookupPrivilegeValue(
            NULL,            // lookup privilege on local system
            lpszPrivilege,   // privilege to lookup 
            &luid))        // receives LUID of privilege
        {
            std::wstring error = L"LookupPrivilegeValue error : " + std::to_wstring(GetLastError());
            Log::SaveLog((wchar_t*)error.c_str());
            return FALSE;
        }

        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        if (bEnablePrivilege)
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        else
            tp.Privileges[0].Attributes = 0;

        // Enable the privilege or disable all privileges.

        if (!AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            (PTOKEN_PRIVILEGES)NULL,
            (PDWORD)NULL))
        {
            std::wstring error = L"AdjustTokenPrivileges error : " + std::to_wstring(GetLastError());
            Log::SaveLog((wchar_t*)error.c_str());
            return FALSE;
        }

        if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

        {
            Log::SaveLog((wchar_t*)L"The token does not have the specified privilege.");
            return FALSE;
        }

        return TRUE;
    }

    //https://learn.microsoft.com/en-us/windows/win32/secauthz/taking-object-ownership-in-c--
    BOOL TakeOwnership(LPTSTR lpszOwnFile)
    {

        BOOL bRetval = FALSE;

        HANDLE hToken = NULL;
        PSID pSIDAdmin = NULL;
        PSID pSIDEveryone = NULL;
        PACL pACL = NULL;
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld =
            SECURITY_WORLD_SID_AUTHORITY;
        SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
        const int NUM_ACES = 2;
        EXPLICIT_ACCESS ea[NUM_ACES];
        DWORD dwRes;

        // Specify the DACL to use.
        // Create a SID for the Everyone group.
        if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
            SECURITY_WORLD_RID,
            0,
            0, 0, 0, 0, 0, 0,
            &pSIDEveryone))
        {
            std::wstring error = L"AllocateAndInitializeSid (Everyone) error : " + std::to_wstring(GetLastError());
            Log::SaveLog((wchar_t*)error.c_str());
            goto Cleanup;
        }

        // Create a SID for the BUILTIN\Administrators group.
        if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pSIDAdmin))
        {
            
            std::wstring error = L"AllocateAndInitializeSid (Admin) error : " + std::to_wstring(GetLastError());
            Log::SaveLog((wchar_t*)error.c_str());
            goto Cleanup;
        }

        ZeroMemory(&ea, NUM_ACES * sizeof(EXPLICIT_ACCESS));

        // Set read access for Everyone.
        ea[0].grfAccessPermissions = GENERIC_READ;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName = (LPTSTR)pSIDEveryone;

        // Set full control for Administrators.
        ea[1].grfAccessPermissions = GENERIC_ALL;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = NO_INHERITANCE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[1].Trustee.ptstrName = (LPTSTR)pSIDAdmin;

        if (ERROR_SUCCESS != SetEntriesInAcl(NUM_ACES,
            ea,
            NULL,
            &pACL))
        {
            
            Log::SaveLog((wchar_t*)L"Failed SetEntriesInAcl");
            goto Cleanup;
        }

        // Try to modify the object's DACL.
        dwRes = SetNamedSecurityInfo(
            lpszOwnFile,                 // name of the object
            SE_FILE_OBJECT,              // type of object
            DACL_SECURITY_INFORMATION,   // change only the object's DACL
            NULL, NULL,                  // do not change owner or group
            pACL,                        // DACL specified
            NULL);                       // do not change SACL

        if (ERROR_SUCCESS == dwRes)
        {
            Log::SaveLog((wchar_t*)L"Successfully changed DACL");
            bRetval = TRUE;
            // No more processing needed.
            goto Cleanup;
        }
        if (dwRes != ERROR_ACCESS_DENIED)
        {
            std::wstring error = L"First SetNamedSecurityInfo call failed: " + std::to_wstring(dwRes);
            Log::SaveLog((wchar_t*)error.c_str());
            goto Cleanup;
        }

        // If the preceding call failed because access was denied, 
        // enable the SE_TAKE_OWNERSHIP_NAME privilege, create a SID for 
        // the Administrators group, take ownership of the object, and 
        // disable the privilege. Then try again to set the object's DACL.

        // Open a handle to the access token for the calling process.
        if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES,
            &hToken))
        {
            std::wstring error = L"OpenProcessToken failed : " + std::to_wstring(GetLastError());
            Log::SaveLog((wchar_t*)error.c_str());
            goto Cleanup;
        }

        // Enable the SE_TAKE_OWNERSHIP_NAME privilege.
        if (!SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, TRUE))
        {
            Log::SaveLog((wchar_t*)L"You must be logged on as Administrator.");
            goto Cleanup;
        }

        // Set the owner in the object's security descriptor.
        dwRes = SetNamedSecurityInfo(
            lpszOwnFile,                 // name of the object
            SE_FILE_OBJECT,              // type of object
            OWNER_SECURITY_INFORMATION,  // change only the object's owner
            pSIDAdmin,                   // SID of Administrator group
            NULL,
            NULL,
            NULL);

        if (dwRes != ERROR_SUCCESS)
        {
            std::wstring error = L"Could not set owner. Error : " + std::to_wstring(dwRes);
            Log::SaveLog((wchar_t*)error.c_str());
            goto Cleanup;
        }

        // Disable the SE_TAKE_OWNERSHIP_NAME privilege.
        if (!SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, FALSE))
        {
            Log::SaveLog((wchar_t*)L"Failed SetPrivilege call unexpectedly.");
            goto Cleanup;
        }

        // Try again to modify the object's DACL,
        // now that we are the owner.
        dwRes = SetNamedSecurityInfo(
            lpszOwnFile,                 // name of the object
            SE_FILE_OBJECT,              // type of object
            DACL_SECURITY_INFORMATION,   // change only the object's DACL
            NULL, NULL,                  // do not change owner or group
            pACL,                        // DACL specified
            NULL);                       // do not change SACL

        if (dwRes == ERROR_SUCCESS)
        {
            Log::SaveLog((wchar_t*)L"Successfully changed DACL");
            bRetval = TRUE;
        }
        else
        {
           
            std::wstring error = L"Second SetNamedSecurityInfo call failed : " + std::to_wstring(dwRes);
            Log::SaveLog((wchar_t*)error.c_str());
        }

    Cleanup:

        if (pSIDAdmin)
            FreeSid(pSIDAdmin);

        if (pSIDEveryone)
            FreeSid(pSIDEveryone);

        if (pACL)
            LocalFree(pACL);

        if (hToken)
            CloseHandle(hToken);

        return bRetval;

    }

    bool ChangePerm(const wchar_t* filePath)
    {
        EXPLICIT_ACCESS ea;
        PACL pOldDACL = NULL, pNewDACL = NULL;
        PSECURITY_DESCRIPTOR pSD = NULL;
        SID_NAME_USE sidType;
        PSID pSid = NULL;
        DWORD cbSid = 0, cbDomainName = 0;
        LPTSTR domainName = NULL;
        TCHAR userName[UNLEN + 1];
        DWORD userNameSize = UNLEN + 1;

        if (!GetUserName(userName, &userNameSize)) {
            return false;
        }
        LookupAccountName(NULL, userName, NULL, &cbSid, NULL, &cbDomainName, &sidType);

        pSid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbSid);
        domainName = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbDomainName * sizeof(TCHAR));

        if (!LookupAccountName(NULL, userName, pSid, &cbSid, domainName, &cbDomainName, &sidType)) {
            HeapFree(GetProcessHeap(), 0, pSid);
            HeapFree(GetProcessHeap(), 0, domainName);
            return false;
        }

        if (GetNamedSecurityInfo((LPTSTR)filePath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
            NULL, NULL, &pOldDACL, NULL, &pSD) != ERROR_SUCCESS) {
            HeapFree(GetProcessHeap(), 0, pSid);
            HeapFree(GetProcessHeap(), 0, domainName);
            return false;
        }

        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = GENERIC_ALL;
        ea.grfAccessMode = GRANT_ACCESS;
        ea.grfInheritance = NO_INHERITANCE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea.Trustee.ptstrName = (LPTSTR)pSid;

        if (SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL) != ERROR_SUCCESS) {
            HeapFree(GetProcessHeap(), 0, pSid);
            HeapFree(GetProcessHeap(), 0, domainName);
            LocalFree(pSD);
            return false;
        }
        if (SetNamedSecurityInfo((LPTSTR)filePath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
            NULL, NULL, pNewDACL, NULL) != ERROR_SUCCESS) {
            HeapFree(GetProcessHeap(), 0, pSid);
            HeapFree(GetProcessHeap(), 0, domainName);
            LocalFree(pSD);
            LocalFree(pNewDACL);
            return false;
        }

        HeapFree(GetProcessHeap(), 0, pSid);
        HeapFree(GetProcessHeap(), 0, domainName);
        LocalFree(pSD);
        LocalFree(pNewDACL);

        return true;
    }
    BOOL IsProcessElevated()
    {
        BOOL fIsElevated = FALSE;
        HANDLE hToken = NULL;
        TOKEN_ELEVATION elevation;
        DWORD dwSize;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            goto Cleanup;  // if Failed, we treat as False
        }


        if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
        {
            goto Cleanup;// if Failed, we treat as False
        }

        fIsElevated = elevation.TokenIsElevated;

    Cleanup:
        if (hToken)
        {
            CloseHandle(hToken);
            hToken = NULL;
        }
        return fIsElevated;
    }

    std::wstring getName() {
        wchar_t userbuff[300];
        wchar_t namebuff[300];
        DWORD usr = _countof(userbuff);
        DWORD n = _countof(namebuff);
        if (!GetUserNameW(userbuff, &usr)) { return L""; }
        if (!GetComputerName(namebuff, &n)) { return L""; }
        std::wstring h = std::wstring(userbuff) + L"@" + namebuff;
        return h;




    }
}