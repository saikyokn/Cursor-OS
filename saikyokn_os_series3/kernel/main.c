#include "font.h"

// ================= 基本型 =================
typedef unsigned long long UINT64;
typedef unsigned int       UINT32;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;
typedef UINT64             EFI_STATUS;
typedef void*              EFI_HANDLE;

#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0
#define EfiLoaderData 4

// ================= GUID =================
typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

// ================= テーブル =================
typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

// ================= Boot Services =================
typedef struct EFI_BOOT_SERVICES EFI_BOOT_SERVICES;

typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP)(
    UINT64 *, void *, UINT64 *, UINT64 *, UINT32 *);

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(
    UINT32, UINT64, void **);

typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES)(
    EFI_HANDLE, UINT64);

typedef EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL)(
    EFI_GUID *, void *, void **);

struct EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;

    void *RaiseTPL;
    void *RestoreTPL;
    void *AllocatePages;
    void *FreePages;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL AllocatePool;
    void *FreePool;
    void *CreateEvent;
    void *SetTimer;
    void *WaitForEvent;
    void *SignalEvent;
    void *CloseEvent;
    void *CheckEvent;
    void *InstallProtocolInterface;
    void *ReinstallProtocolInterface;
    void *UninstallProtocolInterface;
    void *HandleProtocol;
    void *Reserved;
    void *RegisterProtocolNotify;
    void *LocateHandle;
    void *LocateDevicePath;
    void *InstallConfigurationTable;
    void *LoadImage;
    void *StartImage;
    void *Exit;
    void *UnloadImage;
    EFI_EXIT_BOOT_SERVICES ExitBootServices;
    void *GetNextMonotonicCount;
    void *Stall;
    void *SetWatchdogTimer;
    void *ConnectController;
    void *DisconnectController;
    void *OpenProtocol;
    void *CloseProtocol;
    void *OpenProtocolInformation;
    void *ProtocolsPerHandle;
    void *LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL LocateProtocol;
    void *InstallMultipleProtocolInterfaces;
    void *UninstallMultipleProtocolInterfaces;
    void *CalculateCrc32;
    void *CopyMem;
    void *SetMem;
    void *CreateEventEx;
};

// ================= System Table =================
typedef struct {
    EFI_TABLE_HEADER Hdr;
    void *FirmwareVendor;
    UINT32 FirmwareRevision;
    void *padding[6];
    void *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
} EFI_SYSTEM_TABLE;

// ================= GOP =================
typedef struct {
    UINT32 Version;
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelFormat;
    UINT32 PixelInformation[4];
    UINT32 PixelsPerScanLine;
} EFI_GOP_INFO;

typedef struct {
    UINT32 MaxMode;
    UINT32 Mode;
    EFI_GOP_INFO *Info;
    UINT64 SizeOfInfo;
    UINT64 FrameBufferBase;
    UINT64 FrameBufferSize;
} EFI_GOP_MODE;

typedef struct {
    void *QueryMode;
    void *SetMode;
    void *Blt;
    EFI_GOP_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

// ================= PMM =================
typedef struct {
    UINT32 Type;
    UINT32 Pad;
    UINT64 PhysicalStart;
    UINT64 VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef struct {
    UINT64 base;
    UINT64 size;
} MemoryRegion;

#define MAX_REGIONS 128
MemoryRegion regions[MAX_REGIONS];
UINT32 region_count = 0;

void init_pmm(void *map, UINT64 map_size, UINT64 desc_size) {
    UINT8 *ptr = (UINT8*)map;

    for (UINT64 i = 0; i < map_size; i += desc_size) {
        EFI_MEMORY_DESCRIPTOR *desc =
            (EFI_MEMORY_DESCRIPTOR*)(ptr + i);

        if (desc->Type == 7 && region_count < MAX_REGIONS) {
            regions[region_count].base = desc->PhysicalStart;
            regions[region_count].size = desc->NumberOfPages * 4096;
            region_count++;
        }
    }
}

// ================= kmalloc =================
UINT64 heap_current;
UINT64 heap_end;

void init_heap() {
    if (region_count == 0) return;

    heap_current = regions[0].base;
    heap_end     = regions[0].base + regions[0].size;
}

void* kmalloc(UINT64 size) {
    size = (size + 7) & ~7;

    if (heap_current + size > heap_end) return 0;

    void* ptr = (void*)heap_current;
    heap_current += size;
    return ptr;
}

// ================= IDT（安全版） =================
struct IDTEntry {
    UINT16 offset_low;
    UINT16 selector;
    UINT8  ist;
    UINT8  type_attr;
    UINT16 offset_mid;
    UINT32 offset_high;
    UINT32 zero;
} __attribute__((packed));

struct IDTR {
    UINT16 limit;
    UINT64 base;
} __attribute__((packed));

struct IDTEntry idt[256];

// ダミーハンドラ（何もしない）
void dummy_isr() {
    __asm__ volatile ("iretq");
}

void set_idt_entry(int vec, void* handler) {
    UINT64 addr = (UINT64)handler;

    idt[vec].offset_low  = addr & 0xFFFF;
    idt[vec].selector    = 0x08;
    idt[vec].ist         = 0;
    idt[vec].type_attr   = 0x8E;
    idt[vec].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high = (addr >> 32);
    idt[vec].zero        = 0;
}

void load_idt() {
    struct IDTR idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (UINT64)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

// ================= メイン =================
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    EFI_BOOT_SERVICES *BS = SystemTable->BootServices;

    // GOP
    EFI_GUID gop_guid =
        {0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;
    BS->LocateProtocol(&gop_guid, 0, (void**)&gop);

    UINT32 *vram = (UINT32*)gop->Mode->FrameBufferBase;
    UINT32 hr = gop->Mode->Info->HorizontalResolution;
    UINT32 vr = gop->Mode->Info->VerticalResolution;
    UINT32 stride = gop->Mode->Info->PixelsPerScanLine;

    // メモリマップ
    UINT64 map_size = 0, map_key, desc_size;
    UINT32 desc_ver;
    void *map_buf = 0;

    BS->GetMemoryMap(&map_size, 0, &map_key, &desc_size, &desc_ver);
    map_size += 1024;
    BS->AllocatePool(EfiLoaderData, map_size, &map_buf);

    // ExitBootServices
    while (1) {
        if (BS->GetMemoryMap(&map_size, map_buf, &map_key, &desc_size, &desc_ver) != EFI_SUCCESS)
            break;

        if (BS->ExitBootServices(ImageHandle, map_key) == EFI_SUCCESS)
            break;
    }

    // PMM + heap
    init_pmm(map_buf, map_size, desc_size);
    init_heap();

    // kmallocテスト（安全）
    void *p = kmalloc(64);

    // IDT（全部ダミー）
    for (int i = 0; i < 256; i++) {
        set_idt_entry(i, dummy_isr);
    }

    load_idt();
    __asm__("sti");

    // 画面描画
    for (UINT32 y = 0; y < vr; y++) {
        for (UINT32 x = 0; x < hr; x++) {
            vram[y * stride + x] = 0x00202020;
        }
    }

    draw_string(vram, stride, 100, 100, 0xFFFFFF, "STABLE CORE OK");
    draw_string(vram, stride, 100, 120, 0x00FF00, "PMM / KMALLOC / IDT READY");

    while (1) {
        __asm__("hlt");
    }

    return EFI_SUCCESS;
}