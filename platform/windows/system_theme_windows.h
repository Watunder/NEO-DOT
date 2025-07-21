/*************************************************************************/
/*  system_theme_windows.h                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-present Godot Engine contributors (cf. AUTHORS.md).*/
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef SYSTEM_THEME_WINDOWS_H
#define SYSTEM_THEME_WINDOWS_H

#include "core/image.h"

#include <windows.h>

class SystemTheme {
	HWND hWnd;

	static SystemTheme *singleton;

public:
	enum ThemeType {
		DWMWINDOW_BUTTON_CLOSE = 11,
		DWMWINDOW_BUTTON_HELP = 15,
		DWMWINDOW_BUTTON_MAX = 19,
		DWMWINDOW_BUTTON_MIN = 23,
		DWMWINDOW_BUTTON_RESTORE = 27,
	};

	static SystemTheme *get_singleton();

	Ref<Image> get_window_theme_image(const ThemeType &p_type);

	SystemTheme(HWND p_hwnd);
	~SystemTheme();
};

#endif
