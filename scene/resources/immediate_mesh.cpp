/*************************************************************************/
/*  immediate_mesh.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "immediate_mesh.h"

void ImmediateMesh::surface_begin(PrimitiveType p_primitive, const Ref<Material> &p_material) {
	ERR_FAIL_COND_MSG(surface_active, "Already creating a new surface.");
	active_surface_data.primitive = p_primitive;
	active_surface_data.material = p_material;
	surface_active = true;
}
void ImmediateMesh::surface_set_color(const Color &p_color) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");

	if (!uses_colors) {
		colors.resize(vertices.size());
		for (uint32_t i = 0; i < colors.size(); i++) {
			colors[i] = p_color;
		}
		uses_colors = true;
	}

	current_color = p_color;
}
void ImmediateMesh::surface_set_normal(const Vector3 &p_normal) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");

	if (!uses_normals) {
		normals.resize(vertices.size());
		for (uint32_t i = 0; i < normals.size(); i++) {
			normals[i] = p_normal;
		}
		uses_normals = true;
	}

	current_normal = p_normal;
}
void ImmediateMesh::surface_set_tangent(const Plane &p_tangent) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");
	if (!uses_tangents) {
		tangents.resize(vertices.size());
		for (uint32_t i = 0; i < tangents.size(); i++) {
			tangents[i] = p_tangent;
		}
		uses_tangents = true;
	}

	current_tangent = p_tangent;
}
void ImmediateMesh::surface_set_uv(const Vector2 &p_uv) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");
	if (!uses_uvs) {
		uvs.resize(vertices.size());
		for (uint32_t i = 0; i < uvs.size(); i++) {
			uvs[i] = p_uv;
		}
		uses_uvs = true;
	}

	current_uv = p_uv;
}
void ImmediateMesh::surface_set_uv2(const Vector2 &p_uv2) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");
	if (!uses_uv2s) {
		uv2s.resize(vertices.size());
		for (uint32_t i = 0; i < uv2s.size(); i++) {
			uv2s[i] = p_uv2;
		}
		uses_uv2s = true;
	}

	current_uv2 = p_uv2;
}
void ImmediateMesh::surface_add_vertex(const Vector3 &p_vertex) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");
	ERR_FAIL_COND_MSG(vertices.size() && active_surface_data.vertex_2d, "Can't mix 2D and 3D vertices in a surface.");

	if (uses_colors) {
		colors.push_back(current_color);
	}
	if (uses_normals) {
		normals.push_back(current_normal);
	}
	if (uses_tangents) {
		tangents.push_back(current_tangent);
	}
	if (uses_uvs) {
		uvs.push_back(current_uv);
	}
	if (uses_uv2s) {
		uv2s.push_back(current_uv2);
	}
	vertices.push_back(p_vertex);
}

void ImmediateMesh::surface_add_vertex_2d(const Vector2 &p_vertex) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");
	ERR_FAIL_COND_MSG(vertices.size() && !active_surface_data.vertex_2d, "Can't mix 2D and 3D vertices in a surface.");

	if (uses_colors) {
		colors.push_back(current_color);
	}
	if (uses_normals) {
		normals.push_back(current_normal);
	}
	if (uses_tangents) {
		tangents.push_back(current_tangent);
	}
	if (uses_uvs) {
		uvs.push_back(current_uv);
	}
	if (uses_uv2s) {
		uv2s.push_back(current_uv2);
	}
	Vector3 v(p_vertex.x, p_vertex.y, 0);
	vertices.push_back(v);

	active_surface_data.vertex_2d = true;
}
void ImmediateMesh::surface_end(const Array &p_indices) {
	ERR_FAIL_COND_MSG(!surface_active, "Not creating any surface. Use surface_begin() to do it.");
	ERR_FAIL_COND_MSG(!vertices.size(), "No vertices were added, surface cant be created.");

	Vector3 *vertices_data = vertices.ptr();
	Vector3 *normals_data = normals.ptr();
	Plane *tangents_data = tangents.ptr();
	Color *colors_data = colors.ptr();
	Vector2 *uvs_data = uvs.ptr();
	Vector2 *uv2s_data = uv2s.ptr();

	PoolVector<Vector3> vertex_array;
	vertex_array.resize(vertices.size());
	PoolVector<Vector3>::Write vertex_array_write = vertex_array.write();

	PoolVector<Vector3> normal_array;
	normal_array.resize(vertices.size());
	PoolVector<Vector3>::Write normal_array_write = normal_array.write();

	PoolVector<real_t> tangent_array;
	tangent_array.resize(vertices.size() * 4);
	PoolVector<real_t>::Write tangent_array_write = tangent_array.write();

	PoolVector<Color> color_array;
	color_array.resize(vertices.size());
	PoolVector<Color>::Write color_array_write = color_array.write();

	PoolVector<Vector2> uv_array;
	uv_array.resize(vertices.size());
	PoolVector<Vector2>::Write uv_array_write = uv_array.write();

	PoolVector<Vector2> uv2_array;
	uv2_array.resize(vertices.size());
	PoolVector<Vector2>::Write uv2_array_write = uv2_array.write();

	AABB aabb;
	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);

	for (uint32_t i = 0; i < vertices.size(); i++) {
		if (uses_colors) {
			color_array_write[i] = colors_data[i];
		}
		if (uses_normals) {
			normal_array_write[i] = normals_data[i];
		}
		if (uses_tangents) {
			tangent_array_write[i * 4 + 0] = tangents_data[i].normal.x;
			tangent_array_write[i * 4 + 1] = tangents_data[i].normal.y;
			tangent_array_write[i * 4 + 2] = tangents_data[i].normal.z;
			tangent_array_write[i * 4 + 3] = tangents_data[i].d;
		}
		if (uses_uvs) {
			uv_array_write[i] = uvs_data[i];
		}
		if (uses_uv2s) {
			uv2_array_write[i] = uv2s_data[i];
		}
		vertex_array_write[i] = vertices_data[i];
		if (i == 0) {
			aabb.position = vertices_data[i];
		} else {
			aabb.expand_to(vertices_data[i]);
		}
	}

	uint32_t format = ARRAY_FORMAT_VERTEX;
	if (active_surface_data.vertex_2d) {
		format |= ARRAY_FLAG_USE_2D_VERTICES;
	}
	if (uses_colors) {
		format |= ARRAY_FORMAT_COLOR;
		mesh_array[VS::ARRAY_COLOR] = color_array;
	}
	if (uses_normals) {
		format |= ARRAY_FORMAT_NORMAL;
		mesh_array[VS::ARRAY_NORMAL] = normal_array;
	}
	if (uses_tangents) {
		format |= ARRAY_FORMAT_TANGENT;
		mesh_array[VS::ARRAY_TANGENT] = tangent_array;
	}
	if (uses_uvs) {
		format |= ARRAY_FORMAT_TEX_UV;
		mesh_array[VS::ARRAY_TEX_UV] = uv_array;
	}
	if (uses_uv2s) {
		format |= ARRAY_FORMAT_TEX_UV2;
		mesh_array[VS::ARRAY_TEX_UV2] = uv2_array;
	}
	mesh_array[VS::ARRAY_VERTEX] = vertex_array;
	if (p_indices.size() > 0) {
		PoolVector<int> indices_array;
		indices_array.resize(p_indices.size());
		PoolVector<int>::Write indices_array_write = indices_array.write();
		for (uint32_t i = 0; i < p_indices.size(); i++) {
			indices_array_write[i] = p_indices[i];
		}
		mesh_array[VS::ARRAY_INDEX] = indices_array;
	}

	VS::get_singleton()->mesh_add_surface_from_arrays(mesh, VS::PrimitiveType(active_surface_data.primitive), mesh_array, Array(), (VS::ARRAY_COMPRESS_DEFAULT & ~VS::ARRAY_COMPRESS_TEX_UV) & ~VS::ARRAY_COMPRESS_COLOR);

	active_surface_data.aabb = aabb;
	active_surface_data.format = format;
	active_surface_data.array_len = vertices.size();
	surfaces.push_back(active_surface_data);

	colors.clear();
	normals.clear();
	tangents.clear();
	uvs.clear();
	uv2s.clear();
	vertices.clear();

	uses_colors = false;
	uses_normals = false;
	uses_tangents = false;
	uses_uvs = false;
	uses_uv2s = false;

	surface_active = false;
}

void ImmediateMesh::clear_surfaces() {
	VS::get_singleton()->mesh_clear(mesh);
	surfaces.clear();
	surface_active = false;

	colors.clear();
	normals.clear();
	tangents.clear();
	uvs.clear();
	uv2s.clear();
	vertices.clear();

	uses_colors = false;
	uses_normals = false;
	uses_tangents = false;
	uses_uvs = false;
	uses_uv2s = false;
}

int ImmediateMesh::get_surface_count() const {
	return surfaces.size();
}
int ImmediateMesh::surface_get_array_len(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, int(surfaces.size()), -1);
	return surfaces[p_idx].array_len;
}
int ImmediateMesh::surface_get_array_index_len(int p_idx) const {
	return 0;
}
bool ImmediateMesh::surface_is_softbody_friendly(int p_idx) const {
	return false;
}
Array ImmediateMesh::surface_get_arrays(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, int(surfaces.size()), Array());
	return VS::get_singleton()->mesh_surface_get_arrays(mesh, p_surface);
}
Array ImmediateMesh::surface_get_blend_shape_arrays(int p_surface) const {
	return Array();
}
//Dictionary ImmediateMesh::surface_get_lods(int p_surface) const {
//	return Dictionary();
//}
uint32_t ImmediateMesh::surface_get_format(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, int(surfaces.size()), 0);
	return surfaces[p_idx].format;
}
Mesh::PrimitiveType ImmediateMesh::surface_get_primitive_type(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, int(surfaces.size()), Mesh::PRIMITIVE_MAX);
	return surfaces[p_idx].primitive;
}
void ImmediateMesh::surface_set_material(int p_idx, const Ref<Material> &p_material) {
	ERR_FAIL_INDEX(p_idx, int(surfaces.size()));
	surfaces[p_idx].material = p_material;
	RID mat;
	if (p_material.is_valid()) {
		mat = p_material->get_rid();
	}
	VS::get_singleton()->mesh_surface_set_material(mesh, p_idx, mat);
}
Ref<Material> ImmediateMesh::surface_get_material(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, int(surfaces.size()), Ref<Material>());
	return surfaces[p_idx].material;
}
int ImmediateMesh::get_blend_shape_count() const {
	return 0;
}
StringName ImmediateMesh::get_blend_shape_name(int p_index) const {
	return StringName();
}
//void ImmediateMesh::set_blend_shape_name(int p_index, const StringName &p_name) {
//}

AABB ImmediateMesh::get_aabb() const {
	AABB aabb;
	for (uint32_t i = 0; i < surfaces.size(); i++) {
		if (i == 0) {
			aabb = surfaces[i].aabb;
		} else {
			aabb.merge(surfaces[i].aabb);
		}
	}
	return aabb;
}

void ImmediateMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("surface_begin", "primitive", "material"), &ImmediateMesh::surface_begin, DEFVAL(Ref<Material>()));
	ClassDB::bind_method(D_METHOD("surface_set_color", "color"), &ImmediateMesh::surface_set_color);
	ClassDB::bind_method(D_METHOD("surface_set_normal", "normal"), &ImmediateMesh::surface_set_normal);
	ClassDB::bind_method(D_METHOD("surface_set_tangent", "tangent"), &ImmediateMesh::surface_set_tangent);
	ClassDB::bind_method(D_METHOD("surface_set_uv", "uv"), &ImmediateMesh::surface_set_uv);
	ClassDB::bind_method(D_METHOD("surface_set_uv2", "uv2"), &ImmediateMesh::surface_set_uv2);
	ClassDB::bind_method(D_METHOD("surface_add_vertex", "vertex"), &ImmediateMesh::surface_add_vertex);
	ClassDB::bind_method(D_METHOD("surface_add_vertex_2d", "vertex"), &ImmediateMesh::surface_add_vertex_2d);
	ClassDB::bind_method(D_METHOD("surface_end", "indices"), &ImmediateMesh::surface_end, DEFVAL(Array()));

	ClassDB::bind_method(D_METHOD("clear_surfaces"), &ImmediateMesh::clear_surfaces);
}

RID ImmediateMesh::get_rid() const {
	return mesh;
}

ImmediateMesh::ImmediateMesh() {
	mesh = VS::get_singleton()->mesh_create();
}
ImmediateMesh::~ImmediateMesh() {
	VS::get_singleton()->free(mesh);
}
