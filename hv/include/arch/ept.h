#pragma once

#pragma warning(disable:4214)

#define PML4E_ENTRY_COUNT   512
#define PDPTE_ENTRY_COUNT   512
#define PDE_ENTRY_COUNT     512

typedef enum _EPT_MEMORY_TYPE
{
    Uncacheable = 0,
    WriteBack = 6
} EPT_MEMORY_TYPE;

// 24.6.11 Extended-Page-Table Pointer (EPTP) -  Table 24-8. Format of Extended-Page-Table Pointer
typedef struct _EPTP
{
    UINT64 All;
    union {
        EPT_MEMORY_TYPE MemoryType : 3;
        UINT64 PageWalkLength : 3;
        UINT64 EnableAccessedAndDirty : 1;
        UINT64 EnforcementShadowStackPages : 1; // Enables enforcement of access rights for supervisor shadow-stack pages
        UINT64 Reserved : 4;
        UINT64 PML4TableAddress : 40; // Physical-address width supported by the logical processor should be check cpuid.80000008H
        UINT64 Reserved1 : 12;
    } Fields;
} EPTP, *PEPTP;

// Table 28-1. Format of an EPT PML4 Entry (PML4E) that References an EPT Page-Directory-Pointer Table
// 512 64-bit entries - An EPT PML4E is selected using the physical address defined as follows: 
//    63-52                51-12 from EPTP               11-3 are Guest PA 47-39     2-0         
// 000000000000 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX      YYYYYYYYY               000
typedef struct _EPT_PML4_ENTRY
{
    UINT64 All;
    union {
        UINT64 ReadAccess : 1;
        UINT64 WriteAccess : 1;
        UINT64 ExecuteAccessSupervisorMode : 1;
        UINT64 Reserved : 5;
        UINT64 Accessed : 1; // If bit 6 of EPTP is set
        UINT64 Ignored : 1;
        UINT64 ExecuteAccessUserMode : 1; // If the “mode-based execute control for EPT” VM-execution control is 1
        UINT64 Ignored1 : 1;
        UINT64 PDPTableAddress : 40; // Physical address of 4-KByte aligned EPT page-directory-pointer table referenced by this entry
        UINT64 Reserved2 : 12;
    } Fields;
} EPT_PML4_ENTRY, * PEPT_PML4_ENTRY;

// Table 28-3. Format of an EPT Page-Directory-Pointer-Table Entry (PDPTE) that References an EPT Page Directory
// 512 64-bit entries - An EPT PDPTE is selected using the physical address defined as follows:  
//    63-52                51-12 from EPT PML4E           11-3 are Guest PA 38-30     2-0         
// 000000000000 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        YYYYYYYYY              000
typedef struct _EPT_PDPT_ENTRY
{
    UINT64 All;
    union {
        UINT64 ReadAccess : 1;
        UINT64 WriteAccess : 1;
        UINT64 ExecuteAccessSupervisorMode : 1;
        UINT64 Reserved : 5;
        UINT64 Accessed : 1;
        UINT64 Ignored : 1;
        UINT64 ExecuteAccessUserMode : 1;
        UINT64 Ignored1 : 1;
        UINT64 PageDirectoryAddress : 40; // Physical address of 4-KByte aligned EPT page directory referenced by this entry
        UINT64 Reserved2 : 12;
    } Fields;
} EPT_PDPT_ENTRY, *PEPT_PDPT_ENTRY;

// Table 28-5. Format of an EPT Page-Directory Entry (PDE) that References an EPT Page Table
// 512 64-bit entries - An EPT PDPTE is selected using the physical address defined as follows:  
//    63-52                51-12 from EPT PDPTE           11-3 are Guest PA 29-21     2-0         
// 000000000000 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        YYYYYYYYY              000
typedef struct _EPT_PD_ENTRY
{
    UINT64 All;
    union {
        UINT64 ReadAccess : 1;
        UINT64 WriteAccess : 1;
        UINT64 ExecuteAccessSupervisorMode : 1;
        UINT64 Reserved : 5;
        UINT64 Accessed : 1;
        UINT64 Ignored : 1;
        UINT64 ExecuteAccessUserMode : 1;
        UINT64 Ignored1 : 1;
        UINT64 PageTableAddress : 40; // Physical address of 4-KByte aligned EPT page directory referenced by this entry1
        UINT64 Reserved2 : 12;
    } Fields;
} EPT_PD_ENTRY, * PEPT_PD_ENTRY;

// Table 28-6. Format of an EPT Page-Table Entry that Maps a 4-KByte Page
// 512 64-bit entries - An EPT PDPTE is selected using the physical address defined as follows:  
//    63-52                51-12 from EPT PDE           11-3 are Guest PA 20-12     2-0         
// 000000000000 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        YYYYYYYYY            000
typedef struct _EPT_PT_ENTRY
{
    UINT64 All;
    union {
        UINT64 ReadAccess : 1;
        UINT64 WriteAccess : 1;
        UINT64 ExecuteAccessSupervisorMode : 1;
        EPT_MEMORY_TYPE MemoryType : 3;
        UINT64 IgnorePAT : 1;
        UINT64 Ignored : 1;
        UINT64 Accessed : 1;
        UINT64 Dirty : 1;
        UINT64 ExecuteAccessUserMode : 1;
        UINT64 Ignored1 : 1;
        UINT64 EntryPhysicalAddress : 39; // Physical address of the 4-KByte page referenced by this entry
        UINT64 Reserved : 1; // Reserved (must be 0)
        UINT64 Ignored2 : 8;
        UINT64 SupervisorShadowStack : 1;
        UINT64 SubPageWritePermission : 1;
        UINT64 Ignored3 : 1;
        UINT64 SupressVE : 1;
    } Fields;
} EPT_PT_ENTRY, * PEPT_PT_ENTRY;

VOID
InitializeEpt(
    PVP_DATA
);