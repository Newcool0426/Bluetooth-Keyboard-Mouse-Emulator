/**
 * @file usbHid.h
 * @brief USB HID 设备头文件
 *
 * 定义了 USB HID 键盘和鼠标的接口函数。
 * USB HID 模式下直接使用 ESP32-S3 的原生 USB 接口，
 * 不需要额外的蓝牙配对过程，即插即用。
 */

#ifndef USBHID_H
#define USBHID_H

#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"
#include <M5Cardputer.h>

/** @brief 根据按键状态发送 USB 鼠标移动和点击事件 */
void usbMouse();

/** @brief 根据按键状态发送 USB 键盘按键事件 */
void usbKeyboard();

/**
 * @brief USB 模式的统一处理入口
 * @param mouseMode true=鼠标模式, false=键盘模式
 *
 * 根据 mouseMode 参数分发到 usbMouse() 或 usbKeyboard()，
 * 每次调用后延迟5ms以控制发送速率。
 */
void handleUsbMode(bool mouseMode);

#endif
