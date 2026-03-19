typedef unsigned long long EFI_STATUS;
typedef void* EFI_HANDLE;
#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0

// --- 1. UEFI 構造体定義 ---
typedef struct {
    unsigned long long Signature;
    unsigned int Revision;
    unsigned int HeaderSize;
    unsigned int CRC32;
    unsigned int Reserved;
} EFI_TABLE_HEADER;

typedef struct {
    unsigned int Data1; unsigned short Data2; unsigned short Data3; unsigned char Data4[8];
} EFI_GUID;

typedef struct {
    unsigned short ScanCode;
    unsigned short UnicodeChar;
} EFI_INPUT_KEY;

struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef EFI_STATUS(EFIAPI* EFI_INPUT_READ_KEY)(struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This, EFI_INPUT_KEY* Key);

typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    void* Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    void* WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

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
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
    unsigned long long SizeOfInfo;
    unsigned long long FrameBufferBase;
    unsigned long long FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct {
    void* dummy[3];
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    void* dummy1[2]; void* dummy2[5]; void* dummy3[6]; void* dummy4[5];
    void* dummy5[4]; void* dummy6[5]; void* dummy7[5]; void* dummy8[3];
    void* dummy9[2];
    EFI_STATUS(EFIAPI* LocateProtocol)(EFI_GUID* Protocol, void* Registration, void** Interface);
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    unsigned short* FirmwareVendor;
    unsigned int FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    void* ConOut;
    EFI_HANDLE StandardErrorHandle;
    void* StdErr;
    void* RuntimeServices;
    EFI_BOOT_SERVICES* BootServices;
} EFI_SYSTEM_TABLE;

// --- 2. 描画・マウス機能 ---

// マウスの下の背景を保存するバッファ
static unsigned int mouse_back_buffer[12 * 19];

void fill_rect(unsigned int* vram, unsigned int stride, int x, int y, int w, int h, unsigned int color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            vram[i * stride + j] = color;
        }
    }
}

static const char* mouse_cursor[] = {
    "X           ", "XX          ", "X.X         ", "X..X        ",
    "X...X       ", "X....X      ", "X.....X     ", "X......X    ",
    "X-------X   ", "X--------X  ", "X---------X ", "X------XXXXX",
    "X---X--X    ", "X--X X--X   ", "X-X  X--X   ", "XX    X--X  ",
    "      X--X  ", "       XX   ", "            "
};

// マウスを描画する前に背景を保存
void save_mouse_bg(unsigned int* vram, unsigned int stride, int x, int y) {
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 12; j++) {
            mouse_back_buffer[i * 12 + j] = vram[(y + i) * stride + (x + j)];
        }
    }
}

// 保存した背景を書き戻してマウスを消す
void erase_mouse(unsigned int* vram, unsigned int stride, int x, int y) {
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 12; j++) {
            vram[(y + i) * stride + (x + j)] = mouse_back_buffer[i * 12 + j];
        }
    }
}

void draw_mouse(unsigned int* vram, unsigned int stride, int x, int y) {
    save_mouse_bg(vram, stride, x, y); // 描画直前に保存
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 12; j++) {
            if (mouse_cursor[i][j] == 'X') vram[(y + i) * stride + (x + j)] = 0x000000;
            if (mouse_cursor[i][j] == '-') vram[(y + i) * stride + (x + j)] = 0xFFFFFF;
        }
    }
}

// --- 3. メインルーチン ---

EFI_STATUS EFIAPI EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_GUID gop_guid = { 0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a} };
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = 0;

    if (SystemTable->BootServices->LocateProtocol(&gop_guid, 0, (void**)&gop) == EFI_SUCCESS) {
        unsigned int* vram = (unsigned int*)gop->Mode->FrameBufferBase;
        unsigned int width = gop->Mode->Info->HorizontalResolution;
        unsigned int height = gop->Mode->Info->VerticalResolution;
        unsigned int stride = gop->Mode->Info->PixelsPerScanLine;

        // 背景描画
        fill_rect(vram, stride, 0, 0, width, height, 0x001E90FF);    // デスクトップ
        fill_rect(vram, stride, 0, height - 45, width, 45, 0x2D2D2D); // タスクバー
        fill_rect(vram, stride, 5, height - 40, 60, 35, 0x4A4A4A);   // スタートボタン的な

        int mx = 100, my = 100;
        draw_mouse(vram, stride, mx, my); // 初回描画

        while (1) {
            EFI_INPUT_KEY key;
            if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) == 0) {
                erase_mouse(vram, stride, mx, my); // 背景復元

                if (key.ScanCode == 1) my -= 10;
                if (key.ScanCode == 2) my += 10;
                if (key.ScanCode == 3) mx += 10;
                if (key.ScanCode == 4) mx -= 10;

                // 境界チェック
                if (mx < 0) mx = 0; if (my < 0) my = 0;
                if (mx > (int)width - 12) mx = width - 12;
                if (my > (int)height - 19) my = height - 19;

                draw_mouse(vram, stride, mx, my); // 新しい背景を保存して描画
            }
        }
    }
    return 0;
}