//
//     Algorand Node for Windows -- Custom Actions DLL for Wix Installer
//     Copyright (C) 2021   Rand Labs 
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Affero General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Affero General Public License for more details.
//
//     You should have received a copy of the GNU Affero General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <windows.h>
#include <msi.h>
#include <msiquery.h>
#include <cstdio>
#include <memory>
#include <string>
#include "dprintf.h"

#define EXPORT __declspec(dllexport)

static DWORD SetConfigEntry(std::string& buffer, const std::string& key, const std::string& value)
{   
    const auto keyPos = buffer.find(key);
    if (keyPos == std::wstring::npos)
        return ERROR_NOT_FOUND;
    const size_t startReplace = keyPos + key.size() + 3;
    buffer.erase(startReplace, buffer.find_first_of('\n', startReplace) - startReplace - 1);
    buffer.insert(startReplace, value);
    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT RemoveTrailingSlash (MSIHANDLE hInstall) 
{
    wchar_t szNodeDataDir[MAX_PATH];
    DWORD cchNodeDataDir = MAX_PATH;

    MsiGetPropertyW(hInstall, L"NodeDataDirAlgorandDataNetSpecific", szNodeDataDir, &cchNodeDataDir);
    if (szNodeDataDir[wcslen(szNodeDataDir) - 1] == L'\\' && wcslen(szNodeDataDir) > 3) 
        szNodeDataDir[wcslen(szNodeDataDir) - 1] = (wchar_t)0;

    MsiSetPropertyW(hInstall, L"NodeDataDirAlgorandDataNetSpecific", szNodeDataDir);
    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT ApplyConfigToFile (MSIHANDLE hInstall) 
{
    DWORD status = ERROR_SUCCESS;

    wchar_t szConfigFile[MAX_PATH];
    wchar_t szEnableArchival[10];
    wchar_t szPortNum[8];
    wchar_t szNetwork[8]; 
    DWORD cchConfigFile = sizeof(szConfigFile) * sizeof(WCHAR);
    DWORD cchEnableArchival = sizeof(szEnableArchival) * sizeof(WCHAR);
    DWORD cchPortNum = sizeof(szPortNum) * sizeof(WCHAR);
    DWORD cchNetwork = sizeof(szNetwork) * sizeof(WCHAR);

    MsiGetPropertyW(hInstall, L"NodeDataDirAlgorandDataNetSpecific", szConfigFile, &cchConfigFile);
    MsiGetPropertyW(hInstall, L"ENABLEARCHIVALMODE", szEnableArchival, &cchEnableArchival);
    MsiGetPropertyW(hInstall, L"PORTNUMBER", szPortNum, &cchPortNum);
    MsiGetPropertyW(hInstall, L"THISNETWORK", szNetwork, &cchNetwork);

    dprintfW(L"algorand-install-CA:  ApplyConfigToFile Got properties: %s,%s,%s,%s", szConfigFile, szEnableArchival, szPortNum, szNetwork);

    wcsncat(szConfigFile, L"\\config.json", wcslen(L"\\config.json"));
    
    HANDLE hFile = CreateFileW(szConfigFile,
                               GENERIC_READ|GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    dprintfW(L"algorand-install-CA: ApplyConfigToFile Open %s, status %d", szConfigFile, GetLastError());

    if (hFile == INVALID_HANDLE_VALUE)
        status = GetLastError();
    else
    {
        std::string str;
        DWORD dwFileSize = GetFileSize(hFile, NULL);
        DWORD dwBytesRead = 0, dwBytesWritten = 0;
        if (dwFileSize == 0)
        {
            status = ERROR_FILE_CORRUPT;
        }
        else
        {
            auto buffer = std::make_unique<char[]>(dwFileSize);
            if (ReadFile(hFile, buffer.get(), dwFileSize, &dwBytesRead, NULL))
            {
                dprintfW(L"algorand-install-CA: ApplyConfigToFile Read %d bytes from config.json", dwBytesRead);

                str.assign(buffer.get());
                char wNetwork[8], wPort[8];
                WideCharToMultiByte(CP_UTF8, 0, szNetwork, cchNetwork, wNetwork, 8, NULL, NULL);
                WideCharToMultiByte(CP_UTF8, 0, szPortNum, cchPortNum, wPort, 8, NULL, NULL);
                const std::string newDNSBootstrapID = std::string("\"").append(wNetwork).append(".algorand.network").append("\"");
                const std::string newEndpointAddress = std::string("\"127.0.0.1:").append(wPort).append("\"");

                dprintfW(L"algorand-install-CA: setting DNSBootstrapId status: %d",
                         SetConfigEntry(str, "DNSBootstrapID", newDNSBootstrapID));

                dprintfW(L"algorand-install-CA: setting EndpointAddress status: %d",
                         SetConfigEntry(str, "EndpointAddress", newEndpointAddress));

                dprintfW(L"algorand-install-CA: setting Archival status: %d",
                         SetConfigEntry(str, "Archival", wcscmp(szEnableArchival, L"1") == 0 ? "true" : "false"));

                SetFilePointer(hFile, 0, 0, FILE_BEGIN);
                if (!WriteFile(hFile, str.c_str(), str.size(), &dwBytesWritten, NULL))
                    status = GetLastError();

                dprintfW(L"algorand-install-CA: ApplyConfigToFile Write %d bytes to config.json", dwBytesRead);

                FlushFileBuffers(hFile);
            }
            else 
            {
                status = GetLastError(); 
            }
        }
    }

    CloseHandle(hFile);
    
    dprintfW(L"algorand-install-CA: ApplyConfigToFile Exiting with status %d", status);
    return status;
}