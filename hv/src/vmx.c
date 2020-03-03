#include <wdm.h>
#include <intrin.h>


BOOLEAN IsVmxSupported()
{
    INT cpuid[4];

    __cpuid(cpuid, 1);

    // ECX bit 5 == VMX
    if (cpuid[2] & 0x20) {
        return TRUE;
    }
}

VOID EnableVmxOperation()
{
    ULONG cr4;

    cr4 = __readcr4();
    // Set bit 13 from CR4 - Virtual Machine Extensions Enable	
    cr4 |= 0x2000;
    __writecr4(cr4);
}