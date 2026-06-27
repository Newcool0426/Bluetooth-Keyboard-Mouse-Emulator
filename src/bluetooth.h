/**
 * @file bluetooth.h
 * @brief 蓝牙 BLE HID 设备头文件
 *
 * 定义了蓝牙 BLE HID 键盘/鼠标的报告描述符、全局变量和接口函数。
 * 基于 ESP32 BLE 库实现，遵循 HID 1.1 规范。
 *
 * HID 报告描述符定义了设备向主机发送数据的格式:
 *   - 报告 ID 1: 鼠标报告 (4字节: 按键 + X位移 + Y位移 + 滚轮)
 *   - 报告 ID 2: 键盘报告 (8字节: 修饰键 + 保留 + 6个按键码)
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <M5Cardputer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

// 全局 BLE HID 设备及特征值指针
extern BLEHIDDevice* hid;              // BLE HID 设备对象
extern BLECharacteristic* mouseInput;   // 鼠标输入报告特征值
extern BLECharacteristic* keyboardInput;// 键盘输入报告特征值
extern bool isConnected;                // BLE 连接状态标志

/**
 * @brief HID 报告描述符 (HID Report Map)
 *
 * 此描述符定义了鼠标和键盘两个 HID 设备的报告格式。
 * 描述符使用 HID 特定的二进制格式，每个条目由若干字节组成。
 *
 * === 鼠标报告 (Report ID 1) ===
 * 格式: [按键(1B)] [X位移(1B)] [Y位移(1B)] [滚轮(1B)] — 共4字节
 *
 * === 键盘报告 (Report ID 2) ===
 * 格式: [修饰键(1B)] [保留(1B)] [按键1-6(6B)] — 共8字节
 *   - 修饰键: bit0=Ctrl, bit1=Shift, bit2=Alt, bit3=GUI/Win
 *   - 按键码: 每个字节存储一个 HID 键码，最多可同时按下6个键
 */
const uint8_t HID_REPORT_MAP[] = {
    // ==================== 鼠标报告 (Report ID 1) ====================
    0x05, 0x01,        // Usage Page (Generic Desktop)    — 通用桌面设备页面
    0x09, 0x02,        // Usage (Mouse)                   — 用途: 鼠标
    0xA1, 0x01,        // Collection (Application)        — 应用程序集合开始
    0x09, 0x01,        //   Usage (Pointer)               —   用途: 指针
    0xA1, 0x00,        //   Collection (Physical)         —   物理集合开始
    0x85, 0x01,        //     Report ID (1)               —   报告 ID 为 1
    // -- 按键 (3个按钮) --
    0x05, 0x09,        //     Usage Page (Button)          —   按钮页面
    0x19, 0x01,        //     Usage Minimum (0x01)         —   最小按键号: 1 (左键)
    0x29, 0x03,        //     Usage Maximum (0x03)         —   最大按键号: 3 (中键)
    0x15, 0x00,        //     Logical Minimum (0)          —   逻辑最小值: 0 (释放)
    0x25, 0x01,        //     Logical Maximum (1)          —   逻辑最大值: 1 (按下)
    0x95, 0x03,        //     Report Count (3)             —   报告数量: 3个字段
    0x75, 0x01,        //     Report Size (1)              —   每个字段1位
    0x81, 0x02,        //     Input (Data,Var,Abs)         —   输入: 数据/变量/绝对
    // -- 填充位 (凑齐一个字节) --
    0x95, 0x01,        //     Report Count (1)             —   1个字段
    0x75, 0x05,        //     Report Size (5)              —   每个字段5位
    0x81, 0x01,        //     Input (Cnst,Var,Abs)         —   输入: 常量/填充
    // -- X/Y 位移 (有符号相对坐标) --
    0x05, 0x01,        //     Usage Page (Generic Desktop) —   通用桌面页面
    0x09, 0x30,        //     Usage (X)                    —   用途: X轴
    0x09, 0x31,        //     Usage (Y)                    —   用途: Y轴
    0x15, 0x81,        //     Logical Minimum (-127)       —   逻辑最小值: -127
    0x25, 0x7F,        //     Logical Maximum (127)        —   逻辑最大值: 127
    0x75, 0x08,        //     Report Size (8)              —   每个字段8位 (1字节)
    0x95, 0x02,        //     Report Count (2)             —   2个字段 (X+Y)
    0x81, 0x06,        //     Input (Data,Var,Rel)         —   输入: 数据/变量/相对
    0xC0,              //   End Collection                 — 物理集合结束
    0xC0,              // End Collection                   — 应用程序集合结束

    // ==================== 键盘报告 (Report ID 2) ====================
    0x05, 0x01,        // Usage Page (Generic Desktop)    — 通用桌面设备页面
    0x09, 0x06,        // Usage (Keyboard)                — 用途: 键盘
    0xA1, 0x01,        // Collection (Application)        — 应用程序集合开始
    0x85, 0x02,        //   Report ID (2)                 —   报告 ID 为 2
    // -- 修饰键 (8个修饰键，每个1位) --
    0x05, 0x07,        //   Usage Page (Key Codes)         —   键码页面
    0x19, 0xE0,        //   Usage Minimum (224)            —   最小键码: 左Ctrl
    0x29, 0xE7,        //   Usage Maximum (231)            —   最大键码: 右GUI
    0x15, 0x00,        //   Logical Minimum (0)            —   逻辑最小值: 0
    0x25, 0x01,        //   Logical Maximum (1)            —   逻辑最大值: 1
    0x75, 0x01,        //   Report Size (1)                —   每个字段1位
    0x95, 0x08,        //   Report Count (8)               —   8个字段 (8个修饰键)
    0x81, 0x02,        //   Input (Data,Var,Abs)           —   输入: 数据/变量/绝对
    // -- 保留字节 (pad to 1 byte) --
    0x95, 0x01,        //   Report Count (1)               —   1个字段
    0x75, 0x08,        //   Report Size (8)                —   每个字段8位
    0x81, 0x01,        //   Input (Cnst,Var,Abs)           —   输入: 常量 (保留)
    // -- LED 输出报告 (键盘上的 Num/Caps/Scroll Lock 指示灯) --
    0x95, 0x05,        //   Report Count (5)               —   5个字段
    0x75, 0x01,        //   Report Size (1)                —   每个字段1位
    0x05, 0x08,        //   Usage Page (LEDs)              —   LED页面
    0x19, 0x01,        //   Usage Minimum (1)              —   最小LED号: 1
    0x29, 0x05,        //   Usage Maximum (5)              —   最大LED号: 5
    0x91, 0x02,        //   Output (Data,Var,Abs)          —   输出: 数据/变量/绝对
    // -- LED 填充位 --
    0x95, 0x01,        //   Report Count (1)               —   1个字段
    0x75, 0x03,        //   Report Size (3)                —   每个字段3位
    0x91, 0x01,        //   Output (Cnst,Var,Abs)          —   输出: 常量 (填充)
    // -- 按键码数组 (最多同时按下6个键) --
    0x95, 0x06,        //   Report Count (6)               —   6个字段
    0x75, 0x08,        //   Report Size (8)                —   每个字段8位 (1字节)
    0x15, 0x00,        //   Logical Minimum (0)            —   最小键码: 0
    0x25, 0x65,        //   Logical Maximum (101)          —   最大键码: 101
    0x05, 0x07,        //   Usage Page (Key Codes)         —   键码页面
    0x19, 0x00,        //   Usage Minimum (0)             —   最小键码: 0
    0x29, 0x65,        //   Usage Maximum (101)           —   最大键码: 101
    0x81, 0x00,        //   Input (Data,Array)             —   输入: 数据/数组
    0xC0               // End Collection                   — 应用程序集合结束
};

// === 函数声明 ===

/** @brief 初始化蓝牙 BLE HID 设备，创建 HID 服务并开始广播 */
void initBluetooth();

/** @brief 反初始化蓝牙，释放 BLE 资源 */
void deinitBluetooth();

/** @brief 获取当前蓝牙连接状态 */
bool getBluetoothStatus();

/** @brief 根据按键状态发送蓝牙鼠标报告 */
void bluetoothMouse();

/** @brief 根据按键状态发送蓝牙键盘报告 */
void bluetoothKeyboard();

/** @brief 发送空的鼠标和键盘报告，表示没有输入 (防止主机认为设备断开) */
void sendEmptyReports();

/**
 * @brief 蓝牙模式的统一处理入口
 * @param mouseMode true=鼠标模式, false=键盘模式
 */
void handleBluetoothMode(bool mouseMode);

/**
 * @class MyBLEServerCallbacks
 * @brief BLE 服务器连接/断开回调
 *
 * 处理蓝牙设备的连接和断开事件：
 * - 连接时设置连接状态标志
 * - 断开时清理连接并重新开始广播，使设备可被重新发现
 */
class MyBLEServerCallbacks : public BLEServerCallbacks {
public:
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override;
};

#endif // BLUETOOTH_H
