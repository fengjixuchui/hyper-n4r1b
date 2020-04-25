#pragma once

#pragma warning(disable:4201)
#pragma warning(disable:4214)

typedef struct _KDESCRIPTOR
{
    USHORT Pad[3];
    USHORT Limit;
    VOID* Base;
} KDESCRIPTOR, * PKDESCRIPTOR;

typedef union _KGDTENTRY64
{
    struct
    {
        UINT16 LimitLow;
        UINT16 BaseLow;
        union
        {
            struct
            {
                UINT8 BaseMiddle;
                UINT8 Flags1;
                UINT8 Flags2;
                UINT8 BaseHigh;
            } Bytes;
            struct
            {
                UINT32 BaseMiddle : 8;
                UINT32 Type : 5;
                UINT32 Dpl : 2;
                UINT32 Present : 1;
                UINT32 LimitHigh : 4;
                UINT32 System : 1;
                UINT32 LongMode : 1;
                UINT32 DefaultBig : 1;
                UINT32 Granularity : 1;
                UINT32 BaseHigh : 8;
            } Bits;
        };
        UINT32 BaseUpper;
        UINT32 MustBeZero;
    };
    struct
    {
        INT64 DataLow;
        INT64 DataHigh;
    };
} KGDTENTRY64, *PKGDTENTRY64;

// Taken from https://github.com/ionescu007/SimpleVisor/blob/0ed192a566df15476f329b6adc4d638e65d4ea5c/vmx.h#L490
typedef struct _VMX_GDTENTRY64
{
    UINT64 Base;
    UINT32 Limit;
    union
    {
        struct
        {
            UINT8 Flags1;
            UINT8 Flags2;
            UINT8 Flags3;
            UINT8 Flags4;
        } Bytes;
        struct
        {
            UINT16 SegmentType : 4;
            UINT16 DescriptorType : 1;
            UINT16 Dpl : 2;
            UINT16 Present : 1;

            UINT16 Reserved : 4;
            UINT16 System : 1;
            UINT16 LongMode : 1;
            UINT16 DefaultBig : 1;
            UINT16 Granularity : 1;

            UINT16 Unusable : 1;
            UINT16 Reserved2 : 15;
        } Bits;
        UINT32 AccessRights;
    };
    UINT16 Selector;
} VMX_GDTENTRY64, * PVMX_GDTENTRY64;


typedef struct _VMX_SELECTORS
{
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    VMX_GDTENTRY64 GdtEntryCS;
    VMX_GDTENTRY64 GdtEntrySS;
    VMX_GDTENTRY64 GdtEntryDS;
    VMX_GDTENTRY64 GdtEntryES;
    VMX_GDTENTRY64 GdtEntryFS;
    VMX_GDTENTRY64 GdtEntryGS;
    VMX_GDTENTRY64 GdtEntryLDTR;
    VMX_GDTENTRY64 GdtEntryTR;
} VMX_SELECTORS, * PVMX_SELECTORS;

unsigned short __getcs(void);
unsigned short __getss(void);
unsigned short __getds(void);
unsigned short __getes(void);
unsigned short __getfs(void);
unsigned short __getgs(void);
unsigned short __getldtr(void);
unsigned short __gettr(void);
void __sgdt(void*);

NTSTATUS
GetVmxGdtEntry(
    PVOID,
    USHORT, 
    PVMX_GDTENTRY64
);