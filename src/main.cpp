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
bool mouseMode = true;           // true=鼠标模式, false=键盘模式
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
 * 2. 检测蓝牙连接状态变化并更新状态指示器
 * 3. 检测模式切换按键 (BtnA/GO键) 并在键盘/鼠标模式间切换
 * 4. 根据当前模式 (USB/BLE, 键盘/鼠标) 发送相应的 HID 数据
 */
void loop() {
    M5Cardputer.update();

    // 检测蓝牙连接状态变化，更新屏幕状态指示器
    auto bluetoothStatus = getBluetoothStatus();
    if (lastBluetoothStatus != bluetoothStatus) {
        modeIndicator(usbMode, bluetoothStatus);
        lastBluetoothStatus = bluetoothStatus;
    }

    // BtnA (GO 键) 用于切换键盘/鼠标模式
    if (M5Cardputer.BtnA.isPressed()) {
        mouseMode = !mouseMode;
        drawDeviceRect(mouseMode);
        delay(200);  // 防抖延迟
    }

    // 根据当前通信模式分发到对应的处理函数
    if (usbMode) {
        handleUsbMode(mouseMode);
    } else {
        handleBluetoothMode(mouseMode);
    }

}
