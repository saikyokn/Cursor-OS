#include "font.h"

typedef unsigned long long EFI_STATUS;
typedef void* EFI_HANDLE;
#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0

// --- UEFI 構造体定義 ---
typedef struct {
    unsigned long long Signature;
    unsigned int Revision;
    unsigned int HeaderSize;
    unsigned int CRC32;
    unsigned int Reserved;
} EFI_TABLE_HEADER;

typedef struct {
    unsigned int Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} EFI_GUID;

typedef struct {
    int RelativeMovementX;
    int RelativeMovementY;
    int RelativeMovementZ;
    unsigned char LeftButton;
    unsigned char RightButton;
} EFI_SIMPLE_POINTER_STATE;

struct _EFI_SIMPLE_POINTER_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_POINTER_RESET)(struct _EFI_SIMPLE_POINTER_PROTOCOL *This, unsigned char ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_POINTER_GET_STATE)(struct _EFI_SIMPLE_POINTER_PROTOCOL *This, EFI_SIMPLE_POINTER_STATE *State);

typedef struct _EFI_SIMPLE_POINTER_PROTOCOL {
    EFI_SIMPLE_POINTER_RESET Reset;
    EFI_SIMPLE_POINTER_GET_STATE GetState;
    void *WaitForInput;
} EFI_SIMPLE_POINTER_PROTOCOL;

typedef struct {
    unsigned int Version;
    unsigned int HorizontalResolution;
    unsigned int VerticalResolution;
    int PixelFormat;
    unsigned int PixelInformation[4];
    unsigned int PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    unsigned int MaxMode;
    unsigned int Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    unsigned long long SizeOfInfo;
    unsigned long long FrameBufferBase;
    unsigned long long FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct {
    void *dummy[3];
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    void *dummy1[6];
    void *dummy2[2];
    void *dummy3[3];
    void *dummy4[3];
    void *dummy5[2];
    void *dummy6[1];
    void *dummy7[2];
    void *dummy8[2];
    void *dummy9[3];
    void *dummy10[1];
    void *dummy11[1];
    void *dummy12[2];
    void *dummy13[1];
    void *dummy14[2];
    void *dummy15[2];
    void *dummy16[1];
    void *dummy17[3];
    EFI_STATUS (EFIAPI *LocateProtocol)(EFI_GUID *Protocol, void *Registration, void **Interface);
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    unsigned short *FirmwareVendor;
    unsigned int FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    void *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    void *ConOut;
    EFI_HANDLE StandardErrorHandle;
    void *StdErr;
    void *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
} EFI_SYSTEM_TABLE;

// --- ユーティリティ関数 ---

void fill_rect(unsigned int *vram, unsigned int stride, int x, int y, int w, int h, unsigned int color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            vram[i * stride + j] = color;
        }
    }
}

void itoa_hex(unsigned long long val, char *str) {
    const char *hex_chars = "0123456789ABCDEF";
    for (int i = 15; i >= 0; i--) {
        str[i] = hex_chars[val & 0xF];
        val >>= 4;
    }
    str[16] = '\0';
}

void itoa_dec(int val, char *str) {
    int i = 0;
    if (val < 0) {
        str[i++] = '-';
        val = -val;
    }
    char tmp[10];
    int j = 0;
    if (val == 0) tmp[j++] = '0';
    while (val > 0) {
        tmp[j++] = (val % 10) + '0';
        val /= 10;
    }
    while (j > 0) str[i++] = tmp[--j];
    str[i] = '\0';
}

// --- メインルーチン ---

EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_GUID gop_guid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
    EFI_GUID mouse_guid = {0x31878c87, 0x0b75, 0x11d5, {0x9a, 0x4f, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}};
    
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;
    EFI_SIMPLE_POINTER_PROTOCOL *mouse = 0;

    SystemTable->BootServices->LocateProtocol(&gop_guid, 0, (void**)&gop);
    EFI_STATUS m_stat = SystemTable->BootServices->LocateProtocol(&mouse_guid, 0, (void**)&mouse);

    if (gop) {
        unsigned int *vram = (unsigned int *)gop->Mode->FrameBufferBase;
        unsigned int stride = gop->Mode->Info->PixelsPerScanLine;
        unsigned int width = gop->Mode->Info->HorizontalResolution;
        unsigned int height = gop->Mode->Info->VerticalResolution;

        // 背景塗りつぶし
        fill_rect(vram, stride, 0, 0, width, height, 0x000080);
        
        draw_string(vram, stride, 10, 10, 0xFFFFFF, "SAIKYOKN OS REALTIME DEBUGGER");

        char buf[32];
        itoa_hex(m_stat, buf);
        draw_string(vram, stride, 10, 40, 0xFFFFFF, "Mouse Protocol Stat: 0x");
        draw_string(vram, stride, 194, 40, (m_stat == 0) ? 0x00FF00 : 0xFF0000, buf);

        if (m_stat == EFI_SUCCESS && mouse) {
            mouse->Reset(mouse, 0);
            int cur_x = width / 2;
            int cur_y = height / 2;

            while (1) {
                EFI_SIMPLE_POINTER_STATE ms;
                if (mouse->GetState(mouse, &ms) == EFI_SUCCESS) {
                    // 数値表示エリアの消去
                    fill_rect(vram, stride, 10, 70, 200, 100, 0x000080);

                    cur_x += ms.RelativeMovementX / 2;
                    cur_y += ms.RelativeMovementY / 2;
                    
                    // 座標表示
                    draw_string(vram, stride, 10, 70, 0xFFFF00, "X: ");
                    itoa_dec(cur_x, buf);
                    draw_string(vram, stride, 40, 70, 0xFFFFFF, buf);

                    draw_string(vram, stride, 10, 90, 0xFFFF00, "Y: ");
                    itoa_dec(cur_y, buf);
                    draw_string(vram, stride, 40, 90, 0xFFFFFF, buf);

                    draw_string(vram, stride, 10, 110, 0xFFFF00, "Buttons: ");
                    if (ms.LeftButton) draw_string(vram, stride, 80, 110, 0xFF0000, "LEFT ");
                    if (ms.RightButton) draw_string(vram, stride, 130, 110, 0xFF0000, "RIGHT");
                }
            }
        } else {
            draw_string(vram, stride, 10, 70, 0xFF0000, "ERROR: MOUSE NOT DETECTED");
        }
    }
    return 0;
}