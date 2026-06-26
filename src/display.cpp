/**
 * @file display.cpp
 * @brief 显示屏 UI 绘制函数实现
 *
 * 使用 M5Cardputer 内置 TFT 显示屏绘制用户界面。
 * 屏幕旋转设置为1 (横屏)，分辨率为 240x135。
 *
 * UI 布局总览:
 * ┌──────────────────────────────────┐
 * │  KEYBOARD MODE / MOUSE MODE     │  y=0~16   (状态栏)
 * │  [USB/Bluetooth状态] [GO switch] │  y=19~38  (连接行)
 * │                                  │
 * │     ┌──────────────────┐        │  y=40~86  (按键显示)
 * │     │    Pressed Key   │        │
 * │     └──────────────────┘        │
 * │  ┌──────────┐  ┌──────────┐     │  y=89~132 (设备图标)
 * │  │  键盘图标  │  │  鼠标图标  │     │
 * │  └──────────┘  └──────────┘     │
 * └──────────────────────────────────┘
 *
 * 颜色说明:
 *   - 绿色 (TFT_GREEN)  = 当前选中的设备/连接正常
 *   - 白色 (TFT_WHITE)  = 未选中/默认
 *   - 红色 (TFT_RED)    = 蓝牙未连接
 *   - 灰色 (TFT_LIGHTGRAY) = 标题边框
 */

#include "display.h"

// ============================================================
// 布局常量
// ============================================================
#define STATUS_BAR_H    17    // 顶部状态栏高度
#define CONNECTION_Y    19    // 连接指示行 Y 坐标
#define KEY_DISPLAY_Y   40    // 按键显示区域 Y 坐标
#define KEY_DISPLAY_H   47    // 按键显示区域高度
#define DEVICE_RECT_Y   89    // 设备矩形框 Y 坐标
#define DEVICE_RECT_H   44    // 设备矩形框高度

// ============================================================
// 设备模式矩形框
// ============================================================

/**
 * @brief 绘制键盘和鼠标的设备选择矩形框
 * @param reverse true=键盘绿框+鼠标白框 (反向=键盘模式), false=键盘白框+鼠标绿框 (鼠标模式)
 *
 * 在屏幕下半部分绘制两个圆角矩形框，分别代表键盘和鼠标。
 * 绿色框表示当前选中的设备，白色框表示未选中的设备。
 *
 * 布局:
 *   左侧 (x=10, w=~102): 键盘框
 *   右侧 (x=~125, w=~102): 鼠标框
 *   两个框 y=DEVICE_RECT_Y, h=DEVICE_RECT_H
 */
void drawDeviceRect(bool reverse) {
    int w = M5Cardputer.Display.width() / 2 - 15;
    if (reverse) {
        // 键盘模式: 键盘框绿色, 鼠标框白色
        M5Cardputer.Display.drawRoundRect(10, DEVICE_RECT_Y, w, DEVICE_RECT_H, 3, TFT_GREEN);
        M5Cardputer.Display.drawRoundRect(M5Cardputer.Display.width() / 2 + 5, DEVICE_RECT_Y, w, DEVICE_RECT_H, 3, TFT_WHITE);
    } else {
        // 鼠标模式: 键盘框白色, 鼠标框绿色
        M5Cardputer.Display.drawRoundRect(10, DEVICE_RECT_Y, w, DEVICE_RECT_H, 3, TFT_WHITE);
        M5Cardputer.Display.drawRoundRect(M5Cardputer.Display.width() / 2 + 5, DEVICE_RECT_Y, w, DEVICE_RECT_H, 3, TFT_GREEN);
    }
}

// ============================================================
// 图标绘制
// ============================================================

/**
 * @brief 绘制鼠标图标
 * @param x 左上角 X 坐标
 * @param y 左上角 Y 坐标
 *
 * 绘制一个简化的鼠标形状:
 *   - 白色圆角矩形 (身体)
 *   - 中间一条黑色竖线 (左右键分隔线)
 */
void drawMouseIcon(uint8_t x, uint8_t y) {
    uint8_t w = 25;
    uint8_t h = 35;

    // 鼠标主体 — 白色圆角矩形
    M5Cardputer.Display.fillRoundRect(x, y, 25, 35, 5, TFT_WHITE);

    // 左右键分隔线
    M5Cardputer.Display.drawLine(x+w/2, y, x+w/2, y+h/2, TFT_BLACK);
}

/**
 * @brief 绘制键盘图标
 * @param x 左上角 X 坐标
 * @param y 左上角 Y 坐标
 *
 * 绘制一个简化的键盘形状:
 *   - 白色矩形底座
 *   - 两排黑色方块模拟按键 (上排5个, 下排5个)
 */
void drawKeyboardIcon(uint8_t x, uint8_t y) {
    // 键盘外框 — 白色矩形
    M5Cardputer.Display.fillRect(x, y, 40, 20, TFT_WHITE);

    // 第一排按键 (y+2)
    M5Cardputer.Display.fillRect(x + 2, y + 2, 6, 6, TFT_BLACK);   // 按键 1
    M5Cardputer.Display.fillRect(x + 10, y + 2, 6, 6, TFT_BLACK);  // 按键 2
    M5Cardputer.Display.fillRect(x + 18, y + 2, 6, 6, TFT_BLACK);  // 按键 3
    M5Cardputer.Display.fillRect(x + 26, y + 2, 6, 6, TFT_BLACK);  // 按键 4
    M5Cardputer.Display.fillRect(x + 34, y + 2, 6, 6, TFT_BLACK);  // 按键 5

    // 第二排按键 (y+10)
    M5Cardputer.Display.fillRect(x + 2, y + 10, 6, 6, TFT_BLACK);  // 按键 6
    M5Cardputer.Display.fillRect(x + 10, y + 10, 6, 6, TFT_BLACK); // 按键 7
    M5Cardputer.Display.fillRect(x + 18, y + 10, 6, 6, TFT_BLACK); // 按键 8
    M5Cardputer.Display.fillRect(x + 26, y + 10, 6, 6, TFT_BLACK); // 按键 9
    M5Cardputer.Display.fillRect(x + 34, y + 10, 6, 6, TFT_BLACK); // 按键 10

    // 键盘边框线 (由于按键可能覆盖了白色矩形的边框，需要重新绘制)
    M5Cardputer.Display.drawLine(x, y, x, y + 20, TFT_WHITE);           // 左边框
    M5Cardputer.Display.drawLine(x + 40, y, x + 40, y + 20, TFT_WHITE); // 右边框
    M5Cardputer.Display.drawLine(x, y, x + 40, y, TFT_WHITE);           // 上边框
    M5Cardputer.Display.drawLine(x, y + 20, x + 40, y + 20, TFT_WHITE); // 下边框
}

// ============================================================
// 状态指示器
// ============================================================

/**
 * @brief 绘制/更新连接模式指示器
 * @param usbMode         true=USB模式, false=蓝牙模式
 * @param bluetoothStatus true=已连接, false=未连接
 *
 * 在屏幕顶部的状态栏下方绘制连接状态指示。
 * - USB模式: 显示绿色 "USB"
 * - 蓝牙模式已连接: 显示绿色 "Bluetooth"
 * - 蓝牙模式未连接: 显示红色 "Bluetooth"
 *
 * 位置: CONNECTION_Y (y=19~38)
 */
void modeIndicator(bool usbMode, bool bluetoothStatus) {
    M5Cardputer.Display.setTextSize(1.6);
    int y = CONNECTION_Y;

    if (bluetoothStatus || usbMode) {
        // 连接正常 — 绿色指示器
        M5Cardputer.Display.drawRoundRect(10, y, 104, 20, 5, TFT_GREEN);
        M5Cardputer.Display.setTextColor(TFT_GREEN);
    } else {
        // 蓝牙未连接 — 红色指示器
        M5Cardputer.Display.drawRoundRect(10, y, 104, 20, 5, TFT_RED);
        M5Cardputer.Display.setTextColor(TFT_RED);
    }

    // 显示模式文字
    if (usbMode) {
        M5Cardputer.Display.setCursor(50, y + 4);
        M5Cardputer.Display.print("USB");
    } else {
        M5Cardputer.Display.setCursor(23, y + 4);
        M5Cardputer.Display.print("Bluetooth");
    }
}

// ============================================================
// 屏幕初始化
// ============================================================

/**
 * @brief 初始化显示屏设置
 *
 * - 旋转方向设置为1 (横屏显示)
 * - 填充黑色背景
 * - 设置默认文字颜色为黑色
 */
void setupDisplay() {
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_BLACK);

}

// ============================================================
// 欢迎画面
// ============================================================

/**
 * @brief 显示启动欢迎画面
 *
 * 显示内容:
 *   - 设备名称: "M5-Keyboard-Mouse"
 *   - 版本信息: "Version 1.1 - Geo"
 *   - 显示持续约2秒后进入模式选择
 */
void displayWelcomeScreen() {
    // 名称外框
    M5Cardputer.Display.drawRect(9, 47, 220, 40, TFT_LIGHTGRAY);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(18, 58);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.printf("M5-Keyboard-Mouse");

    // 版本信息
    M5Cardputer.Display.setCursor(70, 120);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.printf("Version 1.1 - Geo");

    delay(2000);  // 显示2秒
}

// ============================================================
// 主界面
// ============================================================

/**
 * @brief 绘制完整的主界面
 * @param usbMode         true=USB模式, false=蓝牙模式
 * @param mouseMode       true=鼠标模式, false=键盘模式
 * @param bluetoothStatus true=蓝牙已连接, false=未连接
 *
 * 绘制所有UI元素:
 * 1. 清屏 (黑色)
 * 2. 顶部状态栏: "KEYBOARD MODE" 或 "MOUSE MODE"
 * 3. 连接模式指示器 + "GO switch" 提示
 * 4. 按键显示区域 (初始为空白带边框)
 * 5. 设备矩形框 (键盘/鼠标) 及图标
 */
void displayMainScreen(bool usbMode, bool mouseMode, bool bluetoothStatus) {
    M5Cardputer.Display.fillScreen(TFT_BLACK);

    // 顶部状态栏 — 显示当前模式 (深绿/深蓝 + 白色文字)
    drawStatusBar(mouseMode);

    // 第二行 — 连接状态指示器
    modeIndicator(usbMode, bluetoothStatus);

    // 第二行右侧 — "GO switch" 切换提示
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
    M5Cardputer.Display.drawRoundRect(123, CONNECTION_Y, 106, 20, 5, TFT_LIGHTGREY);
    M5Cardputer.Display.setCursor(136, CONNECTION_Y + 4);
    M5Cardputer.Display.setTextSize(1.6);
    M5Cardputer.Display.print("GO switch");

    // 设备区域和图标
    drawDeviceRect(mouseMode);
    drawMouseIcon(165, DEVICE_RECT_Y + 8);
    drawKeyboardIcon(42, DEVICE_RECT_Y + 9);

    // 初始化按键显示区域 (带边框的空白区域)
    clearKeyDisplayArea();
}

// ============================================================
// 顶部状态栏
// ============================================================

/**
 * @brief 绘制顶部水平状态栏，显示当前工作模式
 * @param mouseMode true=鼠标模式, false=键盘模式
 *
 * 在屏幕最顶部绘制一条全宽的彩色状态栏：
 *   - 键盘模式: 深绿色背景 + 白色 "KEYBOARD MODE" 文字
 *   - 鼠标模式: 深蓝色背景 + 白色 "MOUSE MODE" 文字
 */
void drawStatusBar(bool mouseMode) {
    int w = M5Cardputer.Display.width();
    if (mouseMode) {
        M5Cardputer.Display.fillRect(0, 0, w, STATUS_BAR_H, TFT_NAVY);
        M5Cardputer.Display.setTextColor(TFT_WHITE);
        M5Cardputer.Display.setTextSize(1.5);
        M5Cardputer.Display.setCursor(55, 2);
        M5Cardputer.Display.print("MOUSE MODE");
    } else {
        M5Cardputer.Display.fillRect(0, 0, w, STATUS_BAR_H, TFT_DARKGREEN);
        M5Cardputer.Display.setTextColor(TFT_WHITE);
        M5Cardputer.Display.setTextSize(1.5);
        M5Cardputer.Display.setCursor(32, 2);
        M5Cardputer.Display.print("KEYBOARD MODE");
    }
}

// ============================================================
// 按键显示
// ============================================================

/**
 * @brief 根据 HID 键码返回对应的显示字符/字符串
 * @param key HID 键码
 * @param shifted true=Shift 修饰键被按下
 * @param buf  输出缓冲区 (至少 8 字节)
 * @return 返回 buf 指针，方便链式调用
 *
 * 将标准 HID 键盘使用码转换为可显示的字符串：
 *   - 字母/数字/标点 → 对应的 ASCII 字符
 *   - 特殊键 → 简短的描述文字 (如 "Enter", "Bksp")
 *   - 未知键码 → 空字符串
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
            case 0x28: strcpy(buf, "Enter"); break;
            case 0x29: strcpy(buf, "Esc");   break;
            case 0x2A: strcpy(buf, "Bksp");  break;
            case 0x2B: strcpy(buf, "Tab");   break;
            case 0x2C: strcpy(buf, "Space"); break;
            case 0x2D: strcpy(buf, "-");     break;
            case 0x2E: strcpy(buf, "=");     break;
            case 0x2F: strcpy(buf, "[");     break;
            case 0x30: strcpy(buf, "]");     break;
            case 0x31: strcpy(buf, "\\");    break;
            case 0x33: strcpy(buf, ";");     break;
            case 0x34: strcpy(buf, "'");     break;
            case 0x35: strcpy(buf, "`");     break;
            case 0x36: strcpy(buf, ",");     break;
            case 0x37: strcpy(buf, ".");     break;
            case 0x38: strcpy(buf, "/");     break;
            case 0x4C: strcpy(buf, "Del");   break;
            case 0x4F: strcpy(buf, "Right"); break;
            case 0x50: strcpy(buf, "Left");  break;
            case 0x51: strcpy(buf, "Down");  break;
            case 0x52: strcpy(buf, "Up");    break;
            default: break;
        }
    }
    return buf;
}

/**
 * @brief 从键盘状态构建显示字符串
 * @param mouseMode true=鼠标模式, false=键盘模式
 * @param out       输出缓冲区
 * @param outSize   缓冲区大小
 * @return 显示字符串指针 (同 out)，无按键时返回空字符串
 *
 * 根据当前模式生成不同的显示文本：
 *   - 键盘模式: "修饰键+按键" 格式，如 "Ctrl+A"、"Shift+1"、"Enter"
 *   - 鼠标模式: 方向或按钮名称，如 "Up"、"L-Click"
 */
static const char* getKeyDisplayString(bool mouseMode, char* out, size_t outSize) {
    out[0] = '\0';
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    char buf[8];

    if (mouseMode) {
        // 鼠标模式 — 方向移动
        if (M5Cardputer.Keyboard.isKeyPressed(';'))        { strcpy(out, "Up");    return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('.'))        { strcpy(out, "Down");  return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('/'))        { strcpy(out, "Right"); return out; }
        if (M5Cardputer.Keyboard.isKeyPressed(','))        { strcpy(out, "Left");  return out; }
        // 鼠标按键
        if (status.enter)                                  { strcpy(out, "L-Click"); return out; }
        if (M5Cardputer.Keyboard.isKeyPressed('\\'))       { strcpy(out, "R-Click"); return out; }
        return out;
    }

    // 键盘模式 — 修饰键前缀
    size_t len = 0;
    if (status.ctrl)  len += snprintf(out + len, outSize - len, "Ctrl+");
    if (status.shift) len += snprintf(out + len, outSize - len, "Shift+");
    if (status.alt)   len += snprintf(out + len, outSize - len, "Alt+");
    if (status.opt)   len += snprintf(out + len, outSize - len, "Win+");

    // 特殊按键 (Enter, Space) — 即使不在 hid_keys 中也能检测
    if (status.enter) {
        snprintf(out + len, outSize - len, "Enter");
        return out;
    }
    if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
        if (len == 0) { strcpy(out, "Space"); }
        else         { snprintf(out + len, outSize - len, "Space"); }
        return out;
    }

    // 从 hid_keys 数组中提取按键
    bool hasKey = false;
    for (auto key : status.hid_keys) {
        if (key == 0) continue;  // 跳过空槽位

        // FN 组合键映射 (与 HID 处理器保持一致)
        uint8_t displayKey = key;
        if (status.fn) {
            switch (key) {
                case 0x38: displayKey = 0x4F; break; // / → Right
                case 0x36: displayKey = 0x50; break; // , → Left
                case 0x37: displayKey = 0x51; break; // . → Down
                case 0x33: displayKey = 0x52; break; // ; → Up
                case 0x2A: displayKey = 0x4C; break; // Bksp → Delete
                case 0x35: displayKey = 0x29; break; // ` → Esc
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
        // 只有修饰键被按下 (如单独的 Ctrl)
        out[len - 1] = '\0';  // 去掉尾部的 '+'
    }

    return out;
}

/**
 * @brief 在屏幕中央区域绘制当前按下的按键
 * @param mouseMode true=鼠标模式, false=键盘模式
 *
 * 从键盘状态提取按键信息，以大号字体居中显示在按键显示区域。
 * 如果无按键被按下，则清空显示区域。
 */
void drawKeyDisplay(bool mouseMode) {
    char text[32];
    getKeyDisplayString(mouseMode, text, sizeof(text));

    // 清除显示区域
    int y = KEY_DISPLAY_Y;
    int h = KEY_DISPLAY_H;
    int w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y, w, h, TFT_BLACK);

    // 绘制边框
    M5Cardputer.Display.drawRoundRect(6, y, w - 12, h, 4, TFT_DARKGREY);

    if (strlen(text) == 0) return;

    // 居中绘制文字
    int textSize = 3;
    int charW = 6 * textSize;  // 等宽字体近似宽度
    int textW = strlen(text) * charW;
    int cx = (w - textW) / 2;
    if (cx < 4) cx = 4;
    int cy = y + (h - 8 * textSize) / 2;  // 8px 为字体基准高度

    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.setTextSize(textSize);
    M5Cardputer.Display.setCursor(cx, cy);
    M5Cardputer.Display.print(text);
}

/**
 * @brief 清空按键显示区域
 *
 * 用黑色填充按键显示区域，在模式切换时调用。
 */
void clearKeyDisplayArea() {
    int y = KEY_DISPLAY_Y;
    int h = KEY_DISPLAY_H;
    int w = M5Cardputer.Display.width();
    M5Cardputer.Display.fillRect(0, y, w, h, TFT_BLACK);
    M5Cardputer.Display.drawRoundRect(6, y, w - 12, h, 4, TFT_DARKGREY);
}

// ============================================================
// 模式选择界面
// ============================================================

/**
 * @brief 显示模式选择界面
 * @param mode true=USB模式高亮, false=蓝牙模式高亮
 *
 * 在启动时显示的选择界面，让用户选择 USB 还是蓝牙通信。
 * 当前选中的模式以灰色填充+黑色文字显示，
 * 未选中的模式以黑色填充+灰色文字显示。
 *
 * 操作提示:
 *   按 '.' 或 ';' 切换选择
 *   按 Enter 确认
 */
void displaySelectionScreen(bool mode) {
    M5Cardputer.Display.clear();
    M5Cardputer.Display.setTextSize(1.5);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(70, 10);
    M5Cardputer.Display.printf("Select Mode:");
    M5Cardputer.Display.setTextSize(3);

    // USB 选项 — 选中时灰底黑字，未选中时黑底灰字
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

    // 蓝牙选项 — 未选中时灰底黑字 (mode=true 即USB选中时)，选中时黑底灰字
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
