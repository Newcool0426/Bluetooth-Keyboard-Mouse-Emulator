/**
 * @file display.h
 * @brief 显示屏 UI 绘制函数声明
 *
 * 提供 M5Cardputer 屏幕上的各种 UI 元素的绘制函数，包括：
 * - 欢迎画面
 * - 模式选择界面 (USB/蓝牙)
 * - 主界面 (标题、状态指示、设备图标)
 * - 键盘/鼠标图标
 * - 连接状态指示器
 */

#ifndef DISPLAY_H
#define DSPLAY_H

#include <M5Cardputer.h>

/** @brief 初始化显示屏：设置旋转方向、清屏、设置默认文字颜色 */
void setupDisplay();

/** @brief 显示欢迎画面：显示设备名称和版本信息，持续约2秒 */
void displayWelcomeScreen();

/**
 * @brief 显示模式选择界面
 * @param mouseMode true=USB模式高亮, false=蓝牙模式高亮
 */
void displaySelectionScreen(bool mouseMode);

/**
 * @brief 显示主界面
 * @param usbMode       true=USB模式, false=蓝牙模式
 * @param mouseMode     true=鼠标模式, false=键盘模式
 * @param bluetoothStatus true=蓝牙已连接, false=蓝牙未连接
 */
void displayMainScreen(bool usbMode, bool mouseMode, bool bluetoothStatus);

/**
 * @brief 绘制/更新连接模式指示器
 * @param usbMode         true=USB模式 (显示绿色USB), false=蓝牙模式
 * @param bluetoothStatus true=蓝牙已连接 (绿色), false=未连接 (红色)
 */
void modeIndicator(bool usbMode, bool bluetoothStatus);

/**
 * @brief 绘制设备类型矩形框
 * @param reverse true=键盘绿框+鼠标白框, false=键盘白框+鼠标绿框
 *                用于视觉上指示当前选中的设备类型
 */
void drawDeviceRect(bool reverse);

#endif
