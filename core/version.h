/*************************************************************************/
/*  version.h                                                            */
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

#ifndef GODOT_VERSION_H
#define GODOT_VERSION_H

#include "core/version_generated.gen.h"

// Godot versions are of the form <major>.<minor> for the initial release,
// and then <major>.<minor>.<patch> for subsequent bugfix releases where <patch> != 0
// That's arbitrary, but we find it pretty and it's the current policy.

// Defines the main "branch" version. Patch versions in this branch should be
// forward-compatible.
// Example: "3.1"
#define VERSION_BRANCH "" _MKSTR(VERSION_MAJOR) "." _MKSTR(VERSION_MINOR)
#if VERSION_PATCH
// Example: "3.1.4"
#define VERSION_NUMBER "" VERSION_BRANCH "." _MKSTR(VERSION_PATCH)
#else // patch is 0, we don't include it in the "pretty" version number.
// Example: "3.1" instead of "3.1.0"
#define VERSION_NUMBER "" VERSION_BRANCH
#endif // VERSION_PATCH

// Version number encoded as hexadecimal int with one byte for each number,
// for easy comparison from code.
// Example: 3.1.4 will be 0x030104, making comparison easy from script.
#define VERSION_HEX 0x10000 * VERSION_MAJOR + 0x100 * VERSION_MINOR + VERSION_PATCH

// Describes the full configuration of that Godot version, including the version number,
// the status (beta, stable, etc.) and potential module-specific features (e.g. neo).
// Example: "3.1.4.stable.neo"
#define VERSION_FULL_CONFIG "" VERSION_NUMBER "." VERSION_STATUS VERSION_MODULE_CONFIG

// Similar to VERSION_FULL_CONFIG, but also includes the (potentially custom) VERSION_BUILD
// description (e.g. official, custom_build, etc.).
// Example: "3.1.4.stable.neo.official"
#define VERSION_FULL_BUILD "" VERSION_FULL_CONFIG "." VERSION_BUILD

// Same as above, but prepended with Godot's name and a cosmetic "v" for "version".
// Example: "Godot v3.1.4.stable.official.neo"
#define VERSION_FULL_NAME "" VERSION_NAME " v" VERSION_FULL_BUILD

#endif // GODOT_VERSION_H
