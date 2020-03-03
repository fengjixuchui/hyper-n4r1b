#include <wdm.h>

#include "..\include\logger\logger.h"


DRIVER_UNLOAD DriverUnload;

DRIVER_INITIALIZE DriverEntry;

#define NT_DEVICE_NAME      L"\\Device\\hv-n4r1b"
#define DOS_DEVICE_NAME     L"\\DosDevices\\hv-n4r1b"

#define DRIVER_NAME         "hv-n4r1b"   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif 

NTSTATUS DriverEntry(
    PDRIVER_OBJECT  DriverObject,
    PUNICODE_STRING RegistryPath
)
{

    NTSTATUS        status = STATUS_SUCCESS;
    PDEVICE_OBJECT  deviceObject;
    UNICODE_STRING  ntDeviceName, ntWin32DeviceName;

    UNREFERENCED_PARAMETER(RegistryPath);


    RtlInitUnicodeString(&ntDeviceName, NT_DEVICE_NAME);

    status = IoCreateDevice(
        DriverObject,
        0,
        &ntDeviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject);

    if (!NT_SUCCESS(status))
    {
        HvLogDebug("Couldn't create the device object\n");
        return status;
    }

    DriverObject->DriverUnload = DriverUnload;

    RtlInitUnicodeString(&ntWin32DeviceName, DOS_DEVICE_NAME);


    status = IoCreateSymbolicLink(&ntWin32DeviceName, &ntDeviceName);

    if (!NT_SUCCESS(status))
    {
        HvLogDebug("Couldn't create symbolic link\n");
        IoDeleteDevice(deviceObject);
    }


    return status;
}

VOID DriverUnload(
    PDRIVER_OBJECT DriverObject
)
{

    UNICODE_STRING ntWin32DeviceName;
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;

    PAGED_CODE();

    RtlInitUnicodeString(&ntWin32DeviceName, DOS_DEVICE_NAME);
    IoDeleteSymbolicLink(&ntWin32DeviceName);

    if (deviceObject != NULL) {
        IoDeleteDevice(deviceObject);
    }
}