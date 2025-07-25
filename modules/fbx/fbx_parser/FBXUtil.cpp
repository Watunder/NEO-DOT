/*************************************************************************/
/*  FBXUtil.cpp                                                          */
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

/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file  FBXUtil.cpp
 *  @brief Implementation of internal FBX utility functions
 */

#include "FBXUtil.h"
#include "FBXTokenizer.h"
#include <cstring>
#include <string>

namespace FBXDocParser {
namespace Util {

// ------------------------------------------------------------------------------------------------
const char *TokenTypeString(TokenType t) {
	switch (t) {
		case TokenType_OPEN_BRACKET:
			return "TOK_OPEN_BRACKET";

		case TokenType_CLOSE_BRACKET:
			return "TOK_CLOSE_BRACKET";

		case TokenType_DATA:
			return "TOK_DATA";

		case TokenType_COMMA:
			return "TOK_COMMA";

		case TokenType_KEY:
			return "TOK_KEY";

		case TokenType_BINARY_DATA:
			return "TOK_BINARY_DATA";
	}

	//ai_assert(false);
	return "";
}

// Generated by this formula: T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
static const uint8_t base64DecodeTable[128] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
	255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
	255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
};

uint8_t DecodeBase64(char ch) {
	const auto idx = static_cast<uint8_t>(ch);
	if (idx > 127)
		return 255;
	return base64DecodeTable[idx];
}

size_t ComputeDecodedSizeBase64(const char *in, size_t inLength) {
	if (inLength < 2) {
		return 0;
	}
	const size_t equals = size_t(in[inLength - 1] == '=') + size_t(in[inLength - 2] == '=');
	const size_t full_length = (inLength * 3) >> 2; // div by 4
	if (full_length < equals) {
		return 0;
	}
	return full_length - equals;
}

size_t DecodeBase64(const char *in, size_t inLength, uint8_t *out, size_t maxOutLength) {
	if (maxOutLength == 0 || inLength < 2) {
		return 0;
	}
	const size_t realLength = inLength - size_t(in[inLength - 1] == '=') - size_t(in[inLength - 2] == '=');
	size_t dst_offset = 0;
	int val = 0, valb = -8;
	for (size_t src_offset = 0; src_offset < realLength; ++src_offset) {
		const uint8_t table_value = Util::DecodeBase64(in[src_offset]);
		if (table_value == 255) {
			return 0;
		}
		val = (val << 6) + table_value;
		valb += 6;
		if (valb >= 0) {
			out[dst_offset++] = static_cast<uint8_t>((val >> valb) & 0xFF);
			valb -= 8;
			val &= 0xFFF;
		}
	}
	return dst_offset;
}

static const char to_base64_string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char EncodeBase64(char byte) {
	return to_base64_string[(size_t)byte];
}

/** Encodes a block of 4 bytes to base64 encoding
 *  @param bytes Bytes to encode.
 *  @param out_string String to write encoded values to.
 *  @param string_pos Position in out_string.
 */
void EncodeByteBlock(const char *bytes, std::string &out_string, size_t string_pos) {
	char b0 = (bytes[0] & 0xFC) >> 2;
	char b1 = (bytes[0] & 0x03) << 4 | ((bytes[1] & 0xF0) >> 4);
	char b2 = (bytes[1] & 0x0F) << 2 | ((bytes[2] & 0xC0) >> 6);
	char b3 = (bytes[2] & 0x3F);

	out_string[string_pos + 0] = EncodeBase64(b0);
	out_string[string_pos + 1] = EncodeBase64(b1);
	out_string[string_pos + 2] = EncodeBase64(b2);
	out_string[string_pos + 3] = EncodeBase64(b3);
}

std::string EncodeBase64(const char *data, size_t length) {
	// calculate extra bytes needed to get a multiple of 3
	size_t extraBytes = 3 - length % 3;

	// number of base64 bytes
	size_t encodedBytes = 4 * (length + extraBytes) / 3;

	std::string encoded_string(encodedBytes, '=');

	// read blocks of 3 bytes
	for (size_t ib3 = 0; ib3 < length / 3; ib3++) {
		const size_t iByte = ib3 * 3;
		const size_t iEncodedByte = ib3 * 4;
		const char *currData = &data[iByte];

		EncodeByteBlock(currData, encoded_string, iEncodedByte);
	}

	// if size of data is not a multiple of 3, also encode the final bytes (and add zeros where needed)
	if (extraBytes > 0) {
		char finalBytes[4] = { 0, 0, 0, 0 };
		memcpy(&finalBytes[0], &data[length - length % 3], length % 3);

		const size_t iEncodedByte = encodedBytes - 4;
		EncodeByteBlock(&finalBytes[0], encoded_string, iEncodedByte);

		// add '=' at the end
		for (size_t i = 0; i < 4 * extraBytes / 3; i++)
			encoded_string[encodedBytes - i - 1] = '=';
	}
	return encoded_string;
}

} // namespace Util
} // namespace FBXDocParser
