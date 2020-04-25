#pragma once

#include "util.h"
#include "arch/vmx.h"
#include "arch/vmcs.h"
#include "arch/ept.h"
#include "logger/logger.h"
#include "arch/vmexit.h"

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

    CONTEXT Context;

    // MSR Bitmap
    UINT64 MsrBitmapPad;
    UINT8 MsrBitmap[PAGE_SIZE];

    // EPT related variables
    UINT64 EptPointer;
    UINT32 EptControls;
    EPT_PML4_ENTRY EptPml4[PML4E_ENTRY_COUNT];  // 512 PML4 Entries 
    EPT_PDPT_ENTRY EptPdp[PDPTE_ENTRY_COUNT];   // 512 PDPT Entries
    EPT_PD_ENTRY EptPde[PDE_ENTRY_COUNT];
    
    DECLSPEC_ALIGN(PAGE_SIZE) UINT8 VmmStack[KERNEL_STACK_SIZE];
} VP_DATA, * PVP_DATA;

extern PVP_DATA VmmState;
