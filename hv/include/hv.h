#include "arch/vmx.h"

typedef struct _VCPU_DATA
{
    VMCS VmxOn;
    VMCS Vmcs;
} VCPU_DATA, * PVCPU_DATA;