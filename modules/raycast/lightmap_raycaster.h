/*************************************************************************/
/*  lightmap_raycaster.h                                                 */
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

#include "core/object.h"
#include "scene/3d/lightmapper.h"
#include "scene/resources/mesh.h"

#include <embree3/rtcore.h>

class LightmapRaycasterEmbree : public LightmapRaycaster {
	GDCLASS(LightmapRaycasterEmbree, LightmapRaycaster);

private:
	struct AlphaTextureData {
		Vector<uint8_t> data;
		Vector2i size;

		uint8_t sample(float u, float v) const;
	};

	RTCDevice embree_device;
	RTCScene embree_scene;

	static void filter_function(const struct RTCFilterFunctionNArguments *p_args);

	Map<unsigned int, AlphaTextureData> alpha_textures;
	Set<int> filter_meshes;

public:
	virtual bool intersect(Ray &p_ray);

	virtual void intersect(Vector<Ray> &r_rays);

	virtual void add_mesh(const Vector<Vector3> &p_vertices, const Vector<Vector3> &p_normals, const Vector<Vector2> &p_uv2s, unsigned int p_id);
	virtual void set_mesh_alpha_texture(Ref<Image> p_alpha_texture, unsigned int p_id);
	virtual void commit();

	virtual void set_mesh_filter(const Set<int> &p_mesh_ids);
	virtual void clear_mesh_filter();

	static LightmapRaycaster *create_embree_raycaster();
	static void make_default_raycaster();

	LightmapRaycasterEmbree();
	~LightmapRaycasterEmbree();
};
