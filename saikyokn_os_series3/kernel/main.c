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

// ================= シリアル =================
static inline void outb(UINT16 port, UINT8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline UINT8 inb(UINT16 port) {
    UINT8 v;
    __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}
void init_serial() {
    outb(0x3f8+1,0);
    outb(0x3f8+3,0x80);
    outb(0x3f8+0,3);
    outb(0x3f8+1,0);
    outb(0x3f8+3,3);
    outb(0x3f8+2,0xC7);
}
void serial_puts(const char *s) {
    while(*s){
        while(!(inb(0x3f8+5)&0x20));
        outb(0x3f8,*s++);
    }
}

// ================= メイン =================
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    init_serial();
    serial_puts("\r\n=== Saikyokn OS v3.1: Stable Kernel Entry ===\r\n");

    EFI_BOOT_SERVICES *BS = SystemTable->BootServices;

    EFI_GUID gop_guid =
        {0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;

    BS->LocateProtocol(&gop_guid, 0, (void**)&gop);

    UINT32 *vram = (UINT32*)gop->Mode->FrameBufferBase;
    UINT32 hr = gop->Mode->Info->HorizontalResolution;
    UINT32 vr = gop->Mode->Info->VerticalResolution;
    UINT32 stride = gop->Mode->Info->PixelsPerScanLine;

    // ================= メモリマップ =================
    UINT64 map_size = 0, map_key, desc_size;
    UINT32 desc_ver;
    void *map_buf = 0;

    BS->GetMemoryMap(&map_size, 0, &map_key, &desc_size, &desc_ver);
    map_size += 1024;

    BS->AllocatePool(EfiLoaderData, map_size, &map_buf);

    // ================= ExitBootServices =================
    EFI_STATUS status;

    while (1) {
        status = BS->GetMemoryMap(&map_size, map_buf, &map_key, &desc_size, &desc_ver);
        if (status != EFI_SUCCESS) break;

        status = BS->ExitBootServices(ImageHandle, map_key);
        if (status == EFI_SUCCESS) break;
    }

    // ================= 完全独立後 =================

    // 👉 青固定（PixelFormat無視して確実に青にする）
    UINT32 blue = 0x0000FF;

    for (UINT32 y = 0; y < vr; y++) {
        for (UINT32 x = 0; x < hr; x++) {
            vram[y * stride + x] = blue;
        }
    }

    draw_string(vram, stride, 100, 100, 0xFFFFFF, "SAIKYOKN OS v3.1");
    draw_string(vram, stride, 100, 120, 0x00FF00, "KERNEL: ACTIVE");
    draw_string(vram, stride, 100, 140, 0xFFFF00, "STATUS: INDEPENDENT");

    while (1) {
        __asm__("hlt");
    }

    return EFI_SUCCESS;
}