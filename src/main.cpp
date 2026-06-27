/**
 * @file main.cpp
 * @brief M5Cardputer 蓝牙/USB 键盘鼠标模拟器 - 主程序
 *
 * 本程序将 M5Cardputer 设备模拟为一个蓝牙 BLE HID 或 USB HID 输入设备，
 * 支持在键盘模式和鼠标模式之间切换。
 * 用户可以通过设备上的物理按键来控制光标的移动、鼠标点击以及键盘输入。
 *
 * 硬件平台: M5Stack Cardputer (ESP32-S3)
 * 开发框架: Arduino / PlatformIO
 */

#include <M5Cardputer.h>
#include "bluetooth.h"
#include "display.h"
#include "usbHid.h"
#include <USB.h>

// 当前工作模式标志
bool mouseMode = false;          // true=鼠标模式, false=键盘模式 (默认为键盘)
bool usbMode = true;             // true=USB HID模式, false=蓝牙BLE模式
bool lastBluetoothStatus = false; // 上一次蓝牙连接状态，用于检测状态变化

/**
 * @brief 模式选择界面
 *
 * 在设备启动时显示一个选择界面，允许用户在 USB 模式和蓝牙模式之间切换。
 * 按下 '.' 或 ';' 键切换模式，按下 Enter 键确认选择并退出循环。
 * 只有当模式发生变化时才刷新显示内容，避免不必要的屏幕重绘。
 */
void selectMode() {
    bool lastMode = !usbMode;
    while (true) {
        M5Cardputer.update();

        // 仅在模式切换时刷新屏幕显示
        if (lastMode != usbMode) {
            displaySelectionScreen(usbMode);
            lastMode = usbMode;
        }

        // 检测键盘输入
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

                // '.' 或 ';' 键切换 USB/蓝牙模式
                if(M5Cardputer.Keyboard.isKeyPressed('.') || M5Cardputer.Keyboard.isKeyPressed(';')) {
                    usbMode = !usbMode;
                }

                // Enter 键确认选择，退出选择界面
                if (status.enter) {
                    break;
                }
            }

        }
        delay(10);
    }
}

/**
 * @brief 系统初始化
 *
 * 执行顺序:
 * 1. 初始化 M5Cardputer 硬件
 * 2. 初始化显示屏
 * 3. 显示欢迎画面
 * 4. 进入模式选择界面
 * 5. 根据选择初始化 USB HID 或蓝牙 BLE HID
 * 6. 显示主界面
 */
void setup() {
    // Initialisation du M5Cardputer
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);

    setupDisplay();
    displayWelcomeScreen();

    // 用户选择通信模式
    selectMode();
    if (usbMode) {
        USB.begin();           // 启动 USB HID
    } else {
        initBluetooth();       // 启动蓝牙 BLE HID
    }

    displayMainScreen(usbMode, mouseMode, getBluetoothStatus());
}

/**
 * @brief 主循环
 *
 * 在每个循环迭代中执行以下操作:
 * 1. 更新设备状态
 * 2. 保存键盘变化标志 (必须在 HID 处理前，避免被消费)
 * 3. 检测蓝牙连接状态变化并更新状态栏
 * 4. 检测模式切换按键 (BtnA/GO键) 并在键盘/鼠标模式间切换
 * 5. 根据当前模式 (USB/BLE, 键盘/鼠标) 发送相应的 HID 数据
 * 6. 更新屏幕上的按键显示
 */
void loop() {
    M5Cardputer.update();

    // ★ 关键: 在 HID 处理前保存 isChange() 状态。
    // usbKeyboard() 内部会调用 isChange() 消费掉变化标志，
    // 导致后续 drawKeyDisplay() 永远不被调用，按键信息无法显示。
    bool keyChanged = M5Cardputer.Keyboard.isChange();

    // 检测蓝牙连接状态变化，更新屏幕状态栏
    auto bluetoothStatus = getBluetoothStatus();
    if (lastBluetoothStatus != bluetoothStatus) {
        drawStatusBar(usbMode, mouseMode, bluetoothStatus);
        lastBluetoothStatus = bluetoothStatus;
    }

    // BtnA (G0 键) 用于切换键盘/鼠标模式
    if (M5Cardputer.BtnA.isPressed()) {
        mouseMode = !mouseMode;
        drawStatusBar(usbMode, mouseMode, bluetoothStatus);
        clearKeyDisplayArea();
        drawKeyDisplay(mouseMode);      // 立即绘制新模式的底部区域
        delay(200);  // 防抖延迟
    }

    // 根据当前通信模式分发到对应的处理函数
    // 传入预先保存的 keyChanged，避免重复查询 isChange()
    if (usbMode) {
        handleUsbMode(mouseMode, keyChanged);
    } else {
        handleBluetoothMode(mouseMode);
    }

    // 始终更新按键显示
    // — 键盘模式: 仅 keyChanged 时才需要更新 (文本缓冲区不变)
    // — 鼠标模式: 始终更新 (按键提示需要常驻显示)
    if (keyChanged || mouseMode) {
        drawKeyDisplay(mouseMode);
    }

}
