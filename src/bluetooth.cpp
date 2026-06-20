/**
 * @file bluetooth.cpp
 * @brief 蓝牙 BLE HID 键盘/鼠标实现
 *
 * 实现通过 BLE (Bluetooth Low Energy) HID 协议模拟键盘和鼠标的功能。
 * 主要流程:
 * 1. initBluetooth()   - 初始化 BLE 设备、HID 服务、广播
 * 2. handleBluetoothMode() - 在主循环中被调用，根据连接状态和按键发送 HID 报告
 * 3. bluetoothMouse()  - 将 M5Cardputer 按键映射为鼠标的移动和点击
 * 4. bluetoothKeyboard() - 将 M5Cardputer 按键映射为键盘的 HID 键码
 *
 * 按键映射 (鼠标模式):
 *   方向键 (/, ; , . , ,)  → 鼠标移动
 *   Enter                   → 鼠标左键
 *   \ (反斜杠)              → 鼠标右键
 *
 * 按键映射 (键盘模式):
 *   所有 M5Cardputer 按键  → 标准 HID 键盘按键
 *   Ctrl/Shift/Alt          → 对应修饰键
 *   空格                    → HID 空格键 (0x2C)
 */

#include "bluetooth.h"

// === 全局变量 ===
BLEHIDDevice* hid;                   // BLE HID 设备对象指针
BLECharacteristic* mouseInput;       // 鼠标输入报告特征值 (Report ID 1)
BLECharacteristic* keyboardInput;    // 键盘输入报告特征值 (Report ID 2)
bool bluetoothIsConnected = false;   // 当前蓝牙连接状态

// ============================================================
// BLE 服务器回调实现
// ============================================================

/**
 * @brief 蓝牙设备连接成功回调
 * @param pServer BLE 服务器指针
 *
 * 当有中心设备 (PC/手机/平板) 成功连接到本设备时触发，
 * 设置连接标志为 true。
 */
void MyBLEServerCallbacks::onConnect(BLEServer* pServer) {
    bluetoothIsConnected = true;
}

/**
 * @brief 蓝牙设备断开连接回调
 * @param pServer BLE 服务器指针
 * @param param   GATT 断开连接参数
 *
 * 当中心设备断开连接时触发:
 * 1. 设置连接标志为 false
 * 2. 正确断开客户端连接 (释放资源)
 * 3. 重新启动 BLE 广播，使设备可被重新发现和连接
 */
void MyBLEServerCallbacks::onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
    bluetoothIsConnected = false;

    pServer->disconnect(param->disconnect.conn_id); // 正确断开客户端连接
    pServer->startAdvertising();  // 重新开始广播，使设备可被发现
}

// ============================================================
// 公共接口
// ============================================================

/** @brief 获取蓝牙连接状态 */
bool getBluetoothStatus() {
    return  bluetoothIsConnected;
}

// ============================================================
// 蓝牙鼠标报告
// ============================================================

/**
 * @brief 发送蓝牙鼠标 HID 报告
 *
 * 将 M5Cardputer 按键映射为鼠标操作:
 * | 按键  | 功能       |
 * |-------|-----------|
 * | Enter | 左键单击   |
 * | \     | 右键单击   |
 * | ;     | 向上移动   |
 * | .     | 向下移动   |
 * | /     | 向右移动   |
 * | ,     | 向左移动   |
 *
 * 鼠标报告格式 (4字节):
 *   [0] = 按键状态 (bit0=左键, bit1=右键, bit2=中键)
 *   [1] = X 轴位移 (-127 ~ 127, 正=右)
 *   [2] = Y 轴位移 (-127 ~ 127, 正=下)
 *   [3] = 滚轮位移
 */
void bluetoothMouse() {
    int16_t x = 0;         // X 轴位移 (正=右, 负=左)
    int16_t y = 0;         // Y 轴位移 (正=下, 负=上)
    uint8_t buttons = 0;   // 按键状态 (bit0=左键, bit1=右键)

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    // 左键 — Enter 键
    if (status.enter) {
        buttons |= 0x01;
    }
    // 右键 — 反斜杠键 '\'
    if (M5Cardputer.Keyboard.isKeyPressed('\\')) {
        buttons |= 0x02;
    }

    // 垂直方向移动
    if (M5Cardputer.Keyboard.isKeyPressed(';')) {
        y -= 1;     // 上移
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
        y += 1;     // 下移
    }

    // 水平方向移动
    if (M5Cardputer.Keyboard.isKeyPressed('/')) {
        x += 1;     // 右移
    }
    else if (M5Cardputer.Keyboard.isKeyPressed(',')) {
        x -= 1;     // 左移
    }

    // 组装并发送鼠标报告
    uint8_t report[4] = {buttons, (uint8_t)x, (uint8_t)y, 0};
    mouseInput->setValue(report, sizeof(report));
    mouseInput->notify();  // 通过 BLE 通知发送给主机
}

// ============================================================
// 蓝牙键盘报告
// ============================================================

/**
 * @brief 发送蓝牙键盘 HID 报告
 *
 * 键盘报告格式 (8字节):
 *   [0] = 修饰键 (bit0=Ctrl, bit1=Shift, bit2=Alt, bit3=GUI/Win)
 *   [1] = 保留字节 (始终为0)
 *   [2..7] = 最多6个同时按下的 HID 键码
 *
 * 从 M5Cardputer 键盘状态中提取:
 * - 已按下的 HID 键码 (最多6个)
 * - 空格键单独处理 (M5Cardputer 键盘可能不将其报告在 hid_keys 中)
 * - ESC 键 (FN + `·~` 组合键, HID 0x29)
 * - Ctrl/Shift/Alt 修饰键
 *
 * 发送后延迟50ms，避免发送过快导致主机丢失报告。
 */
void bluetoothKeyboard() {
    uint8_t modifier = 0;
    uint8_t keycode[6] = {0};  // 最多同时报告6个按键

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    // 收集已按下的 HID 键码 (最多6个)
    int count = 0;
    for (auto key : status.hid_keys) {
        if (count < 6) {
            keycode[count] = key;
            count++;
        }
    }

    // 空格键特殊处理 — M5Cardputer 键盘可能不将空格包含在 hid_keys 中
    if (M5Cardputer.Keyboard.isKeyPressed(' ') && count < 6) {
        keycode[count++] = 0x2C;  // HID 空格键码
    }

    // ESC 键 — 通过 FN + `·~` 组合键触发
    // M5Cardputer 键盘上 `·~` 键的 FN 层标注为 ESC
    // 当 FN 按下时键盘固件会在 hid_keys 中加入 0x29，此处作为补充确保一定生效
    if (status.esc && count < 6) {
        // 检查 ESC 是否已在 hid_keys 中 (避免重复)
        bool escInKeys = false;
        for (int i = 0; i < count; i++) {
            if (keycode[i] == 0x29) { escInKeys = true; break; }
        }
        if (!escInKeys) {
            keycode[count++] = 0x29;  // HID ESC 键码
        }
    }

    // 设置修饰键位掩码
    if (status.ctrl)  modifier |= 0x01;   // 左/右 Ctrl
    if (status.shift) modifier |= 0x02;   // 左/右 Shift
    if (status.alt)   modifier |= 0x04;   // 左/右 Alt

    // 组装并发送键盘报告
    uint8_t report[8] = {
        modifier, 0,                      // 修饰键 + 保留字节
        keycode[0], keycode[1], keycode[2], // 按键1-3
        keycode[3], keycode[4], keycode[5]  // 按键4-6
    };
    keyboardInput->setValue(report, sizeof(report));
    keyboardInput->notify();  // 通过 BLE 通知发送给主机

    delay(50);  // 防止发送频率过高
}

// ============================================================
// 空报告 — 空闲状态
// ============================================================

/**
 * @brief 发送空报告 (无输入状态)
 *
 * 当没有按键被按下时，发送全零报告。
 * 这对于鼠标特别重要 — 如果不发送空报告，主机可能认为
 * 上一次的移动/点击仍在持续，导致光标漂移或按钮卡住。
 */
void sendEmptyReports() {
    // 空鼠标报告 — 无移动、无点击
    uint8_t emptyMouseReport[4] = {0, 0, 0, 0};
    mouseInput->setValue(emptyMouseReport, sizeof(emptyMouseReport));
    mouseInput->notify();

    // 空键盘报告 — 无修饰键、无按键
    uint8_t emptyKeyboardReport[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    keyboardInput->setValue(emptyKeyboardReport, sizeof(emptyKeyboardReport));
    keyboardInput->notify();
}

// ============================================================
// 蓝牙模式主处理入口
// ============================================================

/**
 * @brief 蓝牙模式的主处理循环
 * @param mouseMode true=鼠标模式, false=键盘模式
 *
 * 仅在蓝牙已连接时发送报告:
 *   - 有按键按下 → 发送鼠标或键盘报告
 *   - 无按键按下 → 发送空报告 (防止残留移动/点击)
 *
 * 每次循环后延迟7ms，提供约140Hz的报告速率。
 */
void handleBluetoothMode(bool mouseMode) {
    if (bluetoothIsConnected) {
        if (M5Cardputer.Keyboard.isPressed()) {
            if (mouseMode) {
                bluetoothMouse();
            } else {
                bluetoothKeyboard();
            }
        } else {
            sendEmptyReports();  // 无输入时发送空报告
        }
    }
    delay(7);  // 控制发送频率
}

// ============================================================
// 蓝牙初始化 / 反初始化
// ============================================================

/**
 * @brief 初始化蓝牙 BLE HID 设备
 *
 * 完整的初始化流程:
 * 1. 初始化 BLE 设备，设置设备名称为 "M5-Keyboard-Mouse"
 * 2. 创建 BLE 服务器，注册连接/断开回调
 * 3. 创建 HID 设备并配置输入报告特征值
 *    - inputReport(1): 鼠标报告 (Report ID 1)
 *    - inputReport(2): 键盘报告 (Report ID 2)
 * 4. 设置制造商信息、PNP ID、HID 信息
 * 5. 注册 HID 报告描述符
 * 6. 启动 HID 服务
 * 7. 配置 BLE 广播 (外观=鼠标, 服务UUID)
 * 8. 配置 BLE 安全参数 (绑定模式, 无IO能力, 加密密钥)
 */
void initBluetooth() {
    BLEDevice::init("M5-Keyboard-Mouse");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyBLEServerCallbacks());

    // 创建 HID 设备及其输入报告特征值
    hid = new BLEHIDDevice(pServer);
    mouseInput = hid->inputReport(1);    // 鼠标报告 (Report ID 1)
    keyboardInput = hid->inputReport(2);   // 键盘报告 (Report ID 2)

    // 配置 HID 设备信息
    hid->manufacturer()->setValue("M5Stack");
    hid->pnp(0x02, 0x1234, 0x5678, 0x0100);  // PNP ID: 厂商/产品/版本
    hid->hidInfo(0x00, 0x01);                  // HID 版本信息
    hid->reportMap((uint8_t*)HID_REPORT_MAP, sizeof(HID_REPORT_MAP));
    hid->startServices();

    // 配置 BLE 广播参数
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(HID_MOUSE);  // 外观类型: 鼠标
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();

    // 配置 BLE 安全参数
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);     // 绑定模式
    pSecurity->setCapability(ESP_IO_CAP_NONE);              // 无输入/输出能力
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

/**
 * @brief 反初始化蓝牙，释放 BLE 堆栈资源
 *
 * 调用 BLEDevice::deinit() 释放所有 BLE 资源。
 * 延迟1秒确保清理操作完成。
 */
void deinitBluetooth() {
    BLEDevice::deinit();
    delay(1000);
}
