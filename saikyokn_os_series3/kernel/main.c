#include "font.h"

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

// ================= TABLE =================
typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

// ================= Boot Services =================
typedef struct EFI_BOOT_SERVICES EFI_BOOT_SERVICES;

typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP)(UINT64 *, void *, UINT64 *, UINT64 *, UINT32 *);
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(UINT32, UINT64, void **);
typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE, UINT64);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL)(EFI_GUID *, void *, void **);

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
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR*)(ptr + i);
        if (desc->Type == 7 && region_count < MAX_REGIONS) {
            regions[region_count].base = desc->PhysicalStart;
            regions[region_count].size = desc->NumberOfPages * 4096;
            region_count++;
        }
    }
}

// ================= SAFE HEAP =================
UINT64 heap_current;
UINT64 heap_end;

void init_heap() {
    if (region_count == 0) return;
    heap_current = regions[0].base;
    heap_end     = regions[0].base + regions[0].size;
}

// 8バイトアライン + 範囲チェック
void* kmalloc(UINT64 size) {
    size = (size + 7) & ~7;
    for (UINT32 i = 0; i < region_count; i++) {
        UINT64 start = (heap_current + 7) & ~7;
        if (start + size <= regions[i].base + regions[i].size) {
            void* ptr = (void*)start;
            heap_current = start + size;
            return ptr;
        } else {
            heap_current = regions[i].base;
        }
    }
    return 0; // メモリ不足
}

// ================= IDT =================
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

// シリアルデバッグ出力（COM1）
#define COM1_PORT 0x3F8
static inline void serial_write_char(char c) {
    while (!( *(volatile UINT8*)(COM1_PORT + 5) & 0x20 ));
    *(volatile UINT8*)(COM1_PORT) = c;
}
static void serial_write(const char *s) {
    while (*s) serial_write_char(*s++);
}

// 基本ハンドラ
void isr_div_zero()  { serial_write("DIV0\n");  __asm__("iretq"); }
void isr_page_fault(){ serial_write("PF\n");    __asm__("iretq"); }
void dummy_isr()     { __asm__("iretq"); }

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

// ================= 画面描画 =================
static inline void draw_pixel(UINT32 *vram, UINT32 stride, UINT32 x, UINT32 y, UINT32 color) {
    vram[y * stride + x] = color;
}

static void draw_rect(UINT32 *vram, UINT32 stride, UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color) {
    for (UINT32 j=0;j<h;j++) for(UINT32 i=0;i<w;i++) draw_pixel(vram,stride,x+i,y+j,color);
}

// ================= MAIN =================
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_BOOT_SERVICES *BS = SystemTable->BootServices;

    // GOP
    EFI_GUID gop_guid = {0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;
    BS->LocateProtocol(&gop_guid,0,(void**)&gop);
    UINT32 *vram = (UINT32*)gop->Mode->FrameBufferBase;
    UINT32 hr = gop->Mode->Info->HorizontalResolution;
    UINT32 vr = gop->Mode->Info->VerticalResolution;
    UINT32 stride = gop->Mode->Info->PixelsPerScanLine;

    // メモリマップ
    UINT64 map_size = 0, map_key, desc_size;
    UINT32 desc_ver;
    void *map_buf = 0;
    BS->GetMemoryMap(&map_size,0,&map_key,&desc_size,&desc_ver);
    map_size += 1024;
    BS->AllocatePool(EfiLoaderData,map_size,&map_buf);

    // ExitBootServices
    while(1){
        if(BS->GetMemoryMap(&map_size,map_buf,&map_key,&desc_size,&desc_ver)!=EFI_SUCCESS) break;
        if(BS->ExitBootServices(ImageHandle,map_key)==EFI_SUCCESS) break;
    }

    // PMM + heap
    init_pmm(map_buf,map_size,desc_size);
    init_heap();
    void *p = kmalloc(64);

    // IDT設定
    for(int i=0;i<256;i++) set_idt_entry(i,dummy_isr);
    set_idt_entry(0,isr_div_zero);
    set_idt_entry(14,isr_page_fault);
    load_idt();
    __asm__("sti");

    // 背景描画
    draw_rect(vram,stride,0,0,hr,vr,0x00202020);

    // 文字表示
    draw_string(vram,stride,100,100,0xFFFFFF,"STABLE CORE OK");
    draw_string(vram,stride,100,120,0x00FF00,"PMM / SAFE KMALLOC / IDT READY");

    while(1) __asm__("hlt");
    return EFI_SUCCESS;
}