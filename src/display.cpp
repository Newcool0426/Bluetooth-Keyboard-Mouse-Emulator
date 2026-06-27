/**
 * @file display.cpp
 * @brief M5Cardputer TFT 240x135 — 键盘/鼠标模拟器 UI
 *
 * 键盘模式布局:                    鼠标模式布局:
 * ┌────────────────────────┐      ┌────────────────────────┐
 * │ [icon] KEYBOARD  G0 > │ 24   │ [icon] MOUSE     G0 > │
 * │  ┌──────────────────┐ │      │  ┌──────────────────┐ │
 * │  │   Ctrl+Shift+A   │ │ 38   │  │     L-Click     │ │
 * │  └──────────────────┘ │      │  └──────────────────┘ │
 * │  abc def ghi ...     │ 66   │      ┌──┐    ┌────┐   │
 * │  jkl mno pqr ...     │      │      │; │    │Ent │   │
 * │  stu vwx yz          │      │  ┌──┐├──┤┌──┐└────┘   │
 * │                      │      │  │, ││  ││/ │┌────┐   │
 * │                      │      │  └──┘├──┤└──┘│ \  │   │
 * │                      │      │      │. │    └────┘   │
 * │                      │      │      └──┘             │
 * └────────────────────────┘      └────────────────────────┘
 */

#include "display.h"

// ============================================================
// 布局常量
// ============================================================
#define STATUS_BAR_H    24
#define KEY_DISPLAY_Y   26
#define KEY_DISPLAY_H   38
#define BOTTOM_Y        66
#define BOTTOM_H        66
#define TEXT_BUF_SIZE   512

// ============================================================
// 持久状态
// ============================================================
static char g_textBuf[TEXT_BUF_SIZE] = "";
static int  g_textLen = 0;
static char g_lastLiveKey[32] = "";
static bool g_hintsNeedRedraw = true;    // 鼠标提示是否需要重绘

// ============================================================
// 图标: USB (填充版)
// ============================================================
static void drawUsbIcon(uint8_t x, uint8_t y, uint16_t color) {
    M5Cardputer.Display.fillRect(x + 1, y + 2, 15, 3, color);
    M5Cardputer.Display.fillTriangle(x + 1, y + 2, x + 8, y + 2, x + 1, y + 7, color);
    M5Cardputer.Display.fillTriangle(x + 16, y + 2, x + 9, y + 2, x + 16, y + 7, color);
    M5Cardputer.Display.fillRect(x + 7, y + 5, 3, 10, color);
    M5Cardputer.Display.fillCircle(x + 8, y + 17, 3, color);
}

// ============================================================
// 图标: 蓝牙 (填充版)
// ============================================================
static void drawBluetoothIcon(uint8_t x, uint8_t y, uint16_t color) {
    M5Cardputer.Display.fillRect(x + 7, y + 1, 3, 18, color);
    M5Cardputer.Display.fillTriangle(x + 10, y + 1, x + 17, y + 9, x + 10, y + 9, color);
    M5Cardputer.Display.fillTriangle(x + 10, y + 10, x + 17, y + 11, x + 10, y + 19, color);
    M5Cardputer.Display.fillTriangle(x + 7, y + 3, x, y + 9, x + 7, y + 9, color);
    M5Cardputer.Display.fillTriangle(x + 7, y + 10, x, y + 11, x + 7, y + 17, color);
}

// ============================================================
// 初始化 / 欢迎画面 / 主界面
// ============================================================
void setupDisplay() {
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_BLACK);
}

void displayWelcomeScreen() {
    M5Cardputer.Display.drawRect(9, 47, 220, 40, TFT_LIGHTGRAY);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(18, 58);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.printf("M5-Keyboard-Mouse");
    M5Cardputer.Display.setCursor(70, 120);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.printf("Version 1.1 - Geo");
    delay(2000);
}

void displayMainScreen(bool usbMode, bool mouseMode, bool bluetoothStatus) {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    drawStatusBar(usbMode, mouseMode, bluetoothStatus);
    clearKeyDisplayArea();
}

// ============================================================
// 顶部状态栏
// ============================================================
void drawStatusBar(bool usbMode, bool mouseMode, bool bluetoothStatus) {
    int w = M5Cardputer.Display.width();
    uint16_t bgColor = mouseMode ? TFT_NAVY : TFT_DARKGREEN;
    M5Cardputer.Display.fillRect(0, 0, w, STATUS_BAR_H, bgColor);

    uint16_t iconColor;
    if (usbMode) { iconColor = TFT_GREEN; }
    else         { iconColor = bluetoothStatus ? TFT_GREEN : TFT_RED; }
    if (usbMode) drawUsbIcon(2, 3, iconColor);
    else         drawBluetoothIcon(2, 2, iconColor);

    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(21, 4);
    M5Cardputer.Display.print(mouseMode ? "MOUSE" : "KEYBOARD");

    const char* goHint = "G0 >";
    int goW = (int)strlen(goHint) * 12;
    M5Cardputer.Display.setCursor(w - goW - 5, 4);
    M5Cardputer.Display.print(goHint);
}

// ============================================================
// HID → 显示符号
// ============================================================
static const char* hidToDisplay(uint8_t key, bool shifted, char* buf) {
    buf[0] = '\0';
    if (key >= 0x04 && key <= 0x1D) {
        char c = 'a' + (key - 0x04);
        if (shifted) c = c - 'a' + 'A';
        buf[0] = c; buf[1] = '\0';
    } else if (key >= 0x1E && key <= 0x27) {
        static const char* sn = "!@#$%^&*()";
        if (shifted && key - 0x1E < 10) buf[0] = sn[key - 0x1E];
        else if (key == 0x27)           buf[0] = '0';
        else                            buf[0] = '1' + (key - 0x1E);
        buf[1] = '\0';
    } else {
        switch (key) {
            case 0x28: strcpy(buf, "Ent"); break; case 0x29: strcpy(buf, "Esc"); break;
            case 0x2A: strcpy(buf, "Bsp"); break; case 0x2B: strcpy(buf, "Tab"); break;
            case 0x2C: strcpy(buf, "Spc"); break; case 0x2D: strcpy(buf, "-");   break;
            case 0x2E: strcpy(buf, "=");   break; case 0x2F: strcpy(buf, "[");   break;
            case 0x30: strcpy(buf, "]");   break; case 0x31: strcpy(buf, "\\");  break;
            case 0x33: strcpy(buf, ";");   break; case 0x34: strcpy(buf, "'");   break;
            case 0x35: strcpy(buf, "`");   break; case 0x36: strcpy(buf, ",");   break;
            case 0x37: strcpy(buf, ".");   break; case 0x38: strcpy(buf, "/");   break;
            case 0x4C: strcpy(buf, "Del"); break;
            case 0x4F: strcpy(buf, ">");   break; case 0x50: strcpy(buf, "<");   break;
            case 0x51: strcpy(buf, "v");   break; case 0x52: strcpy(buf, "^");   break;
            default: break;
        }
    }
    return buf;
}

// ============================================================
// 实时按键显示字符串
// ============================================================
static const char* getKeyDisplayString(bool mouseMode, char* out, size_t outSize) {
    out[0] = '\0';
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    char buf[8];

    if (mouseMode) {
        if (M5Cardputer.Keyboard.isKeyPressed(';'))        { strcpy(out, "Up");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('.'))        { strcpy(out, "Down");    return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('/'))        { strcpy(out, "Right");   return out; }
        if (M5Cardputer.Keyboard.isKeyPressed(','))        { strcpy(out, "Left");    return out; }
        if (status.enter)                                  { strcpy(out, "L-Click"); return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('\\'))       { strcpy(out, "R-Click"); return out; }
        return out;
    }

    size_t len = 0;
    if (status.ctrl)  len += snprintf(out + len, outSize - len, "Ctrl+");
    if (status.shift) len += snprintf(out + len, outSize - len, "Shift+");
    if (status.alt)   len += snprintf(out + len, outSize - len, "Alt+");
    if (status.opt)   len += snprintf(out + len, outSize - len, "Win+");
    if (status.enter) { snprintf(out + len, outSize - len, "Ent"); return out; }
    if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
        snprintf(out + len, outSize - len, "Spc"); return out;
    }

    bool hasKey = false;
    for (auto key : status.hid_keys) {
        if (key == 0) continue;
        uint8_t dk = key;
        if (status.fn) {
            switch (key) {
                case 0x38: dk = 0x4F; break; case 0x36: dk = 0x50; break;
                case 0x37: dk = 0x51; break; case 0x33: dk = 0x52; break;
                case 0x2A: dk = 0x4C; break; case 0x35: dk = 0x29; break;
                default: break;
            }
        }
        hasKey = true;
        const char* label = hidToDisplay(dk, status.shift, buf);
        if (strlen(label) > 0) snprintf(out + len, outSize - len, "%s", label);
        break;
    }
    if (!hasKey && len > 0) out[len - 1] = '\0';
    return out;
}

// ============================================================
// 文本缓冲区 (键盘模式)
// ============================================================

static char getTypedChar(const Keyboard_Class::KeysState& status) {
    bool any = false;
    for (auto k : status.hid_keys) { if (k != 0) { any = true; break; } }
    if ((status.ctrl || status.shift || status.alt || status.opt)
        && !status.enter && !M5Cardputer.Keyboard.isKeyPressed(' ') && !any) return 0;

    if (status.enter) return '\n';
    if (M5Cardputer.Keyboard.isKeyPressed(' ')) return ' ';

    for (auto key : status.hid_keys) {
        if (key == 0) continue;
        uint8_t k = key;
        if (status.fn) {
            switch (key) {
                case 0x38: k = 0x4F; break; case 0x36: k = 0x50; break;
                case 0x37: k = 0x51; break; case 0x33: k = 0x52; break;
                case 0x2A: k = 0x4C; break; case 0x35: k = 0x29; break;
                default: break;
            }
        }
        if (k >= 0x04 && k <= 0x1D) {
            char c = 'a' + (k - 0x04);
            if (status.shift) c = c - 'a' + 'A';
            return c;
        }
        if (k >= 0x1E && k <= 0x27) {
            static const char* sn = "!@#$%^&*()";
            if (status.shift && k - 0x1E < 10) return sn[k - 0x1E];
            return (k == 0x27) ? '0' : '1' + (k - 0x1E);
        }
        switch (k) {
            case 0x28: return '\n'; case 0x2A: return '\b'; case 0x2B: return '\t';
            case 0x2C: return ' ';  case 0x2D: return '-';  case 0x2E: return '=';
            case 0x2F: return '[';  case 0x30: return ']';  case 0x31: return '\\';
            case 0x33: return ';';  case 0x34: return '\''; case 0x35: return '`';
            case 0x36: return ',';  case 0x37: return '.';  case 0x38: return '/';
            default: return 0;
        }
    }
    return 0;
}

static void appendToTextBuffer(char c) {
    if (c == '\b') {
        if (g_textLen > 0) { g_textLen--; g_textBuf[g_textLen] = '\0'; }
    } else if (c == '\t') {
        if (g_textLen + 2 < TEXT_BUF_SIZE) {
            g_textBuf[g_textLen++] = ' '; g_textBuf[g_textLen++] = ' ';
            g_textBuf[g_textLen] = '\0';
        }
    } else if (c == '\n' || (c >= 0x20 && c <= 0x7E)) {
        if (g_textLen < TEXT_BUF_SIZE - 1) {
            g_textBuf[g_textLen++] = c; g_textBuf[g_textLen] = '\0';
        }
    }
}

static void drawTextBuffer() {
    int y0 = BOTTOM_Y, hh = BOTTOM_H, w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y0, w, hh, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y0, w - 12, hh, 4, TFT_DARKGREY);
    if (g_textLen == 0) return;

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);

    int margin = 10, cpl = (w - 2 * margin) / 12;
    if (cpl < 1) cpl = 1;
    int lineH = 16, maxVis = hh / lineH;
    if (maxVis < 1) maxVis = 1;

    #define RING 48
    struct { int s; int l; } ring[RING];
    int ri = 0, rc = 0;
    auto push = [&](int st, int ln) { ring[ri].s = st; ring[ri].l = ln; ri = (ri + 1) % RING; if (rc < RING) rc++; };

    int seg = 0;
    for (int i = 0; i <= g_textLen; i++) {
        char ch = (i < g_textLen) ? g_textBuf[i] : '\n';
        if (ch == '\t') ch = ' ';
        bool nl = (ch == '\n'), full = (i - seg >= cpl);
        if (nl || full || i == g_textLen) {
            int take = i - seg;
            if (nl && full) { push(seg, cpl); seg += cpl; i--; continue; }
            if (take > 0 || nl) push(seg, take);
            seg = i + 1;
        }
    }

    int start = (rc > maxVis) ? rc - maxVis : 0;
    for (int d = 0; d < maxVis && (start + d) < rc; d++) {
        int idx = (ri - rc + start + d) % RING; if (idx < 0) idx += RING;
        int len = ring[idx].l; if (len <= 0 || len > cpl) continue;
        char lb[64]; int cl = len < 63 ? len : 63;
        for (int j = 0; j < cl; j++) { char c = g_textBuf[ring[idx].s + j]; lb[j] = (c == '\t') ? ' ' : c; }
        lb[cl] = '\0';
        M5Cardputer.Display.setCursor(margin, y0 + 2 + d * lineH);
        M5Cardputer.Display.print(lb);
    }
}

// ============================================================
// 鼠标模式: 方向十字 + 按键方块 (只绘制一次, 避免刷新条纹)
// ============================================================

/**
 * @brief 在方框内居中绘制字符
 * @param bx,by,bw,bh  方框位置和大小
 * @param str          要显示的字符串
 */
static void drawKeyBox(int bx, int by, int bw, int bh, const char* str) {
    // 方框
    M5Cardputer.Display.drawRoundRect(bx, by, bw, bh, 3, TFT_LIGHTGREY);
    // 文字居中 (textSize=2 → 12x16 px)
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    int tw = (int)strlen(str) * 12;
    int cx = bx + (bw - tw) / 2;
    int cy = by + (bh - 16) / 2 + 1;
    M5Cardputer.Display.setCursor(cx, cy);
    M5Cardputer.Display.print(str);
}

static void drawMouseHints() {
    // 首次绘制后不再重绘, 避免每帧刷新产生斜纹
    if (!g_hintsNeedRedraw) return;
    g_hintsNeedRedraw = false;

    int y0 = BOTTOM_Y, hh = BOTTOM_H, w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y0, w, hh, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y0, w - 12, hh, 4, TFT_DARKGREY);

    // === 方向键: 十字排列 (4个方框) ===
    // 方框 32x20, 中心 x≈88
    drawKeyBox(72,  68, 32, 20, ";");   // 上
    drawKeyBox(40,  90, 32, 20, ",");   // 左
    drawKeyBox(104, 90, 32, 20, "/");   // 右
    drawKeyBox(72, 112, 32, 20, ".");   // 下

    // === 点击键: 右侧 (2个方框) ===
    drawKeyBox(155, 72,  44, 20, "Ent");  // 左键
    drawKeyBox(162, 108, 30, 20, "\\");   // 右键
}

// ============================================================
// 按键显示 (实时 + 底部)
// ============================================================
void drawKeyDisplay(bool mouseMode) {
    static bool lastMouseMode = !mouseMode;
    bool modeChanged = (mouseMode != lastMouseMode);
    lastMouseMode = mouseMode;

    char text[32];
    getKeyDisplayString(mouseMode, text, sizeof(text));

    // === 上半: 实时按键 (仅在变化时重绘) ===
    {
        static char lastDrawnKey[32] = "";
        if (strlen(text) > 0) strcpy(g_lastLiveKey, text);
        const char* show = (strlen(text) > 0) ? text : g_lastLiveKey;

        // 仅在按键变化或模式切换时重绘, 避免无效刷新
        if (strcmp(show, lastDrawnKey) != 0 || modeChanged) {
            strcpy(lastDrawnKey, show);
            int y = KEY_DISPLAY_Y, hh = KEY_DISPLAY_H, w = M5Cardputer.Display.width();
            M5Cardputer.Display.fillRect(0, y, w, hh, TFT_BLACK);
            M5Cardputer.Display.drawRoundRect(6, y, w - 12, hh, 4, TFT_DARKGREY);

            if (strlen(show) > 0) {
                int tLen = strlen(show);
                int tSize;
                if (mouseMode)       tSize = 3;
                else if (tLen <= 2)  tSize = 4;
                else if (tLen <= 4)  tSize = 3;
                else                 tSize = 2;

                int cw = 6 * tSize, tw = tLen * cw;
                int cx = (w - tw) / 2; if (cx < 4) cx = 4;
                int cy = y + (hh - 8 * tSize) / 2;

                M5Cardputer.Display.setTextColor(TFT_WHITE);
                M5Cardputer.Display.setTextSize(tSize);
                M5Cardputer.Display.setCursor(cx, cy);
                M5Cardputer.Display.print(show);
            }
        }
    }

    // === 下半 ===
    if (mouseMode) {
        // 鼠标模式: 仅在模式切换时需要重绘提示
        if (modeChanged) g_hintsNeedRedraw = true;
        drawMouseHints();
    } else {
        // 键盘模式: 文本追加 + 绘制
        static char lastAppendKey[32] = "";
        if (strlen(text) > 0) {
            if (strcmp(text, lastAppendKey) != 0) {
                strcpy(lastAppendKey, text);
                Keyboard_Class::KeysState st = M5Cardputer.Keyboard.keysState();
                char c = getTypedChar(st);
                if (c != 0) appendToTextBuffer(c);
            }
        } else {
            lastAppendKey[0] = '\0';
        }
        drawTextBuffer();
    }
}

void clearKeyDisplayArea() {
    g_textLen = 0; g_textBuf[0] = '\0';
    g_lastLiveKey[0] = '\0';
    g_hintsNeedRedraw = true;    // 下次进入鼠标模式时重新绘制提示

    int w = M5Cardputer.Display.width();
    {
        int y = KEY_DISPLAY_Y, hh = KEY_DISPLAY_H;
        M5Cardputer.Display.fillRect(0, y, w, hh, TFT_BLACK);
        M5Cardputer.Display.drawRoundRect(6, y, w - 12, hh, 4, TFT_DARKGREY);
    }
    {
        int y = BOTTOM_Y, hh = BOTTOM_H;
        M5Cardputer.Display.fillRect(0, y, w, hh, TFT_BLACK);
        M5Cardputer.Display.drawRoundRect(6, y, w - 12, hh, 4, TFT_DARKGREY);
    }
}

// ============================================================
// 模式选择界面
// ============================================================
void displaySelectionScreen(bool mode) {
    M5Cardputer.Display.clear();
    M5Cardputer.Display.setTextSize(1.5);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(70, 10);
    M5Cardputer.Display.printf("Select Mode:");
    M5Cardputer.Display.setTextSize(3);

    if (mode) {
        M5Cardputer.Display.fillRect(20, 30, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.drawRect(20, 30, 200, 40, TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_BLACK);
    } else {
        M5Cardputer.Display.fillRect(20, 30, 200, 40, TFT_BLACK);
        M5Cardputer.Display.drawRect(20, 30, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    }
    M5Cardputer.Display.setCursor(95, 40);
    M5Cardputer.Display.printf("USB");

    if (!mode) {
        M5Cardputer.Display.fillRect(20, 80, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.drawRect(20, 80, 200, 40, TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_BLACK);
    } else {
        M5Cardputer.Display.fillRect(20, 80, 200, 40, TFT_BLACK);
        M5Cardputer.Display.drawRect(20, 80, 200, 40, TFT_LIGHTGRAY);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    }
    M5Cardputer.Display.setCursor(42, 90);
    M5Cardputer.Display.printf("Bluetooth");
}
