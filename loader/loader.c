#include <Windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define HV_SERVICE_NAMEW        L"n4r1b-hypervisor-srv"
#define HV_DEVICE_NAMEW         L"\\\\.\\hv-n4r1b"
#define HV_SERVICE_INSTALL      0
#define HV_SERVICE_UNINSTALL    1


_Success_(return == TRUE)
BOOLEAN
InstallDriver(
    _In_    SC_HANDLE   hSCManager,
    _In_    LPCTSTR     ServiceName,
    _In_    LPCTSTR     DriverPath
)
{
    SC_HANDLE   schService;
    DWORD       errCode;

    schService = CreateService(hSCManager,
        ServiceName,
        ServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        DriverPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);
    if (NULL == schService) {
        errCode = GetLastError();

        if (ERROR_SERVICE_EXISTS == errCode) {
            fprintf(stderr, __FUNCTION__ " Service already exists\n");

            return TRUE;
        }
        else {
            fprintf(stderr, __FUNCTION__ " Failed creating service: %#x\n", errCode);

            return FALSE;
        }
    }
    else {
        CloseServiceHandle(schService);

        fprintf(stdout, __FUNCTION__ " Service %S was successfully created\n", ServiceName);

        return TRUE;
    }
}

_Success_(return == TRUE)
BOOLEAN
UninstallDriver(
    _In_	SC_HANDLE	hSCManager,
    _In_	LPCTSTR		ServiceName
)
{
    SC_HANDLE   schService;
    BOOLEAN     bRetStatus = FALSE;

    schService = OpenService(hSCManager,
        ServiceName,
        SERVICE_ALL_ACCESS);
    if (NULL == schService) {
        fprintf(stderr, __FUNCTION__ " Failed opening the service: %#x\n", GetLastError());

        return bRetStatus;
    }

    if (DeleteService(schService)) {
        bRetStatus = TRUE;

        fprintf(stdout, __FUNCTION__ " Service %S was successfully deleted\n", ServiceName);
    }
    else {
        fprintf(stderr, __FUNCTION__ " Failed deleting the service: %#x\n", GetLastError());
    }

    CloseServiceHandle(schService);

    return bRetStatus;
}

_Success_(return == TRUE)
BOOLEAN
StartDriver(
    _In_	SC_HANDLE	hSCManager,
    _In_	LPCTSTR		ServiceName
)
{
    SC_HANDLE   schService;
    DWORD       errCode;
    BOOLEAN     bRetStatus = FALSE;

    schService = OpenService(hSCManager,
        ServiceName,
        SERVICE_ALL_ACCESS);
    if (NULL == schService) {
        fprintf(stderr, __FUNCTION__ " Failed opening the service: %#x\n", GetLastError());

        return bRetStatus;
    }

    if (!StartService(schService,
        0,
        NULL)) {
        errCode = GetLastError();

        if (ERROR_SERVICE_ALREADY_RUNNING == errCode) {
            bRetStatus = TRUE;

            fprintf(stdout, __FUNCTION__ " Service %S already running\n", ServiceName);
        }
        else {
            fprintf(stderr, __FUNCTION__ " Failed starting the service: %#x\n", errCode);
        }
    }
    else {
        bRetStatus = TRUE;

        fprintf(stdout, __FUNCTION__ " Service %S was successfully started\n", ServiceName);
    }

    CloseServiceHandle(schService);

    return bRetStatus;
}

_Success_(return == TRUE)
BOOLEAN
StopDriver(
    _In_	SC_HANDLE	hSCManager,
    _In_	LPCTSTR		ServiceName
)
{
    SC_HANDLE       schService;
    SERVICE_STATUS  serviceStatus;
    BOOLEAN         bRetStatus = FALSE;

    schService = OpenService(hSCManager,
        ServiceName,
        SERVICE_ALL_ACCESS);
    if (NULL == schService) {
        fprintf(stderr, __FUNCTION__ " Failed opening the service: %#x\n", GetLastError());

        return bRetStatus;
    }

    if (ControlService(schService,
        SERVICE_CONTROL_STOP,
        &serviceStatus)) {
        bRetStatus = TRUE;

        fprintf(stdout, __FUNCTION__ " Service %S was successfully stopped\n", ServiceName);
    }
    else {
        fprintf(stderr, __FUNCTION__ " Failed stopping the service: %#x\n", GetLastError());
    }

    CloseServiceHandle(schService);

    return bRetStatus;
}

_Success_(return == TRUE)
BOOLEAN
ManageDriver(
    _In_    LPCTSTR DriverPath,
    _In_    LPCTSTR ServiceName,
    _In_    SIZE_T  Action
)
{
    SC_HANDLE	schSCManager;
    BOOLEAN		bRetVal = TRUE;

    if (NULL == DriverPath || NULL == ServiceName) {
        fprintf(stderr, __FUNCTION__ " Invalid driver name or service name\n");

        return FALSE;
    }

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == schSCManager) {
        fprintf(stderr, __FUNCTION__ " Failed opening a connection to SCM: %#x\n", GetLastError());

        return FALSE;
    }

    switch (Action) {
    case HV_SERVICE_INSTALL:
        if (InstallDriver(schSCManager, ServiceName, DriverPath)) {
            bRetVal = StartDriver(schSCManager, ServiceName);
        }
        else {
            bRetVal = FALSE;
        }

        break;
    case HV_SERVICE_UNINSTALL:
        if (StopDriver(schSCManager, ServiceName)) {
            bRetVal = UninstallDriver(schSCManager, ServiceName);
        }
        else {
            bRetVal = FALSE;
        }

        break;
    default:
        fprintf(stderr, __FUNCTION__ " Unknown action: %zu\n", Action);

        bRetVal = FALSE;

        break;
    }

    if (CloseServiceHandle(schSCManager) == 0) {
        fprintf(stderr, __FUNCTION__ " Failed closing SCM: %#x\n", GetLastError());
    }

    return bRetVal;
}


BOOLEAN
StartHiperVisor(
    VOID
)
{
    HANDLE deviceHandle;

    deviceHandle = CreateFile(HV_DEVICE_NAMEW, 
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    if (deviceHandle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, __FUNCTION__ " Failed opening handle to device %ws: %#x\n", HV_DEVICE_NAMEW, GetLastError());
        return FALSE;
    }

    fprintf(stdout, __FUNCTION__" Press any key to stop the HiperVisor\n");
    getchar();
    if (!CloseHandle(deviceHandle)) {
        fprintf(stderr, __FUNCTION__ " Failed closing handle to device %ws: %#x\n", HV_DEVICE_NAMEW, GetLastError());
        return FALSE;
    }

    return TRUE;

}

int __cdecl wmain(int argc, wchar_t* argv[])
{
    DWORD   retCode = EXIT_SUCCESS;
    DWORD   dwBufferLength = 0;
    BOOL    bFail = FALSE;
    LPWSTR  lpBuffer = NULL;
    LPCWSTR lpDriverName = NULL;

    if (argc > 2) {
        if (_wcsnicmp(argv[1], L"Load", wcslen(L"load")) == 0) {

            lpDriverName = argv[2];

            dwBufferLength = GetCurrentDirectory(dwBufferLength, lpBuffer);
            if (!dwBufferLength) {
                retCode = GetLastError();
                fwprintf(stderr, L"Failed to query current directory length: %08x\n", retCode);

                return retCode;
            }
            else {
                lpBuffer = calloc(dwBufferLength + wcslen(lpDriverName) + 2, sizeof(WCHAR)); // + 2: 1 for \ and 1 for NULL
                if (NULL == lpBuffer) {
                    retCode = GetLastError();
                    fwprintf(stderr, L"Failed allocating a buffer for current directory: %08x\n", retCode);

                    return retCode;
                }

                if (!GetCurrentDirectory(dwBufferLength, lpBuffer)) {
                    retCode = GetLastError();
                    fwprintf(stderr, L"Failed to query current directory length: %08x\n", retCode);

                    goto free_buff;
                }
            }

            if (wcsncat_s(lpBuffer,
                dwBufferLength + wcslen(lpDriverName) + 1,
                L"\\",
                wcslen(L"\\")) != 0) {
                retCode = GetLastError();
                fwprintf(stderr, L"wcsncat_s failed: %08x\n", retCode);

                goto free_buff;
            }
            if (wcsncat_s(lpBuffer,
                dwBufferLength + wcslen(lpDriverName) + 1,
                lpDriverName,
                wcslen(lpDriverName)) != 0) {
                retCode = GetLastError();
                fwprintf(stderr, L"wcsncat_s failed: %08x\n", retCode);

                goto free_buff;
            }

            fwprintf(stdout, L"Absolute path of the driver to load: %lS\n", lpBuffer);

            ManageDriver(lpBuffer, HV_SERVICE_NAMEW, HV_SERVICE_INSTALL);

free_buff:
            free(lpBuffer);
            lpBuffer = NULL;
        }
        else {
            goto usage;
        }
    }
    else if (argc > 1) {
        if (_wcsnicmp(argv[1], L"unload", wcslen(L"unload")) == 0) {
            ManageDriver(L"", HV_SERVICE_NAMEW, HV_SERVICE_UNINSTALL);
        }
        else if (_wcsnicmp(argv[1], L"start-hv", wcslen(L"start-hv")) == 0) {
            StartHiperVisor();
        }
        else {
            goto usage;
        }
    }
    else {
usage:
        fwprintf(stdout, L"[*] Usage: %ws [ load <hv-n4r1b> | unload | start-hv ]\n", argv[0]);
    }

    return retCode;
}