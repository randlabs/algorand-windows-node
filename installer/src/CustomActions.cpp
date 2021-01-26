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
#include <stdexcept>
#include <algorithm>
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


extern "C" UINT EXPORT ValidatePortNumber (MSIHANDLE hInstall) 
{
    wchar_t szPortNum[8];
    DWORD cchPortNum = 8;
    MsiGetPropertyW(hInstall, L"PORTNUMBER", szPortNum, &cchPortNum);

    try
    {
        auto sPort = std::wstring(szPortNum);
        bool valid = std::all_of(sPort.begin(), sPort.end(), [](wchar_t c) { return (c >= L'0' && c <= L'9') || c == L' '; });
        int x = std::stoi(sPort);
        if (x < 0 || x > 65535 || !valid) 
        {
            throw( std::out_of_range("invalid port number"));
        }
        MsiSetPropertyW(hInstall, L"VALIDPORTNUMBER", L"1");
    }
    catch (std::exception& e)
    {
        MsiSetPropertyW(hInstall, L"VALIDPORTNUMBER", L"0");
    }

    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT SetMsgDlgProp_InvalidPort (MSIHANDLE hInstall) 
{
    MsiSetPropertyW(hInstall, L"MESSAGEDLGTITLE", L"Information");
    MsiSetPropertyW(hInstall, L"MESSAGEDLGHEADER", L"Invalid port");
    MsiSetPropertyW(hInstall, L"MESSAGEDLGTEXT", L"Please enter a valid port number in the range from 0 to 65535.");
    MsiSetPropertyW(hInstall, L"MESSAGEDLGICONID", L"StatusAlert");
    return ERROR_SUCCESS;
}

static DWORD StartServiceRoutine (MSIHANDLE hInstall)
{
    auto hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM)
        return GetLastError();

    wchar_t szNetwork[8];
    DWORD cchNetwork = 8;
    MsiGetPropertyW(hInstall, L"THISNETWORK", szNetwork, &cchNetwork);

    if (  (wcsncmp(szNetwork, L"testnet" ,8) != 0)
        && (wcsncmp(szNetwork, L"betanet" ,8) != 0)
        && (wcsncmp(szNetwork,  L"mainnet" ,8) != 0))
        {
            dprintfW(L"algorand-install-CA: bad network name: %s", szNetwork);
            return ERROR_INVALID_NETNAME;
        }
        
    wchar_t szSvcName[32] = {0};
    wcsncpy(szSvcName, L"algodsvc_", 9);
    wcsncat(szSvcName, szNetwork, wcslen(szNetwork));

    auto hService = OpenService(hSCM, szSvcName, SERVICE_ALL_ACCESS);
    if(!hService)
    {
        CloseServiceHandle(hSCM);
        return  GetLastError();
    }

    SERVICE_STATUS_PROCESS ssStatus; 
    DWORD dwBytesNeeded = 0;
    if(!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
    {
        dprintfW(L"algorand-install-CA: QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(hService); 
        CloseServiceHandle(hSCM);
        return GetLastError();
    }

    //
    // We expect this to be stopped at this point.
    //
    if (ssStatus.dwCurrentState != SERVICE_STOPPED)
    {
        dprintfW(L"algorand-install-CA: Unexpected svc status (%d)\n", ssStatus.dwCurrentState);
        CloseServiceHandle(hService); 
        CloseServiceHandle(hSCM);
        return ERROR_SERVICE_ALREADY_RUNNING;
    }

    DWORD cTime = 0;
    const auto SLEEP_INTERVAL = 0;
    const auto START_WAIT_LIMIT = 30*1000; // 30 secs.
    if (StartService(hService, 0, NULL))
    {
        QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);

        while (ssStatus.dwCurrentState == SERVICE_START_PENDING && cTime <= START_WAIT_LIMIT)
        {
            Sleep(SLEEP_INTERVAL);
            cTime += SLEEP_INTERVAL;
            QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
        }

        if (ssStatus.dwCurrentState == SERVICE_RUNNING)
        {
            dprintfW(L"algorand-install-CA: StartService success.");
        }
        else
        {
            dprintfW(L"algorand-install-CA: Service did not pass to RUNNING state after %d ms.", START_WAIT_LIMIT);
        }
    }
    else 
    {
        dprintfW(L"algorand-install-CA: StartService failed (%d)\n", ssStatus.dwCurrentState);
    }

    CloseServiceHandle(hService); 
    CloseServiceHandle(hSCM);
    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT StartServiceOnExit (MSIHANDLE hInstall)
{
    wchar_t szIsChecked[4];
    DWORD cchIsChecked = 4;
    MsiGetPropertyW(hInstall, L"WIXUI_EXITDIALOGOPTIONALCHECKBOX", szIsChecked, &cchIsChecked);
    
    if (!wcsncmp(szIsChecked, L"1", 4))
    {
        return StartServiceRoutine(hInstall);
    }

    return ERROR_SUCCESS;
}


extern "C" UINT EXPORT RemoveTrailingSlash (MSIHANDLE hInstall) 
{
    wchar_t szNodeDataDir[MAX_PATH];
    DWORD cchNodeDataDir = MAX_PATH;

    MsiGetPropertyW(hInstall, L"NodeDataDirAlgorandDataNetSpecific", szNodeDataDir, &cchNodeDataDir);
    if (szNodeDataDir[wcslen(szNodeDataDir) - 1] == L'\\' && wcslen(szNodeDataDir) > 3) 
        szNodeDataDir[wcslen(szNodeDataDir) - 1] = (wchar_t)0;

    MsiSetPropertyW(hInstall, L"NodeDataDirAlgorandDataNetSpecific2", szNodeDataDir);
    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT ApplyConfigToFile (MSIHANDLE hInstall) 
{
    DWORD status = ERROR_SUCCESS;

    wchar_t szConfigFile[MAX_PATH];
    wchar_t szEnableArchival[10];
    wchar_t szPortNum[8];
    wchar_t szNetwork[8]; 
    DWORD cchConfigFile = sizeof(szConfigFile) / sizeof(wchar_t);
    DWORD cchEnableArchival = sizeof(szEnableArchival) / sizeof(wchar_t);
    DWORD cchPortNum = sizeof(szPortNum) / sizeof(wchar_t);
    DWORD cchNetwork = sizeof(szNetwork) / sizeof(wchar_t);

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