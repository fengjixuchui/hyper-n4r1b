#include <wdm.h>


#define MSR_IA32_FEATURE_CONTROL 0x3a

// https://tandasat.github.io/HyperPlatform/doxygen/union_ia32_feature_control_msr.html
typedef union _IA32_FEATURE_CONTROL_MSR
{
    UINT64 All;
    struct
    {
        UINT64 Lock : 1;
        UINT64 EnableSMX : 1;
        UINT64 EnableVmxon : 1;
        UINT64 Reserved2 : 5;
        UINT64 EnableLocalSENTER : 7;
        UINT64 EnableGlobalSENTER : 1;
        UINT64 Reserved3a : 16;
        UINT64 Reserved3b : 32;
    } Fields;
} IA32_FEATURE_CONTROL_MSR, * PIA32_FEATURE_CONTROL_MSR;


typedef struct _VMCS
{
    union {
        UINT32 All;
        struct {
            UINT32 RevisionId : 31;
            UINT32 ShadowVmcsIndicator : 1;
        } Bits;
    } Header;

    UINT32 AbortIndicator;
    UINT8 Data[PAGE_SIZE - 8];
} VMCS, * PVMCS;

BOOLEAN IsVmxSupported();
VOID EnableVmxOperation();