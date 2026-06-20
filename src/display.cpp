/**
 * @file display.cpp
 * @brief 显示屏 UI 绘制函数实现
 *
 * 使用 M5Cardputer 内置 TFT 显示屏绘制用户界面。
 * 屏幕旋转设置为1 (横屏)，分辨率为 240x135。
 *
 * UI 布局总览:
 * ┌──────────────────────────────────┐
 * │  M5-Keyboard-Mouse  (标题)       │  y=10~30
 * │  [USB/Bluetooth状态] [GO switch] │  y=39~59
 * │                                  │
 * │  ┌──────────┐  ┌──────────┐     │  y=70~125
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
 *   两个框 y=70, h=~55
 */
void drawDeviceRect(bool reverse) {
    if (reverse) {
        // 键盘模式: 键盘框绿色, 鼠标框白色
        M5Cardputer.Display.drawRoundRect(10, 70, M5Cardputer.Display.width() / 2 - 15, M5Cardputer.Display.height() - 80, 3, TFT_WHITE); // 键盘框
        M5Cardputer.Display.drawRoundRect(M5Cardputer.Display.width() / 2 + 5, 70, M5Cardputer.Display.width() / 2 - 15, M5Cardputer.Display.height() - 80,  3, TFT_GREEN);  // 鼠标框
    } else {
        // 鼠标模式: 键盘框白色, 鼠标框绿色
        M5Cardputer.Display.drawRoundRect(10, 70, M5Cardputer.Display.width() / 2 - 15, M5Cardputer.Display.height() - 80, 3, TFT_GREEN);
        M5Cardputer.Display.drawRoundRect(M5Cardputer.Display.width() / 2 + 5, 70, M5Cardputer.Display.width() / 2 - 15, M5Cardputer.Display.height() - 80,  3, TFT_WHITE);
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
 * 在屏幕顶部的标题下方绘制连接状态指示。
 * - USB模式: 显示绿色 "USB"
 * - 蓝牙模式已连接: 显示绿色 "Bluetooth"
 * - 蓝牙模式未连接: 显示红色 "Bluetooth"
 *
 * 位置: 标题栏下方 y=39~59
 */
void modeIndicator(bool usbMode, bool bluetoothStatus) {
    M5Cardputer.Display.setTextSize(1.6);

    if (bluetoothStatus || usbMode) {
        // 连接正常 — 绿色指示器
        M5Cardputer.Display.drawRoundRect(10, 39, 104, 20, 5, TFT_GREEN);
        M5Cardputer.Display.setTextColor(TFT_GREEN);

    } else {
        // 蓝牙未连接 — 红色指示器
        M5Cardputer.Display.drawRoundRect(10, 39, 104, 20, 5, TFT_RED);
        M5Cardputer.Display.setTextColor(TFT_RED);
    }

    // 显示模式文字
    if (usbMode) {
        M5Cardputer.Display.setCursor(50, 43);
        M5Cardputer.Display.print("USB");
    } else {
        M5Cardputer.Display.setCursor(23, 43);
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
 * 2. 标题栏: "M5-Keyboard-Mouse" (灰底黑字)
 * 3. "GO switch" 提示文字 (提示用户切换按键位置)
 * 4. 设备矩形框 (键盘/鼠标)
 * 5. 鼠标图标 和 键盘图标
 * 6. 连接模式指示器
 */
void displayMainScreen(bool usbMode, bool mouseMode, bool bluetoothStatus) {
    M5Cardputer.Display.fillScreen(TFT_BLACK);

    // 标题栏 — 灰色圆角矩形底 + 黑色文字
    M5Cardputer.Display.fillRoundRect(10, 10, M5Cardputer.Display.width()-20, 20, 5, TFT_LIGHTGREY);
    M5Cardputer.Display.setCursor(19, 13);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(TFT_BLACK);
    M5Cardputer.Display.print("M5-Keyboard-Mouse");

    // "GO switch" 切换提示
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
    M5Cardputer.Display.drawRoundRect(123, 39, 106, 20, 5, TFT_LIGHTGREY);
    M5Cardputer.Display.setCursor(136, 43);
    M5Cardputer.Display.setTextSize(1.6);
    M5Cardputer.Display.print("GO switch");

    // 绘制设备区域和图标
    drawDeviceRect(true);
    drawMouseIcon(165, 80);     // 鼠标图标 — 右侧
    drawKeyboardIcon(42, 87);   // 键盘图标 — 左侧
    modeIndicator(usbMode, bluetoothStatus);
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
