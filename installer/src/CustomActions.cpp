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
#include <shellapi.h>
#include <shlwapi.h>
#include "dprintf.h"

#include "inc/json.hpp"

using json = nlohmann::json;

#define EXPORT __declspec(dllexport) __stdcall
#define MAX_ACTION_DATA 2048

static VOID WriteLog(MSIHANDLE hInstall, LPCWSTR szFormatW, ...)
{
    WCHAR szBufW[2048];
    MSIHANDLE hRecord;
    va_list argptr;

    va_start(argptr, szFormatW);
    _vsnwprintf_s(szBufW, _countof(szBufW), _TRUNCATE, szFormatW, argptr);
    va_end(argptr);
    hRecord = ::MsiCreateRecord(1);
    if (hRecord != 0)
    {
        ::MsiRecordSetStringW(hRecord, 1, szBufW);
        ::MsiRecordSetStringW(hRecord, 0, L"[1]");
        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_INFO, hRecord);
        ::MsiCloseHandle(hRecord);
    }
    return;
}

static UINT __stdcall ScheduleDeferredCA (MSIHANDLE hInstall, LPCWSTR szActionName, LPWSTR szActionData)
{
    if (!szActionName || !szActionData)
        return ERROR_INVALID_PARAMETER;

    if (szActionName[0] == L'\0')
        return ERROR_INVALID_PARAMETER;

    UINT r = MsiSetPropertyW(hInstall, szActionName, szActionData);
    if (r != ERROR_SUCCESS)
        return r;

    return MsiDoActionW(hInstall, szActionName);
}

static DWORD CalcSvcControlWaitTime(LPSERVICE_STATUS ssStatus)
{
    // Do not wait longer than the wait hint. A good interval is 
    // one-tenth of the wait hint but not less than 1 second  
    // and not more than 10 seconds. 
 
    DWORD dwWaitTime = ssStatus->dwWaitHint / 10;

    if(dwWaitTime < 1000)
        dwWaitTime = 1000;
    else if (dwWaitTime > 10000)
        dwWaitTime = 10000;

    return dwWaitTime;
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

extern "C" UINT EXPORT SchedApplyConfigToFile(MSIHANDLE hInstall)
{
    wchar_t szConfigFile[MAX_PATH];
    wchar_t szEnableArchival[10];
    wchar_t szPortNum[8];
    wchar_t szNetwork[8]; 
    wchar_t szPublicAccess[8];
    DWORD cchConfigFile = sizeof(szConfigFile) / sizeof(wchar_t);
    DWORD cchEnableArchival = sizeof(szEnableArchival) / sizeof(wchar_t);
    DWORD cchPortNum = sizeof(szPortNum) / sizeof(wchar_t);
    DWORD cchNetwork = sizeof(szNetwork) / sizeof(wchar_t);
    DWORD cchPublicAccess = sizeof(szPublicAccess) / sizeof (wchar_t);

    MsiGetPropertyW(hInstall, L"NodeDataDirAlgorandDataNetSpecific", szConfigFile, &cchConfigFile);
    MsiGetPropertyW(hInstall, L"ENABLEARCHIVALMODE", szEnableArchival, &cchEnableArchival);
    MsiGetPropertyW(hInstall, L"PORTNUMBER", szPortNum, &cchPortNum);
    MsiGetPropertyW(hInstall, L"THISNETWORK", szNetwork, &cchNetwork);
    MsiGetPropertyW(hInstall, L"PUBLICACCESS", szPublicAccess, &cchPublicAccess);

    if (wcscmp(szPublicAccess, L"") == 0)
        wcscpy_s(szPublicAccess, L"0");

    if (wcscmp(szEnableArchival, L"") == 0)
        wcscpy_s(szEnableArchival, L"0");

    wchar_t szActionData[MAX_ACTION_DATA] = { 0 };
    wcscpy_s(szActionData, L"\"");
    wcscat_s(szActionData, szConfigFile);
    wcscat_s(szActionData, L"\" ");
    wcscat_s(szActionData, szEnableArchival);
    wcscat_s(szActionData, L" ");
    wcscat_s(szActionData, szPortNum);
    wcscat_s(szActionData, L" ");
    wcscat_s(szActionData, szNetwork);
    wcscat_s(szActionData, L" ");
    wcscat_s(szActionData, szPublicAccess);

    return ScheduleDeferredCA(hInstall, L"ApplyConfigToFile", szActionData);
}

extern "C" UINT EXPORT RequestUninstallDataDir(MSIHANDLE hInstall)
{
    int ret = 0;
    wchar_t szCaData[MAX_ACTION_DATA] = { 0 };
    DWORD cchCaData = MAX_ACTION_DATA;
    MsiGetPropertyW(hInstall, L"CustomActionData", szCaData, &cchCaData);
 
    szCaData[wcslen(szCaData)+1] = wchar_t(0); // Ensure double zero at end for ShFileOperationW

    WriteLog(hInstall, L"algorand-install-CA: Initiating SHFileOperationW to remove %s", szCaData);

    SHFILEOPSTRUCTW fop;
    ZeroMemory(&fop, sizeof(SHFILEOPSTRUCTW));
    fop.wFunc = FO_DELETE;
    fop.pFrom = szCaData;
    fop.fFlags = FOF_NO_UI;
    if ( (ret = SHFileOperationW(&fop)) )
    {
        WriteLog(hInstall, L"algorand-install-CA: SHFileOperationW FAILED with code %d", ret);
        return ERROR_IO_DEVICE;
    }
    else 
    {
        WriteLog(hInstall, L"algorand-install-CA: SHFileOperationW success.", ret);
    }

    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT SchedRequestUninstallDataDir (MSIHANDLE hInstall)
{
    MSIHANDLE hRec = MsiCreateRecord(0);
    MsiRecordSetStringW(hRec, 0, L"Do you want to keep your data directory?"
        "\nThis includes configuration files and Algorand chain synchronization data.\n\n"
        "If you delete your chain data, you will need to wait for resync of  your node from scratch.");
    int ret = MsiProcessMessage(hInstall, (INSTALLMESSAGE) (INSTALLMESSAGE_USER | MB_ICONWARNING | MB_YESNOCANCEL |  MB_DEFBUTTON1), hRec);
    MsiCloseHandle(hRec);

    if (ret == IDCANCEL)
    {
        return ERROR_INSTALL_USEREXIT;
    } 
    else if (ret == IDNO) 
    {
        wchar_t szDir[MAX_PATH + 1] = { 0 };        // we need double-zeroed end for SHFileOperationW
        DWORD cchDir = MAX_PATH;

        MsiGetPropertyW(hInstall, L"ALGORANDNETDATAFOLDER", szDir, &cchDir);
        return ScheduleDeferredCA(hInstall, L"RequestUninstallDataDir", szDir);
    }
    return ERROR_SUCCESS;
}

extern "C" DWORD EXPORT StartServiceRoutine (MSIHANDLE hInstall)
{
    wchar_t szCaData[MAX_ACTION_DATA] = { 0 };
    DWORD cchCaData = MAX_ACTION_DATA;
    MsiGetPropertyW(hInstall, L"CustomActionData", szCaData, &cchCaData);

    auto hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM)
        return GetLastError();

        
    wchar_t szSvcName[32] = {0};
    wcsncpy(szSvcName, L"algodsvc_", 9);
    wcsncat(szSvcName, szCaData, wcslen(szCaData));

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
        WriteLog(hInstall, L"algorand-install-CA: QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(hService); 
        CloseServiceHandle(hSCM);
        return GetLastError();
    }

    //
    // We expect this to be stopped at this point.
    //
    if (ssStatus.dwCurrentState != SERVICE_STOPPED)
    {
        WriteLog(hInstall, L"algorand-install-CA: Unexpected svc status (%d)\n", ssStatus.dwCurrentState);
        CloseServiceHandle(hService); 
        CloseServiceHandle(hSCM);
        return ERROR_SERVICE_ALREADY_RUNNING;
    }

    // Do not wait longer than the wait hint. A good interval is 
    // one-tenth of the wait hint but not less than 1 second  
    // and not more than 10 seconds. 
 
    DWORD dwWaitTime = CalcSvcControlWaitTime((LPSERVICE_STATUS)&ssStatus);

    if (StartService(hService, 0, NULL))
    {
        QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);

        while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
        {
            Sleep(dwWaitTime);
            QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
        }

        if (ssStatus.dwCurrentState == SERVICE_RUNNING)
        {
            WriteLog(hInstall, L"algorand-install-CA: StartService success.");
        }
        else
        {
            WriteLog(hInstall, L"algorand-install-CA: Service did not pass to RUNNING state");
        }
    }
    else 
    {
        WriteLog(hInstall, L"algorand-install-CA: StartService failed (%d)\n", ssStatus.dwCurrentState);
    }

    CloseServiceHandle(hService); 
    CloseServiceHandle(hSCM);
    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT StopService (MSIHANDLE hInstall)
{
    wchar_t szCaData[MAX_ACTION_DATA] = { 0 };
    DWORD cchCaData = MAX_ACTION_DATA;
    MsiGetPropertyW(hInstall, L"CustomActionData", szCaData, &cchCaData);

    auto hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM)
        return GetLastError();
        
    wchar_t szSvcName[32] = {0};
    wcsncpy(szSvcName, L"algodsvc_", 9);
    wcsncat(szSvcName, szCaData, wcslen(szCaData));

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
        WriteLog(hInstall, L"algorand-install-CA: QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(hService); 
        CloseServiceHandle(hSCM);
        return GetLastError();
    }

    if(!ControlService(hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssStatus))
    {
        WriteLog(hInstall, L"algorand-install-CA: ControlService to stop failed (%d)\n", GetLastError());
        CloseServiceHandle(hService); 
        CloseServiceHandle(hSCM);
        return GetLastError();
    }
    
    DWORD dwWaitTime = CalcSvcControlWaitTime((LPSERVICE_STATUS)&ssStatus);

    while (ssStatus.dwCurrentState != SERVICE_STOPPED) 
    {
        Sleep( dwWaitTime );
        QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);    
    }

    CloseServiceHandle(hService); 
    CloseServiceHandle(hSCM);
    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT SchedStartServiceOnExit (MSIHANDLE hInstall)
{
    wchar_t szIsChecked[4];
    DWORD cchIsChecked = 4;
    MsiGetPropertyW(hInstall, L"WIXUI_EXITDIALOGOPTIONALCHECKBOX", szIsChecked, &cchIsChecked);
    
    if (!wcsncmp(szIsChecked, L"1", 4))
    {
        wchar_t szNetwork[8];
        DWORD cchNetwork = 8;
        MsiGetPropertyW(hInstall, L"THISNETWORK", szNetwork, &cchNetwork);

        if (  (wcsncmp(szNetwork, L"testnet" ,8) != 0)
            && (wcsncmp(szNetwork, L"betanet" ,8) != 0)
            && (wcsncmp(szNetwork,  L"mainnet" ,8) != 0))
            {
                WriteLog(hInstall, L"algorand-install-CA: StartServiceRoutine: bad network name: %s", szNetwork);
                return ERROR_INVALID_NETNAME;
            }
       
        return ScheduleDeferredCA(hInstall, L"StartServiceRoutine", szNetwork);
    }

    return ERROR_SUCCESS;
}

extern "C" UINT EXPORT SchedStopService(MSIHANDLE hInstall)
{
    wchar_t szNetwork[8];
    DWORD cchNetwork = 8;
    MsiGetPropertyW(hInstall, L"THISNETWORK", szNetwork, &cchNetwork);

    if ((wcsncmp(szNetwork, L"testnet", 8) != 0) && (wcsncmp(szNetwork, L"betanet", 8) != 0) && (wcsncmp(szNetwork, L"mainnet", 8) != 0))
    {
        WriteLog(hInstall, L"algorand-install-CA: SchedStopService: bad network name: %s", szNetwork);
        return ERROR_INVALID_NETNAME;
    }

    return ScheduleDeferredCA(hInstall, L"StopService", szNetwork);
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

extern "C" UINT EXPORT ReadConfigFromFile (MSIHANDLE hInstall)
{
    wchar_t szConfigFile[MAX_PATH];
    
    DWORD cchConfigFile = sizeof(szConfigFile) / sizeof(wchar_t);
    MsiGetPropertyW(hInstall, L"NodeDataDirAlgorandDataNetSpecific", szConfigFile, &cchConfigFile);

    wcsncat(szConfigFile, L"config.json", wcslen(L"config.json"));

    HANDLE hFile = CreateFileW(szConfigFile,
                               GENERIC_READ,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    WriteLog(hInstall, L"algorand-install-CA: ReadConfigFromFile Open %s, status %d", szConfigFile, GetLastError());

    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwFileSize = GetFileSize(hFile, NULL);
        DWORD dwBytesRead = 0;
        if (dwFileSize > 0)
        {
            auto buffer = std::make_unique<char[]>(dwFileSize);
            if (ReadFile(hFile, buffer.get(), dwFileSize, &dwBytesRead, NULL))
            {
                WriteLog(hInstall, L"algorand-install-CA: ReadConfigFromFile Read %d bytes from config.json", dwBytesRead);

                auto configJson = json::parse(buffer.get(), nullptr, false);
                if (configJson != json::value_t::discarded) 
                {
                    wchar_t szPortNum[8];

                    bool archival = static_cast<bool>(configJson["Archival"]) ? "true" : "false";
                    std::string endpointAddr = configJson["EndpointAddress"]; 
                    
                    endpointAddr.erase(0, endpointAddr.find_first_not_of(" \t"));
                    StringCchPrintf(szPortNum, 8, L"%d", std::stoi(endpointAddr.substr(endpointAddr.find(":") + 1)));

                    WriteLog(hInstall, L"algorand-install-CA: Found config: archival %d, port %s", archival, szPortNum);
                    MsiSetPropertyW(hInstall, L"ENABLEARCHIVALMODE", archival ? L"1" : L"0");
                    MsiSetPropertyW(hInstall, L"PORTNUMBER", szPortNum);
                    MsiSetPropertyW(hInstall, L"PUBLICACCESS", (endpointAddr.at(0) == ':') ? L"1" : L"0");
                }
                else 
                {
                    WriteLog(hInstall, L"algorand-install-CA: ReadConfigFromFile cannot parse existing JSON, ignoring");
                }                
            }
        }

        CloseHandle(hFile);
    }

    return ERROR_SUCCESS;

}

extern "C" UINT EXPORT ApplyConfigToFile (MSIHANDLE hInstall) 
{
    DWORD status = ERROR_SUCCESS;

    wchar_t szCaData[MAX_ACTION_DATA] = { 0 };
    DWORD cchCaData = MAX_ACTION_DATA;
    int numArgs;
    MsiGetPropertyW(hInstall, L"CustomActionData", szCaData, &cchCaData);

    LPWSTR* argv = CommandLineToArgvW(szCaData, &numArgs );

    if (numArgs != 5) 
    {
        WriteLog(hInstall, L"algorand-install-CA: invalid num of arguments %d (expected 5)", numArgs);
        return ERROR_BAD_ARGUMENTS;
    }

    LPWSTR szConfigPath = argv[0];
    LPWSTR szEnableArchival = argv[1];
    LPWSTR szPortNum = argv[2];
    LPWSTR szNetwork = argv[3];
    LPWSTR szPublicAccess = argv[4];

    WriteLog(hInstall, L"algorand-install-CA:  ApplyConfigToFile custom data Got properties: %s,%s,%s,%s,%s", 
        szConfigPath, szEnableArchival, szPortNum, szNetwork, szPublicAccess);

    std::wstring configFile = std::wstring(szConfigPath).append(L"config.json");

    HANDLE hFile = CreateFileW(configFile.c_str(),
                               GENERIC_READ|GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    WriteLog(hInstall, L"algorand-install-CA: ApplyConfigToFile Open %s, status %d", configFile.c_str(), GetLastError());

    if (hFile == INVALID_HANDLE_VALUE)
        status = GetLastError();
    else
    {
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
                char wNetwork[8], wPort[8];
                WideCharToMultiByte(CP_UTF8, 0, szNetwork, 8, wNetwork, 8, NULL, NULL);
                WideCharToMultiByte(CP_UTF8, 0, szPortNum, 8, wPort, 8, NULL, NULL);
                
                WriteLog(hInstall, L"algorand-install-CA: ApplyConfigToFile Read %d bytes from config.json", dwBytesRead);

                auto configJson = json::parse(buffer.get(), nullptr, false);
                if (configJson != json::value_t::discarded)
                {
                    std::string addr = wcscmp(szPublicAccess, L"1") == 0 ? ":" : "127.0.0.1:"; 
                    configJson["DNSBootstrapID"] = std::string(wNetwork).append(".algorand.network");
                    configJson["EndpointAddress"] = addr.append(wPort);
                    configJson["Archival"] = wcscmp(szEnableArchival, L"1") == 0 ? true : false;

                    std::string str = configJson.dump(4);

                    SetFilePointer(hFile, 0, 0, FILE_BEGIN);
                    if (!WriteFile(hFile, str.c_str(), str.size(), &dwBytesWritten, NULL))
                        status = GetLastError();

                    SetEndOfFile(hFile);

                    WriteLog(hInstall, L"algorand-install-CA: ApplyConfigToFile Write %d bytes to config.json", dwBytesRead);

                    FlushFileBuffers(hFile);
                }
                else 
                {
                    WriteLog(hInstall, L"algorand-install-CA: json::parse failed");
                    status = ERROR_FILE_CORRUPT;
                }
            }
            else 
            {
                status = GetLastError(); 
            }
        }
    }

    CloseHandle(hFile);
    
    WriteLog(hInstall, L"algorand-install-CA: ApplyConfigToFile Exiting with status %d", status);
    return status;
}