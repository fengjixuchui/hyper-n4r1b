#include <ntddk.h>

#include "../include/hv.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif 

PVP_DATA VmmState;

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

    DriverObject->MajorFunction[IRP_MJ_CREATE] = HvCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = HvClose;
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


PVP_DATA AllocVpData(
    VOID
)
{
    PVP_DATA virtualProcessorData;

    virtualProcessorData =
        ExAllocatePoolWithTag(NonPagedPool, sizeof(VP_DATA), VP_DATA_TAG);

    if (!virtualProcessorData) {
        HvLogDebug("Error allocating Virtual Processor data\n");
        return NULL;
    }

    RtlSecureZeroMemory(virtualProcessorData, sizeof(VP_DATA));

    return virtualProcessorData;
}

NTSTATUS InitHv(
    VOID
)
{

    NTSTATUS status = STATUS_SUCCESS;
    INT ProcessorsCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
    KAFFINITY kAffinityMask;

    if (!IsVmxSupported()) {
        HvLogDebug("VMX operation is not supported\n");
        return STATUS_NOT_SUPPORTED;
    }


    VmmState = ExAllocatePoolWithTag(NonPagedPool, sizeof(VP_DATA) * ProcessorsCount, VMM_STATE_TAG);

    for (INT i = 0; i < ProcessorsCount; i++) {
        
        // Taken from Sinae 
        kAffinityMask = ipow(2, i);
        KeSetSystemAffinityThread(kAffinityMask);

        EnableVmxOperation();

        VmmState[i] = *AllocVpData();

        AllocAndInitVmxonRegion(&VmmState[i]);
        AllocAndInitVmcsRegion(&VmmState[i]);
    }


    return status;
}

NTSTATUS HvCreate(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    InitHv();

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS HvClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    ExitVmxOperation();

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}