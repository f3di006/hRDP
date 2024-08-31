#include"Config.h"
#include<sstream>
#include<random>
namespace Config {

    std::string Ver = "";
    BOOL is64 = FALSE;
    const char* iniConf = "confrdp.ini"; // some offsets can be obtained from rdpwrap project . https://github.com/stascorp/rdpwrap
    const char* urlConf = "https://raw.githubusercontent.com/sebaxakerhtc/rdpwrap.ini/master/rdpwrap.ini"; // contains updated config , can be changed...
    char iniConf_path[MAX_PATH + 1];

    bool init() {
        DWORD result = GetFullPathNameA(
            iniConf, 
            MAX_PATH,
            iniConf_path,
            NULL
        );

        if (result > 0 && result < MAX_PATH) {
            return true;
        }
        
        else {
            return false;
        }
        
    }
    bool checkArch(char *buffer) {
        IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)buffer;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return false;
        }
        IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)buffer + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            return false;
        }
        WORD machine = ntHeaders->FileHeader.Machine;

        if (machine == IMAGE_FILE_MACHINE_AMD64) {
            is64 = TRUE;
        }
        
        return true;
    }

    bool getver(wchar_t* file) {
        DWORD dummy;
        DWORD versionInfoSize = GetFileVersionInfoSizeW(file, &dummy);
        if (versionInfoSize == 0) {
            return false;
        }

        BYTE* versionInfo = new BYTE[versionInfoSize];
        if (!GetFileVersionInfoW(file, 0, versionInfoSize, versionInfo)) {
            delete[] versionInfo;
            return false;
        }

        VS_FIXEDFILEINFO* fileInfo = NULL;
        UINT fileInfoSize = 0;
        if (!VerQueryValueW(versionInfo, L"\\", (LPVOID*)&fileInfo, &fileInfoSize)) {
            delete[] versionInfo;
            return false;
        }

        DWORD fileVersionMS = fileInfo->dwFileVersionMS;
        DWORD fileVersionLS = fileInfo->dwFileVersionLS;

        DWORD majorVersion = HIWORD(fileVersionMS);
        DWORD minorVersion = LOWORD(fileVersionMS);
        DWORD buildNumber = HIWORD(fileVersionLS);
        DWORD revisionNumber = LOWORD(fileVersionLS);
        std::stringstream ss;
        ss << majorVersion << '.' << minorVersion << '.' << buildNumber << '.' << revisionNumber;
        Ver = ss.str();
        delete[] versionInfo;
        return true;
    }

    std::wstring randStr(size_t length) {
        const std::wstring characters = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd; 
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, characters.size() - 1);

        std::wstring randomString;
        for (size_t i = 0; i < length; ++i) {
            randomString += characters[distribution(generator)];
        }

        return randomString;
    }

    bool GetVer(BYTE* buffer, DWORD bufferSize) {
        FILE* fp;
        bool status = false;
        std::wstring path =  randStr(5)+L".dll";
        _wfopen_s(&fp, path.c_str(), L"wb");
        if (!fp) {
            goto end;
        }
        fwrite(buffer, 1, bufferSize, fp);
        fclose(fp);
        if (!checkArch((char*)buffer)) { std::cout << "[-] Error file check ...\n"; goto end; }
        status=getver((wchar_t*)path.c_str());
    end:
        DeleteFile(path.c_str());
        return status;
    }



    BYTE* hstoB(const char* hexstr, _Out_ size_t* sz) {
        size_t hLen = strlen(hexstr);
        *sz = 0;
        if (hLen % 2 != 0) {
            return NULL;
        }

        *sz = hLen / 2;
        BYTE* bytesarr = (BYTE*)malloc(*sz);
        if (bytesarr == NULL) {
            return NULL;
        }
        for (size_t i = 0; i < *sz; i++) {
            int scanned = sscanf_s(hexstr + 2 * i, "%2hhx", &bytesarr[i]);
            if (scanned != 1) {
                free(bytesarr);
                return NULL;
            }
        }

        return bytesarr;
    }
    BOOL getKeyVal(const char* filepath, const char* sect, const char* key, _Out_ char* buff,int buff_len) {

        if (!GetPrivateProfileStringA(
            sect,
            key,
            NULL,
            buff,
            buff_len, 
            filepath 
        )) {
            return FALSE;
        }

        return TRUE;

	}



}