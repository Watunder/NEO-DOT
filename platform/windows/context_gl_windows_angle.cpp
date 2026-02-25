/*************************************************************************/
/*  context_gl_windows_angle.cpp                                         */
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

#if defined(OPENGL_ENABLED) && defined(ANGLE_ENABLED)

#include "context_gl_windows_angle.h"

#include <dwmapi.h>

void ContextGL_Windows::release_current() {
	eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void ContextGL_Windows::make_current() {
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
}

EGLDisplay ContextGL_Windows::get_egl_display() const {
	return egl_display;
}

EGLContext ContextGL_Windows::get_egl_context() const {
	return egl_context;
}

int ContextGL_Windows::get_window_width() {
	return OS::get_singleton()->get_video_mode().width;
}

int ContextGL_Windows::get_window_height() {
	return OS::get_singleton()->get_video_mode().height;
}

bool ContextGL_Windows::should_vsync_via_compositor() {
	if (OS::get_singleton()->is_window_fullscreen() || !OS::get_singleton()->is_vsync_via_compositor_enabled()) {
		return false;
	}

	BOOL dwm_enabled;
	if (SUCCEEDED(DwmIsCompositionEnabled(&dwm_enabled))) {
		return dwm_enabled;
	}
	return false;
}

void ContextGL_Windows::swap_buffers() {
	eglSwapBuffers(egl_display, egl_surface);

	if (use_vsync) {
		bool vsync_via_compositor_now = should_vsync_via_compositor();

		if (vsync_via_compositor_now) {
			DwmFlush();
		}

		if (vsync_via_compositor_now != vsync_via_compositor) {
			set_use_vsync(true);
		}
	}
}

void ContextGL_Windows::set_use_vsync(bool p_use) {
	vsync_via_compositor = p_use && should_vsync_via_compositor();

	int swap_interval = (p_use && !vsync_via_compositor) ? 1 : 0;
	eglSwapInterval(egl_display, swap_interval);

	use_vsync = p_use;
}

bool ContextGL_Windows::is_using_vsync() const {
	return use_vsync;
}

Error ContextGL_Windows::initialize() {
	const EGLint display_attribs[] = {
		EGL_PLATFORM_ANGLE_TYPE_ANGLE,
		EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE,
		EGL_NONE
	};

	egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, display_attribs);
	if (egl_display == EGL_NO_DISPLAY) {
		ERR_PRINT("[ANGLE] Failed to get EGL display");
		return ERR_CANT_CREATE;
	}

	EGLint major, minor;
	if (!eglInitialize(egl_display, &major, &minor)) {
		ERR_PRINT("[ANGLE] Failed to initialize EGL");
		return ERR_CANT_CREATE;
	}

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		ERR_PRINT("[ANGLE] Failed to bind OpenGL ES API");
		eglTerminate(egl_display);
		return ERR_CANT_CREATE;
	}

	const EGLint config_attribs[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, OS::get_singleton()->is_layered_allowed() ? 8 : 0,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 0,
		EGL_RENDERABLE_TYPE, opengl_3_context ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint config_count = 0;
	if (!eglChooseConfig(egl_display, config_attribs, &egl_config, 1, &config_count) || config_count == 0) {
		ERR_PRINT("[ANGLE] Failed to choose EGL config");
		eglTerminate(egl_display);
		return ERR_CANT_CREATE;
	}

	egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)hWnd, NULL);
	if (egl_surface == EGL_NO_SURFACE) {
		ERR_PRINT("[ANGLE] Failed to create EGL surface");
		eglTerminate(egl_display);
		return ERR_CANT_CREATE;
	}

	EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, opengl_3_context ? 3 : 2,
		EGL_NONE
	};

	egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);
	if (egl_context == EGL_NO_CONTEXT) {
		ERR_PRINT("[ANGLE] Failed to create EGL context");
		eglDestroySurface(egl_display, egl_surface);
		eglTerminate(egl_display);
		return ERR_CANT_CREATE;
	}

	if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
		eglDestroyContext(egl_display, egl_context);
		eglDestroySurface(egl_display, egl_surface);
		eglTerminate(egl_display);
		return ERR_CANT_CREATE;
	}

	return OK;
}

ContextGL_Windows::ContextGL_Windows(HWND hwnd, bool p_opengl_3_context) {
	hWnd = hwnd;
	opengl_3_context = p_opengl_3_context;
	egl_display = EGL_NO_DISPLAY;
	egl_surface = EGL_NO_SURFACE;
	egl_context = EGL_NO_CONTEXT;
	egl_config = NULL;
	use_vsync = false;
	vsync_via_compositor = false;
}

ContextGL_Windows::~ContextGL_Windows() {
	if (egl_display != EGL_NO_DISPLAY) {
		release_current();

		if (egl_context != EGL_NO_CONTEXT) {
			eglDestroyContext(egl_display, egl_context);
			egl_context = EGL_NO_CONTEXT;
		}

		if (egl_surface != EGL_NO_SURFACE) {
			eglDestroySurface(egl_display, egl_surface);
			egl_surface = EGL_NO_SURFACE;
		}

		eglTerminate(egl_display);
		egl_display = EGL_NO_DISPLAY;
	}
}

#endif
