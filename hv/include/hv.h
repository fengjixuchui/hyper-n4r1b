#include "arch/vmx.h"

#define VMM_STATE_TAG  "Hvvs"
#define VP_DATA_TAG "Hvvp"


typedef struct _VP_DATA
{
    // Vmxon region
    UINT64 VmxonPad;
    PVMCS Vmxon;

    // Vmcs
    UINT64 VmcsPad;
    PVMCS Vmcs;
} VP_DATA, * PVP_DATA;

PVP_DATA VmmState;