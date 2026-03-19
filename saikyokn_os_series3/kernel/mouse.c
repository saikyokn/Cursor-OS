// マウスのドット絵（12x19サイズ）
static const char cursor_data[19][12] = {
    "* ",
    "** ",
    "*.* ",
    "*..* ",
    "*...* ",
    "*....* ",
    "*.....* ",
    "*......* ",
    "*.......* ",
    "*........* ",
    "*.........* ",
    "*..........*",
    "*......*****",
    "*...*..* ",
    "*..* *..* ",
    "*.* *..* ",
    "** *..* ",
    "* *..* ",
    "       ** "
};

// マウスの下に隠れた背景を保存するバッファ
static unsigned int cursor_back[19 * 12];

// マウスを描画しつつ、背景を保存する
void draw_mouse(unsigned int *vram, unsigned int stride, int x, int y) {
    for (int dy = 0; dy < 19; dy++) {
        for (int dx = 0; dx < 12; dx++) {
            // 背景を保存
            cursor_back[dy * 12 + dx] = vram[(y + dy) * stride + (x + dx)];
            
            // マウスのピクセルを描画
            if (cursor_data[dy][dx] == '*') {
                vram[(y + dy) * stride + (x + dx)] = 0x00000000; // 黒
            } else if (cursor_data[dy][dx] == '.') {
                vram[(y + dy) * stride + (x + dx)] = 0x00FFFFFF; // 白
            }
        }
    }
}

// 保存しておいた背景を戻して、マウスを消す
void erase_mouse(unsigned int *vram, unsigned int stride, int x, int y) {
    for (int dy = 0; dy < 19; dy++) {
        for (int dx = 0; dx < 12; dx++) {
            vram[(y + dy) * stride + (x + dx)] = cursor_back[dy * 12 + dx];
        }
    }
}