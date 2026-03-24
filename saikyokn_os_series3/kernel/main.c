#include "font.h"
#include "console.h"

// ===== 基本型 =====
typedef unsigned long long UINT64;
typedef unsigned int       UINT32;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;

typedef UINT64 EFI_STATUS;
typedef void*  EFI_HANDLE;

#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0
#define EfiLoaderData 4

// ===== GUID =====
typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

// ===== テーブル =====
typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

// ===== BootServices =====
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

// ===== SystemTable =====
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

// ===== GOP =====
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

// ===== shell =====
extern void shell_run(char* input);

// ===== I/O =====
static inline UINT8 in8(UINT16 port){
    UINT8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// ===== キーボード =====
int keyboard_ready(){ return in8(0x64) & 1; }
UINT8 keyboard_read(){ return in8(0x60); }

// ===== スキャンコード =====
char keytable[128] = {
0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,
'\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
0,'a','s','d','f','g','h','j','k','l',';',39,'`',
0,'\\','z','x','c','v','b','n','m',',','.','/',0,
'*',0,' '
};

// ===== 入力 =====
#define INPUT_MAX 128
char input[INPUT_MAX];
int input_len = 0;

// ===== 画面クリア =====
void clear_screen(UINT32* vram, UINT32 stride, UINT32 w, UINT32 h){
    for(UINT32 y = 0; y < h; y++){
        for(UINT32 x = 0; x < w; x++){
            vram[y * stride + x] = 0x00000000;
        }
    }
}

// ===== MAIN =====
EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){

    EFI_BOOT_SERVICES *BS = SystemTable->BootServices;

    // ===== GOP =====
    EFI_GUID gop_guid =
        {0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;
    BS->LocateProtocol(&gop_guid, 0, (void**)&gop);

    UINT32 *vram  = (UINT32*)gop->Mode->FrameBufferBase;
    UINT32 stride = gop->Mode->Info->PixelsPerScanLine;
    UINT32 width  = gop->Mode->Info->HorizontalResolution;
    UINT32 height = gop->Mode->Info->VerticalResolution;

    // ===== ExitBootServices =====
    UINT64 map_size=0, map_key, desc_size;
    UINT32 desc_ver;
    void *map_buf=0;

    BS->GetMemoryMap(&map_size,0,&map_key,&desc_size,&desc_ver);
    map_size += 1024;
    BS->AllocatePool(EfiLoaderData,map_size,&map_buf);

    while(1){
        if(BS->GetMemoryMap(&map_size,map_buf,&map_key,&desc_size,&desc_ver)!=EFI_SUCCESS)
            continue;
        if(BS->ExitBootServices(ImageHandle,map_key)==EFI_SUCCESS)
            break;
    }

    // ===== カーネル開始 =====
    console_init(vram, stride);

    // 🔥 黒背景
    clear_screen(vram, stride, width, height);

    console_write("SAIKYOKN OS\n> ");
    console_render();

    int shift = 0;

    // ===== メインループ =====
    while(1){

        if(keyboard_ready()){

            UINT8 sc = keyboard_read();

            if(sc == 0x2A || sc == 0x36){ shift = 1; continue; }
            if(sc == 0xAA || sc == 0xB6){ shift = 0; continue; }

            if(sc & 0x80) continue;

            char c = keytable[sc];
            if(!c) continue;

            if(shift && c >= 'a' && c <= 'z'){
                c -= 32;
            }

            // Enter
            if(c == '\n'){
                input[input_len] = 0;
                shell_run(input);
                input_len = 0;
                console_write("> ");
                console_render();
                continue;
            }

            // Backspace
            if(c == 8){
                if(input_len > 0){
                    input_len--;
                    console_putc('\b');
                    console_render();
                }
                continue;
            }

            // 通常入力
            if(input_len < INPUT_MAX-1){
                input[input_len++] = c;
                console_putc(c);
                console_render();
            }
        }
    }

    return EFI_SUCCESS;
}