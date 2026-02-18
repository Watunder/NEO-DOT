/*************************************************************************/
/*  platform_stdinc.h                                                    */
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

#ifndef PLATFORM_STDINC_H
#define PLATFORM_STDINC_H

#include "configs/platform_defines.h"

#include <stddef.h>
#include <stdint.h>

/*************************************************************************/

#if (__cplusplus >= 201402L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
#define CPP14_OR_GREATER 1
#endif

#if !defined(CPP14_OR_GREATER)
#error this project requires C++14
#endif

/*************************************************************************/

#if defined(PLATFORM_WINDOWS)

#include <malloc.h>

/**
 * Windows badly defines a lot of stuff we'll never use. Undefine it.
 */

#undef min // override standard definition
#undef max // override standard definition
#undef ERROR // override (really stupid) wingdi.h standard definition
#undef DELETE // override (another really stupid) winnt.h standard definition
#undef MessageBox // override winuser.h standard definition
#undef MIN // override standard definition
#undef MAX // override standard definition
#undef CLAMP // override standard definition
#undef Error
#undef OK
#undef CONNECT_DEFERRED // override from Windows SDK, clashes with Object enum

/*************************************************************************/

#elif defined(PLATFORM_LINUX)

#include <alloca.h>

/*************************************************************************/

#elif defined(PLATFORM_ANDROID)

#include <alloca.h>
#include <malloc.h> // ndk

/*************************************************************************/

#elif defined(PLATFORM_APPLE)

#include <alloca.h>
#define PTHREAD_RENAME_SELF

/*************************************************************************/

#elif defined(PLATFORM_EMSCRIPTEN)

#include <alloca.h>

/*************************************************************************/

#elif defined(PLATFORM_BSD) || defined(PLATFORM_OPENBSD) || defined(PLATFORM_NETBSD) || defined(PLATFORM_DRAGONFLY)

#include <stdlib.h>
// FreeBSD and OpenBSD use pthread_set_name_np, while other platforms,
// include NetBSD, use pthread_setname_np. NetBSD's version however requires
// a different format, we handle this directly in thread_posix.
#if defined(PLATFORM_NETBSD)
#define PTHREAD_NETBSD_SET_NAME
#else
#define PTHREAD_BSD_SET_NAME
#endif

/*************************************************************************/

#endif

#endif // PLATFORM_STDINC_H
