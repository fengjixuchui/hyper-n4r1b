/*
    VmExit Handler, highly based on Ionescu's simple visor. I believe he has one of the cleaneast codes
    when it comes to keeping the context and resuming the vm execution
*/

#include <ntifs.h>
#include <intrin.h>

#include "../include/hv.h"

DECLSPEC_NORETURN
VOID
VmxResume(
    VOID
)
{
    __vmx_vmresume();
}

VOID
MoveInstructionPointer(
    VOID
)
{
    uintptr_t guestRip;
    size_t instrLen;

    __vmx_vmread(GUEST_RIP, &guestRip);
    __vmx_vmread(VM_EXIT_INSTRUCTION_LEN, &instrLen);

    __vmx_vmwrite(GUEST_RIP, ((uintptr_t)guestRip + instrLen));
}


VOID
VmExitHandler(
    PCONTEXT Context
)
{
    size_t exitReason = 0;
    size_t exitQualifitaction = 0;

    // Restore the real value of rcx when the vmexit was triggered. We pushed this value to the stack in the VmExitStub
    Context->Rcx = *(UINT64*)((uintptr_t)_ReturnAddress() - sizeof(Context->Rcx));

    __vmx_vmread(EXIT_REASON, &exitReason);
    exitReason = exitReason & 0xFFFF;

    __vmx_vmread(EXIT_QUALIFICATION, &exitQualifitaction);

    HvLogDebug("Exit Reason: 0x%02X\n", exitReason);
    HvLogDebug("Exit Qualification: 0x%X\n", exitQualifitaction);

    switch (exitReason) {
        case EXIT_REASON_VMCLEAR:
        case EXIT_REASON_VMPTRLD:
        case EXIT_REASON_VMPTRST:
        case EXIT_REASON_VMREAD:
        case EXIT_REASON_VMRESUME:
        case EXIT_REASON_VMWRITE:
        case EXIT_REASON_VMXOFF:
        case EXIT_REASON_VMXON:
        case EXIT_REASON_VMLAUNCH:
            break;
        case EXIT_REASON_HLT:
            HvLogDebug("Execution of HLT detected... \n");
            break;
        default:
            DbgBreakPoint();
    }

    MoveInstructionPointer();

}