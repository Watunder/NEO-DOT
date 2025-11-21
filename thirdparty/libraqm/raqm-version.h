/*
 * Copyright © 2015 Information Technology Authority (ITA) <foss@ita.gov.om>
 * Copyright © 2016-2023 Khaled Hosny <khaled@aliftype.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifndef _RAQM_H_IN_
#error "Include <raqm.h> instead."
#endif

#ifndef _RAQM_VERSION_H_
#define _RAQM_VERSION_H_

#define RAQM_VERSION_MAJOR 0
#define RAQM_VERSION_MINOR 10
#define RAQM_VERSION_MICRO 3

#define RAQM_VERSION_STRING "0.10.3"

#define RAQM_VERSION_ATLEAST(major,minor,micro) \
	((major)*10000+(minor)*100+(micro) <= \
	 RAQM_VERSION_MAJOR*10000+RAQM_VERSION_MINOR*100+RAQM_VERSION_MICRO)

#endif /* _RAQM_VERSION_H_ */
