#ifndef GRAPHICS_H
#define GRAPHICS_H

typedef unsigned long long EFI_STATUS;
typedef void* EFI_HANDLE;
#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0

typedef struct { 
    unsigned int Data1; unsigned short Data2; unsigned short Data3; unsigned char Data4[8]; 
} EFI_GUID;

// GOP関連
typedef struct {
    unsigned int Version; unsigned int HorizontalResolution; unsigned int VerticalResolution;
    int PixelFormat; unsigned int PixelInformation[4]; unsigned int PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    unsigned int MaxMode; unsigned int Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    unsigned long long SizeOfInfo; unsigned long long FrameBufferBase; unsigned long long FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct {
    void *dummy[3];
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

// 入出力関連
typedef struct { unsigned short ScanCode; unsigned short UnicodeChar; } EFI_INPUT_KEY;
struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key);
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    void *Reset; EFI_INPUT_READ_KEY ReadKeyStroke; void *WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, unsigned short *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void *Reset; EFI_TEXT_STRING OutputString; void *TestString; void *QueryMode; 
    void *SetMode; void *SetAttribute; EFI_TEXT_CLEAR ClearScreen;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

// システムテーブル
typedef struct {
    char dummy[52];
    EFI_STATUS (EFIAPI *LocateProtocol)(EFI_GUID *Protocol, void *Registration, void **Interface);
} EFI_BOOT_SERVICES;

typedef struct {
    char header[24]; // Standard Header
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    void *dummy[2];
    EFI_BOOT_SERVICES *BootServices;
} EFI_SYSTEM_TABLE;

// 描画関数
void fill_rect(unsigned int *vram, unsigned int stride, int x, int y, int w, int h, unsigned int color);
void draw_mouse(unsigned int *vram, unsigned int stride, int x, int y);
void erase_mouse(unsigned int *vram, unsigned int stride, int x, int y);
void draw_string(unsigned int *vram, unsigned int stride, int x, int y, char *s, unsigned int color);

#endif