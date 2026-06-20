/**
 * @file usbHid.cpp
 * @brief USB HID 键盘/鼠标实现
 *
 * 通过 ESP32-S3 原生 USB OTG 接口模拟 HID 键盘和鼠标。
 * 与蓝牙模式不同，USB 模式下设备直接通过 USB 线缆连接到主机，
 * 无需配对过程，即插即用，延迟更低。
 *
 * 按键映射与蓝牙模式相同:
 *   鼠标模式: 方向键 → 移动, Enter → 左键, \ → 右键
 *   键盘模式: 所有按键 → 标准 HID 键码
 */

#include "usbHid.h"

// === USB HID 设备实例 ===
USBHIDMouse mouse;       // USB HID 鼠标设备
USBHIDKeyboard keyboard; // USB HID 键盘设备

// ============================================================
// USB 模式主处理入口
// ============================================================

/**
 * @brief USB 模式的主处理循环
 * @param mouseMode true=鼠标模式, false=键盘模式
 *
 * 根据当前设备模式分发到对应的处理函数。
 * 每次调用后延迟5ms，提供约200Hz的报告速率。
 */
void handleUsbMode(bool mouseMode) {
    if (mouseMode) {
        usbMouse();
    } else  {
        usbKeyboard();
    }
    delay(5);
}

// ============================================================
// USB 鼠标
// ============================================================

/**
 * @brief 发送 USB 鼠标 HID 报告
 *
 * 键盘按键到鼠标操作的映射:
 * | 按键   | 功能       |
 * |--------|-----------|
 * | ;      | 向上移动   |
 * | .      | 向下移动   |
 * | /      | 向右移动   |
 * | ,      | 向左移动   |
 * | Enter  | 左键按下   |
 * | \      | 右键按下   |
 *
 * 每次移动步长为1像素。由于主循环以高频率运行 (~200Hz)，
 * 长按方向键时鼠标将持续移动，实现流畅的光标控制。
 *
 * 注意: 每次调用都先执行 mouse.begin() 以确保 USB HID 设备已就绪。
 */
void usbMouse() {
    mouse.begin();       // 确保鼠标 HID 设备已初始化
    int moveX = 0;
    int moveY = 0;
    if (M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

        // 水平移动
        if (M5Cardputer.Keyboard.isKeyPressed('/')) {
            moveX = 1;   // 向右移动
        }

        if (M5Cardputer.Keyboard.isKeyPressed(',')) {
            moveX = -1;  // 向左移动
        }

        // 垂直移动
        if (M5Cardputer.Keyboard.isKeyPressed(';')) {
            moveY = -1;  // 向上移动
        }

        if (M5Cardputer.Keyboard.isKeyPressed('.')) {
            moveY = 1;   // 向下移动
        }

        // 鼠标按键 — 按下状态
        if (status.enter) {
            mouse.press(MOUSE_BUTTON_LEFT);    // Enter = 左键
        } else if (M5Cardputer.Keyboard.isKeyPressed('\\')) {
            mouse.press(MOUSE_BUTTON_RIGHT);   // \ = 右键
        }
        // 发送移动报告
        mouse.move(moveX, moveY);

    } else {
        // 无按键按下时释放所有鼠标按键
        mouse.release(MOUSE_BUTTON_LEFT);
        mouse.release(MOUSE_BUTTON_RIGHT);
    }
}

// ============================================================
// USB 键盘
// ============================================================

/**
 * @brief 发送 USB 键盘 HID 报告
 *
 * 处理流程:
 * 1. 首次调用时初始化 USB HID 键盘 (延迟初始化，static 变量确保只执行一次)
 * 2. 仅在键盘状态发生变化时才发送报告 (避免重复报告)
 * 3. 从 M5Cardputer 键盘提取:
 *    - 修饰键 (Ctrl/Shift/Alt/GUI) 的位掩码
 *    - 当前按下的 HID 键码 (最多6个)
 *    - 空格键 (单独检测，因为可能不在 hid_keys 中)
 *    - ESC 键 (FN + `·~` 组合键, HID 0x29)
 * 4. 组装 KeyReport 并发送
 *
 * 键盘报告优化:
 *   - 使用 isChange() 检测状态变化，避免在无变化时重复发送相同报告
 *   - 当没有按键按下且没有修饰键时，发送 releaseAll() 而非空报告
 */
void usbKeyboard() {
    // 延迟初始化 — 只在第一次调用时执行
    static bool inited = false;
    if (!inited) { keyboard.begin(); inited = true; }

    // 仅在键盘状态发生变化时才处理 (避免重复发送相同的报告)
    if (!M5Cardputer.Keyboard.isChange()) return;

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    // 构建 USB HID 键盘报告
    KeyReport report = {0};
    report.modifiers = status.modifiers;  // 直接复制修饰键掩码

    // 填充按键码 (最多6个)
    uint8_t idx = 0;
    for (auto k : status.hid_keys) {
        if (idx < 6) report.keys[idx++] = k;
        else break;  // 超过6个按键，忽略多余的
    }

    // 空格键特殊处理
    if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
        const uint8_t HID_SPACE = 0x2C;
        // 检查空格是否已经在按键列表中 (避免重复)
        bool present = false;
        for (uint8_t i = 0; i < idx; ++i) if (report.keys[i] == HID_SPACE) { present = true; break; }
        if (!present && idx < 6) report.keys[idx++] = HID_SPACE;
    }

    // === FN 组合键映射 ===
    // 旧版 M5Cardputer 库不会自动转换 FN 层按键，hid_keys 中仍是原始键码。
    // 此处直接替换原始 HID 码为功能键码，避免同时输出原键和功能键。
    // FN + /(0x38)→右(0x4F) | FN + ,(0x36)→左(0x50) | FN + .(0x37)→下(0x51)
    // FN + ;(0x33)→上(0x52) | FN + Backspace(0x2A)→Delete(0x4C)
    // FN + `·~(0x35)→ESC(0x29)
    if (status.fn) {
        // 映射表: {原始HID码, 目标HID码}
        static const uint8_t fnMap[][2] = {
            {0x38, 0x4F}, // / → 右
            {0x36, 0x50}, // , → 左
            {0x37, 0x51}, // . → 下
            {0x33, 0x52}, // ; → 上
            {0x2A, 0x4C}, // Backspace → Delete
            {0x35, 0x29}, // ` → ESC
        };
        for (auto& m : fnMap) {
            for (uint8_t i = 0; i < idx; ++i) {
                if (report.keys[i] == m[0]) { report.keys[i] = m[1]; break; }
            }
        }
    }

    // 发送报告
    if (idx == 0 && report.modifiers == 0) {
        // 无任何输入 — 发送释放所有按键的报告
        keyboard.releaseAll();
    } else {
        // 有按键或修饰键 — 发送完整的键盘报告
        keyboard.sendReport(&report);
    }
}
