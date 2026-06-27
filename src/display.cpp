/**
 * @file display.cpp
 * @brief 显示屏 UI 绘制函数实现
 *
 * 使用 M5Cardputer 内置 TFT 显示屏绘制用户界面。
 * 屏幕旋转设置为1 (横屏)，分辨率为 240x135。
 *
 * UI 布局总览:
 * ┌──────────────────────────────────┐
 * │ [USB/BT] KEYBOARD          GO ▶ │  y=0~21   (状态栏: 图标+模式+GO提示)
 * │                                  │
 * │     ┌──────────────────┐        │  y=24~129 (按键显示 - 106px 大区域)
 * │     │   Last / Current │        │
 * │     │       Key        │        │
 * │     └──────────────────┘        │
 * └──────────────────────────────────┘
 *
 * 颜色说明:
 *   - 绿色 (TFT_GREEN)  = 连接正常 / USB模式
 *   - 白色 (TFT_WHITE)  = 未选中/默认
 *   - 红色 (TFT_RED)    = 蓝牙未连接
 *   - 灰色 (TFT_LIGHTGRAY) = 标题边框
 */

#include "display.h"

// ============================================================
// 布局常量
// ============================================================
#define STATUS_BAR_H    22    // 顶部状态栏高度 (略高以容纳更好的图标)
#define KEY_DISPLAY_Y   24    // 按键显示区域 Y 坐标 (紧跟状态栏)
#define KEY_DISPLAY_H   106   // 按键显示区域高度 (占满剩余屏幕)

// 记录最后一次按下的按键，松开后保持显示
static char g_lastKeyText[32] = "";

// ============================================================
// 图标绘制函数 (美化版)
// ============================================================

/**
 * @brief 绘制标准 USB 三叉戟图标 (16x17 px)
 *
 * 形状参考标准 USB 标志:
 *     ═══        ← 横杆
 *    ╱ │ ╲       ← 两侧斜线
 *      │          ← 竖杆
 *      ●          ← 底部圆点 (连接器)
 */
static void drawUsbIcon(uint8_t x, uint8_t y, uint16_t color) {
    uint8_t cx = x + 8;   // 中心 X
    uint8_t topY = y + 2;
    uint8_t botY = y + 14;

    // 底部圆点 (USB 连接器)
    M5Cardputer.Display.fillCircle(cx, botY + 1, 3, color);

    // 竖杆
    M5Cardputer.Display.drawLine(cx, topY, cx, botY - 1, color);

    // 顶部横杆
    M5Cardputer.Display.drawLine(x + 2, topY, x + 14, topY, color);

    // 左上斜线 (箭头)
    M5Cardputer.Display.drawLine(x, y + 5, cx - 1, topY, color);

    // 右上斜线 (箭头)
    M5Cardputer.Display.drawLine(x + 16, y + 5, cx + 1, topY, color);
}

/**
 * @brief 绘制标准蓝牙北欧符文图标 (16x18 px)
 *
 * 蓝牙标志来自 Younger Futhark 的组合符文 (ᚼ + ᛒ):
 *   中心竖线 + 右侧上下两个指向右的折角 = 侧放的 "B" 形
 */
static void drawBluetoothIcon(uint8_t x, uint8_t y, uint16_t color) {
    uint8_t cx = x + 8;

    // 中心竖线 (贯穿整个图标)
    M5Cardputer.Display.drawLine(cx, y + 1, cx, y + 17, color);

    // === 右侧: 上下两个指向右的尖角 ===
    // 上尖角 — 从顶部斜向右, 再折回中部
    M5Cardputer.Display.drawLine(cx, y + 1, x + 15, y + 7, color);
    M5Cardputer.Display.drawLine(cx, y + 8, x + 15, y + 7, color);

    // 下尖角 — 从底部斜向右, 再折回中部
    M5Cardputer.Display.drawLine(cx, y + 17, x + 15, y + 12, color);
    M5Cardputer.Display.drawLine(cx, y + 9, x + 15, y + 12, color);

    // === 左侧: 上下两个小尖角 (形成 "B" 的左边弧) ===
    // 上左尖角
    M5Cardputer.Display.drawLine(cx, y + 3, x + 1, y + 7, color);
    M5Cardputer.Display.drawLine(cx, y + 8, x + 1, y + 7, color);

    // 下左尖角
    M5Cardputer.Display.drawLine(cx, y + 9, x + 1, y + 13, color);
    M5Cardputer.Display.drawLine(cx, y + 15, x + 1, y + 13, color);
}

// ============================================================
// 屏幕初始化
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

    // 顶部状态栏 — 含连接图标 + 模式 + GO 提示
    drawStatusBar(usbMode, mouseMode, bluetoothStatus);

    // 按键显示区域 (初始为空白带边框)
    clearKeyDisplayArea();
}

// ============================================================
// 顶部状态栏
// ============================================================

/**
 * @brief 绘制顶部状态栏
 *
 * 布局 (y=0~21, h=22):
 *   [USB/BT 图标 x=2] [模式文字 x=20]  ...  [GO ▶ x=右侧]
 *
 * 左侧图标:
 *   - USB 模式: 绿色 USB 三叉戟
 *   - 蓝牙已连接: 绿色蓝牙符文
 *   - 蓝牙未连接: 红色蓝牙符文
 *
 * 右侧: "GO ▶" 提示 — 按 GO 键切换键盘/鼠标模式
 */
void drawStatusBar(bool usbMode, bool mouseMode, bool bluetoothStatus) {
    int w = M5Cardputer.Display.width();
    uint16_t bgColor = mouseMode ? TFT_NAVY : TFT_DARKGREEN;
    M5Cardputer.Display.fillRect(0, 0, w, STATUS_BAR_H, bgColor);

    // === 左侧: 连接图标 ===
    uint16_t iconColor;
    if (usbMode) {
        iconColor = TFT_GREEN;                          // USB 始终绿色
    } else {
        iconColor = bluetoothStatus ? TFT_GREEN : TFT_RED; // BT: 连接=绿, 未连接=红
    }

    if (usbMode) {
        drawUsbIcon(2, 3, iconColor);
    } else {
        drawBluetoothIcon(2, 2, iconColor);
    }

    // === 中间: 模式文字 ===
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.setTextSize(1.5);
    M5Cardputer.Display.setCursor(20, 4);
    M5Cardputer.Display.print(mouseMode ? "MOUSE" : "KEYBOARD");

    // === 右侧: GO 切换提示 ===
    // 小字体右对齐, 提示用户按 GO 键切换模式
    M5Cardputer.Display.setTextSize(1.2);
    const char* goHint = "GO >";
    int goW = strlen(goHint) * 7;  // 字号1.2 ≈ 7px/char
    M5Cardputer.Display.setCursor(w - goW - 6, 5);
    M5Cardputer.Display.print(goHint);
}

// ============================================================
// 按键 → 显示符号 映射
// ============================================================

/**
 * @brief 将 HID 键码转换为紧凑的显示符号
 *
 * 使用业界常见的键帽标注方式:
 *   - 字母: a-z / A-Z (Shift)
 *   - 数字: 1-9, 0 / !@#$%^&*() (Shift)
 *   - 方向键: ^ v < >  (业界标准方向表示)
 *   - 编辑键: Ent Esc Bsp Tab Spc Del
 *   - 标点符号: 保持原样
 */
static const char* hidToDisplay(uint8_t key, bool shifted, char* buf) {
    buf[0] = '\0';

    if (key >= 0x04 && key <= 0x1D) {
        // a - z
        char c = 'a' + (key - 0x04);
        if (shifted) c = c - 'a' + 'A';
        buf[0] = c;
        buf[1] = '\0';
    } else if (key >= 0x1E && key <= 0x27) {
        // 1-9, 0
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
            // --- 编辑/导航键 (紧凑符号) ---
            case 0x28: strcpy(buf, "Ent");  break;  // Enter / Return
            case 0x29: strcpy(buf, "Esc");  break;  // Escape
            case 0x2A: strcpy(buf, "Bsp");  break;  // Backspace
            case 0x2B: strcpy(buf, "Tab");  break;  // Tab
            case 0x2C: strcpy(buf, "Spc");  break;  // Space
            case 0x4C: strcpy(buf, "Del");  break;  // Delete (Forward)
            // --- 方向键 (ASCII 箭头) ---
            case 0x4F: strcpy(buf, ">");    break;  // Right Arrow
            case 0x50: strcpy(buf, "<");    break;  // Left Arrow
            case 0x51: strcpy(buf, "v");    break;  // Down Arrow
            case 0x52: strcpy(buf, "^");    break;  // Up Arrow
            // --- 标点符号 ---
            case 0x2D: strcpy(buf, "-");    break;
            case 0x2E: strcpy(buf, "=");    break;
            case 0x2F: strcpy(buf, "[");    break;
            case 0x30: strcpy(buf, "]");    break;
            case 0x31: strcpy(buf, "\\");   break;
            case 0x33: strcpy(buf, ";");    break;
            case 0x34: strcpy(buf, "'");    break;
            case 0x35: strcpy(buf, "`");    break;
            case 0x36: strcpy(buf, ",");    break;
            case 0x37: strcpy(buf, ".");    break;
            case 0x38: strcpy(buf, "/");    break;
            default: break;
        }
    }
    return buf;
}

// ============================================================
// 按键显示字符串构建
// ============================================================

/**
 * @brief 从键盘状态构建显示字符串
 *
 * 键盘模式: "Ctrl+A", "Shift+1", "Ent", "^" (方向键)
 * 鼠标模式: "^" "v" "L-Clk" "R-Clk"
 */
static const char* getKeyDisplayString(bool mouseMode, char* out, size_t outSize) {
    out[0] = '\0';
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    char buf[8];

    if (mouseMode) {
        // 方向移动 — 使用箭头符号
        if (M5Cardputer.Keyboard.isKeyPressed(';'))        { strcpy(out, "^");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('.'))        { strcpy(out, "v");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('/'))        { strcpy(out, ">");      return out; }
        if (M5Cardputer.Keyboard.isKeyPressed(','))        { strcpy(out, "<");      return out; }
        // 鼠标按键
        if (status.enter)                                  { strcpy(out, "L-Clk");  return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('\\'))       { strcpy(out, "R-Clk");  return out; }
        return out;
    }

    // === 键盘模式 ===
    size_t len = 0;

    // 修饰键前缀
    if (status.ctrl)  len += snprintf(out + len, outSize - len, "Ctrl+");
    if (status.shift) len += snprintf(out + len, outSize - len, "Shift+");
    if (status.alt)   len += snprintf(out + len, outSize - len, "Alt+");
    if (status.opt)   len += snprintf(out + len, outSize - len, "Win+");

    // 特殊按键 (Enter, Space)
    if (status.enter) {
        snprintf(out + len, outSize - len, "Ent");
        return out;
    }
    if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
        snprintf(out + len, outSize - len, "Spc");
        return out;
    }

    // 从 hid_keys 数组中提取按键
    bool hasKey = false;
    for (auto key : status.hid_keys) {
        if (key == 0) continue;

        // FN 组合键映射
        uint8_t displayKey = key;
        if (status.fn) {
            switch (key) {
                case 0x38: displayKey = 0x4F; break; // /  → Right
                case 0x36: displayKey = 0x50; break; // ,  → Left
                case 0x37: displayKey = 0x51; break; // .  → Down
                case 0x33: displayKey = 0x52; break; // ;  → Up
                case 0x2A: displayKey = 0x4C; break; // Bsp → Delete
                case 0x35: displayKey = 0x29; break; // `  → Esc
                default: break;
            }
        }

        hasKey = true;
        const char* label = hidToDisplay(displayKey, status.shift, buf);
        if (strlen(label) > 0) {
            snprintf(out + len, outSize - len, "%s", label);
        }
        break;  // 只显示第一个按键
    }

    if (!hasKey && len > 0) {
        out[len - 1] = '\0';  // 去掉尾部的 '+'
    }

    return out;
}

// ============================================================
// 按键显示区域绘制
// ============================================================

/**
 * @brief 在屏幕中央区域绘制当前/上一次按下的按键
 *
 * 行为:
 *   - 有按键按下: 显示当前按键, 并保存到 g_lastKeyText
 *   - 无按键按下: 继续显示上次保存的按键 (不会消失)
 *   - clearKeyDisplayArea() 可清空保存的按键
 */
void drawKeyDisplay(bool mouseMode) {
    char text[32];
    getKeyDisplayString(mouseMode, text, sizeof(text));

    // 记住最后按下的按键 (有新的则更新)
    if (strlen(text) > 0) {
        strcpy(g_lastKeyText, text);
    }

    // 清除显示区域 + 绘制边框
    int y = KEY_DISPLAY_Y;
    int h = KEY_DISPLAY_H;
    int w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y, w, h, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y, w - 12, h, 4, TFT_DARKGREY);

    // 选择显示内容: 当前按键优先, 否则显示上次按键
    const char* displayText = (strlen(text) > 0) ? text : g_lastKeyText;
    if (strlen(displayText) == 0) return;

    // 根据文本长度自适应字号:
    //   1-2 字符 (如 "^" "v" "A" "Ent"): textSize=5 (超大)
    //   3-4 字符 (如 "Bsp" "Tab" "Del"): textSize=4
    //   5+  字符 (如 "Ctrl+A" "Shift+1"): textSize=3
    int tLen = strlen(displayText);
    int textSize;
    if (tLen <= 2)       textSize = 5;
    else if (tLen <= 4)  textSize = 4;
    else                 textSize = 3;

    int charW = 6 * textSize;
    int textW = tLen * charW;
    int cx = (w - textW) / 2;
    if (cx < 4) cx = 4;
    int cy = y + (h - 8 * textSize) / 2;

    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.setTextSize(textSize);
    M5Cardputer.Display.setCursor(cx, cy);
    M5Cardputer.Display.print(displayText);
}

/**
 * @brief 清空按键显示区域并重置记忆
 */
void clearKeyDisplayArea() {
    g_lastKeyText[0] = '\0';  // 清空上次按键记忆

    int y = KEY_DISPLAY_Y;
    int h = KEY_DISPLAY_H;
    int w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y, w, h, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y, w - 12, h, 4, TFT_DARKGREY);
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

    // USB 选项
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

    // 蓝牙选项
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
