#pragma once

#include "util.h"
#include "arch/vmx.h"
#include "logger/logger.h"

// Driver's name
#define NT_DEVICE_NAME      L"\\Device\\hv-n4r1b"
#define DOS_DEVICE_NAME     L"\\DosDevices\\hv-n4r1b"

#define DRIVER_NAME         "hv-n4r1b"   

// Pooltags
#define VMM_STATE_TAG   'Hvvs'
#define VP_DATA_TAG     'Hvvp'


// Function declarations
DRIVER_UNLOAD DriverUnload;

DRIVER_INITIALIZE DriverEntry;

DRIVER_DISPATCH HvCreate;
DRIVER_DISPATCH HvClose;


// Structures
typedef struct _VP_DATA
{
    // Vmxon region
    UINT64 VmxonPad;
    PVMCS Vmxon;

    // Vmcs
    UINT64 VmcsPad;
    PVMCS Vmcs;
} VP_DATA, * PVP_DATA;

extern PVP_DATA VmmState;