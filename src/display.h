/**
 * @file display.h
 * @brief 显示屏 UI 绘制函数声明
 *
 * 提供 M5Cardputer 屏幕上的各种 UI 元素的绘制函数，包括：
 * - 欢迎画面
 * - 模式选择界面 (USB/蓝牙)
 * - 主界面 (状态栏、按键显示)
 * - 顶部状态栏 (含USB/BT图标, 键盘/鼠标模式)
 * - 实时按键显示 (大字体居中)
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
 * @param usbMode         true=USB模式, false=蓝牙模式
 * @param mouseMode       true=鼠标模式, false=键盘模式
 * @param bluetoothStatus true=蓝牙已连接, false=蓝牙未连接
 */
void displayMainScreen(bool usbMode, bool mouseMode, bool bluetoothStatus);

/**
 * @brief 绘制顶部状态栏，包含连接图标和当前模式
 * @param usbMode         true=USB模式, false=蓝牙模式
 * @param mouseMode       true=鼠标模式(蓝色), false=键盘模式(绿色)
 * @param bluetoothStatus true=蓝牙已连接(绿图标), false=未连接(红图标)
 */
void drawStatusBar(bool usbMode, bool mouseMode, bool bluetoothStatus);

/**
 * @brief 在屏幕中央区域显示当前按下的按键
 * @param mouseMode true=鼠标模式, false=键盘模式
 *
 * 从 M5Cardputer 键盘状态中提取按键信息，在中央区域以大号字体显示。
 * 无按键按下时清空显示区域。
 */
void drawKeyDisplay(bool mouseMode);

/**
 * @brief 清空按键显示区域 (黑色填充)
 */
void clearKeyDisplayArea();

#endif
