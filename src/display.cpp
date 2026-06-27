/**
 * @file display.cpp
 * @brief 显示屏 UI 绘制函数实现 (M5Cardputer TFT 240x135)
 *
 * 键盘模式布局:
 * ┌──────────────────────────────────┐
 * │ [USB/BT]  KEYBOARD        G0 >  │ y=0~23   (状态栏 24px)
 * │     ┌──────────────────┐        │ y=26~69  (当前按键 44px)
 * │     │   Ctrl+Shift+A   │        │
 * │     └──────────────────┘        │
 * │  abc def ghi ...               │ y=72~131 (文本缓冲区 60px)
 * └──────────────────────────────────┘
 *
 * 鼠标模式布局:
 * ┌──────────────────────────────────┐
 * │ [USB/BT]  MOUSE           G0 >  │ y=0~23   (状态栏)
 * │     ┌──────────────────┐        │ y=26~69  (当前操作 44px)
 * │     │     L-Click      │        │
 * │     └──────────────────┘        │
 * │  ;=Up  .=Dn  /=Rt  ,=Lt       │ y=72~131 (按键提示 60px)
 * │  Enter=左键  \=右键            │
 * └──────────────────────────────────┘
 */

#include "display.h"

// ============================================================
// 布局常量
// ============================================================
#define STATUS_BAR_H    24
#define KEY_DISPLAY_Y   26
#define KEY_DISPLAY_H   44
#define BOTTOM_Y        72
#define BOTTOM_H        60
#define TEXT_BUF_SIZE   512

// ============================================================
// 持久状态
// ============================================================
static char g_textBuf[TEXT_BUF_SIZE] = "";
static int  g_textLen = 0;
static char g_lastLiveKey[32] = "";

// ============================================================
// 图标: USB 三叉戟 (填充版, 16x18)
// ============================================================
static void drawUsbIcon(uint8_t x, uint8_t y, uint16_t color) {
    // 顶部横杆
    M5Cardputer.Display.fillRect(x + 1, y + 2, 15, 3, color);
    // 左侧箭尖 (填充三角)
    M5Cardputer.Display.fillTriangle(x + 1, y + 2, x + 8, y + 2, x + 1, y + 7, color);
    // 右侧箭尖
    M5Cardputer.Display.fillTriangle(x + 16, y + 2, x + 9, y + 2, x + 16, y + 7, color);
    // 竖杆
    M5Cardputer.Display.fillRect(x + 7, y + 5, 3, 10, color);
    // 底部圆点
    M5Cardputer.Display.fillCircle(x + 8, y + 17, 3, color);
}

// ============================================================
// 图标: 蓝牙符文 (填充版, 16x20)
// ============================================================
static void drawBluetoothIcon(uint8_t x, uint8_t y, uint16_t color) {
    // 中心竖杆 (加粗)
    M5Cardputer.Display.fillRect(x + 7, y + 1, 3, 18, color);
    // 右上尖角
    M5Cardputer.Display.fillTriangle(x + 10, y + 1, x + 17, y + 9, x + 10, y + 9, color);
    // 右下尖角
    M5Cardputer.Display.fillTriangle(x + 10, y + 10, x + 17, y + 11, x + 10, y + 19, color);
    // 左上小角
    M5Cardputer.Display.fillTriangle(x + 7, y + 3, x, y + 9, x + 7, y + 9, color);
    // 左下小角
    M5Cardputer.Display.fillTriangle(x + 7, y + 10, x, y + 11, x + 7, y + 17, color);
}

// ============================================================
// 初始化 / 欢迎画面
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

// ============================================================
// 主界面
// ============================================================
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

    // 连接图标 (y 偏移增大以适应更高状态栏)
    uint16_t iconColor;
    if (usbMode) {
        iconColor = TFT_GREEN;
    } else {
        iconColor = bluetoothStatus ? TFT_GREEN : TFT_RED;
    }
    if (usbMode) {
        drawUsbIcon(2, 3, iconColor);
    } else {
        drawBluetoothIcon(2, 2, iconColor);
    }

    // 模式文字 (放大字号)
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(21, 4);
    M5Cardputer.Display.print(mouseMode ? "MOUSE" : "KEYBOARD");

    // 右侧 G0 切换提示
    M5Cardputer.Display.setTextSize(1);
    const char* goHint = "G0 >";
    M5Cardputer.Display.setCursor(w - (int)strlen(goHint) * 7 - 5, 8);
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
        static const char* shiftedNum = "!@#$%^&*()";
        if (shifted && key - 0x1E < 10) { buf[0] = shiftedNum[key - 0x1E]; }
        else if (key == 0x27)           { buf[0] = '0'; }
        else                            { buf[0] = '1' + (key - 0x1E); }
        buf[1] = '\0';
    } else {
        switch (key) {
            case 0x28: strcpy(buf, "Ent"); break;
            case 0x29: strcpy(buf, "Esc"); break;
            case 0x2A: strcpy(buf, "Bsp"); break;
            case 0x2B: strcpy(buf, "Tab"); break;
            case 0x2C: strcpy(buf, "Spc"); break;
            case 0x2D: strcpy(buf, "-");   break;
            case 0x2E: strcpy(buf, "=");   break;
            case 0x2F: strcpy(buf, "[");   break;
            case 0x30: strcpy(buf, "]");   break;
            case 0x31: strcpy(buf, "\\");  break;
            case 0x33: strcpy(buf, ";");   break;
            case 0x34: strcpy(buf, "'");   break;
            case 0x35: strcpy(buf, "`");   break;
            case 0x36: strcpy(buf, ",");   break;
            case 0x37: strcpy(buf, ".");   break;
            case 0x38: strcpy(buf, "/");   break;
            case 0x4C: strcpy(buf, "Del"); break;
            case 0x4F: strcpy(buf, ">");   break;
            case 0x50: strcpy(buf, "<");   break;
            case 0x51: strcpy(buf, "v");   break;
            case 0x52: strcpy(buf, "^");   break;
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
        // 鼠标模式 — 使用描述性文字
        if (M5Cardputer.Keyboard.isKeyPressed(';'))        { strcpy(out, "Up");        return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('.'))        { strcpy(out, "Down");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('/'))        { strcpy(out, "Right");     return out; }
        if (M5Cardputer.Keyboard.isKeyPressed(','))        { strcpy(out, "Left");      return out; }
        if (status.enter)                                  { strcpy(out, "L-Click");   return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('\\'))       { strcpy(out, "R-Click");   return out; }
        return out;
    }

    // 键盘模式
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
// 文本缓冲区 (仅键盘模式)
// ============================================================

static char getTypedChar(const Keyboard_Class::KeysState& status) {
    // 仅修饰键按下 → 不追加
    bool anyHidKey = false;
    for (auto k : status.hid_keys) { if (k != 0) { anyHidKey = true; break; } }
    bool onlyMods = (status.ctrl || status.shift || status.alt || status.opt)
                    && !status.enter && !M5Cardputer.Keyboard.isKeyPressed(' ')
                    && !anyHidKey;
    if (onlyMods) return 0;

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
            g_textBuf[g_textLen++] = ' ';
            g_textBuf[g_textLen++] = ' ';
            g_textBuf[g_textLen] = '\0';
        }
    } else if (c == '\n' || (c >= 0x20 && c <= 0x7E)) {
        if (g_textLen < TEXT_BUF_SIZE - 1) {
            g_textBuf[g_textLen++] = c;
            g_textBuf[g_textLen] = '\0';
        }
    }
}

static void drawTextBuffer() {
    int y0 = BOTTOM_Y, hh = BOTTOM_H, w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y0, w, hh, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y0, w - 12, hh, 4, TFT_DARKGREY);
    if (g_textLen == 0) return;

    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
    int margin = 10, charsPerLine = (w - 2 * margin) / 6;
    if (charsPerLine < 1) charsPerLine = 1;
    int lineH = 9, maxVis = hh / lineH;
    if (maxVis < 1) maxVis = 1;

    // 环形缓冲区收集所有显示行
    #define RING_CAP 48
    struct { int start; int len; } ring[RING_CAP];
    int ri = 0, rc = 0;
    auto push = [&](int s, int l) { ring[ri].start = s; ring[ri].len = l; ri = (ri + 1) % RING_CAP; if (rc < RING_CAP) rc++; };

    int seg = 0;
    for (int i = 0; i <= g_textLen; i++) {
        char ch = (i < g_textLen) ? g_textBuf[i] : '\n';
        if (ch == '\t') ch = ' ';
        bool nl = (ch == '\n'), full = (i - seg >= charsPerLine);
        if (nl || full || i == g_textLen) {
            int take = i - seg;
            if (nl && full) { push(seg, charsPerLine); seg += charsPerLine; i--; continue; }
            if (take > 0 || nl) push(seg, take);
            seg = i + 1;
        }
    }

    int start = (rc > maxVis) ? rc - maxVis : 0;
    for (int d = 0; d < maxVis && (start + d) < rc; d++) {
        int idx = (ri - rc + start + d) % RING_CAP; if (idx < 0) idx += RING_CAP;
        int len = ring[idx].len; if (len <= 0 || len > charsPerLine) continue;
        char lb[64]; int cl = len < 63 ? len : 63;
        for (int j = 0; j < cl; j++) { char c = g_textBuf[ring[idx].start + j]; lb[j] = (c == '\t') ? ' ' : c; }
        lb[cl] = '\0';
        M5Cardputer.Display.setCursor(margin, y0 + 3 + d * lineH);
        M5Cardputer.Display.print(lb);
    }
}

// ============================================================
// 鼠标模式按键提示
// ============================================================
static void drawMouseHints() {
    int y0 = BOTTOM_Y, hh = BOTTOM_H, w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y0, w, hh, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y0, w - 12, hh, 4, TFT_DARKGREY);

    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);

    struct { const char* txt; int x; int y; } lines[] = {
        {";=Up",   14, y0 + 8},
        {".=Down",  80, y0 + 8},
        {"/=Right", 146, y0 + 8},
        {",=Left",  14, y0 + 20},
        {"Ent=L-Clk", 14, y0 + 34},
        {"\\=R-Clk",  14, y0 + 46},
    };
    for (auto& L : lines) {
        M5Cardputer.Display.setCursor(L.x, L.y);
        M5Cardputer.Display.print(L.txt);
    }
}

// ============================================================
// 按键显示 (实时 + 累积/提示)
// ============================================================
void drawKeyDisplay(bool mouseMode) {
    char text[32];
    getKeyDisplayString(mouseMode, text, sizeof(text));

    // === 实时按键区域 (上半) ===
    {
        int y = KEY_DISPLAY_Y, hh = KEY_DISPLAY_H, w = M5Cardputer.Display.width();
        M5Cardputer.Display.fillRect(0, y, w, hh, TFT_BLACK);
        M5Cardputer.Display.drawRoundRect(6, y, w - 12, hh, 4, TFT_DARKGREY);

        if (strlen(text) > 0) strcpy(g_lastLiveKey, text);
        const char* show = (strlen(text) > 0) ? text : g_lastLiveKey;

        if (strlen(show) > 0) {
            int tLen = strlen(show);
            int tSize;
            if (mouseMode)       tSize = 3;   // 鼠标模式固定字号
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

    // === 下半区域 ===
    if (mouseMode) {
        // 鼠标模式: 显示按键提示
        drawMouseHints();
    } else {
        // 键盘模式: 追加按键到文本缓冲区并显示
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

    int w = M5Cardputer.Display.width();
    // 上半: 实时按键区
    { int y = KEY_DISPLAY_Y, hh = KEY_DISPLAY_H;
      M5Cardputer.Display.fillRect(0, y, w, hh, TFT_BLACK);
      M5Cardputer.Display.drawRoundRect(6, y, w - 12, hh, 4, TFT_DARKGREY); }
    // 下半: 初始清空 (具体内容由 drawKeyDisplay 重绘)
    { int y = BOTTOM_Y, hh = BOTTOM_H;
      M5Cardputer.Display.fillRect(0, y, w, hh, TFT_BLACK);
      M5Cardputer.Display.drawRoundRect(6, y, w - 12, hh, 4, TFT_DARKGREY); }
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
