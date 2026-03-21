#include "font.h"
#include "console.h"

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

extern void irq1_wrapper();

// ================= キーボードバッファ =================
#define KB_BUF 256
volatile char kb_buf[KB_BUF];
volatile UINT32 kb_head = 0;
volatile UINT32 kb_tail = 0;

// asmから呼ばれる
__attribute__((sysv_abi))
void kb_push(char c){
    kb_buf[kb_head] = c;
    kb_head = (kb_head + 1) % KB_BUF;
}

int kb_pop(){
    if(kb_head == kb_tail) return -1;
    char c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF;
    return c;
}

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
    void *ConsoleInHandle;
    void *ConIn;
    void *ConsoleOutHandle;
    void *ConOut;
    void *StandardErrorHandle;
    void *StdErr;
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

// ================= IDT =================
struct IDTEntry {
    UINT16 offset_low, selector;
    UINT8 ist, type_attr;
    UINT16 offset_mid;
    UINT32 offset_high, zero;
} __attribute__((packed));

struct IDTR {
    UINT16 limit;
    UINT64 base;
} __attribute__((packed));

struct IDTEntry idt[256];

__attribute__((naked))
void dummy_isr(){
    __asm__("iretq");
}

void set_idt_entry(int vec, void* handler){
    UINT64 addr = (UINT64)handler;

    idt[vec].offset_low  = addr & 0xFFFF;
    idt[vec].selector    = 0x08;
    idt[vec].ist         = 0;
    idt[vec].type_attr   = 0x8E;
    idt[vec].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high = (addr >> 32);
    idt[vec].zero        = 0;
}

void load_idt(){
    struct IDTR idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (UINT64)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

// ================= PIC =================
void pic_init(){
    *(volatile UINT8*)0x20 = 0x11;
    *(volatile UINT8*)0xA0 = 0x11;
    *(volatile UINT8*)0x21 = 0x20;
    *(volatile UINT8*)0xA1 = 0x28;
    *(volatile UINT8*)0x21 = 0x04;
    *(volatile UINT8*)0xA1 = 0x02;
    *(volatile UINT8*)0x21 = 0x01;
    *(volatile UINT8*)0xA1 = 0x01;

    *(volatile UINT8*)0x21 = 0xFD;
    *(volatile UINT8*)0xA1 = 0xFF;
}

// ================= MAIN =================
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){

    EFI_BOOT_SERVICES *BS = SystemTable->BootServices;

    // ================= GOP =================
    EFI_GUID gop_guid =
        {0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;
    BS->LocateProtocol(&gop_guid, 0, (void**)&gop);

    UINT32 *vram   = (UINT32*)gop->Mode->FrameBufferBase;
    UINT32 stride  = gop->Mode->Info->PixelsPerScanLine;

    // ================= ExitBootServices =================
    UINT64 map_size=0, map_key, desc_size;
    UINT32 desc_ver;
    void *map_buf=0;

    BS->GetMemoryMap(&map_size,0,&map_key,&desc_size,&desc_ver);
    map_size += 1024;
    BS->AllocatePool(EfiLoaderData,map_size,&map_buf);

    while(1){
        if(BS->GetMemoryMap(&map_size,map_buf,&map_key,&desc_size,&desc_ver)!=EFI_SUCCESS)
            break;

        if(BS->ExitBootServices(ImageHandle,map_key)==EFI_SUCCESS)
            break;
    }

    // ================= IDT + IRQ =================
    for(int i=0;i<256;i++)
        set_idt_entry(i,dummy_isr);

    set_idt_entry(0x21, irq1_wrapper);

    pic_init();
    load_idt();
    __asm__("sti");

    // ================= コンソール =================
    console_init(vram, stride);

    console_write("SAIKYOKN OS CONSOLE READY\n");
    console_write("> ");
    console_render();

    // ================= メインループ =================
    while(1){

        int c = kb_pop();

        if(c != -1){

            console_putc((char)c);

            if(c == '\n'){
                console_write("> ");
            }

            console_render();
        }

        __asm__("hlt");
    }

    return EFI_SUCCESS;
}