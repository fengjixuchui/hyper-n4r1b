#include <wdm.h>
#include <intrin.h>

#include "..\include\arch\vmx.h"
#include "..\include\logger\logger.h"

BOOLEAN IsVmxSupported()
{
    INT cpuid[4];
    
    __cpuid(cpuid, 1);

    // ECX bit 5 == VMX
    if (!(cpuid[2] & 0x20)) {
        return FALSE;
    }

    IA32_FEATURE_CONTROL_MSR Control = { 0 };
    Control.All = __readmsr(MSR_IA32_FEATURE_CONTROL);

    if (Control.Fields.Lock == 0) {
        Control.Fields.Lock = TRUE;
        Control.Fields.EnableVmxon = TRUE;
        __writemsr(MSR_IA32_FEATURE_CONTROL, Control.All);
    }
    else if (Control.Fields.EnableVmxon == FALSE) {
        HvLogDebug("VMX locked in BIOS/UEFI");
        return FALSE;
        }

    return TRUE;
}

VOID EnableVmxOperation()
{
    UINT64 cr4;

    cr4 = __readcr4();
    // Set bit 13 from CR4 - Virtual Machine Extensions Enable	
    cr4 |= 0x2000;
    __writecr4(cr4);

    HvLogDebug("VMX operation enabled");
}