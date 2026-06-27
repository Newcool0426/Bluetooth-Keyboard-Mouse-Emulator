/**
 * @file display.cpp
 * @brief 显示屏 UI 绘制函数实现 (M5Cardputer TFT 240x135)
 *
 * UI 布局总览:
 * ┌──────────────────────────────────┐
 * │ [USB/BT] KEYBOARD          GO ▶ │ y=0~21   (状态栏 22px)
 * │                                  │
 * │     ┌──────────────────┐        │ y=24~67  (当前按键 44px, 大字体)
 * │     │   Ctrl+Shift+A   │        │
 * │     └──────────────────┘        │
 * │  abc                           │ y=70~129 (文本缓冲区 60px, 小字体)
 * │  def ghi ...                   │          累积键入内容, 支持换行
 * └──────────────────────────────────┘
 */

#include "display.h"

// ============================================================
// 布局常量
// ============================================================
#define STATUS_BAR_H    22
#define KEY_DISPLAY_Y   24
#define KEY_DISPLAY_H   44
#define TEXT_BUF_Y      70
#define TEXT_BUF_H      60
#define TEXT_BUF_SIZE   512

// ============================================================
// 持久状态
// ============================================================
static char g_textBuf[TEXT_BUF_SIZE] = "";   // 文本累积缓冲区
static int  g_textLen = 0;                    // 缓冲区已用长度
static char g_lastLiveKey[32] = "";           // 上一帧的实时按键 (去重用)

// ============================================================
// 图标: USB 三叉戟 (16x17 px)
// ============================================================
static void drawUsbIcon(uint8_t x, uint8_t y, uint16_t color) {
    uint8_t cx = x + 8;
    M5Cardputer.Display.fillCircle(cx, y + 15, 3, color);
    M5Cardputer.Display.drawLine(cx, y + 2, cx, y + 12, color);
    M5Cardputer.Display.drawLine(x + 2, y + 2, x + 14, y + 2, color);
    M5Cardputer.Display.drawLine(x, y + 5, cx - 1, y + 2, color);
    M5Cardputer.Display.drawLine(x + 16, y + 5, cx + 1, y + 2, color);
}

// ============================================================
// 图标: 蓝牙北欧符文 (16x18 px)
// ============================================================
static void drawBluetoothIcon(uint8_t x, uint8_t y, uint16_t color) {
    uint8_t cx = x + 8;
    // 中心竖线
    M5Cardputer.Display.drawLine(cx, y + 1, cx, y + 17, color);
    // 右侧尖角
    M5Cardputer.Display.drawLine(cx, y + 1, x + 15, y + 7, color);
    M5Cardputer.Display.drawLine(cx, y + 8, x + 15, y + 7, color);
    M5Cardputer.Display.drawLine(cx, y + 17, x + 15, y + 12, color);
    M5Cardputer.Display.drawLine(cx, y + 9, x + 15, y + 12, color);
    // 左侧尖角
    M5Cardputer.Display.drawLine(cx, y + 3, x + 1, y + 7, color);
    M5Cardputer.Display.drawLine(cx, y + 8, x + 1, y + 7, color);
    M5Cardputer.Display.drawLine(cx, y + 9, x + 1, y + 13, color);
    M5Cardputer.Display.drawLine(cx, y + 15, x + 1, y + 13, color);
}

// ============================================================
// 初始化
// ============================================================
void setupDisplay() {
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_BLACK);
}

// ============================================================
// 欢迎画面
// ============================================================
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

    // 连接图标
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

    // 模式文字
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.setTextSize(1.5);
    M5Cardputer.Display.setCursor(20, 4);
    M5Cardputer.Display.print(mouseMode ? "MOUSE" : "KEYBOARD");

    // 右侧 GO 提示
    M5Cardputer.Display.setTextSize(1.2);
    const char* goHint = "GO >";
    M5Cardputer.Display.setCursor(w - (int)strlen(goHint) * 7 - 6, 5);
    M5Cardputer.Display.print(goHint);
}

// ============================================================
// HID 键码 → 显示符号
// ============================================================
static const char* hidToDisplay(uint8_t key, bool shifted, char* buf) {
    buf[0] = '\0';
    if (key >= 0x04 && key <= 0x1D) {
        char c = 'a' + (key - 0x04);
        if (shifted) c = c - 'a' + 'A';
        buf[0] = c; buf[1] = '\0';
    } else if (key >= 0x1E && key <= 0x27) {
        static const char* shiftedNum = "!@#$%^&*()";
        if (shifted && key - 0x1E < 10) {
            buf[0] = shiftedNum[key - 0x1E];
        } else if (key == 0x27) {
            buf[0] = '0';
        } else {
            buf[0] = '1' + (key - 0x1E);
        }
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
        if (M5Cardputer.Keyboard.isKeyPressed(';'))        { strcpy(out, "^");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('.'))        { strcpy(out, "v");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('/'))        { strcpy(out, ">");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed(','))        { strcpy(out, "<");      return out; }
        if (status.enter)                                  { strcpy(out, "L-Clk");  return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('\\'))       { strcpy(out, "R-Clk");  return out; }
        return out;
    }

    size_t len = 0;
    if (status.ctrl)  len += snprintf(out + len, outSize - len, "Ctrl+");
    if (status.shift) len += snprintf(out + len, outSize - len, "Shift+");
    if (status.alt)   len += snprintf(out + len, outSize - len, "Alt+");
    if (status.opt)   len += snprintf(out + len, outSize - len, "Win+");

    if (status.enter) { snprintf(out + len, outSize - len, "Ent"); return out; }
    if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
        snprintf(out + len, outSize - len, "Spc");
        return out;
    }

    bool hasKey = false;
    for (auto key : status.hid_keys) {
        if (key == 0) continue;
        uint8_t displayKey = key;
        if (status.fn) {
            switch (key) {
                case 0x38: displayKey = 0x4F; break;
                case 0x36: displayKey = 0x50; break;
                case 0x37: displayKey = 0x51; break;
                case 0x33: displayKey = 0x52; break;
                case 0x2A: displayKey = 0x4C; break;
                case 0x35: displayKey = 0x29; break;
                default: break;
            }
        }
        hasKey = true;
        const char* label = hidToDisplay(displayKey, status.shift, buf);
        if (strlen(label) > 0) snprintf(out + len, outSize - len, "%s", label);
        break;
    }

    if (!hasKey && len > 0) out[len - 1] = '\0';
    return out;
}

// ============================================================
// 文本缓冲区: 将按键转为可累积的 ASCII 字符
// ============================================================

/**
 * @brief 根据按键状态返回应添加到文本缓冲区的字符
 * @param mouseMode 鼠标模式 (不累积)
 * @param status    键盘状态
 * @return 要追加的 ASCII 字符; '\b'=退格, '\n'=回车, '\t'=Tab, 0=不追加
 */
static char getTypedChar(bool mouseMode, const Keyboard_Class::KeysState& status) {
    if (mouseMode) return 0;

    // 仅修饰键按下 — 不追加
    bool onlyMods = (status.ctrl || status.shift || status.alt || status.opt)
                    && !status.enter
                    && !M5Cardputer.Keyboard.isKeyPressed(' ');
    bool anyHidKey = false;
    for (auto k : status.hid_keys) { if (k != 0) { anyHidKey = true; break; } }
    if (onlyMods && !anyHidKey) return 0;

    // Enter
    if (status.enter) return '\n';
    // Space
    if (M5Cardputer.Keyboard.isKeyPressed(' ')) return ' ';

    for (auto key : status.hid_keys) {
        if (key == 0) continue;
        uint8_t k = key;
        if (status.fn) {
            switch (key) {
                case 0x38: k = 0x4F; break; // / → Right
                case 0x36: k = 0x50; break; // , → Left
                case 0x37: k = 0x51; break; // . → Down
                case 0x33: k = 0x52; break; // ; → Up
                case 0x2A: k = 0x4C; break; // Bsp → Delete
                case 0x35: k = 0x29; break; // ` → Esc
                default: break;
            }
        }

        // 字母 a-z
        if (k >= 0x04 && k <= 0x1D) {
            char c = 'a' + (k - 0x04);
            if (status.shift) c = c - 'a' + 'A';
            return c;
        }
        // 数字 1-9, 0
        if (k >= 0x1E && k <= 0x27) {
            static const char* shiftedNum = "!@#$%^&*()";
            if (status.shift && k - 0x1E < 10) return shiftedNum[k - 0x1E];
            if (k == 0x27) return '0';
            return '1' + (k - 0x1E);
        }
        // 特殊键
        switch (k) {
            case 0x28: return '\n';   // Enter
            case 0x2A: return '\b';   // Backspace
            case 0x2B: return '\t';   // Tab
            case 0x2C: return ' ';    // Space
            case 0x2D: return '-';
            case 0x2E: return '=';
            case 0x2F: return '[';
            case 0x30: return ']';
            case 0x31: return '\\';
            case 0x33: return ';';
            case 0x34: return '\'';
            case 0x35: return '`';
            case 0x36: return ',';
            case 0x37: return '.';
            case 0x38: return '/';
            default: return 0;  // 方向键等不累积
        }
    }
    return 0;
}

/**
 * @brief 将字符追加到文本缓冲区
 */
static void appendToTextBuffer(char c) {
    if (c == '\b') {
        if (g_textLen > 0) {
            g_textLen--;
            g_textBuf[g_textLen] = '\0';
        }
    } else if (c == '\t') {
        // Tab → 2 空格
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

/**
 * @brief 绘制文本缓冲区 (小字体, 多行, 底部对齐)
 *
 * 单遍扫描: 按 \n 分段 + 按屏宽折行, 存入环形缓冲区,
 * 只绘制最后 maxVisLines 行.
 */
static void drawTextBuffer() {
    int y0 = TEXT_BUF_Y;
    int h  = TEXT_BUF_H;
    int w  = M5Cardputer.Display.width();

    M5Cardputer.Display.fillRect(0, y0, w, h, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y0, w - 12, h, 4, TFT_DARKGREY);
    if (g_textLen == 0) return;

    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);

    int margin = 10;
    int charsPerLine = (w - 2 * margin) / 6;
    if (charsPerLine < 1) charsPerLine = 1;
    int lineH = 9;
    int maxVisLines = h / lineH;
    if (maxVisLines < 1) maxVisLines = 1;

    // 环形缓冲区: 存储每行在 g_textBuf 中的起始位置和长度
    #define RING_CAP 48
    struct { int start; int len; } ring[RING_CAP];
    int ringIdx = 0, ringCount = 0;  // ringCount = min(total, RING_CAP)

    auto pushLine = [&](int start, int len) {
        ring[ringIdx].start = start;
        ring[ringIdx].len   = len;
        ringIdx = (ringIdx + 1) % RING_CAP;
        if (ringCount < RING_CAP) ringCount++;
    };

    int segStart = 0;  // 当前逻辑段起始
    for (int i = 0; i <= g_textLen; i++) {
        char ch = (i < g_textLen) ? g_textBuf[i] : '\n'; // 结尾视为 \n
        if (ch == '\t') ch = ' ';  // Tab → 空格 (仅显示)

        bool atNewline = (ch == '\n');
        bool segFull    = (i - segStart >= charsPerLine);

        if (atNewline || segFull || i == g_textLen) {
            int take = i - segStart;
            if (atNewline && segFull) {
                // 需要折行: 先输出 charsPerLine 个, 换行符留给下一段
                pushLine(segStart, charsPerLine);
                segStart += charsPerLine;
                // 剩余字符 + \n 在循环中继续处理
                i--;  // 回退, 下次处理 \n
                continue;
            }
            if (take > 0 || atNewline) {
                pushLine(segStart, take);
            }
            segStart = i + 1;  // 下一段从 \n 之后开始
        }
    }

    // 绘制最后 maxVisLines 行
    int startIdx = (ringCount > maxVisLines) ? ringCount - maxVisLines : 0;
    for (int di = 0; di < maxVisLines && (startIdx + di) < ringCount; di++) {
        int ri = (ringIdx - ringCount + startIdx + di) % RING_CAP;
        if (ri < 0) ri += RING_CAP;

        int len = ring[ri].len;
        if (len <= 0) continue;
        if (len > charsPerLine) len = charsPerLine;

        char lineBuf[64];
        int copyLen = len < (int)sizeof(lineBuf) - 1 ? len : (int)sizeof(lineBuf) - 1;
        int src = ring[ri].start;
        for (int j = 0; j < copyLen; j++) {
            char c = g_textBuf[src + j];
            lineBuf[j] = (c == '\t') ? ' ' : c;
        }
        lineBuf[copyLen] = '\0';

        M5Cardputer.Display.setCursor(margin, y0 + 3 + di * lineH);
        M5Cardputer.Display.print(lineBuf);
    }
}

// ============================================================
// 按键显示 (实时 + 累积)
// ============================================================

void drawKeyDisplay(bool mouseMode) {
    char text[32];
    getKeyDisplayString(mouseMode, text, sizeof(text));

    // --- 实时按键区域 (上半) ---
    {
        int y = KEY_DISPLAY_Y;
        int h = KEY_DISPLAY_H;
        int w = M5Cardputer.Display.width();
        M5Cardputer.Display.fillRect(0, y, w, h, TFT_BLACK);
        M5Cardputer.Display.drawRoundRect(6, y, w - 12, h, 4, TFT_DARKGREY);

        // 使用 g_lastLiveKey 保持最后按下的实时按键
        if (strlen(text) > 0) {
            strcpy(g_lastLiveKey, text);
        }
        const char* showKey = (strlen(text) > 0) ? text : g_lastLiveKey;

        if (strlen(showKey) > 0) {
            int tLen = strlen(showKey);
            int textSize;
            if (tLen <= 2)       textSize = 4;
            else if (tLen <= 4)  textSize = 3;
            else                 textSize = 2;

            int charW = 6 * textSize;
            int textW = tLen * charW;
            int cx = (w - textW) / 2;
            if (cx < 4) cx = 4;
            int cy = y + (h - 8 * textSize) / 2;

            M5Cardputer.Display.setTextColor(TFT_WHITE);
            M5Cardputer.Display.setTextSize(textSize);
            M5Cardputer.Display.setCursor(cx, cy);
            M5Cardputer.Display.print(showKey);
        }
    }

    // --- 文本累积 (下半) ---
    // 用 static 变量记录上一帧追加的按键, 避免按住不放时重复追加
    {
        static char lastAppendKey[32] = "";
        if (strlen(text) > 0) {
            if (strcmp(text, lastAppendKey) != 0) {
                strcpy(lastAppendKey, text);
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
                char c = getTypedChar(mouseMode, status);
                if (c != 0) appendToTextBuffer(c);
            }
        } else {
            lastAppendKey[0] = '\0';  // 全部松开后重置, 允许下次再追加
        }
    }

    drawTextBuffer();
}

void clearKeyDisplayArea() {
    // 清空缓冲区
    g_textLen = 0;
    g_textBuf[0] = '\0';
    g_lastLiveKey[0] = '\0';

    int w = M5Cardputer.Display.width();

    // 实时按键区
    {
        int y = KEY_DISPLAY_Y;
        int h = KEY_DISPLAY_H;
        M5Cardputer.Display.fillRect(0, y, w, h, TFT_BLACK);
        M5Cardputer.Display.drawRoundRect(6, y, w - 12, h, 4, TFT_DARKGREY);
    }
    // 文本缓冲区
    {
        int y = TEXT_BUF_Y;
        int h = TEXT_BUF_H;
        M5Cardputer.Display.fillRect(0, y, w, h, TFT_BLACK);
        M5Cardputer.Display.drawRoundRect(6, y, w - 12, h, 4, TFT_DARKGREY);
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

    // USB
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

    // 蓝牙
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
