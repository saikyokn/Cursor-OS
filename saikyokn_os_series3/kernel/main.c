typedef unsigned long long EFI_STATUS;
typedef void* EFI_HANDLE;
#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0

// --- 1. 基本的な UEFI 構造体の定義 ---
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

// キー入力用
typedef struct {
    unsigned short ScanCode;
    unsigned short UnicodeChar;
} EFI_INPUT_KEY;

struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key);

typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    void *Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    void *WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

// グラフィックス用
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

// ブートサービス
typedef struct {
    EFI_TABLE_HEADER Hdr;
    void *dummy1[2]; void *dummy2[5]; void *dummy3[6]; void *dummy4[5];
    void *dummy5[4]; void *dummy6[5]; void *dummy7[5]; void *dummy8[3];
    void *dummy9[2];
    EFI_STATUS (EFIAPI *LocateProtocol)(EFI_GUID *Protocol, void *Registration, void **Interface);
} EFI_BOOT_SERVICES;

// システムテーブル (これが無いと EfiMain が動かへん)
typedef struct {
    EFI_TABLE_HEADER Hdr;
    unsigned short *FirmwareVendor;
    unsigned int FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    void *ConOut;
    EFI_HANDLE StandardErrorHandle;
    void *StdErr;
    void *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
} EFI_SYSTEM_TABLE;

// --- 2. 便利関数とマウス描画 ---

void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
void puts_serial(char *s) { while (*s) outb(0x3f8, *s++); }

void fill_rect(unsigned int *vram, unsigned int stride, int x, int y, int w, int h, unsigned int color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            vram[i * stride + j] = color;
        }
    }
}

static const char *mouse_cursor[] = {
    "X           ", "XX          ", "X.X         ", "X..X        ",
    "X...X       ", "X....X      ", "X.....X     ", "X......X    ",
    "X.......X   ", "X........X  ", "X.........X ", "X......XXXXX",
    "X...X..X    ", "X..X X..X   ", "X.X  X..X   ", "XX    X..X  ",
    "      X..X  ", "       XX   ", "            "
};

void draw_mouse(unsigned int *vram, unsigned int stride, int x, int y) {
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 12; j++) {
            if (mouse_cursor[i][j] == 'X') vram[(y + i) * stride + (x + j)] = 0x000000;
            if (mouse_cursor[i][j] == '.') vram[(y + i) * stride + (x + j)] = 0xFFFFFF;
        }
    }
}

void erase_mouse(unsigned int *vram, unsigned int stride, int x, int y) {
    fill_rect(vram, stride, x, y, 12, 19, 0x001E90FF);
}

// --- 3. メインルーチン ---

EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    puts_serial("\n--- Saikyokn OS: Mouse Control Mode ---\n");

    EFI_GUID gop_guid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;

    EFI_STATUS status = SystemTable->BootServices->LocateProtocol(&gop_guid, 0, (void**)&gop);

    if (status == EFI_SUCCESS && gop) {
        unsigned int *vram = (unsigned int *)gop->Mode->FrameBufferBase;
        unsigned int width  = gop->Mode->Info->HorizontalResolution;
        unsigned int height = gop->Mode->Info->VerticalResolution;
        unsigned int stride = gop->Mode->Info->PixelsPerScanLine;

        fill_rect(vram, stride, 0, 0, width, height, 0x001E90FF);
        fill_rect(vram, stride, 0, height - 45, width, 45, 0x002D2D2D);
        fill_rect(vram, stride, 5, height - 40, 60, 35, 0x004A4A4A);

        int mx = 100, my = 100;
        draw_mouse(vram, stride, mx, my);

        while (1) {
            EFI_INPUT_KEY key;
            if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) == 0) {
                erase_mouse(vram, stride, mx, my);
                if (key.ScanCode == 1) my -= 10;
                if (key.ScanCode == 2) my += 10;
                if (key.ScanCode == 3) mx += 10;
                if (key.ScanCode == 4) mx -= 10;

                if (mx < 0) mx = 0;
                if (my < 0) my = 0;
                if (mx > (int)width - 12) mx = width - 12;
                if (my > (int)height - 19) my = height - 19;

                draw_mouse(vram, stride, mx, my);
            }
        }
    }
    return 0;
}