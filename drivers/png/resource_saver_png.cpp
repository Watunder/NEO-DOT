/*************************************************************************/
/*  resource_saver_png.cpp                                               */
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

#include "resource_saver_png.h"

#include "core/image.h"
#include "core/os/file_access.h"
#include "drivers/png/png_driver_common.h"
#include "scene/resources/texture.h"

Error ResourceSaverPNG::save(const String &p_path, const RES &p_resource, uint32_t p_flags) {
	Ref<ImageTexture> texture = p_resource;

	ERR_FAIL_COND_V_MSG(!texture.is_valid(), ERR_INVALID_PARAMETER, "Can't save invalid texture as PNG.");
	ERR_FAIL_COND_V_MSG(!texture->get_width(), ERR_INVALID_PARAMETER, "Can't save empty texture as PNG.");

	Ref<Image> img = texture->get_data();

	Error err = save_image(p_path, img);

	return err;
};

Error ResourceSaverPNG::save_image(const String &p_path, const Ref<Image> &p_img) {
	PoolVector<uint8_t> buffer;
	Error err = PNGDriverCommon::image_to_png(p_img, buffer);
	ERR_FAIL_COND_V_MSG(err, err, "Can't convert image to PNG.");
	FileAccess *file = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_V_MSG(err, err, vformat("Can't save PNG at path: '%s'.", p_path));

	PoolVector<uint8_t>::Read reader = buffer.read();

	file->store_buffer(reader.ptr(), buffer.size());
	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
		memdelete(file);
		return ERR_CANT_CREATE;
	}

	file->close();
	memdelete(file);

	return OK;
}

PoolVector<uint8_t> ResourceSaverPNG::save_image_to_buffer(const Ref<Image> &p_img) {
	PoolVector<uint8_t> buffer;
	Error err = PNGDriverCommon::image_to_png(p_img, buffer);
	ERR_FAIL_COND_V_MSG(err, PoolVector<uint8_t>(), "Can't convert image to PNG.");
	return buffer;
}

bool ResourceSaverPNG::recognize(const RES &p_resource) const {
	return (p_resource.is_valid() && p_resource->is_class("ImageTexture"));
}

void ResourceSaverPNG::get_recognized_extensions(const RES &p_resource, List<String> *p_extensions) const {
	if (Object::cast_to<ImageTexture>(*p_resource)) {
		p_extensions->push_back("png");
	}
}

ResourceSaverPNG::ResourceSaverPNG() {
	Image::save_png_func = &save_image;
	Image::save_png_buffer_func = &save_image_to_buffer;
};
