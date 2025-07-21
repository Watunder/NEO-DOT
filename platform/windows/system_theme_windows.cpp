/*************************************************************************/
/*  system_theme_windows.cpp                                             */
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

#include "system_theme_windows.h"

#include <shlwapi.h>
#include <uxtheme.h>
#include <vssym32.h>

SystemTheme *SystemTheme::singleton = NULL;

SystemTheme *SystemTheme::get_singleton() {
	return singleton;
}

Ref<Image> SystemTheme::get_window_theme_image(const ThemeType &p_type) {
	HTHEME theme = OpenThemeData(hWnd, L"DWMWINDOW");

	wchar_t msstyle_path[1024];
	SHRegGetPathW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ThemeManager", L"DllName", (LPWSTR)&msstyle_path, 0);
	HMODULE msstyle = LoadLibraryExW((LPWSTR)msstyle_path, 0, LOAD_LIBRARY_AS_DATAFILE);

	VOID *buffer = NULL;
	DWORD buffer_size = 0;
	GetThemeStream(theme, 0, 0, TMT_DISKSTREAM, &buffer, &buffer_size, msstyle);

	RECT rect;
	GetThemeRect(theme, p_type, 0, TMT_ATLASRECT, &rect);

	PoolByteArray image_data;
	image_data.resize(buffer_size);
	copymem(image_data.write().ptr(), buffer, buffer_size);

	Ref<Image> image;
	image.instance();
	image->load_png_from_buffer(image_data);

	Ref<Image> ret = image->get_rect(Rect2(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top));

	FreeLibrary(msstyle);
	CloseThemeData(theme);

	return ret;
}

SystemTheme::SystemTheme(HWND p_hwnd) {
	hWnd = p_hwnd;
	singleton = this;
}

SystemTheme::~SystemTheme() {
	singleton = NULL;
}
