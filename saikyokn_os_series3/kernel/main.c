#include "font.h"

typedef unsigned long long EFI_STATUS;
typedef void* EFI_HANDLE;
#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0

typedef struct { unsigned int Data1; unsigned short Data2; unsigned short Data3; unsigned char Data4[8]; } EFI_GUID;

// --- BootServicesを「正確に」定義する（これが一番確実や！） ---
typedef struct {
    char signature[8]; // "BOOTSERV"
    unsigned int revision;
    unsigned int header_size;
    unsigned int crc32;
    unsigned int reserved;

    // ここから関数ポインタ（規格の順番通りに並べるんや！）
    void *RaiseTPL; void *RestoreTPL;
    void *AllocatePages; void *FreePages; void *GetMemoryMap; void *AllocatePool; void *FreePool;
    void *CreateEvent; void *SetTimer; void *WaitForEvent; void *SignalEvent; void *CloseEvent; void *CheckEvent;
    void *InstallProtocolInterface; void *ReinstallProtocolInterface; void *UninstallProtocolInterface;
    void *HandleProtocol; void *Void_Reserved; void *RegisterProtocolNotify;
    void *LocateHandle; void *LocateDevicePath; void *InstallConfigurationTable;
    void *LoadImage; void *StartImage; void *Exit; void *UnloadImage; void *ExitBootServices;
    void *GetNextMonotonicCount; void *Stall; void *SetWatchdogTimer;
    void *ConnectController; void *DisconnectController;
    void *OpenProtocol; void *CloseProtocol; void *OpenProtocolInformation;
    void *ProtocolsPerHandle; void *LocateHandleBuffer;
    
    // これが本命の LocateProtocol (ついに構造体経由でアクセスや！)
    EFI_STATUS (EFIAPI *LocateProtocol)(EFI_GUID *Protocol, void *Registration, void **Interface);
    
    // ... (以下略)
} EFI_BOOT_SERVICES;

// シリアル関数群（信頼と実績のコード）
static inline void outb(unsigned short port, unsigned char val) { __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); }
static inline unsigned char inb(unsigned short port) { unsigned char val; __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port)); return val; }
void init_serial() { outb(0x3f8 + 1, 0x00); outb(0x3f8 + 3, 0x80); outb(0x3f8 + 0, 0x03); outb(0x3f8 + 1, 0x00); outb(0x3f8 + 3, 0x03); outb(0x3f8 + 2, 0xC7); }
void serial_puts(const char *s) { while (*s) { while ((inb(0x3f8 + 5) & 0x20) == 0); outb(0x3f8, *s++); } }

EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, void **SystemTablePtr) {
    init_serial();
    serial_puts("\r\n--- Saikyokn OS: Direct Struct Access ---\r\n");

    // BootServicesのアドレスを構造体として解釈
    EFI_BOOT_SERVICES *BS = (EFI_BOOT_SERVICES *)SystemTablePtr[12];

    // 署名チェック
    if (BS->signature[0] != 'B') {
        serial_puts("ERROR: Invalid BootServices address!\r\n");
        while(1);
    }

    EFI_GUID gop_guid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
    void *gop = 0;

    serial_puts("Locating GOP via Struct...\r\n");
    
    // 直接呼び出す！アセンブラを使わなくても、EFIAPIをつけて定義すればGCCがやってくれるんや。
    EFI_STATUS status = BS->LocateProtocol(&gop_guid, 0, &gop);

    if (status == EFI_SUCCESS && gop != 0) {
        serial_puts("!!! SUCCESS AT LAST !!!\r\n");
        
        // VRAM取得（GOP構造体の先頭から40バイト目あたりにあるFBBaseを直接狙う）
        unsigned long long *fb_ptr = (unsigned long long *)((unsigned char *)gop + 40);
        unsigned int *vram = (unsigned int *)(*fb_ptr);
        
        // 画面を燃えるような「赤(0xFF0000)」にするやで！
        for(int i=0; i<1000000; i++) vram[i] = 0xFF0000;
        serial_puts("SCREEN IS RED. WE WON!\r\n");
    } else {
        serial_puts("Still Failed. Check GUID or Table definition.\r\n");
    }

    while(1);
    return 0;
}