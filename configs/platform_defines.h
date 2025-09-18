/*************************************************************************/
/*  platform_defines.h                                                   */
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

#ifndef PLATFORM_DEFINES_H
#define PLATFORM_DEFINES_H

/*************************************************************************/

#if defined(_WIN32)

#define PLATFORM_WINDOWS 1

#if defined(_MSC_VER) && defined(__has_include)
#if __has_include(<winapifamily.h>)
#define HAVE_WINAPIFAMILY_H 1
#endif
#elif defined(_MSC_VER) && (_MSC_VER >= 1700 && !_USING_V110_SDK71_)
#define HAVE_WINAPIFAMILY_H 1
#endif

#if defined(HAVE_WINAPIFAMILY_H)
#include <winapifamily.h>
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define TARGET_UWP 1
#endif
#endif

#if !defined(TARGET_UWP)
#define TARGET_UWP 0
#endif

/*************************************************************************/

#elif defined(__ANDROID__)

#define PLATFORM_ANDROID 1

/*************************************************************************/

#elif defined(__linux__)

#define PLATFORM_LINUX 1

/*************************************************************************/

#elif defined(__APPLE__)

#define PLATFORM_APPLE 1

#include <AvailabilityMacros.h>
#ifndef __has_extension
#define __has_extension(x) 0
#include <TargetConditionals.h>
#undef __has_extension
#else
#include <TargetConditionals.h>
#endif

#if defined(TARGET_OS_OSX) && TARGET_OS_OSX
#define TARGET_OSX 1
#elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define TARGET_IOS 1
#endif

#if !defined(TARGET_OSX)
#define TARGET_OSX 0
#endif

#if !defined(TARGET_IOS)
#define TARGET_IOS 0
#endif

/*************************************************************************/

#elif defined(__EMSCRIPTEN__)

#define PLATFORM_EMSCRIPTEN 1

/*************************************************************************/

#elif defined(__FreeBSD__)

#define PLATFORM_FREEBSD 1

#elif defined(__OpenBSD__)

#define PLATFORM_OPENBSD 1

#elif defined(__NetBSD__)

#define PLATFORM_NETBSD 1

#elif defined(__DragonFly__)

#define PLATFORM_DRAGONFLY 1

/*************************************************************************/

#else

#error this platform is not supported

#endif

/*************************************************************************/

#if !defined(PLATFORM_WINDOWS)

#define UNIX_ENABLED 1

#endif

/*************************************************************************/

#endif // PLATFORM_DEFINES_H
